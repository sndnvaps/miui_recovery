SET(SRC_LIST \
    recovery_ui.cpp \
    mount.cpp \
    bootloader.cpp \
    install.cpp \
    roots.cpp \
    nandroid.cpp \
    verifier.cpp \
    reboot.cpp \
    root_device.cpp \
    utils_func.cpp \
    recovery.cpp \
    sideload.cpp )
ADD_EXECUTABLE(recovery ${SRC_LIST})


ADD_SUBDIRECTORY(bmlutils)
ADD_SUBDIRECTORY(falshutils)
ADD_SUBDIRECTORY(libcrecovery)
ADD_SUBDIRECTORY(miui)
ADD_SUBDIRECTORY(minelf)
ADD_SUBDIRECTORY(minzip)
ADD_SUBDIRECTORY(mtdutils)
ADD_SUBDIRECTORY(mmcutils)
ADD_SUBDIRECTORY(tools)
ADD_SUBDIRECTORY(edify)
ADD_SUBDIRECTORY(updater)
ADD_SUBDIRECTORY(minadbd)
ADD_SUBDIRECTORY(applypatch)

