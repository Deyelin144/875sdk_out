obj-y += buffer.o \
         bufferevent.o \
         bufferevent_filter.o \
         bufferevent_pair.o \
         bufferevent_ratelim.o \
         bufferevent_sock.o \
         event.o \
         evmap.o \
         evthread.o \
         evutil.o \
         evutil_rand.o \
         evutil_time.o \
         listener.o \
         log.o\
         signal.o \
         strlcpy.o \
         evthread_pthread.o \
         select.o \
         evthread_hal.o \
         event_tagging.o \
         http.o \
         evdns.o \
         evrpc.o \
         bufferevent_mbedtls.o \
		 platform/libevent/basic_test.o \
		 platform/libevent/get_ip.o \
		 platform/libevent/libevent_test.o \
		 platform/tls/HAL_TLS_mbedtls.o \
         platform/utils/src/json_parser.o \
         platform/utils/src/json_token.o \
         platform/utils/src/utils_list.o \
         platform/utils/src/utils_lite_string.o \
         platform/utils/src/utils_log.o \
         platform/utils/src/utils_ringbuffer.o \
		 platform/freertos/HAL_File_freertos.o \
		 platform/freertos/HAL_TCP_lwip.o \
		 platform/freertos/HAL_OS_freertos.o \
		 platform/freertos/HAL_Timer_freertos.o \
         platform/main_test.o \
         platform/net_test.o \
         platform/os_test.o \


ifeq ($(CONFIG_COMPONENTS_LWIP_2_1_2),y)
LWIP = lwip-2.1.2
else ifeq ($(CONFIG_COMPONENTS_LWIP_2_1_3),y)
LWIP = lwip-2.1.3
$(info rocky -- lwip --- 2.1.3 )
else ifeq ($(CONFIG_COMPONENTS_LWIP_2_0_3),y)
LWIP = lwip-2.0.3
endif

CFLAGS += -Icomponents/common/thirdparty/network/libevent/include
CFLAGS += -Icomponents/common/thirdparty/network/libevent/platform
CFLAGS += -Icomponents/common/thirdparty/network/libevent/platform/include
CFLAGS += -Icomponents/common/thirdparty/network/libevent/platform/internal_inc
CFLAGS += -Icomponents/common/thirdparty/network/libevent/platform/freertos
CFLAGS += -Icomponents/common/thirdparty/network/libevent/platform/tls
CFLAGS += -Icomponents/common/thirdparty/network/libevent/platform/libevent
CFLAGS += -Icomponents/common/thirdparty/network/libevent/platform/utils/inc

CFLAGS += -Icomponents/common/aw/xradio/include

CFLAGS += -Icomponents/common/thirdparty/network/$(LWIP)/src/include
CFLAGS += -Icomponents/common/thirdparty/network/$(LWIP)/src/include/lwip
CFLAGS += -Icomponents/common/thirdparty/network/$(LWIP)/src/include/compat/posix
CFLAGS += -Icomponents/common/thirdparty/network/mbedtls-2.16.0/include

CFLAGS += -DLWIP_IPV4 \
        -DLWIP_COMPAT_SOCKETS \
        -DLWIP_DNS \
        -DLWIP_SOCKET \
        -DLWIP_MDNS_RESPONDER \
        -DPLATFORM_FREERTOS \
        -DPLATFORM_XR875_RTOS

CFLAGS += -Werror

$(info rocky ----------------------LWIP = $(LWIP))
$(info rocky ----------------------CC = $(CC))