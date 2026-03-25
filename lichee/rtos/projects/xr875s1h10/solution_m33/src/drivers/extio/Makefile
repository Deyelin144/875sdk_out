ifeq ($(CONFIG_DSP_USE_EXTIO_DCDC_EN_IO),y)
CUR_MAKEFILE_DIR := $(patsubst %/, %, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
CUR_SRC_FILES := $(notdir $(patsubst %.c, %.o, $(wildcard $(CUR_MAKEFILE_DIR)/*.c)))

# 添加源文件
obj-y += $(CUR_SRC_FILES)

# 添加头文件
CFLAGS += -I $(CUR_MAKEFILE_DIR)/

CFLAGS += -I $(BASE)/lichee/rtos-hal/include/hal
endif