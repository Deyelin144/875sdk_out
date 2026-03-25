SUN20IW2_BASE := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
BT_DRV_AMP_DIRS += $(shell find $(SUN20IW2_BASE)/hal -type d)
BT_DRV_SER_DIRS += $(shell find $(SUN20IW2_BASE)/pal -type d)

ifeq ($(CONFIG_KASAN), y)
CFLAGS_REMOVE_pal_sys_pm.o += $(CFLAGS_KASAN)
endif
