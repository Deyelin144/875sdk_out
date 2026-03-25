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
#define TAG "AP-Resample"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "audio_plugin.h"
#include "AudioBase.h"

struct resample_data {
	struct as_pcm_config *src_config;
	struct as_pcm_config *dst_config;

	void *src_buf;
	uint32_t src_buf_size;

	void *resample_buff;
	int carrySize;
	uint64_t curoffset;
	uint64_t step;
	double stepDist;
};

static int resample_ap_init(struct audio_plugin *ap)
{
	struct resample_data *rd;

	if (ap->private_data != NULL)
		return 0;

	rd = as_alloc(sizeof(struct resample_data));
	if (!rd)
		fatal("no memory");

	ap->private_data = rd;

	return 0;
}

static bool resample_ap_update_mode(struct audio_plugin *ap, struct as_pcm_config *src_config, struct as_pcm_config *dst_config)
{
	struct resample_data *rd;
	uint32_t in_rate;
	uint32_t out_rate;

	_debug("");
	if (dst_config->rate == src_config->rate) {
		ap->mode = AP_MODE_BYPASS;
		return 0;
	}

	if (!ap->private_data)
		resample_ap_init(ap);
	rd = ap->private_data;

	rd->src_config = src_config;
	rd->dst_config = dst_config;

	if (rd->src_buf != NULL) {
		as_free(rd->src_buf);
		rd->src_buf = NULL;
	}

	in_rate = rd->src_config->rate;
	out_rate = rd->dst_config->rate;

	int resample_buff_size = 0;
	// add rd->src_config->channels for resample release
	if (rd->src_config->frame_bytes > rd->dst_config->frame_bytes) {
		resample_buff_size = ((rd->src_config->period_frames + rd->src_config->channels) * rd->src_config->frame_bytes);
	} else {
		resample_buff_size = ((rd->src_config->period_frames + rd->src_config->channels) * rd->dst_config->frame_bytes);
	}
	rd->resample_buff = as_alloc(resample_buff_size);
	if (rd->resample_buff == NULL) {
		fatal("no memory resample buff\n");
		return -1;
	}
	memset(rd->resample_buff, 0, resample_buff_size);

	rd->curoffset = 0;
	rd->stepDist = ((double)rd->src_config->rate / (double)rd->dst_config->rate);
#ifdef CONFIG_ARCH_SUN20IW2P1
	// HOSC = 40M, audio_dev = 1920Mhz
	// 24.58333Mhz = 40 * 59 / 2 / 24 / 8  --> for 8,16,32,48K
	// 22.588Mhz = 1920 / 85               --> for 22.05, 44.1K
	double Coefficient = 0;
	if (ap->stream == 0) { //playback
		if (rd->dst_config->rate % 8000 == 0) {
			// Coefficient = (48 / (40 * 59 * 2 / 24 / 8 * 1000 / 128 / 4));
			Coefficient = 0.999702;
		} else {
			// Coefficient = (44.1 / (1920 / 85 * 1000 / 128 / 4));
			Coefficient = 0.9996;
		}
	} else { // record
		if (rd->dst_config->rate % 8000 == 0) {
			// Coefficient = ((40 * 59 * 2 / 24 / 8 * 1000 / 128 / 4) / 48);
			Coefficient = 1.000298;
		} else {
			// Coefficient = ((1920 / 85 * 1000 / 128 / 4) / 44.1);
			Coefficient = 1.0004;
		}
	}
	_debug("stepDist is %f, Coefficient is %f\n", rd->stepDist, Coefficient);
	rd->stepDist *= Coefficient;
#endif
	const uint64_t fixedFraction = (1LL << 32);
	rd->step = ((uint64_t)(rd->stepDist * fixedFraction + 0.5));

	rd->src_buf_size = RESAMPLE_ADJUST(rd->dst_config->period_frames * rd->dst_config->frame_bytes);
	rd->src_buf = as_alloc(rd->src_buf_size);
	if (!rd->src_buf) {
		as_free(rd->resample_buff);
		fatal("no memory");
	}

	ap->mode = AP_MODE_WORK;

	_debug("src buf:%p, size:%u", rd->src_buf, rd->src_buf_size);
	_info("create resample: %u -> %u", in_rate, out_rate);

	return 0;
}

