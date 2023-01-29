// SPDX-License-Identifier: GPL-2.0-only

#include <linux/clockchips.h>
#include <linux/init.h>
#include <asm/time.h>
#include <asm/idle.h>
#include <linux/interrupt.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/sched_clock.h>

#include <mach-rtl83xx.h>

/* 
 * Timer registers
 * the RTL9300/9310 SoCs have 6 timers, each register block 0x10 apart
 */
#define RTL9300_TC_DATA		0x0
#define RTL9300_TC_CNT		0x4
#define RTL9300_TC_CTRL		0x8
#define RTL9300_TC_CTRL_MODE	BIT(24)
#define RTL9300_TC_CTRL_EN	BIT(28)
#define RTL9300_TC_INT		0xc
#define RTL9300_TC_INT_IP	BIT(16)
#define RTL9300_TC_INT_IE	BIT(20)

// Timer modes
#define TIMER_MODE_REPEAT	1
#define TIMER_MODE_ONCE		0

// Minimum divider is 2
#define DIVISOR_RTL9300		2

#define N_BITS			28

#define RTL9300_CLOCK_RATE	87500000

struct rtl9300_clk_dev {
	struct clock_event_device clkdev;
	void __iomem *base;
};

static void __iomem *rtl9300_tc_base(struct clock_event_device *clk)
{
	struct rtl9300_clk_dev *rtl_clk = container_of(clk, struct rtl9300_clk_dev, clkdev);

	return rtl_clk->base;
}

static irqreturn_t rtl9300_timer_interrupt(int irq, void *dev_id)
{
	struct rtl9300_clk_dev *rtl_clk = dev_id;
	struct clock_event_device *clk = &rtl_clk->clkdev;

	u32 v = readl(rtl_clk->base + RTL9300_TC_INT);

	// Acknowledge the IRQ
	v |= RTL9300_TC_INT_IP;
	writel(v, rtl_clk->base + RTL9300_TC_INT);

	clk->event_handler(clk);
	return IRQ_HANDLED;
}

static void rtl9300_clock_stop(void __iomem *base)
{
	u32 v;

	writel(0, base + RTL9300_TC_CTRL);

	// Acknowledge possibly pending IRQ
	v = readl(base + RTL9300_TC_INT);
	writel(v | RTL9300_TC_INT_IP, base + RTL9300_TC_INT);
}

static void rtl9300_timer_start(void __iomem *base, bool periodic)
{
	u32 v = (periodic ? RTL9300_TC_CTRL_MODE : 0) | RTL9300_TC_CTRL_EN | DIVISOR_RTL9300;

	writel(0, base + RTL9300_TC_CNT);
	pr_debug("------------- starting timer base %08x\n", (u32)base);
	writel(v, base + RTL9300_TC_CTRL);
}

static int rtl9300_next_event(unsigned long delta, struct clock_event_device *clk)
{
	void __iomem *base = rtl9300_tc_base(clk);

	rtl9300_clock_stop(base);
	writel(delta, base + RTL9300_TC_DATA);
	rtl9300_timer_start(base, TIMER_MODE_ONCE);

	return 0;
}

static int rtl9300_state_periodic(struct clock_event_device *clk)
{
	void __iomem *base = rtl9300_tc_base(clk);

	pr_debug("------------- rtl9300_state_periodic %08x\n", (u32)base);
	rtl9300_clock_stop(base);
	writel(RTL9300_CLOCK_RATE / HZ, base + RTL9300_TC_DATA);
	rtl9300_timer_start(base, TIMER_MODE_REPEAT);
	return 0;
}

static int rtl9300_state_oneshot(struct clock_event_device *clk)
{
	void __iomem *base = rtl9300_tc_base(clk);

	pr_debug("------------- rtl9300_state_oneshot %08x\n", (u32)base);
	rtl9300_clock_stop(base);
	writel(RTL9300_CLOCK_RATE / HZ, base + RTL9300_TC_DATA);
	rtl9300_timer_start(base, TIMER_MODE_ONCE);
	return 0;
}

static int rtl9300_shutdown(struct clock_event_device *clk)
{
	void __iomem *base = rtl9300_tc_base(clk);

	pr_debug("------------- rtl9300_shutdown %08x\n", (u32)base);
	rtl9300_clock_stop(base);
	return 0;
}

static void rtl9300_clock_setup(void __iomem *base)
{
	u32 v;

	// Disable timer
	writel(0, base + RTL9300_TC_CTRL);

	// Acknowledge possibly pending IRQ
	v = readl(base + RTL9300_TC_INT);
	writel(v | RTL9300_TC_INT_IP, base + RTL9300_TC_INT);

	// Setup maximum period (for use as clock-source)
	writel(0x0fffffff, base + RTL9300_TC_DATA);
}

static DEFINE_PER_CPU(struct rtl9300_clk_dev, rtl9300_clockevent);
static DEFINE_PER_CPU(char [18], rtl9300_clock_name);

void rtl9300_clockevent_init(void)
{
	int cpu = smp_processor_id();
	int irq;
	struct rtl9300_clk_dev *rtl_clk = &per_cpu(rtl9300_clockevent, cpu);
	struct clock_event_device *cd = &rtl_clk->clkdev;
	unsigned char *name = per_cpu(rtl9300_clock_name, cpu);
	unsigned long flags =  IRQF_PERCPU | IRQF_TIMER;
	struct device_node *node;

	pr_info("%s called for cpu%d\n", __func__, cpu);
	BUG_ON(cpu > 3);	/* Only have 4 general purpose timers */

	node = of_find_compatible_node(NULL, NULL, "realtek,rtl9300clock");
	if (!node) {
		pr_err("No DT entry found for realtek,rtl9300clock\n");
		return;
	}

	irq = irq_of_parse_and_map(node, cpu);
	pr_info("%s using IRQ %d\n", __func__, irq);

	rtl_clk->base = of_iomap(node, cpu);
	if (!rtl_clk->base) {
		pr_err("cannot map timer for cpu %d", cpu);
		return;
	}

	rtl9300_clock_setup(rtl_clk->base);

	sprintf(name, "rtl9300-counter-%d", cpu);
	cd->name		= name;
	cd->features		= CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT;

	clockevent_set_clock(cd, RTL9300_CLOCK_RATE);

	cd->max_delta_ns	= clockevent_delta2ns(0x0fffffff, cd);
	cd->max_delta_ticks	= 0x0fffffff;
	cd->min_delta_ns	= clockevent_delta2ns(0x20, cd);
	cd->min_delta_ticks	= 0x20;
	cd->rating		= 300;
	cd->irq			= irq;
	cd->cpumask		= cpumask_of(cpu);
	cd->set_next_event	= rtl9300_next_event;
	cd->set_state_shutdown	= rtl9300_shutdown;
	cd->set_state_periodic	= rtl9300_state_periodic;
	cd->set_state_oneshot	= rtl9300_state_oneshot;
	clockevents_register_device(cd);

	irq_set_affinity(irq, cd->cpumask);

	if (request_irq(irq, rtl9300_timer_interrupt, flags, name, rtl_clk))
		pr_err("Failed to request irq %d (%s)\n", irq, name);

	writel(RTL9300_TC_INT_IE, rtl_clk->base + RTL9300_TC_INT);
}
