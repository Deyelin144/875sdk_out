ifeq ($(CONFIG_LIB_MULTIMEDIA),y)
RTPLAYER_INCLUDE_DIR := components/aw/multimedia/include

CFLAGS += -I $(RTPLAYER_INCLUDE_DIR)
CFLAGS += -I $(RTPLAYER_INCLUDE_DIR)/include/cedarx/xplayer/include
CFLAGS += -I $(RTPLAYER_INCLUDE_DIR)/libcore/base/include
CFLAGS += -I $(RTPLAYER_INCLUDE_DIR)/libcore/playback/include
CFLAGS += -I $(RTPLAYER_INCLUDE_DIR)/Cdx2.0Plugin/include
CFLAGS += -I $(RTPLAYER_INCLUDE_DIR)/libcore/parser/include
CFLAGS += -I $(RTPLAYER_INCLUDE_DIR)/libcore/stream/include

ifeq ($(CONFIG_COMPONENTS_AW_AUDIO_SYSTEM),y)
CFLAGS +=  -I components/common/aw/AudioSystem/include

obj-y += sound_ctrl/rtosSound_as.o
endif #CONFIG_COMPONENTS_AW_AUDIO_SYSTEM

ifeq ($(CONFIG_SUN20IW2_BT_CONTROLLER),y)
CFLAGS +=  -I components/common/aw/wireless/btmanager/src

ifeq ($(CONFIG_BLEHOST),y)
CFLAGS +=  -I components/common/aw/xradio/include
CFLAGS +=  -I components/common/thirdparty/bluetooth/zephyr/include
CFLAGS +=  -I components/common/thirdparty/bluetooth/zephyr/include/ble
endif #CONFIG_BLEHOST

obj-y += sound_ctrl/rtosSound_bt.o

endif #CONFIG_SUN20IW2_BT_CONTROLLER

obj-y += rtplayer.o
obj-y += sound_ctrl/rtosSoundControl.o

obj-y += stream/CdxFifoStream.o
# add new ssl stream
obj-y += stream/CdxSSLStream.o
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
else ifeq ($(CONFIG_COMPONENTS_MBEDTLS_USER),y)
MBEDTLS = mbedtls-2.16.0
endif

CFLAGS += -I components/common/thirdparty/network/$(MBEDTLS)/include/mbedtls
CFLAGS += -I components/common/thirdparty/network/$(MBEDTLS)/include
CFLAGS += -I components/common/thirdparty/network/$(LWIP)/src/include/lwip
CFLAGS += -I components/common/thirdparty/network/$(LWIP)/src/include
endif #CONFIG_LIB_MULTIMEDIA

ifeq ($(CONFIG_LIB_RECORDER),y)
RTRECORDER_INCLUDE_DIR := components/aw/multimedia/include

CFLAGS += -I $(RTPLAYER_INCLUDE_DIR)
CFLAGS += -I $(RTPLAYER_INCLUDE_DIR)/libcore/record/include
obj-y += capture_ctrl/rtosCaptureControl.o
endif

CFLAGS += -Werror
