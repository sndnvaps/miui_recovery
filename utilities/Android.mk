LOCAL_PATH := $(call my-dir)

RELINK := $(LOCAL_PATH)/relink.sh

#dummy file to trigger required modules
include $(CLEAR_VARS)
LOCAL_MODULE := miui_rec
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE_CLASS := RECOVERY_EXECUTABLES
LOCAL_MODULE_PATH := $(TARGET_RECOVERY_ROOT_OUT)/sbin

# Manage list 
RELINK_SOURCE_FILES += $(TARGET_OUT_OPTIONAL_EXECUTABLES)/busybox
RELINK_SOURCE_FILES += $(TARGET_OUT_OPTIONAL_EXECUTABLES)/pigz
RELINK_SOURCE_FILES += $(TARGET_OUT_EXECUTABLES)/linker
RELINK_SOURCE_FILES += $(TARGET_OUT_EXECUTABLES)/e2fsck
RELINK_SOURCE_FILES += $(TARGET_OUT_EXECUTABLES)/mke2fs
RELINK_SOURCE_FILES += $(TARGET_OUT_EXECUTABLES)/tune2fs
RELINK_SOURCE_FILES += $(TARGET_OUT_SHARED_LIBRARIES)/libaosprecovery.so
RELINK_SOURCE_FILES += $(TARGET_OUT_SHARED_LIBRARIES)/libc.so
RELINK_SOURCE_FILES += $(TARGET_OUT_SHARED_LIBRARIES)/libcutils.so
RELINK_SOURCE_FILES += $(TARGET_OUT_SHARED_LIBRARIES)/libdl.so
RELINK_SOURCE_FILES += $(TARGET_OUT_SHARED_LIBRARIES)/libext2_blkid.so
RELINK_SOURCE_FILES += $(TARGET_OUT_SHARED_LIBRARIES)/libext2_com_err.so
RELINK_SOURCE_FILES += $(TARGET_OUT_SHARED_LIBRARIES)/libext2_e2p.so
RELINK_SOURCE_FILES += $(TARGET_OUT_SHARED_LIBRARIES)/libext2fs.so
RELINK_SOURCE_FILES += $(TARGET_OUT_SHARED_LIBRARIES)/libext2_profile.so
RELINK_SOURCE_FILES += $(TARGET_OUT_SHARED_LIBRARIES)/libext2_uuid.so
RELINK_SOURCE_FILES += $(TARGET_OUT_SHARED_LIBRARIES)/liblog.so
RELINK_SOURCE_FILES += $(TARGET_OUT_SHARED_LIBRARIES)/libm.so
RELINK_SOURCE_FILES += $(TARGET_OUT_SHARED_LIBRARIES)/libstdc++.so
RELINK_SOURCE_FILES += $(TARGET_OUT_SHARED_LIBRARIES)/libstlport.so
RELINK_SOURCE_FILES += $(TARGET_OUT_SHARED_LIBRARIES)/libz.so
RELINK_SOURCE_FILES += $(TARGET_OUT_SHARED_LIBRARIES)/libmtdutils.so
RELINK_SOURCE_FILES += $(TARGET_OUT_SHARED_LIBRARIES)/libmmcutils.so
RELINK_SOURCE_FILES += $(TARGET_OUT_SHARED_LIBRARIES)/libbmlutils.so
RELINK_SOURCE_FILES += $(TARGET_OUT_SHARED_LIBRARIES)/libflashutils.so
RELINK_SOURCE_FILES += $(TARGET_OUT_SHARED_LIBRARIES)/liberase_image.so
RELINK_SOURCE_FILES += $(TARGET_OUT_SHARED_LIBRARIES)/libflash_image.so
RELINK_SOURCE_FILES += $(TARGET_OUT_SHARED_LIBRARIES)/libdump_image.so
#RELINK_SOURCE_FILES += $(TARGET_OUT_SHARED_LIBRARIES)/libmiui.so
RELINK_SOURCE_FILES += $(TARGET_OUT_SHARED_LIBRARIES)/libext4_utils.so
RELINK_SOURCE_FILES += $(TARGET_OUT_SHARED_LIBRARIES)/libsparse.so
RELINK_SOURCE_FILES += $(TARGET_OUT_SHARED_LIBRARIES)/libft2.so
RELINK_SOURCE_FILES += $(TARGET_OUT_SHARED_LIBRARIES)/libselinux.so
RELINK_SOURCE_FILES += $(TARGET_OUT_SHARED_LIBRARIES)/libpng.so

    #RELINK_SOURCE_FILES += $(TARGET_OUT_SHARED_LIBRARIES)/libcrypto.so
