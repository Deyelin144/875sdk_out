CUR_MAKEFILE_DIR := $(patsubst %/, %, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
CUR_SRC_FILES := $(notdir $(patsubst %.c, %.o, $(wildcard $(CUR_MAKEFILE_DIR)/*.c)))

# 添加源文件
obj-y += $(CUR_SRC_FILES)
obj-y += mfbd/

# 添加头文件

CFLAGS += -I $(CUR_MAKEFILE_DIR)/
CFLAGS += -I $(CUR_MAKEFILE_DIR)/$(CUR_SUBDIR)
CFLAGS += -I $(BASE)/components/common/aw/xradio/include
