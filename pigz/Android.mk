LOCAL_PATH := $(call my-dir)

pigz_src_files := pigz.c yarn.c
zopfli_src_files := zopfli/blocksplitter.c \
					zopfli/cache.c \
					zopfli/deflate.c \
					zopfli/tree.c \
					zopfli/lz77.c \
					zopfli/hash.c \
					zopfli/util.c \
					zopfli/squeeze.c \
					zopfli/katajainen.c

include $(CLEAR_VARS)
LOCAL_MODULE := libzopfli
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(zopfli_src_files)
LOCAL_C_INCLUDES := $(LOCAL_PATH) external/zlib zopfli
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libpigz
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(pigz_src_files)
LOCAL_C_INCLUDES := $(LOCAL_PATH) external/zlib zopfli
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libminipigz
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(pigz_src_files)
LOCAL_C_INCLUDES := $(LOCAL_PATH) external/zlib zopfli
LOCAL_CFLAGS := -DWITHOUT_ZOPFLI
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := pigz
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := main.c
LOCAL_SHARED_LIBRARIES := libz libc
LOCAL_STATIC_LIBRARIES := libpigz libzopfli
#LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_MODULE_CLASS := RECOVERY_EXECUTABLES 
LOCAL_MODULE_PATH := $(TARGET_RECOVERY_ROOT_OUT)/sbin
include $(BUILD_EXECUTABLE)

PIGZ_TOOLS := unpigz
SYMLINKS := $(addprefix $(TARGET_OUT_OPTIONAL_EXECUTABLES)/,$(PIGZ_TOOLS))
$(SYMLINKS): PIGZ_BINARY := $(LOCAL_MODULE)
$(SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "Symlink: $@ -> $(PIGZ_BINARY)"
	@mkdir -p $(dir $@)
	@rm -rf $@
	$(hide) ln -sf $(PIGZ_BINARY) $@

ALL_DEFAULT_INSTALLED_MODULES += $(SYMLINKS)

# We need this so that the installed files could be picked up based on the
# local module name
ALL_MODULES.$(LOCAL_MODULE).INSTALLED := \
    $(ALL_MODULES.$(LOCAL_MODULE).INSTALLED) $(SYMLINKS)
