LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := graphics.c events.c resources.c

LOCAL_C_INCLUDES +=\
    external/libpng\
    external/zlib


ifeq ($(TARGET_RECOVERY_PIXEL_FORMAT),"RGBX_8888")
  LOCAL_CFLAGS += -DRECOVERY_RGBX
endif
ifeq ($(TARGET_RECOVERY_PIXEL_FORMAT),"BGRA_8888")
  LOCAL_CFLAGS += -DRECOVERY_BGRA
endif

ifeq ($(BOARD_HAS_FLIPPED_SCREEN), true)
    LOCAL_CFLAGS += -DBOARD_HAS_FLIPPED_SCREEN
endif

LOCAL_MODULE := libminui

include $(BUILD_STATIC_LIBRARY)
