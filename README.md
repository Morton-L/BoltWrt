# BoltWrt

 -----------------------------------------------------
            ____        _ ___          __   _   
           |  _ \      | | \ \        / /  | |  
           | |_) | ___ | | |\ \  /\  / / __| |_ 
           |  _ < / _ \| | __\ \/  \/ / '__| __|
           | |_) | (_) | | |_ \  /\  /| |  | |_ 
           |____/ \___/|_|\__| \/  \/ |_|   \__|
                                      
 -----------------------------------------------------

## 中文

### 编译

## English

### Make

To build your own firmware you need a Linux, BSD or MacOSX system (case
sensitive filesystem required). Cygwin is unsupported because of the lack
of a case sensitive file system.

You need gcc, binutils, bzip2, flex, python, perl, make, find, grep, diff,
unzip, gawk, getopt, subversion, libz-dev and libc headers installed.

1. Run "./scripts/feeds update -a" to obtain all the latest package definitions
defined in feeds.conf / feeds.conf.default

2. Run "./scripts/feeds install -a" to install symlinks for all obtained
packages into package/feeds/

3. Run "make menuconfig" to select your preferred configuration for the
toolchain, target system & firmware packages.

4. Run "make" to build your firmware. This will download all sources, build
the cross-compile toolchain and then cross-compile the Linux kernel & all
chosen applications for your target system.

## Acknowledgments

- [OpenWrt](https://github.com/openwrt/openwrt)
- [argon](https://github.com/jerrykuku/luci-theme-argon)

## License

[GPL](https://github.com/Morton-L/BoltWrt/blob/main/LICENSE)
