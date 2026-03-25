# AllWinner Audio Driver Framework
obj-y += adf/

# Audio device driver core
obj-y += core/

# Sunxi Internal Codecs
obj-y += codecs/

# Sunxi Daudio
obj-y += platform/

# Audio lib
obj-y += component/

# Sunxi common
obj-y += common/

# Soundcards Register Select
ifneq ($(CONFIG_KERNEL_FREERTOS), y)
obj-y += card_v2_default.o
endif
ifeq ($(CONFIG_KERNEL_FREERTOS)_$(CONFIG_ARCH_DSP), y_y)
obj-y += card_v2_default.o
endif

ifeq ($(CONFIG_COMPONENTS_PM), y)
CFLAGS += -Icomponents/common/aw/pm/include/
endif
