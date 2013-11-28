`EN README:`

- Hold the miui recovery source code
- Modify by Gaojiquan LaoYang
- Port recovery binary to c++
- now can build on CM10.2 source code

#support functions
- `ORS(Open Recovery Script)` [http://teamw.in/OpenRecoveryScript](http://teamw.in/OpenRecoveryScript)

- `full touch support `

- `set brightness`

- `root devices`

- `disable restore the official Recovery`

- `dup && tar backup method `

- `support tar.gz backup method`


- `adb sideload` [@PeterCxy](https://github.com/PeterCxy)

- `libiniparser` make it support ini files

#Dev opts 

- `Turn on/off md5sum generate`

- `compact loki devices` 

- `signaturecheck` 


------------------------------------------------------------


---------------------------------------------------------
#Recovery: Loki patching support 
- Add support for automatically checking and making loki images
for locked bootloader bypass on loki supported devices. Loki
support is enabled with ENABLE_LOKI_RECOVERY.

- Add `ENABLE_LOKI_RECOVERY=true` to `BoardConfig.mk` 

-------------------------------------------------------


if you need to use the device info from the `recovery menu`
you should add the line to `Boardconfig.mk`

```bash
MIUI_DEVICE_CONF := ../../../device/ZTE/N909/device.conf
MIUI_INIT_CONF := ../../../device/ZTE/N909/init.conf
```

you should add the below line to `init.rc`

```bash
export LD_LIBRARY_PATH .:/sbin/
```

`zh-CN README`

这里是托管 `MIUI RECOVERY`源代码的地方

由[搞机圈论坛](http://www.gaojiquan.com) 老杨进行修改

源代码已经从C语言修改成C++

为了让动态编译的recovery binary可以运行需要在`init.rc`文件中添加下面的定义

`export LD_LIBRARY_PATH .:/sbin`

如何让`MIUI_RECOVERY` 显示自己设定的信息：
修改方法请参考

[how to support you device](/devices/README.md)

#Releases:

tag `v3.5.0` && `v3.2.0` only can build on cm-10.1 source code tree 


<a href="https://github.com/sndnvaps/miui_recovery/archive/v3.2.0.tar.gz"> v3.2.0.tar.gz </a>

[V3.5.0](https://github.com/sndnvaps/miui_recovery/releases/tag/v3.5.0)


<a href="https://codeload.github.com/sndnvaps/miui_recovery/zip/v3.5.0"> v3.5.0.zip </a>

<a href="https://codeload.github.com/sndnvaps/miui_recovery/tar.gz/v3.5.0"> v3.5.0.tar.gz </a

----------------------------------------------------------------------


----------------------------------------------------------
#settings.ini 例子
```
 #/sdcard/miui_recovery/settings.ini
 # miui recovery settings.ini file
 # version 3.2.0
[zipflash]
CDI=0
md5sum=1

[dev]
signaturecheck=0

```


