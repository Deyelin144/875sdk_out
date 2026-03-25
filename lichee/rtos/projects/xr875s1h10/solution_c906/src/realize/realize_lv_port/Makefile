CUR_MAKEFILE_DIR := $(patsubst %/, %, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
CUR_SRC_FILES := $(notdir $(patsubst %.c, %.o, $(wildcard $(CUR_MAKEFILE_DIR)/*.c)))
obj-y += $(CUR_SRC_FILES)
obj-y += disp2_thread/sunxifb_thread.o
obj-y += disp2_thread/extra_layer.o


CFLAGS += -I $(CUR_MAKEFILE_DIR)/
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_c906/src/include
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_c906/src/lvgl
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_c906/src/include/platform
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_c906/src/include/components/GUI
CFLAGS += -I $(BASE)/components/common/thirdparty/littlevgl-8
CFLAGS += -I $(BASE)/components/common/thirdparty/littlevgl-8/lvgl
CFLAGS += -I $(BASE)/components/common/aw/xradio/include
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_c906/src/apps/
CFLAGS += -I $(BASE)/components/common/thirdparty/libjpeg-turbo/include/
CFLAGS += -I $(BASE)/components/common/thirdparty/libjpeg-turbo/source/
CFLAGS += -I $(BASE)/drivers/rtos-hal/hal/source/csi/
CFLAGS += -I $(BASE)/drivers/rtos-hal/hal/source/csi/jpeg/
CFLAGS += -I $(BASE)/drivers/rtos-hal/hal/source/csi/jpeg/jpeg_reg/
CFLAGS += -I $(BASE)/drivers/rtos-hal/hal/source/csi/csi_camera/
CFLAGS += -I $(BASE)/drivers/rtos-hal/hal/source/csi/csi_camera/csi_reg/
CFLAGS += -DLV_CONF_INCLUDE_SIMPLE
CFLAGS += -DLV_LVGL_H_INCLUDE_SIMPLE

ifeq ($(CONFIG_DRIVERS_SPILCD),y)
obj-y += disp2_thread/spilcd_thread.o
CFLAGS+=-DUSE_SPILCD
CFLAGS+=-D'USE_SUNXIFB=1'
CFLAGS+=-DUSE_SUNXIFB_G2D
CFLAGS+=-DUSE_SUNXIFB_G2D_ROTATE
CFLAGS+=-DUSE_SUNXIFB_CACHE
CFLAGS+=-DUSE_SUNXIFB_DOUBLE_BUFFER
# CFLAGS+=-DLV_USE_DIRECT_MODE
else
obj-y += disp2_thread/disp2_thread.o
CFLAGS+=-DUSE_DISP2
CFLAGS+=-D'USE_SUNXIFB=1'
CFLAGS+=-DUSE_SUNXIFB_CACHE
endif

CFLAGS+=-DUSE_EVDEV
