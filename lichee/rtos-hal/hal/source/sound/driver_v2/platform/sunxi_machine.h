/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the people's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef __SUNXI_MACHINE_H
#define __SUNXI_MACHINE_H

#include <sound_v2/sunxi_adf_core.h>

#if defined(CONFIG_SUNXI_SOUND_PLATFORM_CPUDAI) && !defined(CONFIG_SUNXI_SOUND_CODEC_AUDIOCODEC_DAC) && !defined(CONFIG_SUNXI_SOUND_CODEC_AUDIOCODEC_ADC)

int sunxi_codec_component_probe();

void sunxi_codec_component_remove();

#else
int sunxi_codec_component_probe()
{
	return 0;
}

void sunxi_codec_component_remove()
{
	return;
}
#endif


#if defined(CONFIG_SUNXI_SOUND_CODEC_AUDIOCODEC_DAC)

int sunxi_codec_dac_component_probe();

void sunxi_codec_dac_component_remove();

#else
int sunxi_codec_dac_component_probe()
{
	return 0;
}

void sunxi_codec_dac_component_remove()
{
	return;
}

#endif

#if defined(CONFIG_SUNXI_SOUND_CODEC_AUDIOCODEC_ADC)

int sunxi_codec_adc_component_probe();

void sunxi_codec_adc_component_remove();

#else
int sunxi_codec_adc_component_probe()
{
	return 0;
}

void sunxi_codec_adc_component_remove()
{
	return;
}

#endif


#if defined(CONFIG_SUNXI_SOUND_CODEC_DUMMY)

int sunxi_dummy_codec_component_probe();
void sunxi_dummy_codec_component_remove();

#else
int sunxi_dummy_codec_component_probe()
{
	return 0;
}

void sunxi_dummy_codec_component_remove()
{
	return;
}
#endif

#if defined(CONFIG_SUNXI_SOUND_PLATFORM_CPUDAI)

int sunxi_aaudio_component_probe(enum snd_platform_type plat_type);

void sunxi_aaudio_component_remove(enum snd_platform_type plat_type);

#else
int sunxi_aaudio_component_probe(enum snd_platform_type plat_type)
{
	return 0;
}

void sunxi_aaudio_component_remove(enum snd_platform_type plat_type)
{
	return;
}
#endif

#if defined(CONFIG_SUNXI_SOUND_PLATFORM_DMIC)

int sunxi_dmic_component_probe();

void sunxi_dmic_component_remove();

#else
int sunxi_dmic_component_probe()
{
	return 0;
}

void sunxi_dmic_component_remove()
{
	return;
}
#endif

#if defined(CONFIG_SUNXI_SOUND_PLATFORM_OWA)

int sunxi_owa_component_probe();

void sunxi_owa_component_remove();

#else
int sunxi_owa_component_probe()
{
	return 0;
}

void sunxi_owa_component_remove()
{
	return;
}
#endif


#if defined(CONFIG_SUNXI_SOUND_PLATFORM_I2S)

int sunxi_i2s_component_probe(enum snd_platform_type plat_type);

void sunxi_i2s_component_remove(enum snd_platform_type plat_type);

#else
int sunxi_i2s_component_probe(enum snd_platform_type plat_type)
{
	return 0;
}

void sunxi_i2s_component_remove(enum snd_platform_type plat_type)
{
	return;
}
#endif

#if defined(CONFIG_SUNXI_SOUND_CODEC_AC101S)

int sunxi_ac101s_component_probe();

void sunxi_ac101s_component_remove();

#endif

#if defined(CONFIG_SUNXI_SOUND_CODEC_AC108)

int sunxi_ac108_component_probe();

void sunxi_ac108_component_remove();

#endif

#endif /* __SUNXI_MACHINE_H */
