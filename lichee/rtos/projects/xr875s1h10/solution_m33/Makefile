
obj-y += src/assert.o
obj-y += src/hooks.o
obj-y += src/alsa_config.o
obj-y += src/card_default.o

ifeq ($(CONFIG_PROJECT_BUILD_RECOVERY),y)
obj-y += src/recovery/
obj-y += src/main_ota.o

else
obj-y += src/main.o
obj-y += src/drivers/
endif

ifeq ($(CONFIG_PROJECT_BUILD_RECOVERY),y)
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_c906/src/
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_m33/src/recovery/src/
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
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_m33/src/recovery/
CFLAGS += -I $(BASE)/components/common/aw/wireless/wifimanager/core/include/
CFLAGS += -I $(BASE)/components/common/thirdparty/littlevgl-8/lv_recovery/
CFLAGS += -I $(BASE)/components/common/thirdparty/littlevgl-8/lvgl/
CFLAGS += -I $(BASE)/components/common/thirdparty/network/httpclient/
CFLAGS += -I $(BASE)/components/common/thirdparty/network/$(MBEDTLS)/include/mbedtls
CFLAGS += -I $(BASE)/components/common/thirdparty/network/$(MBEDTLS)/include/
CFLAGS += -I $(BASE)/components/common/thirdparty/network/$(LWIP)/src/include/lwip/
CFLAGS += -I $(BASE)/components/common/thirdparty/network/$(LWIP)/src/include/
CFLAGS += -I $(BASE)/drivers/rtos-hal/hal/source/prcm/
endif

ifeq ($(CONFIG_COMPONENTS_AW_AUDIO_SYSTEM),y)
CFLAGS += -I $(BASE)/components/common/aw/AudioSystem/include
endif

ifeq ($(CONFIG_COMPONENTS_WLAN_CSFC),y)
CFLAGS += -I $(BASE)/components/common/aw/network/wlan_csfc
endif

