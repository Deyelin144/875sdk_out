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
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
//#include <FreeRTOS.h>
#include <sunxi_hal_sound.h>
#include "common.h"
#include <hal_time.h>
#include <hal_timer.h>
#include <aw-tiny-sound-lib/sunxi_tiny_sound_pcm.h>

unsigned int g_aw_verbose;

void sunxi_pcm_xrun(struct sunxi_pcm *handle)
{
	int ret;

	printf("Xrun...\n");
	ret = sunxi_pcm_prepare(handle);
	if (ret < 0) {
		printf("prepare failed in xrun. return %d\n", ret);
	}
}

void sunxi_do_pause(struct sunxi_pcm *handle)
{
	int ret = 0;
	printf("[%s] line:%d pause start...\n", __func__, __LINE__);
	ret = sunxi_pcm_pause(handle, 1);
	if (ret < 0)
		printf("pause failed!, return %d\n", ret);
	hal_sleep(5);
	ret = sunxi_pcm_pause(handle, 0);
	if (ret < 0)
		printf("pause release failed!, return %d\n", ret);
	printf("[%s] line:%d pause end...\n", __func__, __LINE__);
}

void sunxi_do_other_test(struct sunxi_pcm *handle)
{
	sunxi_do_pause(handle);
	return;
}

sound_mgr_t *sunxi_sound_mgr_create(void)
{
	sound_mgr_t *sound_mgr = NULL;

	sound_mgr = malloc(sizeof(sound_mgr_t));
	if (!sound_mgr) {
		printf("no memory\n");
		return NULL;
	}
	memset(sound_mgr, 0, sizeof(sound_mgr_t));
	sound_mgr->card = 0;
	sound_mgr->device = 0;
	sound_mgr->format = SND_PCM_FORMAT_S16_LE;
	sound_mgr->rate = 16000;
	sound_mgr->channels = 2;
	sound_mgr->period_size = 1024;
	sound_mgr->buffer_size = 4096;
	return sound_mgr;
}

void sunxi_sound_mgr_release(sound_mgr_t *mgr)
{
	if (!mgr) {
		printf("%s: mgr null !\n", __func__);
		return;
	}
	free(mgr);
}

int sunxi_set_param(struct sunxi_pcm *handle, snd_pcm_format_t format,
			unsigned int rate, unsigned int channels,
			snd_pcm_uframes_t period_size,
			snd_pcm_uframes_t buffer_size)
{
	int ret = 0;
	sunxi_sound_pcm_sw_params_t *sw_params = NULL;
	sunxi_sound_pcm_hw_params_t *params = NULL;
	snd_pcm_uframes_t period_size_tmp = period_size;
	snd_pcm_uframes_t buffer_size_tmp = buffer_size;

	/* HW params */
	sunxi_sound_pcm_hw_params_alloca(&params);
	ret = sunxi_sound_pcm_hw_params_any(handle, params);
	if (ret < 0) {
		printf("no configurations available\n");
		return ret;
	}
	ret = sunxi_sound_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (ret < 0) {
		printf("failed to set access\n");
		return ret;
	}
	ret = sunxi_sound_pcm_hw_params_set_format(handle, params, format);
	if (ret < 0) {
		printf("failed to set format\n");
		return ret;
	}
	ret = sunxi_sound_pcm_hw_params_set_channels(handle, params, channels);
	if (ret < 0) {
		printf("failed to set channels\n");
		return ret;
	}
	ret = sunxi_sound_pcm_hw_params_set_rate(handle, params, rate, 0);
	if (ret < 0) {
		printf("failed to set rate\n");
		return ret;
	}

	ret = sunxi_sound_pcm_hw_params_set_period_size(handle, params, period_size_tmp, 0);
	if (ret < 0) {
		printf("failed to set period size\n");
		return ret;
	}
	ret = sunxi_sound_pcm_hw_params_set_buffer_size(handle, params, buffer_size_tmp);
	if (ret < 0) {
		printf("failed to set buffer size\n");
		return ret;
	}

	ret = sunxi_sound_pcm_set_hw_params(handle, params);
	if (ret < 0) {
		printf("Unable to install hw prams! (return: %d)\n", ret);
		return ret;
	}

	/* SW params */
	sunxi_sound_pcm_sw_params_alloca(&sw_params);
	sunxi_sound_pcm_sw_params_current(handle, sw_params);
	if (sunxi_pcm_stream(handle) == SND_PCM_STREAM_CAPTURE) {
		sunxi_sound_pcm_sw_params_set_start_threshold(handle, sw_params, 1);
	} else {
		snd_pcm_uframes_t boundary = 0;
		sunxi_sound_pcm_sw_params_get_boundary(sw_params, &boundary);
		sunxi_sound_pcm_sw_params_set_start_threshold(handle, sw_params, buffer_size);
		/* set silence size, in order to fill silence data into ringbuffer */
		sunxi_sound_pcm_sw_params_set_silence_size(handle, sw_params, boundary);
	}
	sunxi_sound_pcm_sw_params_set_stop_threshold(handle, sw_params, buffer_size);
	sunxi_sound_pcm_sw_params_set_avail_min(handle, sw_params, period_size);
	ret = sunxi_sound_pcm_set_sw_params(handle ,sw_params);
	if (ret < 0) {
		printf("Unable to install sw prams!\n");
		return ret;
	}

	if (g_aw_verbose)
		sunxi_pcm_dump_setup(handle);

	return ret;
}

