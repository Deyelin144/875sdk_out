CUR_MAKEFILE_DIR := $(patsubst %/, %, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
CUR_SRC_FILES := $(notdir $(patsubst %.c, %.o, $(wildcard $(CUR_MAKEFILE_DIR)/*.c)))
obj-y += $(CUR_SRC_FILES)

CFLAGS += -I $(CUR_MAKEFILE_DIR)/