LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

commands_recovery_local_path := $(LOCAL_PATH)

LOCAL_SRC_FILES := \
	recovery_ui.cpp \
	mount.cpp \
	bootloader.cpp \
	install.cpp \
	roots.cpp \
	firmware.cpp \
	nandroid.cpp \
	verifier.cpp \
	root_device.cpp \
	reboot.cpp \
	miui_func.cpp \
	utils_func.cpp \
	recovery.cpp \
        sideload.cpp   \
	../../system/core/toolbox/newfs_msdos.c \
        ../../system/core/toolbox/dynarray.c \
        ../../system/vold/vdc.c \
        prop.c



LOCAL_MODULE := recovery

#LOCAL_FORCE_STATIC_EXECUTABLE := true

RECOVERY_API_VERSION := 3
MYDEFINE_CFLAGS :=  -D_GLIBCXX_DEBUG_PEDANTIC \
                  -DFT2_BUILD_LIBRARY=1 \
                  -DDARWIN_NO_CARBON \
				  -D_MIUI_NODEBUG=1
LOCAL_CFLAGS += -DRECOVERY_API_VERSION=$(RECOVERY_API_VERSION) 
LOCAL_CFLAGS += $(MYDEFINE_CFLAGS)


ifeq ($(ENABLE_LOKI_RECOVERY),true)
LOCAL_CFLAGS += -DENABLE_LOKI
LOCAL_SRC_FILES += \
		   compact_loki.c 
endif

#LOCAL_CFLAGS += -DRECOVERY_API_VERSION=$(RECOVERY_API_VERSION)

LOCAL_STATIC_LIBRARIES :=
LOCAL_SHARED_LIBRARIES := 

ifeq ($(TARGET_USERIMAGES_USE_EXT4), true)
LOCAL_CFLAGS += -DUSE_EXT4
LOCAL_C_INCLUDES += system/extras/ext4_utils
#LOCAL_STATIC_LIBRARIES += libext4_utils libz
LOCAL_SHARED_LIBRARIES += libext4_utils libz
endif

ifeq ($(BOARD_HAS_REMOVABLE_STORAGE), true) 
	LOCAL_CFLAGS += -DBOARD_HAS_REMOVABLE_STORAGE
endif

ifeq ($(ENABLE_LOKI_RECOVERY),true)
  LOCAL_CFLAGS += -DENABLE_LOKI
  LOCAL_SRC_FILES += \
    compact_loki.c
endif



BOARD_RECOVERY_DEFINES := BOARD_RECOVERY_SWIPE BOARD_HAS_NO_SELECT_BUTTON BOARD_UMS_LUNFILE BOARD_RECOVERY_ALWAYS_WIPES BOARD_RECOVERY_HANDLES_MOUNT BOARD_TOUCH_RECOVERY RECOVERY_EXTEND_NANDROID_MENU TARGET_USE_CUSTOM_LUN_FILE_PATH TARGET_DEVICE TARGET_RECOVERY_FSTAB

