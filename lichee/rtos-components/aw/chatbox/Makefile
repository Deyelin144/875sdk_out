ifeq ($(CONFIG_COMPONENTS_CHAT_BOX),y)
obj-y += port/audio_codec.o
obj-y += port/opus_wraper.o
obj-y += port/audio_process.o
obj-y += port/display.o
obj-y += port/port_network.o
obj-y += port/mac.o

obj-y += core/application.o
obj-y += core/protocol.o
obj-y += core/transport.o
obj-y += core/websocket.o
obj-y += core/http.o
obj-y += core/iot.o
obj-y += core/ota.o
obj-y += core/posix/background_sched.o
obj-y += core/posix/event_group.o
obj-y += core/posix/queue.o
obj-y += core/posix/schedule.o
obj-y += core/posix/task.o
obj-y += core/posix/mutex.o
obj-y += core/posix/sys_time.o
obj-y += core/ringbuffer.o
obj-y += core/list_buff.o
obj-y += core/storage.o

obj-y += test/audio_test.o
obj-y += test/app_test.o
obj-y += test/test_protocol.o
obj-y += test/test_transport.o

CFLAGS +=  -I components/common/aw/chatbox/
CFLAGS +=  -I components/common/aw/chatbox/include
CFLAGS +=  -I components/common/aw/chatbox/include/utils
CFLAGS +=  -I components/common/aw/chatbox/port/include

CFLAGS +=  -I components/common/aw/AudioSystem/include
CFLAGS +=  -I components/common/thirdparty/webrtc/modules/audio_processing/aec/include
CFLAGS +=  -I components/common/thirdparty/webrtc/modules/audio_processing/ns/include
CFLAGS +=  -I components/common/thirdparty/webrtc/common_audio/vad/include/
CFLAGS +=  -I components/common/thirdparty/opus/include
CFLAGS +=  -I components/common/thirdparty/
# CFLAGS += -Werror
CFLAGS +=  -I components/common/thirdparty/network/websocket/include
CFLAGS +=  -I components/common/thirdparty/cjson/cJSON-1.7.18

ifeq ($(CONFIG_COMPONENTS_LWIP_2_1_2),y)
LWIP = lwip-2.1.2
else ifeq ($(CONFIG_COMPONENTS_LWIP_2_1_3),y)
LWIP = lwip-2.1.3
else ifeq ($(CONFIG_COMPONENTS_LWIP_2_0_3),y)
LWIP = lwip-2.0.3
endif

ifeq ($(CONFIG_COMPONENTS_MBEDTLS_2_16_0),y)
MBEDTLS = mbedtls-2.16.0
CFLAGS += -DCONFIG_COMPONENTS_MBEDTLS_2_16_0
else ifeq ($(CONFIG_COMPONENTS_MBEDTLS_2_7_0),y)
MBEDTLS = mbedtls-2.7.0
CFLAGS += -DCONFIG_COMPONENTS_MBEDTLS_2_7_0
endif

CFLAGS += -Icomponents/common/thirdparty/network/$(MBEDTLS)/include/mbedtls/configs
CFLAGS += -Icomponents/common/thirdparty/network/$(MBEDTLS)/include/mbedtls
CFLAGS += -Icomponents/common/thirdparty/network/$(MBEDTLS)/include
CFLAGS += -Icomponents/common/thirdparty/network/$(LWIP)/src/include/lwip
CFLAGS += -Icomponents/common/thirdparty/network/$(LWIP)/src/include
CFLAGS += -Icomponents/common/aw/xradio/include/
CFLAGS += -Idrivers/drv/wireless/xradio/include/
endif
