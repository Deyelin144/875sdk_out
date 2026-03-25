
#HERE1 := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
#BT_BASE:=$(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))


#$(LINKER_SCRIPTS): CFLAGS += -I $(HERE1)zephyr/include/

ifeq ($(CONFIG_ARCH_SUN20IW2P1), y)
$(XRADIO2_OBJS):CFLAGS +=-DCONFIG_OS_TINA
$(XRADIO2_OBJS):CFLAGS += -I $(BASE)/include/arch/arm/armv8m/
$(XRADIO2_OBJS):CFLAGS += -I $(BASE)/include/arch/arm/mach

$(XRADIO2_OBJS):CFLAGS += -I $(BASE)/components/common/aw/xradio
$(XRADIO2_OBJS):CFLAGS += -I $(BASE)/components/common/aw/xradio/os
$(XRADIO2_OBJS):CFLAGS += -I $(BASE)/components/common/aw/xradio/os/include
$(XRADIO2_OBJS):CFLAGS += -I $(BASE)/components/common/aw/xradio/include
endif