snd_pcm_sframes_t sunxi_pcm_write(struct sunxi_pcm *handle, char *data, snd_pcm_uframes_t frames_total, unsigned int frame_bytes)
{
	snd_pcm_sframes_t size;
	snd_pcm_uframes_t frames_loop = 256;
	snd_pcm_uframes_t frames_count = 0;
	snd_pcm_uframes_t frames = 0;

	while (1) {
		if ((frames_total - frames_count) < frames_loop)
			frames = frames_total - frames_count;
		if (frames == 0)
			frames = frames_loop;
		/*hal_usleep(500000);*/
		size = sunxi_pcm_writei(handle, data, frames);
		if (size != frames) {
			printf("sunxi_pcm_writei return %ld\n", size);
		}
		if (size == -EAGAIN) {
			hal_usleep(10000);
			continue;
		} else if (size == -EPIPE) {
			sunxi_pcm_xrun(handle);
			continue;
		} else if (size == -ESTRPIPE) {

			continue;
		} else if (size < 0) {
			printf("-----snd_pcm_writei failed!!, return %ld\n", size);
			return size;
		}
		data += (size * frame_bytes);
		frames_count += size;
		frames -= size;
		if (frames_total == frames_count)
			break;
	}

	return frames_count;
}

snd_pcm_sframes_t sunxi_pcm_read(struct sunxi_pcm *handle, const char *data, snd_pcm_uframes_t frames_total, unsigned int frame_bytes)
{
	int ret = 0;
	snd_pcm_sframes_t size;
	snd_pcm_uframes_t frames_loop = 256;
	snd_pcm_uframes_t frames_count = 0;
	snd_pcm_uframes_t frames = 0;
	unsigned int offset = 0;

	while (1) {
		if ((frames_total - frames_count) < frames_loop)
			frames = frames_total - frames_count;
		if (frames == 0)
			frames = frames_loop;
		/*printf("snd_pcm_readi %ld frames\n", frames);*/
		size = sunxi_pcm_readi(handle, (void *)(data + offset), frames);
		if (size < 0)
			printf("sunxi_pcm_readi return %ld\n", size);
		if (size == -EAGAIN) {
			/* retry */
			hal_usleep(10000);
			continue;
		} else if (size == -EPIPE) {
			sunxi_pcm_xrun(handle);
			continue;
		} else if (size == -ESTRPIPE) {

			continue;
		} else if (size < 0) {
			printf("-----snd_pcm_readi failed!!, return %ld\n", size);
			ret = (int)size;
			goto err;
		}
		offset += (size * frame_bytes);
		frames_count += size;
		frames -= size;
		if (frames_total == frames_count)
			break;
	}
err:
	return frames_count > 0 ? frames_count : ret;
}
