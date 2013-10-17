LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libmd5
LOCAL_MODULE_TAGS := eng
LOCAL_SRC_FILES := md5.c
include $(BUILD_STATIC_LIBRARY)


