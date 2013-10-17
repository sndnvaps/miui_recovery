LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)


libmiui_common_includes = $(LOCAL_PATH)/include \
			  external/zlib \
			  external/freetype/build \
			  external/freetype/include
libmiui_common_src_files := \
    libs/minutf8/minutf8.c \
    src/edify/lex.yy.c \
	src/edify/parser.c \
	src/edify/expr.c \
    src/libs/miui_array.c \
    src/libs/miui_freetype.c \
    src/libs/miui_graph.c \
    src/libs/miui_input.c \
    src/libs/miui_languages.c \
    src/libs/miui_libs.c \
    src/libs/miui_png.c \
    src/controls/miui_control_button.c \
    src/controls/miui_control_check.c \
    src/controls/miui_control_optbox.c \
    src/controls/miui_control_textbox.c \
    src/controls/miui_control_checkbox.c \
    src/controls/miui_control_menubox.c \
    src/controls/miui_control_sdmenu.c \
    src/controls/miui_control_title.c \
    src/controls/miui_controls.c \
    src/controls/miui_control_threads.c \
    src/main/miui_ui.c \
    src/main/common_ui.c \
    src/main/lang_ui.c \
    src/main/sd_file.c \
    src/main/sd_ui.c \
    src/main/power_ui.c \
    src/main/mount_ui.c \
    src/main/wipe_ui.c \
    src/main/backup_ui.c \
    src/main/info_ui.c \
    src/main/menu_node.c \
    src/main/miui_installer.c \
    src/main/miui.c \
    src/main/root_ui.c 

# libs/freetype/autofit/autofit.c \
    libs/freetype/base/basepic.c \
    libs/freetype/base/ftapi.c \
    libs/freetype/base/ftbase.c \
    libs/freetype/base/ftbbox.c \
    libs/freetype/base/ftbitmap.c \
    libs/freetype/base/ftdbgmem.c \
    libs/freetype/base/ftdebug.c \
    libs/freetype/base/ftglyph.c \
    libs/freetype/base/ftinit.c \
    libs/freetype/base/ftpic.c \
    libs/freetype/base/ftstroke.c \
    libs/freetype/base/ftsynth.c \
    libs/freetype/base/ftsystem.c \
    libs/freetype/cff/cff.c \
    libs/freetype/pshinter/pshinter.c \
    libs/freetype/psnames/psnames.c \
    libs/freetype/raster/raster.c \
    libs/freetype/sfnt/sfnt.c \
    libs/freetype/smooth/smooth.c \
    libs/freetype/truetype/truetype.c \
    libs/png/png.c \
    libs/png/pngerror.c \
    libs/png/pnggccrd.c \
    libs/png/pngget.c \
    libs/png/pngmem.c \
    libs/png/pngpread.c \
    libs/png/pngread.c \
    libs/png/pngrio.c \
    libs/png/pngrtran.c \
    libs/png/pngrutil.c \
    libs/png/pngset.c \
    libs/png/pngtrans.c \
    libs/png/pngvcrd.c \
   # src/main/tool_ui.c 

LOCAL_SRC_FILES := \
    $(libmiui_common_src_files) \
    ../miui_intent.c 
LOCAL_C_INCLUDES += $(libmiui_common_includes) \
		    external/freetype/build \
		    external/freetype/include \
		    external/libpng
LOCAL_CFLAGS := $(MYDEFINE_CFLAGS)
LOCAL_CFLAGS += -DDEBUG
#LOCAL_CFLAGS += -fPIC -DPIC
LOCAL_CFLAGS += -I$(LOCAL_PATH)/include \
		-I$(LOCAL_PATH)/../../../external/freetype/include \
		-I$(LOCAL_PATH)/../../../external/freetype/builds \
		-I$(LOCAL_PATH)/../../../external/zlib \
		-I$(LOCAL_PATH)/../../../external/libpng
#LOCAL_CFLAGS += -D_MIUI_NODEBUG
LOCAL_SHARED_LIBRARIES += libc libm libz
LOCAL_STATIC_LIBRARIES := libft2 libpng
LOCAL_MODULE := libmiui
LOCAL_MODULE_TAGS := eng
ifeq ($(BUILD_CHN_REC),)
	LOCAL_CFLAGS := -DBUILD_CHN_REC
endif

ifeq ($(TARGET_RECOVERY_PIXEL_FORMAT), "RGBX_8888")
	LOCAL_CFLAGS += -DGGL_PIXEL_FORMAT_RGBX_8888
endif
ifeq ($(TARGET_RECOVERY_PIXEL_FORMAT), "BGRA_8888")
	LOCAL_CFLAGS += -DGGL_PIXEL_FORMAT_BGRA_8888
endif
ifeq ($(TARGET_RECOVERY_PIXEL_FORMAT), "BGR_565")
	LOCAL_CFLAGS += -DPIXEL_FORMAT_BGR_565 
endif

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)


