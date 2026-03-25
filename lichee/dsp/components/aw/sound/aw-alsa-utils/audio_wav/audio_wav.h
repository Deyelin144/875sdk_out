/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the People's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY'S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS'SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY'S TECHNOLOGY.
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

#ifndef _AUDIO_WAV_H
#define _AUDIO_WAV_H
#include <aw_common.h>

#ifdef CONFIG_COMPONENTS_AW_ALSA_UTILS_BUILTIN_WAV_DSP
/*
 * boot_package will larger than 2M if define ALL_WAV_FILE
 */
//#define ALL_WAV_FILE

extern const unsigned char music_16K_16bit_1ch_start;
extern const unsigned char music_16K_16bit_1ch_end;
#ifdef ALL_WAV_FILE
extern const unsigned char music_8K_16bit_2ch_start;
extern const unsigned char music_8K_16bit_2ch_end;
extern const unsigned char music_16K_16bit_2ch_start;
extern const unsigned char music_16K_16bit_2ch_end;
extern const unsigned char music_16K_24bit_2ch_start;
extern const unsigned char music_16K_24bit_2ch_end;
extern const unsigned char music_44K_16bit_2ch_start;
extern const unsigned char music_44K_16bit_2ch_end;
extern const unsigned char music_48K_16bit_2ch_start;
extern const unsigned char music_48K_16bit_2ch_end;
#endif
#endif


typedef struct {
	const char *name;
	const unsigned char *start;
	const unsigned char *end;
} wav_file_t;

#define WAV_FILE_LABEL(constant) \
{ \
	.name	= #constant, \
	.start	= &music_##constant##_start, \
	.end	= &music_##constant##_end, \
}

static wav_file_t wav_file_array[] = {
#ifdef CONFIG_COMPONENTS_AW_ALSA_UTILS_BUILTIN_WAV_DSP
	WAV_FILE_LABEL(16K_16bit_1ch),
#ifdef ALL_WAV_FILE
	WAV_FILE_LABEL(8K_16bit_2ch),
	WAV_FILE_LABEL(16K_16bit_2ch),
	WAV_FILE_LABEL(16K_24bit_2ch),
	WAV_FILE_LABEL(44K_16bit_2ch),
	WAV_FILE_LABEL(48K_16bit_2ch),
#endif
#endif
};

static inline void wav_file_list(void)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(wav_file_array); i++)
		printf("%s\n", wav_file_array[i].name);
}

static inline wav_file_t *find_builtin_wav_file(const char *name)
{
	int i;
#if 0
	wav_hw_params_t params;
	for (i = 0; i < ARRAY_SIZE(wav_file_array); i++) {
		printf("name:%s, start:%p, end:%p\n",
			wav_file_array[i].name,
			wav_file_array[i].start,
			wav_file_array[i].end);
		check_wav_header((wav_header_t *)wav_file_array[i].start, &params);
	}
#endif
	if (!name)
		return &wav_file_array[0];
	for (i = 0; i < ARRAY_SIZE(wav_file_array); i++) {
		if (!strcmp(wav_file_array[i].name, name)) {
			return &wav_file_array[i];
		}
	}
	return NULL;
}

#endif
