This folder hold the device config for 
samsung D720 (nexus 4g)
zte N909
honor
ville


zh-cn
如何适配更多的机型信息呢。
如果你的机子不在支持的列表中，你有两个方法可以让你的MIUI RECOVERY显示你设定的信息。

#方法一：
在你的机型的device目录里面创建两个文件，分别是
```bash
device.conf
init.conf
```
可以参考下面的例子:

```makefile
#这个文件是device.conf
ini_set("customkeycode_up",     "115");                                 
ini_set("customkeycode_down",   "114");
ini_set("customkeycode_select", "116");
ini_set("customkeycode_menu",   "229");
ini_set("customkeycode_back",   "158");
ini_set("sd_ext", "1"); 
ini_set("rom_device", "ZTE N909");
ini_set("rom_name", "MIUI Recovery for N909 @Gaojiquan.com");
ini_set("rom_version", "v3.1.0");
ini_set("rom_date", "2013-09-22");
ini_set("rom_author", "GTO LaoYang");
```

```makefile
#这个文件是init.conf
#set usb storage 
ini_set("lun_file","/sys/devices/platform/msm_hsusb/gadget/lun");
#set Brightness 
ini_set("brightness_path", "/sys/devices/platform/msm_fb.525057/leds/lcd-backlight/brightness");
```

创建完成了init.conf,device.conf后，请往Boardconf.mk文件里面写入如下的配置信息，目的是让MIUI RECOVERY编译的时候，可以复制device.conf,init.conf到指定的目录里面

```makefile
## MIUI RECOVERY
MIUI_DEVICE_CONF := ../../../device/ZTE/N909/device.conf
MIUI_INIT_CONF := ../../../device/ZTE/N909/init.conf
```

#方法二:
在源代码目录里面的device目录里面创建你的机型目录，例如N909
目录结构如下：
```
bootable/recovery/device/N909
```

在N909目录里面创建 device.conf,init.conf文件
里面的内容可以参考 <b>方法一</b>中的例子

创建完成后，修改
```
bootable/recovery/device/Android.mk
```
在这个Android.mk文件里面分别定义了如何复制*.conf文件到编译好的目录中。
为了适配机型，需要在以下的位置插入代码:
```makefile
LOCAL_SRC_FILES := $(MIUI_DEVICE_CONF)
```

```makefile
LOCAL_SRC_FILES := $(MIUI_INIT_CONF)
```
---------------------------------------------------
在$(MIUI_DEVICE_CONF)代码的下一行，插入如下的代码
```makefile
else ifeq ($(TARGET_PRODUCT), cm_N909)  #这一行中的cm_N909，需要根据你的机型信息来修改
        LOCAL_SRC_FILES := N909/device.conf #请修改N909为你的机型目录
```


------------------------------------
在$(MIUI_INIT_CONF)代码的下一行，插入如下的代码
```makefile
else ifeq $(TARGET_PRODUCT), cm_N909) #同上面的解释
         LOCAL_SRC_FILES := N909/init.conf
```


