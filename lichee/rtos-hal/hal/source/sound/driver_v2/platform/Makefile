# Common dummy dai
obj-$(CONFIG_SUNXI_SOUND_PLATFORM_CPUDAI) += sunxi-aaudio.o

# Common dmaengine pcm
obj-$(CONFIG_SUNXI_SOUND_PLATFORM_PCM) += sunxi-pcm.o

# Common Platform Daudio
obj-$(CONFIG_SUNXI_SOUND_PLATFORM_I2S) += sunxi-i2s.o

# Common Platform DMIC
obj-$(CONFIG_SUNXI_SOUND_PLATFORM_DMIC) += sunxi-dmic.o

# Common Platform OWA
obj-$(CONFIG_SUNXI_SOUND_PLATFORM_OWA) += sunxi-owa.o

# Common Platform MAD
obj-$(CONFIG_SUNXI_SOUND_PLATFORM_MAD) += sunxi-mad.o
ifneq (, $(filter y m, $(CONFIG_ARCH_SUN20IW2)))
obj-$(CONFIG_SUNXI_SOUND_PLATFORM_MAD) += platforms/mad-sun20iw2.o
endif

# Sun8iw19p1 Daudio
obj-$(CONFIG_SUNXI_SOUND_PLATFORM_SUN8IW19_I2S) += sun8iw19-i2s.o

obj-$(CONFIG_SUNXI_SOUND_PLATFORM_MACH) += sunxi_machine.o

CFLAGS += -I drivers/rtos-hal/hal/source/sound/driver_v2/common
CFLAGS += -I drivers/rtos-hal/hal/source/sound/driver_v2/platform