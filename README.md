# BoltWrt

 -----------------------------------------------------
     __________       .__   __   __      __          __   
     \______   \ ____ |  |_/  |_/  \    /  \________/  |_ 
      |    |  _//  _ \|  |\   __\   \/\/   /\_  __ \   __\
      |    |   (  <_> )  |_|  |  \        /  |  | \/|  |  
      |______  /\____/|____/__|   \__/\  /   |__|   |__|  
             \/                        \/                 
                                      
 -----------------------------------------------------

## 中文说明

### 标准部署流程

#### 固件选择

您可以通过访问 [固件](https://github.com/Morton-L/BoltWrt/releases) 页面来获取正式版固件,并根据具体情况来选择使用ext4文件系统或squashfs文件系统
使用ext4文件系统或squashfs文件系统不会影响设备的正常运行或引起兼容性问题,他们只有一个区别,即前者
为可读写固件后者为只读固件,通常情况下我建议使用squashfs文件系统,因为当系统遇到灾难性故障时可能需
要通过"恢复出厂设置"功能来快速恢复设备状态,这时我们会利用到squashfs文件系统的特性,您可以放心的是
squashfs文件系统并不会影响正常的软件安装,更新或卸载,它犹如您的IPhone或Android手机一样提供可靠保障

#### 固件安装



### 高级部署流程




## Acknowledgments

- [OpenWrt](https://github.com/openwrt/openwrt)
- [argon](https://github.com/jerrykuku/luci-theme-argon)

## License

[GPL](https://github.com/Morton-L/BoltWrt/blob/main/LICENSE)