$(foreach board_define,$(BOARD_RECOVERY_DEFINES), \
  $(if $($(board_define)), \
    $(eval LOCAL_CFLAGS += -D$(board_define)=\"$($(board_define))\") \
  ) \
  )

LOCAL_STATIC_LIBRARIES :=

LOCAL_CFLAGS += -DUSE_EXT4 -DMINIVOLD
LOCAL_C_INCLUDES += system/extras/ext4_utils system/core/fs_mgr/include external/fsck_msdos
LOCAL_C_INCLUDES += system/vold

LOCAL_STATIC_LIBRARIES += libext4_utils_static libz libsparse_static

# This binary is in the recovery ramdisk, which is otherwise a copy of root.
# It gets copied there in config/Makefile.  LOCAL_MODULE_TAGS suppresses
# a (redundant) copy of the binary in /system/bin for user builds.
# TODO: Build the ramdisk image in a more principled way.

LOCAL_MODULE_TAGS := eng

#LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
#LOCAL_LDLIBS += $(LOCAL_PATH)/lib


LOCAL_C_INCLUDES += bionic \
		    external/libselinux/include \
		    external/stlport/stlport \
		    external/freetype/include \
		    external/freetype/build \
		    external/zlib \
		    external/libpng

LOCAL_CFLAGS += -DHAVE_SELINUX

LOCAL_STATIC_LIBRARIES += libvoldclient libsdcard \
			  libminipigz libfsck_msdos

LOCAL_STATIC_LIBRARIES += libminzip libunz libmincrypt \
			  libmkyaffs2image_static \
			  libunyaffs_static \
			  libdedupe libselinux \
			  libedify libcrecovery \
			  libcrypto_static  \
			  libmd5  libmiui

LOCAL_STATIC_LIBRARIES += libft2 libpng libminadbd \
			  libfs_mgr liblog 

ifeq ($(TARGET_USERIMAGES_USE_F2FS), true)
 LOCAL_CFLAGS += -DUSE_F2FS
 LOCAL_STATIC_LIBRARIES += libmake_f2fs libfsck_f2fs libfibmap_f2fs
 endif


#LOCAL_STATIC_LIBRARIES += libminzip libunz libmtdutils libmincrypt
LOCAL_SHARED_LIBRARIES +=  libext4_utils libz libmtdutils  \
			   libflashutils libmmcutils \
			   libbmlutils  liberase_image \
			   libdump_image libflash_image \
			   libcutils libstdc++ libc libm \
			   libsparse libstlport

LOCAL_SHARED_LIBRARIES += \
			  libaosprecovery
LOCAL_LDFLAGS := -ldl 
LOCAL_LDFLAGS += -Wl,--no-fatal-warnings

ifeq ($(TARGET_USERIMAGES_USE_F2FS), true)
LOCAL_CFLAGS += -DUSE_F2FS
LOCAL_STATIC_LIBRARIES += libmake_f2fs libfsck_f2fs libfibmap_f2fs
endif
 


include $(BUILD_EXECUTABLE)

RECOVERY_LINKS := bu edify  flash_image dump_image mkyaffs2image \
	unyaffs erase_image nandroid reboot  dedupe minizip \
	start stop setup_adbd fsck_msdos newfs_msdos vdc \
	sdcard  

ifeq ($(TARGET_USERIMAGES_USE_F2FS), true)
 RECOVERY_LINKS += mkfs.f2fs fsck.f2fs fibmap.f2fs
 endif
  

# nc is provided by external/netcat
RECOVERY_SYMLINKS := $(addprefix $(TARGET_RECOVERY_ROOT_OUT)/sbin/,$(RECOVERY_LINKS))
$(RECOVERY_SYMLINKS): RECOVERY_BINARY := $(LOCAL_MODULE)
$(RECOVERY_SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "Symlink: $@ -> $(RECOVERY_BINARY)"
	@mkdir -p $(dir $@)
	@rm -rf $@
	$(hide) ln -sf $(RECOVERY_BINARY) $@

ALL_DEFAULT_INSTALLED_MODULES += $(RECOVERY_SYMLINKS)


ALL_DEFAULT_INSTALLED_MODULES += $(RECOVERY_SYMLINKS)
# Now let's do recovery symlinks
BUSYBOX_LINKS := $(shell cat external/busybox/busybox-full.links)
exclude := tune2fs mke2fs  
RECOVERY_BUSYBOX_SYMLINKS := $(addprefix $(TARGET_ROOT_OUT)/sbin/,$(filter-out $(exclude),$(notdir $(BUSYBOX_LINKS))))
$(RECOVERY_BUSYBOX_SYMLINKS): BUSYBOX_BINARY := busybox
$(RECOVERY_BUSYBOX_SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "Symlink: $@ -> $(BUSYBOX_BINARY)"
	@mkdir -p $(dir $@)
	@rm -rf $@
	$(hide) ln -sf $(BUSYBOX_BINARY) $@

ALL_DEFAULT_INSTALLED_MODULES += $(RECOVERY_BUSYBOX_SYMLINKS)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := verifier_test.cpp verifier.cpp

LOCAL_C_INCLUDES += system/extras/ext4_utils system/core/fs_mgr/include

LOCAL_MODULE := verifier_test

#LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_CPPFLAGS := -std=c90
LOCAL_MODULE_TAGS := tests

LOCAL_STATIC_LIBRARIES := libmincrypt 
LOCAL_SHARED_LIBRARIES :=  libcutils libstdc++ libc

include $(BUILD_EXECUTABLE)


#add extra library
#include bionic/libm/Android.mk
#include external/yaffs2/Android.mk
#add from cm7
include $(commands_recovery_local_path)/bmlutils/Android.mk
include $(commands_recovery_local_path)/flashutils/Android.mk
include $(commands_recovery_local_path)/libcrecovery/Android.mk
#end
include $(commands_recovery_local_path)/miui/Android.mk
include $(commands_recovery_local_path)/minelf/Android.mk
#end
#add libminadbd for sideload
include $(commands_recovery_local_path)/minadbd/Android.mk
#add libminui 
include $(commands_recovery_local_path)/minui/Android.mk

include $(commands_recovery_local_path)/minzip/Android.mk
include $(commands_recovery_local_path)/mtdutils/Android.mk
#add from cm7
include $(commands_recovery_local_path)/mmcutils/Android.mk
#end
include $(commands_recovery_local_path)/tools/Android.mk
include $(commands_recovery_local_path)/edify/Android.mk
include $(commands_recovery_local_path)/updater/Android.mk
include $(commands_recovery_local_path)/applypatch/Android.mk

#add by sndnvaps@gmail.com from Gaojiquan
#include $(commands_recovery_local_path)/supersu/Android.mk
#end 

#add dedupe to replace the tar backup method
include $(commands_recovery_local_path)/dedupe/Android.mk
#add some shell script
include $(commands_recovery_local_path)/utilities/Android.mk
#add yaffs2_static
include $(commands_recovery_local_path)/yaffs2_static/Android.mk
#add digest
include $(commands_recovery_local_path)/digest/Android.mk
#add device conf
include $(commands_recovery_local_path)/devices/Android.mk
#include $(commands_recovery_local_path)/su/Android.mk
#add device_image
include $(commands_recovery_local_path)/device_image/Android.mk
#add pigz to support tar.gz 
include $(commands_recovery_local_path)/pigz/Android.mk
include $(commands_recovery_local_path)/voldclient/Android.mk
commands_recovery_local_path :=

