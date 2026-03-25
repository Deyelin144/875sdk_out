# Sun8iw19p1 Platform Codec
obj-$(CONFIG_SUNXI_SOUND_CODEC_SUN8IW18_AUDIOCODEC) += sun8iw18-codec.o

# Sun8iw19p1 Platform Codec
obj-$(CONFIG_SUNXI_SOUND_CODEC_SUN8IW19_AUDIOCODEC) += sun8iw19-codec.o

# Sun8iw20p1 Platform Codec
obj-$(CONFIG_SUNXI_SOUND_CODEC_SUN8IW20_AUDIOCODEC) += sun8iw20-codec.o

# Sun20iw2p1 Platform Codec
ifeq (, $(CONFIG_SUNXI_SOUND_CODEC_AUDIOCODEC_DAC)$(CONFIG_SUNXI_SOUND_CODEC_AUDIOCODEC_ADC))
obj-$(CONFIG_SUNXI_SOUND_CODEC_SUN20IW2_AUDIOCODEC) += sun20iw2-codec.o
else
ifneq (, $(filter y m, $(CONFIG_SUNXI_SOUND_CODEC_AUDIOCODEC_DAC)))
obj-$(CONFIG_SUNXI_SOUND_CODEC_SUN20IW2_AUDIOCODEC) += sun20iw2-codec-dac.o
endif
ifneq (, $(filter y m, $(CONFIG_SUNXI_SOUND_CODEC_AUDIOCODEC_ADC)))
obj-$(CONFIG_SUNXI_SOUND_CODEC_SUN20IW2_AUDIOCODEC) += sun20iw2-codec-adc.o
endif
endif

# Common dummy_codec
obj-$(CONFIG_SUNXI_SOUND_CODEC_DUMMY) += dummy_codec.o

# AC108 Codec
obj-$(CONFIG_SUNXI_SOUND_CODEC_AC108) += ac108.o
# AC101s Codec
obj-$(CONFIG_SUNXI_SOUND_CODEC_AC101S) += ac101s.o

CFLAGS += -I drivers/rtos-hal/hal/source/sound/driver_v2/common
CFLAGS += -I drivers/rtos-hal/hal/source/sound/driver_v2/platform