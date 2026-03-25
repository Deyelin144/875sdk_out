CUR_MAKEFILE_DIR := $(patsubst %/, %, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
CUR_SRC_FILES := $(notdir $(patsubst %.c, %.o, $(wildcard $(CUR_MAKEFILE_DIR)/*.c)))
obj-y += $(CUR_SRC_FILES)

CFLAGS += -I $(CUR_MAKEFILE_DIR)/
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_c906/src/include
CFLAGS += -I $(BASE)/projects/xr875s1h10/solution_c906/src/include/platform

# RTPLAYER_INCLUDE_DIR := components/aw/multimedia/include

# CFLAGS += -I $(RTPLAYER_INCLUDE_DIR)
# CFLAGS += -I $(RTPLAYER_INCLUDE_DIR)/include/cedarx/xplayer/include/
# CFLAGS += -I $(RTPLAYER_INCLUDE_DIR)/libcore/base/include/
# CFLAGS += -I $(RTPLAYER_INCLUDE_DIR)/libcore/playback/include/
# CFLAGS += -I $(RTPLAYER_INCLUDE_DIR)/libcore/parser/include/
# CFLAGS += -I $(RTPLAYER_INCLUDE_DIR)/libcore/stream/include/
# CFLAGS += -I $(RTPLAYER_INCLUDE_DIR)/Cdx2.0Plugin/include/

