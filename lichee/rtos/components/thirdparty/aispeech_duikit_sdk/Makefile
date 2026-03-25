obj-y += src/duiskill_process.o
obj-y += src/main.o
obj-y += lib/libduikit_components.o
obj-y += lib/libduikit_business.o

CFLAGS += -I components/thirdparty/aispeech_duikit_sdk/include/common
CFLAGS += -I components/thirdparty/aispeech_duikit_sdk/include/dds_device_sdk
CFLAGS += -I components/thirdparty/aispeech_duikit_sdk/include/dds_service
CFLAGS += -I components/thirdparty/aispeech_duikit_sdk/include/dsp_manager
CFLAGS += -I components/thirdparty/aispeech_duikit_sdk/include/media_manager
CFLAGS += -I components/thirdparty/aispeech_duikit_sdk/include/business
CFLAGS += -I components/thirdparty/aispeech_duikit_sdk/include/dsp_adapter
CFLAGS += -I components/thirdparty/aispeech_duikit_sdk/include/media_adapter
CFLAGS += -I components/thirdparty/aispeech_duikit_sdk/include

CFLAGS +=  -I components/common/aw/AudioSystem/include

CFLAGS +=-DCONFIG_ADS_COMPONENT_MEDIA_MANAGER

ifeq ($(CONFIG_DISP2_SUNXI),y)
CFLAGS +=-DUSE_DISP2
CFLAGS +=-I drivers/rtos-hal/hal/source/disp2/disp
endif

ifeq ($(CONFIG_COMPONENTS_MBEDTLS_2_16_0), y)
MBEDTLS = mbedtls-2.16.0
else ifeq ($(CONFIG_COMPONENTS_MBEDTLS_2_7_0), y)
MBEDTLS = mbedtls-2.7.0
endif

CFLAGS += -Icomponents/common/thirdparty/network/$(MBEDTLS)
CFLAGS += -Icomponents/common/thirdparty/network/$(MBEDTLS)/include
CFLAGS += -Icomponents/common/thirdparty/network/$(MBEDTLS)/include/mbedtls/
CFLAGS += -Icomponents/common/thirdparty/network/$(MBEDTLS)/include/mbedtls/configs
CFLAGS += -DMBEDTLS_CONFIG_FILE='<config-xr-mini-cliserv.h>'
