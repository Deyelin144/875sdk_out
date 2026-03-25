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

#ifndef __COMMON_H
#define __COMMON_H

#include <portmacro.h>
#include <FreeRTOS-Plus-CLI/FreeRTOS_CLI.h>
#ifdef CONFIG_COMPONENTS_FREERTOS_CLI
#include <console.h>
#endif
#include <aw_common.h>
#include <aw_list.h>

#if __cplusplus
extern "C" {
#endif
typedef struct {
	snd_pcm_t *handle;
	snd_pcm_format_t format;
	unsigned int rate;
	unsigned int channels;
	snd_pcm_uframes_t period_size;
	snd_pcm_uframes_t buffer_size;

	snd_pcm_uframes_t frame_bytes;
	snd_pcm_uframes_t chunk_size;

	unsigned in_aborting;
	unsigned int capture_duration;
} audio_mgr_t;

void xrun(snd_pcm_t *handle);
void do_other_test(snd_pcm_t *handle);
void do_pause(snd_pcm_t *handle);

int pcm_read(snd_pcm_t *handle, const char *data, snd_pcm_uframes_t frames_total,
		unsigned int frame_bytes);
int arecord_data(const char *card_name, snd_pcm_format_t format, unsigned int rate,
		unsigned int channels, const void *data, unsigned int datalen);

int pcm_write(snd_pcm_t *handle, char *data, snd_pcm_uframes_t frames_total,
		unsigned int frame_bytes);
int aplay_data(const char *card_name, snd_pcm_format_t format, unsigned int rate,
		unsigned int channels, const char *data, unsigned int datalen,
		unsigned int loop_count);

int set_param(snd_pcm_t *handle, snd_pcm_format_t format,
		unsigned int rate, unsigned int channels,
		snd_pcm_uframes_t period_size, snd_pcm_uframes_t buffer_size);

audio_mgr_t *audio_mgr_create(void);
void audio_mgr_dump_args(audio_mgr_t *audio_mgr);
void audio_mgr_release(audio_mgr_t *mgr);

int amixer_sset_enum_ctl(const char *card_name, const char *ctl_name,
			const char *ctl_val);

void FUNCTION_THREAD_STOP_LINE_PRINTF(const char *string);
void FUNCTION_THREAD_LINE_PRINTF(const char *string, const unsigned int line);
void FUNCTION_THREAD_START_LINE_PRINTF(const char *string);

#if __cplusplus
};  // extern "C"
#endif
#endif /* __COMMON_H */
