/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the People's Republic of China and other countries.
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
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <aw-alsa-lib/pcm.h>
#include <math.h>
#include "common.h"

#include <tinatest.h>
#define PLAYBACK_TIME_SECS (1)
#define LOOP_COUNTS		CONFIG_PLAYBACK_LOOP_COUNTS

#define PI (3.1415926)
static int playback_sine_generate(void **buf, uint32_t *len, uint32_t rate, uint32_t channels, uint8_t bits)
{
	int16_t *data_16;
	int32_t *data_32;
	int sine_hz = 1000;
	int sine_point;
	int accuracy;
	int i, j;

	sine_point = rate / sine_hz;

	if (bits == 16) {
		data_16 = malloc(sine_point * sizeof(int16_t) * channels);
		if(!data_16){
                   return -1;
		}
		accuracy = INT16_MAX;
		for (i = 0; i < sine_point; i++) {
			int16_t value = (int16_t)(accuracy * sin(2 * (double)PI * i / sine_point));
			for (j = 0; j < channels; j++)
				data_16[(i * channels) + j] = value;
		}

		*buf = data_16;
		*len = sine_point * sizeof(int16_t) * channels;
	} else if (bits == 32) {
		data_32 = malloc(sine_point * sizeof(int32_t) * channels);
		if(!data_32)
		{
                   return -1;
		}
		accuracy = INT32_MAX;
		for (i = 0; i < sine_point; i++) {
			int32_t value = (int32_t)(accuracy * sin(2 * (double)PI * i / sine_point));
			for (j = 0; j < channels; j++)
				data_32[(i * channels) + j] = value;
		}

		*buf = data_32;
		*len = sine_point * sizeof(int32_t) * channels;
	} else {
		*buf = NULL;
		*len = 0;
		printf("unsupport bits:%u\n", bits);
		return -1;
	}

	return 0;
}

static int audio_playback_loop(int argc, char **argv)
{
	int loop_count = 10;
	char *card = "hw:audiocodecdac";
	snd_pcm_t *pcm;
	int ret;
	char string[128];
	void *sine_buf = NULL;
	uint32_t sine_buf_len = 0;
	int frame_write;
	int frame_size;
	unsigned int total_frames = 0;
	unsigned int stop_frames = 0;

	loop_count = strtoul(LOOP_COUNTS, NULL, 0);
	printf("test loop_count:%d\n", loop_count);

	if (loop_count < 0) {
		printf("loop_count error :%d\n", loop_count);
		return -1;
	}

	snprintf(string, sizeof(string),
		"playback stream playback loop test start(count %d).\n",
		loop_count);
	ttips(string);

	ret = snd_pcm_open(&pcm, card, SND_PCM_STREAM_PLAYBACK, 0);
	if (ret < 0) {
		printf("audio open error:%d\n", ret);
		goto err_pcm_open_pcm;
	}

	ret = set_param(pcm, SND_PCM_FORMAT_S16_LE, 16000, 1, 960, 3840);
	if (ret < 0) {
		printf("audio set pcm param error:%d\n", ret);
		goto err_set_param_pcm;
	}

	ret = playback_sine_generate(&sine_buf, &sine_buf_len, 16000, 1, 16);
	if (ret < 0) {
		printf("playback_sine_generate failed\n");
		goto err_sine_generate;
	}

	frame_size = snd_pcm_frames_to_bytes(pcm, 1);
	frame_write = snd_pcm_bytes_to_frames(pcm, sine_buf_len);
	stop_frames = 16000 * PLAYBACK_TIME_SECS;

	while (loop_count--) {
		ret = snd_pcm_prepare(pcm);
		if (ret < 0) {
			printf("prepare failed. return %d\n", ret);
			break;
		}
		while (1) {
			ret = pcm_write(pcm, sine_buf, frame_write, frame_size);
			if (ret < 0) {
				printf("pcm_write error:%d\n", ret);
				break;
			}
			total_frames += frame_write;
			if (total_frames >= stop_frames)
				break;
		}
		snd_pcm_drain(pcm);
		total_frames = 0;
		usleep(1000*1000);
		printf("remain count %d\n", loop_count);
	}

	if (pcm != NULL)
		snd_pcm_close(pcm);
	if (sine_buf)
		free(sine_buf);

	ttips("playback loop test finish.\n");

	return 0;

err_sine_generate:
err_set_param_pcm:
	if (pcm != NULL)
		snd_pcm_close(pcm);
err_pcm_open_pcm:

	return ret;
}

testcase_init(audio_playback_loop, playbackloop, playback loop for tinatest);
