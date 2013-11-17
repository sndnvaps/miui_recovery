EN README:

hold the miui recovery source code
modify by Gaojiquan LaoYang
port recovery binary to c++
now can build on CM10.1 source code

#support functions
ORS
full touch control  
set brightness
root devices
disable restore the official Recovery
dup && tar backup method 
adb sideload (thanks petercxy)
...
......

if you need to use the device info from the 'recovery menu'
you should add the line to Boardconfig.mk

```bash
MIUI_DEVICE_CONF := ../../../device/ZTE/N909/device.conf
MIUI_INIT_CONF := ../../../device/ZTE/N909/init.conf
```

you should add the below line to init.rc

```bash
export LD_LIBRARY_PATH .:/sbin/
```

zh-CN README

这里是托管 MIUI RECOVERY源代码的地方
 
由[搞机圈论坛](http://www.gaojiquan.com) 老杨进行修改

源代码已经从C语言修改成C++

修改为动态库编译，所以需要在init.rc文件中添加下面的定义
export LD_LIBRARY_PATH .:/sbin

如何让MIUI_RECOVERY显示自己设定的信息：
修改方法请参考

[how to support you device](devices/README.md)


#Fix mtk platform soft link of '/sdcard' && '/external-sd'
You need to add the below line to 'BoardConfig.mk'
```
FIX_MTK_PLATFORM_SDCARD_SOFT_LINK := true
```
```
             /* change the soft link to the /sdcard && /external_sd
	      *  if not inject '/external_sd 
	      * '/sdcard' origin soft link to /storage/sdcard0
	      * ---------------------------------------------
	      * if inject '/external_sd , we need to change the soft link 
	      *  '/sdcard -> '/storege/sdcard1' 
	      *  '/external_sd -> '/storage/sdcard0'
	      *  ----------------------------------
	      */
```







