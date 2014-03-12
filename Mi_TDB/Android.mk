LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := xiaomi_DoubleBoot.c
LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_MODULE := DBS 
LOCAL_STATIC_LIBRARIES := libc \
	                  libcrecovery 
LOCAL_C_INCLUDES := bootable/recovery 
LOCAL_MODULE_CLASS := RECOVERY_EXECUTABLES 
LOCAL_MODULE_PATH := $(TARGET_RECOVERY_ROOT_OUT)/sbin 
include $(BUILD_EXECUTABLE)

