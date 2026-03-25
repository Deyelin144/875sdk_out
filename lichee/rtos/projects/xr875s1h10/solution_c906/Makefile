obj-y += src/main.o
obj-y += src/sdk_entry.o
obj-y += src/sdk_runtime.o
obj-y += src/sdk_exception_runtime.o
obj-y += src/sdk_log_runtime.o
obj-y += src/sdk_mem_runtime.o
obj-y += src/assert.o
obj-y += src/hooks.o
obj-y += src/alsa_config.o
obj-y += src/fs_demo.o

ifeq ($(CONFIG_DRIVERS_SOUND),y)
obj-y += src/card_default.o
endif

ifeq ($(CONFIG_DRIVERS_SOUND_V2),y)
obj-y += src/card_v2_default.o
endif


obj-y += src/drivers/
obj-y += src/realize/
obj-y += src/wrappers/

LVGLDIR := ../../../components/common/thirdparty/littlevgl-8/lvgl/src
include $(BASE)/components/common/thirdparty/littlevgl-8/lvgl/src/Makefile
obj-y := $(filter-out \
	$(LVGLDIR)/extra/libs/freetype/lv_freetype.o \
	$(LVGLDIR)/extra/libs/fsdrv/lv_fs_fatfs.o, \
	$(obj-y))

CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_c906/src/include
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_c906/src
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_c906/src/lvgl
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_c906/src/include/platform
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_c906/src/include/components/GUI
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_c906/src/apps
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_c906/src/realize/
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_c906/src/drivers/
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_c906/src/transparent/
CFLAGS += -I $(BASE)/components
CFLAGS += -I $(BASE)/components/common/thirdparty/littlevgl-8
CFLAGS += -I $(BASE)/components/common/thirdparty/littlevgl-8/lvgl
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_c906/src/module/mod_twecall/iot_video_ipc/include  
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_c906/src/module/mod_twecall/iot_video_ipc/include/explorer_inc
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_c906/src/module/mod_twecall/iot_video_ipc/iot_video_ipc/av
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_c906/src/module/mod_twecall/iot_video_ipc/iot_video_ipc/cloud_storage
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_c906/src/module/mod_twecall/iot_video_ipc/iot_video_ipc/model
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_c906/src/module/mod_twecall/iot_video_ipc/iot_video_ipc/system
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_c906/src/module/mod_twecall/iot_video_ipc/iot_video_ipc/upgrade
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_c906/src/module/mod_twecall/iot_video_ipc/iot_video_ipc/virtual_ipc
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_c906/src/include
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_c906/src/include/platform
CFLAGS += -I $(BASE)/../rtos-components/thirdparty/network/libevent/platform/include
CFLAGS += -I $(BASE)/../rtos-components/thirdparty/network/libevent/platform/utils/inc

CFLAGS += -I $(BASE)/components/common/aw/xradio/include
CFLAGS += -I $(BASE)/drivers/rtos-hal/hal/source/prcm/
CFLAGS += -DLV_CONF_INCLUDE_SIMPLE
CFLAGS += -DLV_LVGL_H_INCLUDE_SIMPLE

ifeq ($(CONFIG_COMPONENTS_AW_AUDIO_SYSTEM),y)
CFLAGS += -I $(BASE)/components/common/aw/AudioSystem/include
endif

ifeq ($(CONFIG_COMPONENTS_WLAN_CSFC),y)
CFLAGS += -I $(BASE)/components/common/aw/network/wlan_csfc
endif

CFLAGS += -I components/common/thirdparty/libjpeg-turbo/source/
CFLAGS += -I components/common/thirdparty/libjpeg-turbo/include/
