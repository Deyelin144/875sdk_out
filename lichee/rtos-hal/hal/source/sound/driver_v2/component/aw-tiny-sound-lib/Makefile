obj-y += sunxi_tiny_sound_mixer.o
obj-y += sunxi_tiny_sound_pcm.o

ifneq ($(CONFIG_ARCH_DSP), y)
ifeq ($(CONFIG_COMPONENTS_AW_TINY_SOUND_LIB_UTILS), y)
obj-y += utils/tt-aw-tiny-sound-lib.o
endif
endif

ifeq ($(CONFIG_KERNEL_FREERTOS), y)
CFLAGS += -I $(BASE)/include/
CFLAGS += -I $(BASE)/components/thirdparty/console/
CFLAGS += -I $(BASE)/drivers/rtos-hal/include/hal/sound_v2/
endif
