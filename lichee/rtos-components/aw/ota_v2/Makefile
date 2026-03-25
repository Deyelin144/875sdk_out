obj-$(CONFIG_COMPONENTS_AW_OTA_V2) += aw_ota_core.o
obj-$(CONFIG_COMPONENTS_AW_OTA_FILE) += aw_ota_stream/aw_ota_file.o
obj-$(CONFIG_COMPONENTS_AW_OTA_HTTP) += aw_ota_stream/aw_ota_http.o


obj-$(CONFIG_COMPONENTS_AW_OTA_COMPOENT) += aw_ota_compoents/aw_ota_compoents.o \
											aw_ota_compoents/aw_ota_parser.o \
											aw_ota_compoents/aw_ota_update.o

ifeq ($(CONFIG_COMPONENTS_AW_OTA_HTTP),y)

ifeq ($(CONFIG_COMPONENTS_LWIP_2_1_2),y)
LWIP = lwip-2.1.2
else ifeq ($(CONFIG_COMPONENTS_LWIP_2_1_3),y)
LWIP = lwip-2.1.3
else ifeq ($(CONFIG_COMPONENTS_LWIP_2_0_3),y)
LWIP = lwip-2.0.3
endif #CONFIG_COMPONENTS_LWIP_2_1_2
CFLAGS += -I components/common/thirdparty/network/$(LWIP)/src/include

ifeq ($(CONFIG_COMPONENTS_MBEDTLS_2_7_0),y)
MBEDTLS = mbedtls-2.7.0
else ifeq ($(CONFIG_COMPONENTS_MBEDTLS_2_16_0),y)
MBEDTLS = mbedtls-2.16.0
endif #CONFIG_COMPONENTS_MBEDTLS_2_7_0
CFLAGS += -I components/common/thirdparty/network/$(MBEDTLS)/include

CFLAGS += -I components/common/thirdparty/network/httpclient
CFLAGS += -I components/common/aw/xradio/include/

endif #CONFIG_COMPONENTS_AW_OTA_HTTP

ifeq ($(CONFIG_COMPONENTS_AW_OTA_COMPOENT),y)
CFLAGS += -I components/common/thirdparty/md5
endif

CFLAGS += -I components/common/aw/ota_v2
CFLAGS += -Werror
