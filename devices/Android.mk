LOCAL_PATH := $(call my-dir)

#device conf file
include $(CLEAR_VARS)
LOCAL_MODULE := device.conf
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE_CLASS := RECOVERY_EXECUTABLES
LOCAL_MODULE_PATH := $(TARGET_RECOVERY_ROOT_OUT)/res
ifneq ($(MIUI_DEVICE_CONF),)
	LOCAL_SRC_FILES := $(MIUI_DEVICE_CONF)
else ifeq ($(TARGET_PRODUCT), cm_crespo)
	LOCAL_SRC_FILES := crespo4g/device.conf
else ifeq ($(TARGET_PRODUCT), cm_N909)
	LOCAL_SRC_FILES := N909/device.conf
else 
	LOCAL_SRC_FILES := $(LOCAL_MODULE)
endif
include $(BUILD_PREBUILT)

#init conf file
include $(CLEAR_VARS)
LOCAL_MODULE := init.conf
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE_CLASS := RECOVERY_EXECUTABLES
LOCAL_MODULE_PATH := $(TARGET_RECOVERY_ROOT_OUT)/res
ifneq ($(MIUI_INIT_CONF),)
	LOCAL_SRC_FILES := $(MIUI_INIT_CONF)
else ifeq ($(TARGET_PRODUCT), cm_crespo)
	LOCAL_SRC_FILES := crespo4g/init.conf
else ifeq ($(TARGET_PRODUCT), cm_N909)
	LOCAL_SRC_FILES := N909/init.conf
else
	LOCAL_SRC_FILES := $(LOCAL_MODULE)
endif
include $(BUILD_PREBUILT)