static uint32_t Resample_s32(struct resample_data *rd, const int16_t *input, int16_t *output, uint64_t inputSize)
{
	int channels = rd->dst_config->channels;
	float *re_buff = (float *)rd->resample_buff;
	float *re_beff_new = (re_buff + (rd->carrySize * channels));
	int total_frame = (rd->carrySize + inputSize);
	const float *input_bounds = (re_buff + (total_frame * channels));
	uint32_t act_outputSize = 0;

	const uint64_t fixedFraction = (1ULL << 32);
	const double normFixed = 1.0 / (1ULL << 32);

	memcpy(re_beff_new, input, (inputSize * 4 * channels));

	while (1) {
		// at least two data
		if ((re_buff + (channels * 2)) > input_bounds)
			break;

		double t = (((rd->curoffset) & (fixedFraction - 1)) * normFixed);
		int c = 0;

		for (c = 0; c < channels; c++) {
			int16_t a = re_buff[c];
			int16_t b = re_buff[c + channels];

			double interpolated = (a + ((b - a) * t));
			if (interpolated > 2147483647) {
				interpolated = 2147483647;
			} else if (interpolated < -2147483647) {
				interpolated = -2147483647;
			}

			*output++ = (float)interpolated;
		}

		act_outputSize++;

		rd->curoffset += rd->step;
		re_buff += (rd->curoffset >> 32) * channels;
		rd->curoffset &= (fixedFraction - 1);
	}

	rd->carrySize = ((input_bounds - re_buff) / channels);
	memmove(rd->resample_buff, re_buff, (rd->carrySize * 4 * channels));

	return act_outputSize;
}

static uint32_t Resample_s16(struct resample_data *rd, const int16_t *input, int16_t *output, uint64_t inputSize)
{
	int channels = rd->dst_config->channels;
	int16_t *re_buff = (int16_t *)rd->resample_buff;
	int16_t *re_beff_new = (re_buff + (rd->carrySize * channels));
	int total_frame = (rd->carrySize + inputSize);
	const int16_t *input_bounds = (re_buff + (total_frame * channels));
	uint32_t act_outputSize = 0;

	const uint64_t fixedFraction = (1ULL << 32);
	const double normFixed = 1.0 / (1ULL << 32);

	memcpy(re_beff_new, input, (inputSize * 2 * channels));

	while (1) {
		// at least two data
		if ((re_buff + (channels * 2)) > input_bounds)
			break;

		double t = (((rd->curoffset) & (fixedFraction - 1)) * normFixed);
		int c = 0;

		for (c = 0; c < channels; c++) {
			int16_t a = re_buff[c];
			int16_t b = re_buff[c + channels];

			double interpolated = (a + ((b - a) * t));
			if (interpolated > 32767) {
				interpolated = 32767;
			} else if (interpolated < -32767) {
				interpolated = -32767;
			}

			*output++ = (int16_t)interpolated;
		}

		act_outputSize++;

		rd->curoffset += rd->step;
		re_buff += (rd->curoffset >> 32) * channels;
		rd->curoffset &= (fixedFraction - 1);
	}

	rd->carrySize = ((input_bounds - re_buff) / channels);

	memmove(rd->resample_buff, re_buff, (rd->carrySize * 2 * channels));

	return act_outputSize;
}

static int resample_ap_process(struct audio_plugin *ap, void *in_data, uint32_t in_size, void **out_data, uint32_t *out_size)
{
	struct resample_data *rd = ap->private_data;
	uint32_t src_frames = in_size;
	uint32_t dst_frames;
	int16_t *dst = (int16_t *)rd->src_buf;

	if (!dst)
		fatal("src buf is NULL");

	if (rd->dst_config->format == SND_PCM_FORMAT_S32_LE) {
		dst_frames = Resample_s32(rd, in_data, dst, src_frames);
	} else {
		dst_frames = Resample_s16(rd, in_data, dst, src_frames);
	}

	/*_debug("src_frames:%u, dst_frames:%u", src_frames, dst_frames);*/
	*out_data = dst;
	*out_size = dst_frames;

	return 0;
}

static int resample_ap_release(struct audio_plugin *ap)
{
	struct resample_data *rd = ap->private_data;

	if (!rd)
		return 0;

	if (rd->resample_buff) {
		as_free(rd->resample_buff);
		rd->resample_buff = NULL;
	}

	if (rd->src_buf) {
		as_free(rd->src_buf);
		rd->src_buf = NULL;
	}

	as_free(rd);

	return 0;
}

/*
 * 运行resample插件,保证format,channels一致，且以slave_pcm的为准
 * 注意不要使用src_pcm的frame_bytes, 如果之前执行过bitsconv或者chmap，那么src_pcm的frame_bytes会不一致
 */
const struct audio_plugin resample_ap = {
	.ap_name = "resample",
	.ap_init = resample_ap_init,
	.ap_process = resample_ap_process,
	.ap_release = resample_ap_release,
	.ap_update_mode = resample_ap_update_mode,
	.mode = AP_MODE_BYPASS,
};
