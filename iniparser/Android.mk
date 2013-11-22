LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	dictionary.c \
	iniparser.c 
LOCAL_MODULE := libiniparser
LOCAL_MODULE_TAGS := eng

include $(BUILD_STATIC_LIBRARY)


