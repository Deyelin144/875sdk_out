CUR_MAKEFILE_DIR := $(patsubst %/, %, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
CUR_SRC_FILES := $(notdir $(patsubst %.c, %.o, $(wildcard $(CUR_MAKEFILE_DIR)/*.c)))

obj-y += $(CUR_SRC_FILES)

ifeq ($(CONFIG_COMPONENTS_LWIP_2_1_2),y)
LWIP = lwip-2.1.2
else ifeq ($(CONFIG_COMPONENTS_LWIP_2_1_3),y)
LWIP = lwip-2.1.3
else ifeq ($(CONFIG_COMPONENTS_LWIP_2_0_3),y)
LWIP = lwip-2.0.3
endif

CFLAGS += -I $(CUR_MAKEFILE_DIR)/
CFLAGS += -I $(BASE)/components/common/aw/wireless/wifimanager/core/include
CFLAGS += -I $(BASE)/drivers/drv/wireless/xradio/include
CFLAGS += -I $(BASE)/components/common/thirdparty/network/$(LWIP)/src/include/lwip/
CFLAGS += -I $(BASE)/components/common/thirdparty/network/$(LWIP)/src/include/
