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


#ifndef SUNXI_TINY_SOUND_PCM_H
#define SUNXI_TINY_SOUND_PCM_H

#include <stddef.h>
#include <sys/types.h>
#include <sunxi_hal_sound.h>
#include <sound_v2/sunxi_sound_pcm_common.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define AWTINYSND_LOG_COLOR_NONE		"\e[0m"
#define AWTINYSND_LOG_COLOR_RED		"\e[31m"
#define AWTINYSND_LOG_COLOR_GREEN	"\e[32m"
#define AWTINYSND_LOG_COLOR_YELLOW		"\e[33m"
#define AWTINYSND_LOG_COLOR_BLUE		"\e[34m"

#ifdef AWTINYSND_DEBUG
#define awtinysnd_debug(fmt, args...) \
	printf(AWTINYSND_LOG_COLOR_GREEN "[AWTINYSND_DEBUG][%s:%d]" fmt \
		AWTINYSND_LOG_COLOR_NONE, __FUNCTION__, __LINE__, ##args)
#else
#define awtinysnd_debug(fmt, args...)
#endif

#if 0
#define awtinysnd_info(fmt, args...) \
	printf(AWTINYSND_LOG_COLOR_BLUE "[AWTINYSND_INFO][%s:%d]" fmt \
		AWTINYSND_LOG_COLOR_NONE, __FUNCTION__, __LINE__, ##args)
#else
#define awtinysnd_info(fmt, args...)
#endif

#define awtinysnd_err(fmt, args...) \
	printf(AWTINYSND_LOG_COLOR_RED "[AWTINYSND_ERR][%s:%d]" fmt \
		AWTINYSND_LOG_COLOR_NONE, __FUNCTION__, __LINE__, ##args)

size_t sunxi_sound_pcm_hw_params_sizeof(void);
size_t sunxi_sound_pcm_sw_params_sizeof(void);

#define __sound_alloca(ptr,type) do { *ptr = (type##_t *) alloca(type##_sizeof()); memset(*ptr, 0, type##_sizeof()); } while (0)
#define sunxi_sound_pcm_hw_params_alloca(ptr) __sound_alloca(ptr, sunxi_sound_pcm_hw_params)

#define sunxi_sound_pcm_sw_params_alloca(ptr) __sound_alloca(ptr, sunxi_sound_pcm_sw_params)


struct sunxi_pcm_config {
    snd_pcm_access_t access;
    unsigned int channels;
    unsigned int rate;
    unsigned int period_time;
    snd_pcm_uframes_t period_size;
    unsigned int period_count;
    snd_pcm_format_t format;
    unsigned int avail_min;
    unsigned int start_threshold;
    unsigned int stop_threshold;
    unsigned int silence_threshold;
};


struct sunxi_pcm;

int sunxi_pcm_open(struct sunxi_pcm **pcmp, unsigned int card, unsigned int device,
                     snd_pcm_stream_t stream, int mode);

int sunxi_pcm_close(struct sunxi_pcm *pcm);

int sunxi_pcm_set_config(struct sunxi_pcm *pcm, const struct sunxi_pcm_config *config);

ssize_t sunxi_sound_pcm_format_size(snd_pcm_format_t format, size_t samples);

int sunxi_sound_pcm_hw_params_pause_able(const sunxi_sound_pcm_hw_params_t *params);

ssize_t sunxi_sound_pcm_frames_to_bytes(struct sunxi_pcm *pcm, snd_pcm_sframes_t frames);

snd_pcm_sframes_t sunxi_sound_pcm_bytes_to_frames(struct sunxi_pcm *pcm, ssize_t bytes);

snd_pcm_sframes_t sunxi_pcm_writei(struct sunxi_pcm *pcm, const void *buffer, snd_pcm_uframes_t size);

snd_pcm_sframes_t sunxi_pcm_readi(struct sunxi_pcm *pcm, void *buffer, snd_pcm_uframes_t size);

int sunxi_pcm_prepare(struct sunxi_pcm *pcm);

int sunxi_pcm_reset(struct sunxi_pcm *pcm);

int sunxi_pcm_start(struct sunxi_pcm *pcm);

int sunxi_pcm_stop(struct sunxi_pcm *pcm);

int sunxi_pcm_drain(struct sunxi_pcm *pcm);

int sunxi_pcm_pause(struct sunxi_pcm *pcm, int enable);

int sunxi_pcm_state(struct sunxi_pcm *pcm);

int sunxi_pcm_hwsync(struct sunxi_pcm *pcm);

int sunxi_pcm_recover(struct sunxi_pcm *pcm, int err, int silent);

int sunxi_pcm_delay(struct sunxi_pcm *pcm, snd_pcm_sframes_t *delayp);

int sunxi_pcm_dump_setup(struct sunxi_pcm *pcm);

snd_pcm_stream_t sunxi_pcm_stream(struct sunxi_pcm *pcm);

int sunxi_sound_pcm_hw_params_any(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_hw_params *params);

int sunxi_sound_pcm_hw_params_set_access(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_hw_params *params, snd_pcm_access_t access);
int sunxi_sound_pcm_hw_params_set_format(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_hw_params *params, snd_pcm_format_t format);
int sunxi_sound_pcm_hw_params_set_channels(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_hw_params *params, unsigned int val);
int sunxi_sound_pcm_hw_params_set_rate(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_hw_params *params, unsigned int val, int dir);
int sunxi_sound_pcm_hw_params_set_period_size(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_hw_params *params, snd_pcm_uframes_t val, int dir);
int sunxi_sound_pcm_hw_params_set_periods(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_hw_params *params, unsigned int val, int dir);
int sunxi_sound_pcm_hw_params_set_period_time(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_hw_params *params, unsigned int us, int dir);
int sunxi_sound_pcm_hw_params_set_buffer_time(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_hw_params *params, unsigned int us);
int sunxi_sound_pcm_hw_params_set_buffer_size(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_hw_params *params, snd_pcm_uframes_t val);

int sunxi_sound_pcm_set_hw_params(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_hw_params *params);

int sunxi_sound_pcm_hw_params_get_format(const struct sunxi_sound_pcm_hw_params *params, snd_pcm_format_t *format);
int sunxi_sound_pcm_hw_params_get_channels(const struct sunxi_sound_pcm_hw_params *params, unsigned int *val);
int sunxi_sound_pcm_hw_params_get_rate(const struct sunxi_sound_pcm_hw_params *params, unsigned int *val, int *dir);
int sunxi_sound_pcm_hw_params_get_period_time(const struct sunxi_sound_pcm_hw_params *params, unsigned int *val, int *dir);
int sunxi_sound_pcm_hw_params_get_buffer_time(const struct sunxi_sound_pcm_hw_params *params, snd_pcm_uframes_t *val, int *dir);
int sunxi_sound_pcm_hw_params_get_period_size(const struct sunxi_sound_pcm_hw_params *params, snd_pcm_uframes_t *val, int *dir);
int sunxi_sound_pcm_hw_params_get_periods(const struct sunxi_sound_pcm_hw_params *params, unsigned int *val, int *dir);
int sunxi_sound_pcm_hw_params_get_buffer_size(const struct sunxi_sound_pcm_hw_params *params, snd_pcm_uframes_t *val);


int sunxi_sound_pcm_sw_params_current(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_sw_params *params);

int sunxi_sound_pcm_sw_params_set_start_threshold(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_sw_params *params, snd_pcm_uframes_t val);
int sunxi_sound_pcm_sw_params_get_start_threshold(struct sunxi_sound_pcm_sw_params *params, snd_pcm_uframes_t *val);

int sunxi_sound_pcm_sw_params_set_stop_threshold(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_sw_params *params, snd_pcm_uframes_t val);
int sunxi_sound_pcm_sw_params_get_stop_threshold(struct sunxi_sound_pcm_sw_params *params, snd_pcm_uframes_t *val);

int sunxi_sound_pcm_sw_params_set_silence_size(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_sw_params *params, snd_pcm_uframes_t val);

int sunxi_sound_pcm_sw_params_set_avail_min(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_sw_params *params, snd_pcm_uframes_t val);
int sunxi_sound_pcm_sw_params_get_avail_min(const struct sunxi_sound_pcm_sw_params *params, snd_pcm_uframes_t *val);

int sunxi_sound_pcm_sw_params_get_boundary(const struct sunxi_sound_pcm_sw_params *params, snd_pcm_uframes_t *val);

int sunxi_sound_pcm_set_sw_params(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_sw_params *params);

#if defined(__cplusplus)
}  /* extern "C" */
#endif

#endif

