obj-y += lv_port/lv_init.o lv_port/lv_port_disp.o lv_port/lv_port_indev.o lv_port/evdev.o \
	     lv_port/sunximem.o \
		 lv_port/disp2_thread/sunxifb_thread.o
obj-y += native_app_ota/native_app_ota.o
obj-y += src/fs_adapter/fs_adapter.o
obj-y += src/md5_adapter/md5_adapter.o
obj-y += src/wifi_adapter/wifi_adapter.o
obj-y += src/os_adapter/os_adapter.o
obj-y += src/upgrade_logic/upgrade_logic.o src/upgrade_logic/upgrade_fw.o src/upgrade_logic/upgrade_app.o

ifeq ($(CONFIG_DRIVERS_G2D), y)
obj-y += lv_port/sunxig2d.o
endif

ifeq ($(CONFIG_DRIVERS_SPILCD),y)
obj-y += lv_port/disp2_thread/spilcd_thread.o
CFLAGS+=-DUSE_SPILCD
CFLAGS+=-D'USE_SUNXIFB=1'

ifeq ($(CONFIG_DRIVERS_G2D), y)
CFLAGS+=-DUSE_SUNXIFB_G2D
CFLAGS+=-DUSE_SUNXIFB_G2D_ROTATE
endif

CFLAGS+=-DUSE_SUNXIFB_CACHE
CFLAGS+=-DUSE_SUNXIFB_DOUBLE_BUFFER
# CFLAGS+=-DLV_USE_DIRECT_MODE
else
obj-y += lv_port/disp2_thread/disp2_thread.o
CFLAGS+=-DUSE_DISP2
CFLAGS+=-D'USE_SUNXIFB=1'
CFLAGS+=-DUSE_SUNXIFB_CACHE
endif
CFLAGS+=-DUSE_EVDEV

ifeq ($(CONFIG_COMPONENTS_LWIP_2_1_2),y)
LWIP = lwip-2.1.2
else ifeq ($(CONFIG_COMPONENTS_LWIP_2_1_3),y)
LWIP = lwip-2.1.3
else ifeq ($(CONFIG_COMPONENTS_LWIP_2_0_3),y)
LWIP = lwip-2.0.3
endif

ifeq ($(CONFIG_COMPONENTS_MBEDTLS_2_16_0),y)
MBEDTLS = mbedtls-2.16.0
else ifeq ($(CONFIG_COMPONENTS_MBEDTLS_2_7_0),y)
MBEDTLS = mbedtls-2.7.0
endif

CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_m33/src/recovery/src/
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_m33/src/recovery/lv_port/
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_m33/src/recovery/lv_port/disp2_thread/
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_m33/src/recovery/native_app_ota/
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_m33/src/recovery/
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_c906/src/
CFLAGS += -I $(BASE)/components/common/aw/wireless/wifimanager/core/include/
CFLAGS += -I $(BASE)/components/common/thirdparty/littlevgl-8/lv_recovery/
CFLAGS += -I $(BASE)/components/common/thirdparty/littlevgl-8/lvgl/
CFLAGS += -I $(BASE)/components/common/thirdparty/littlevgl-8/
CFLAGS += -I $(BASE)/components/common/thirdparty/md5/
CFLAGS += -I $(BASE)/components/common/thirdparty/network/httpclient/
CFLAGS += -I $(BASE)/components/common/thirdparty/network/$(MBEDTLS)/include/mbedtls
CFLAGS += -I $(BASE)/components/common/thirdparty/network/$(MBEDTLS)/include/
CFLAGS += -I $(BASE)/components/common/thirdparty/network//$(LWIP)/src/include/lwip/
CFLAGS += -I $(BASE)/components/common/thirdparty/network/$(LWIP)/src/include/