#ifeq ($(TARGET_USERIMAGES_USE_EXT4), true)
RELINK_SOURCE_FILES += $(TARGET_OUT_EXECUTABLES)/make_ext4fs
  
#endif

TWRP_AUTOGEN := $(intermediates)/miui_rec

GEN := $(intermediates)/miui_rec
$(GEN): $(RELINK)
$(GEN): $(RELINK_SOURCE_FILES) $(call intermediates-dir-for,EXECUTABLES,recovery)/recovery
	$(RELINK) $(TARGET_RECOVERY_ROOT_OUT)/sbin $(RELINK_SOURCE_FILES)
	$(RELINK) $(call intermediates-dir-for,EXECUTABLES,recovery) $(call intermediates-dir-for,EXECUTABLES,recovery)/recovery

LOCAL_GENERATED_SOURCES := $(GEN)
LOCAL_SRC_FILES := miui_rec $(GEN)
include $(BUILD_PREBUILT)

#fix_permissions
include $(CLEAR_VARS)
LOCAL_MODULE := fix_permissions
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE_CLASS := RECOVERY_EXECUTABLES
LOCAL_MODULE_PATH := $(TARGET_RECOVERY_ROOT_OUT)/sbin
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

#busybox
#include $(CLEAR_VARS)
#LOCAL_MODULE := busybox
#LOCAL_MODULE_TAGS := eng
#LOCAL_MODULE_CLASS := RECOVERY_EXECUTABLES
#LOCAL_MODULE_PATH := $(TARGET_RECOVERY_ROOT_OUT)/sbin
#LOCAL_SRC_FILES := $(LOCAL_MODULE)
#include $(BUILD_PREBUILT)

#mke2fs.conf
include $(CLEAR_VARS)
LOCAL_MODULE := mke2fs.conf
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE_CLASS := RECOVERY_EXECUTABLES
LOCAL_MODULE_PATH := $(TARGET_RECOVERY_ROOT_OUT)/etc
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# verifier_test.sh
include $(CLEAR_VARS)
LOCAL_MODULE := verifier_test.sh
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE_CLASS := RECOVERY_EXECUTABLES
LOCAL_MODULE_PATH := $(TARGET_RECOVERY_ROOT_OUT)/sbin
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := ors-mount.sh
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE_CLASS := RECOVERY_EXECUTABLES
LOCAL_MODULE_PATH := $(TARGET_RECOVERY_ROOT_OUT)/sbin
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := killrecovery.sh
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE_CLASS := RECOVERY_EXECUTABLES
LOCAL_MODULE_PATH := $(TARGET_RECOVERY_ROOT_OUT)/sbin
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

ifeq ($(BUILD_CHN_REC),)
	include $(CLEAR_VARS)
LOCAL_MODULE := DroidSansFallback.ttf
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE_CLASS := RECOVERY_EXECUTABLES
LOCAL_MODULE_PATH := $(TARGET_RECOVERY_ROOT_OUT)/res/ttf
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

endif

ifeq ($(BOARD_HAS_NO_REAL_SDCARD),)
	#parted
	include $(CLEAR_VARS)
	LOCAL_MODULE := parted
	LOCAL_MODULE_TAGS := eng
	LOCAL_MODULE_CLASS := RECOVERY_EXECUTABLES
	LOCAL_MODULE_PATH := $(TARGET_RECOVERY_ROOT_OUT)/sbin
	LOCAL_SRC_FILES := $(LOCAL_MODULE)
	include $(BUILD_PREBUILT)
endif

ifeq ($(TW_EXCLUDE_SUPERSU),)
	#su binary
	include $(CLEAR_VARS)
	LOCAL_MODULE := su
	LOCAL_MODULE_TAGS := eng
	LOCAL_MODULE_CLASS := RECOVERY_EXECUTABLES
	LOCAL_MODULE_PATH := $(TARGET_RECOVERY_ROOT_OUT)/supersu
	LOCAL_SRC_FILES := $(LOCAL_MODULE)
	include $(BUILD_PREBUILT)

 #su.ext binary 
include $(CLEAR_VARS)
LOCAL_MODULE := su.ext
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE_CLASS := RECOVERY_EXECUTABLES
LOCAL_MODULE_PATH := $(TARGET_RECOVERY_ROOT_OUT)/supersu
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

	#SuperSU special installer APK
	include $(CLEAR_VARS)
	LOCAL_MODULE := Superuser.apk
	LOCAL_MODULE_TAGS := eng
	LOCAL_MODULE_CLASS := RECOVERY_EXECUTABLES
	LOCAL_MODULE_PATH := $(TARGET_RECOVERY_ROOT_OUT)/supersu
	LOCAL_SRC_FILES := $(LOCAL_MODULE)
	include $(BUILD_PREBUILT)
endif
