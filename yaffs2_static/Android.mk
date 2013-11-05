# Copyright 2013 The Android Open Source Project
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := \
	mkyaffs2image.c \
        yaffs_packedtags2.c \
        yaffs_ecc.c \
	yaffs_tagsvalidity.c
LOCAL_CFLAGS =   -O2 -Wall -DCONFIG_YAFFS_UTIL -DCONFIG_YAFFS_DOES_ECC
LOCAL_CFLAGS+=   -Wshadow -Wpointer-arith -Wwrite-strings -Wstrict-prototypes -Wmissing-declarations
LOCAL_CFLAGS+=   -Wmissing-prototypes -Wredundant-decls -Wnested-externs -Winline
LOCAL_CFLAGS+=   -DS_IWRITE=0200 -DS_IREAD=0400
LOCAL_C_INCLUDES := external/yaffs2/yaffs2
LOCAL_MODULE := libmkyaffs2image_static
LOCAL_MODULE_TAGS := eng
#LOCAL_CFLAGS += -Dmain=mkyaffs2image_main
ifeq ($(HAVE_SELINUX), true)
LOCAL_C_INCLUDES += bootable/recovery/libselinux/include
LOCAL_STATIC_LIBRARIES += libselinux
LOCAL_CFLAGS += -DHAVE_SELINUX
endif # HAVE_SELINUX
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libunyaffs_static
LOCAL_SRC_FILES := unyaffs.c
LOCAL_MODULE_TAGS := eng
LOCAL_CFLAGS =   -O2 -Wall -DCONFIG_YAFFS_UTIL -DCONFIG_YAFFS_DOES_ECC
LOCAL_CFLAGS+=   -Wshadow -Wpointer-arith -Wwrite-strings -Wstrict-prototypes -Wmissing-declarations
LOCAL_CFLAGS+=   -Wmissing-prototypes -Wredundant-decls -Wnested-externs -Winline
LOCAL_CFLAGS+=   -DS_IWRITE=0200 -DS_IREAD=0400
LOCAL_C_INCLUDES := external/yaffs2/yaffs2
#LOCAL_CFLAGS += -Dmain=unyaffs_main

include $(BUILD_STATIC_LIBRARY)
