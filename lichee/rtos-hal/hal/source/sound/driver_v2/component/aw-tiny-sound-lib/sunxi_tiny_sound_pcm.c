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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <aw_common.h>
#include <hal_mutex.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <assert.h>

#include <sound_v2/sunxi_sound_pcm_misc.h>
#include <aw-tiny-sound-lib/sunxi_tiny_sound_pcm.h>

typedef hal_mutex_t sunxi_sound_pcm_mutex_t;
static int sunxi_sound_pcm_sw_params_default(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_sw_params *params);

struct sunxi_pcm {
    sound_device_t *pcm_dev;
    snd_pcm_stream_t stream;
    int setup:1;
    int running:1;
    int prepared:1;
    int underruns;
    unsigned int sample_bits;
    unsigned int frame_bits;
    snd_pcm_uframes_t buffer_size;
    snd_pcm_uframes_t boundary;
    struct sunxi_pcm_config config;
    unsigned int subdevice;
    sunxi_sound_pcm_mutex_t mutex;
};

sunxi_sound_pcm_mutex_t sunxi_sound_mutex_init(void)
{
	return hal_mutex_create();
}

int sunxi_sound_mutex_lock(sunxi_sound_pcm_mutex_t mutex)
{
	return hal_mutex_lock(mutex);
}

int sunxi_sound_mutex_unlock(sunxi_sound_pcm_mutex_t mutex)
{
	return hal_mutex_unlock(mutex);
}

void sunxi_sound_mutex_destroy(sunxi_sound_pcm_mutex_t mutex)
{
	hal_mutex_delete(mutex);
}

static inline void sunxi_sound_pcm_lock(struct sunxi_pcm *pcm)
{
	sunxi_sound_mutex_lock(pcm->mutex);
}

static inline void sunxi_sound_pcm_unlock(struct sunxi_pcm *pcm)
{
	sunxi_sound_mutex_unlock(pcm->mutex);
}

size_t sunxi_sound_pcm_hw_params_sizeof(void)
{
    return sizeof(sunxi_sound_pcm_hw_params_t);
}

size_t sunxi_sound_pcm_sw_params_sizeof(void)
{
    return sizeof(sunxi_sound_pcm_sw_params_t);
}

int sunxi_sound_pcm_hw_params_any(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_hw_params *params)
{
    return sunxi_sound_device_control(pcm->pcm_dev, SUNXI_SOUND_CTRL_PCM_HW_REFINE, (unsigned long)params);
}

static inline int sunxi_sound_pcm_hw_param_change(
        struct sunxi_sound_pcm_hw_params *params, snd_pcm_hw_param_t var)
{
    params->cmask |= (1 << var);
    return 0;
}

static inline int sunxi_sound_pcm_hw_param_changed_able(
        struct sunxi_sound_pcm_hw_params *params, snd_pcm_hw_param_t var)
{
    return params->cmask & (1 << var);
}

int _sunxi_sound_pcm_hw_param_set(struct sunxi_sound_pcm_hw_params *params, snd_pcm_hw_param_t var, unsigned int val)
{
    union snd_interval *interval = NULL;

    assert(params);
    interval = &params->intervals[var - SND_PCM_HW_PARAM_FIRST_INTERVAL];
    if (var <= SND_PCM_HW_PARAM_LAST_MASK) {
        if (!(interval->mask & (1 << val)) &&
                !sunxi_sound_pcm_hw_param_changed_able(params, var))
        goto err_out;
        interval->mask = (1<<val);
    } else {
        if ((val < interval->range.min || val > interval->range.max) &&
                !sunxi_sound_pcm_hw_param_changed_able(params, var))
        goto err_out;
    	interval->range.min = val;
    }
    return 0;

err_out:
    awtinysnd_err("invalid value (%u) for interval %d\n", val, var);
    return -EINVAL;
}

int sunxi_sound_pcm_hw_params_set_access(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_hw_params *params, snd_pcm_access_t access)
{
    awtinysnd_debug("\n");
    sunxi_sound_pcm_hw_param_change(params, SND_PCM_HW_PARAM_ACCESS);
    return _sunxi_sound_pcm_hw_param_set(params, SND_PCM_HW_PARAM_ACCESS, access);
}

int sunxi_sound_pcm_hw_params_set_format(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_hw_params *params, snd_pcm_format_t format)
{
    int ret;
    awtinysnd_debug("\n");
    sunxi_sound_pcm_hw_param_change(params, SND_PCM_HW_PARAM_FORMAT);
    ret = _sunxi_sound_pcm_hw_param_set(params, SND_PCM_HW_PARAM_FORMAT, format);
    if (ret < 0)
    return ret;

    /* set sample bits */
    sunxi_sound_pcm_hw_param_change(params, SND_PCM_HW_PARAM_SAMPLE_BITS);
    ret = _sunxi_sound_pcm_hw_param_set(params, SND_PCM_HW_PARAM_SAMPLE_BITS, sunxi_sound_pcm_format_physical_width(format));
    if (ret < 0)
    return ret;

    return 0;
}

int sunxi_sound_pcm_hw_params_set_channels(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_hw_params *params, unsigned int val)
{
    awtinysnd_debug("\n");
    sunxi_sound_pcm_hw_param_change(params, SND_PCM_HW_PARAM_CHANNELS);
    return _sunxi_sound_pcm_hw_param_set(params, SND_PCM_HW_PARAM_CHANNELS, val);
}

int sunxi_sound_pcm_hw_params_set_rate(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_hw_params *params, unsigned int val, int dir)
{
    awtinysnd_debug("\n");
    sunxi_sound_pcm_hw_param_change(params, SND_PCM_HW_PARAM_RATE);
    return _sunxi_sound_pcm_hw_param_set(params, SND_PCM_HW_PARAM_RATE, val);
}

int sunxi_sound_pcm_hw_params_set_period_size(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_hw_params *params, snd_pcm_uframes_t val, int dir)
{
    awtinysnd_debug("\n");
    sunxi_sound_pcm_hw_param_change(params, SND_PCM_HW_PARAM_PERIOD_SIZE);
    return _sunxi_sound_pcm_hw_param_set(params, SND_PCM_HW_PARAM_PERIOD_SIZE, val);
}

int sunxi_sound_pcm_hw_params_set_periods(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_hw_params *params, unsigned int val, int dir)
{
    awtinysnd_debug("\n");
    sunxi_sound_pcm_hw_param_change(params, SND_PCM_HW_PARAM_PERIODS);
    return _sunxi_sound_pcm_hw_param_set(params, SND_PCM_HW_PARAM_PERIODS, val);
}

int sunxi_sound_pcm_hw_params_set_period_time(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_hw_params *params, unsigned int us, int dir)
{
	awtinysnd_debug("\n");
	sunxi_sound_pcm_hw_param_change(params, SND_PCM_HW_PARAM_PERIOD_TIME);
	return _sunxi_sound_pcm_hw_param_set(params, SND_PCM_HW_PARAM_PERIOD_TIME, us);
}

int sunxi_sound_pcm_hw_params_set_buffer_time(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_hw_params *params, unsigned int us)
{
	awtinysnd_debug("\n");
	sunxi_sound_pcm_hw_param_change(params, SND_PCM_HW_PARAM_BUFFER_TIME);
	return _sunxi_sound_pcm_hw_param_set(params, SND_PCM_HW_PARAM_BUFFER_TIME, us);
}

int sunxi_sound_pcm_hw_params_set_buffer_size(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_hw_params *params, snd_pcm_uframes_t val)
{
    awtinysnd_debug("\n");
    sunxi_sound_pcm_hw_param_change(params, SND_PCM_HW_PARAM_BUFFER_SIZE);
    return _sunxi_sound_pcm_hw_param_set(params, SND_PCM_HW_PARAM_BUFFER_SIZE, val);
}

int sunxi_sound_pcm_hw_params_get_access(const struct sunxi_sound_pcm_hw_params *params, snd_pcm_access_t *access)
{
    assert(params && access);
    *access = params_access(params);
    return 0;
}

int sunxi_sound_pcm_hw_params_get_format(const struct sunxi_sound_pcm_hw_params *params, snd_pcm_format_t *format)
{
	assert(params && format);
	*format = params_format(params);
	return 0;
}

int sunxi_sound_pcm_hw_params_get_channels(const struct sunxi_sound_pcm_hw_params *params, unsigned int *val)
{
	assert(params && val);
	*val = params_channels(params);
	return 0;
}

int sunxi_sound_pcm_hw_params_get_rate(const struct sunxi_sound_pcm_hw_params *params, unsigned int *val, int *dir)
{
	assert(params && val);
	*val = params_rate(params);
	return 0;
}

int sunxi_sound_pcm_hw_params_get_period_time(const struct sunxi_sound_pcm_hw_params *params, unsigned int *val, int *dir)
{
	assert(params && val);
	*val = params_period_time(params);
	return 0;
}

int sunxi_sound_pcm_hw_params_get_buffer_time(const struct sunxi_sound_pcm_hw_params *params, snd_pcm_uframes_t *val, int *dir)
{
	assert(params && val);
	*val = params_buffer_time(params);
	return 0;
}

int sunxi_sound_pcm_hw_params_get_period_size(const struct sunxi_sound_pcm_hw_params *params, snd_pcm_uframes_t *val, int *dir)
{
    assert(params && val);
    *val = params_period_size(params);
    return 0;
}

int sunxi_sound_pcm_hw_params_get_periods(const struct sunxi_sound_pcm_hw_params *params, unsigned int *val, int *dir)
{
    assert(params && val);
    *val = params_periods(params);
    return 0;
}

int sunxi_sound_pcm_hw_params_get_buffer_size(const struct sunxi_sound_pcm_hw_params *params, snd_pcm_uframes_t *val)
{
    assert(params && val);
    *val = params_buffer_size(params);
    return 0;
}

int _sunxi_sound_pcm_hw_params_internal(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_hw_params *params)
{
    int ret;
    sunxi_sound_pcm_sw_params_t sw;

    awtinysnd_debug("\n");

    assert(pcm->pcm_dev);
    ret = sunxi_sound_device_control(pcm->pcm_dev, SUNXI_SOUND_CTRL_PCM_HW_PARAMS, (unsigned long)params);
    if (ret != 0){
        awtinysnd_err("cannot set hw params %d\n", ret);
        return ret;
    }

    pcm->setup = 1;

    sunxi_sound_pcm_hw_params_get_access(params, &pcm->config.access);
    sunxi_sound_pcm_hw_params_get_format(params, &pcm->config.format);
    sunxi_sound_pcm_hw_params_get_channels(params, &pcm->config.channels);
    sunxi_sound_pcm_hw_params_get_rate(params, &pcm->config.rate, NULL);
    sunxi_sound_pcm_hw_params_get_period_time(params, &pcm->config.period_time, NULL);
    sunxi_sound_pcm_hw_params_get_period_size(params, &pcm->config.period_size, NULL);
    sunxi_sound_pcm_hw_params_get_periods(params, &pcm->config.period_count, NULL);
    sunxi_sound_pcm_hw_params_get_buffer_size(params, &pcm->buffer_size);

    pcm->sample_bits = sunxi_sound_pcm_format_physical_width(pcm->config.format);
    pcm->frame_bits = pcm->sample_bits * pcm->config.channels;

    awtinysnd_debug("access:%u\n", pcm->config.access);
    awtinysnd_debug("format:%u\n", pcm->config.format);
    awtinysnd_debug("channels:%u\n", pcm->config.channels);
    awtinysnd_debug("rate:%u\n", pcm->config.rate);
    awtinysnd_debug("period_time:%u\n", pcm->config.period_time);
    awtinysnd_debug("period_size:%lu\n", pcm->config.period_size);
    awtinysnd_debug("buffer_size:%lu\n", pcm->config.period_size * pcm->config.period_count);
    awtinysnd_debug("sample_bits:%u\n", pcm->sample_bits);
    awtinysnd_debug("frame_bits:%u\n", pcm->frame_bits);

    sunxi_sound_pcm_sw_params_default(pcm, &sw);
    ret = sunxi_sound_pcm_set_sw_params(pcm, &sw);
    if (ret < 0)
        return ret;

    return 0;
}

int sunxi_sound_pcm_set_hw_params(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_hw_params *params)
{
    int ret;
    assert(pcm && params);
    awtinysnd_debug("\n");

    /*TODO: dmix can't support pause */
    params->can_paused = 1;

    ret = _sunxi_sound_pcm_hw_params_internal(pcm, params);
    if (ret < 0)
        return ret;
    return ret;
}

int sunxi_sound_pcm_set_hw_free(struct sunxi_pcm *pcm)
{
    int ret;

    assert(pcm);
    awtinysnd_debug("\n");

    if (!pcm->setup)
        return 0;

    assert(pcm->pcm_dev);
    ret = sunxi_sound_device_control(pcm->pcm_dev, SUNXI_SOUND_CTRL_PCM_HW_FREE, (unsigned long)NULL);
    if (ret < 0) {
        awtinysnd_err("cannot set hw free %d\n", ret);
        return ret;
    }
    pcm->setup = 0;

    return ret;
}

static int sunxi_sound_pcm_sw_params_default(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_sw_params *params)
{
	params->avail_min = pcm->config.period_size;
	params->start_threshold = 1;
	params->stop_threshold = pcm->buffer_size;
	params->silence_size = 0;
	params->boundary = pcm->buffer_size;
	if (!pcm->buffer_size) {
		awtinysnd_info("buffer size is 0...\n");
		return 0;
	}
	while (params->boundary * 2 <= LONG_MAX - pcm->buffer_size)
		params->boundary *= 2;
	return 0;
}

int sunxi_sound_pcm_sw_params_get_boundary(const struct sunxi_sound_pcm_sw_params *params, snd_pcm_uframes_t *val)
{
	assert(params && val);
	*val = params->boundary;
	return 0;
}

int sunxi_sound_pcm_sw_params_current(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_sw_params *params)
{
	assert(pcm && params);
	params->avail_min = pcm->config.avail_min;
	params->start_threshold = pcm->config.start_threshold;
	params->stop_threshold = pcm->config.stop_threshold;
	params->silence_size = pcm->config.silence_threshold;
	params->boundary = pcm->boundary;
	return 0;
}

int sunxi_sound_pcm_sw_params_set_start_threshold(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_sw_params *params, snd_pcm_uframes_t val)
{
	assert(pcm && params);
	params->start_threshold = val;
	return 0;
}

int sunxi_sound_pcm_sw_params_get_start_threshold(struct sunxi_sound_pcm_sw_params *params, snd_pcm_uframes_t *val)
{
	assert(params && val);
	*val = params->start_threshold;
	return 0;
}

int sunxi_sound_pcm_sw_params_set_stop_threshold(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_sw_params *params, snd_pcm_uframes_t val)
{
	assert(pcm && params);
	params->stop_threshold = val;
	return 0;
}

int sunxi_sound_pcm_sw_params_get_stop_threshold(struct sunxi_sound_pcm_sw_params *params, snd_pcm_uframes_t *val)
{
	assert(params && val);
	*val = params->stop_threshold;
	return 0;
}

int sunxi_sound_pcm_sw_params_set_silence_size(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_sw_params *params, snd_pcm_uframes_t val)
{
	assert(pcm && params);
	params->silence_size = val;
	return 0;
}

int sunxi_sound_pcm_sw_params_set_avail_min(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_sw_params *params, snd_pcm_uframes_t val)
{
	assert(pcm && params);
	params->avail_min = val;
	return 0;
}

int sunxi_sound_pcm_sw_params_get_avail_min(const struct sunxi_sound_pcm_sw_params *params, snd_pcm_uframes_t *val)
{
	assert(params && val);
	*val = params->avail_min;
	return 0;
}

int sunxi_sound_pcm_set_sw_params(struct sunxi_pcm *pcm, struct sunxi_sound_pcm_sw_params *params)
{
	int ret = 0;

    assert(pcm && params);
    /* the hw_params must be set at first!!! */
    if (!pcm->setup) {
        awtinysnd_err("PCM not set up\n");
        return -EIO;
    }
    if (!params->avail_min) {
        awtinysnd_err("params->avail_min is 0\n");
        return -EINVAL;
    }

    assert(pcm->pcm_dev);
    sunxi_sound_pcm_lock(pcm);
    ret = sunxi_sound_device_control(pcm->pcm_dev, SUNXI_SOUND_CTRL_PCM_SW_PARAMS, (unsigned long)params);
    if (ret < 0) {
        awtinysnd_err("cannot set sw params %d\n", ret);
        sunxi_sound_pcm_unlock(pcm);
        return ret;
    }

    pcm->config.avail_min = params->avail_min;
    pcm->config.start_threshold = params->start_threshold;
    pcm->config.stop_threshold = params->stop_threshold;
    pcm->config.silence_threshold = params->silence_size;
    pcm->boundary = params->boundary;
    sunxi_sound_pcm_unlock(pcm);
    return ret;
}

int sunxi_pcm_set_config(struct sunxi_pcm *pcm, const struct sunxi_pcm_config *config)
{
    int ret;

    if (pcm == NULL)
        return -EFAULT;
    else if (config == NULL) {
        config = &pcm->config;
        pcm->config.channels = 2;
        pcm->config.rate = 48000;
        pcm->config.period_size = 1024;
        pcm->config.period_count = 4;
        pcm->config.format = SND_PCM_FORMAT_S16_LE;
        pcm->config.start_threshold = config->period_count * config->period_size;
        pcm->config.stop_threshold = config->period_count * config->period_size;
        pcm->config.silence_threshold = 0;
    } else
        pcm->config = *config;

    struct sunxi_sound_pcm_hw_params *params;
    /* HW params */
    sunxi_sound_pcm_hw_params_alloca(&params);
    ret =  sunxi_sound_pcm_hw_params_any(pcm, params);
    if (ret < 0) {
        awtinysnd_err("no configurations available\n");
        return ret;
    }
    sunxi_sound_pcm_hw_params_set_access(pcm, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    ret = sunxi_sound_pcm_hw_params_set_format(pcm, params, config->format);
    ret = sunxi_sound_pcm_hw_params_set_channels(pcm, params, config->channels);
    ret = sunxi_sound_pcm_hw_params_set_rate(pcm, params, config->rate, 0);
    ret = sunxi_sound_pcm_hw_params_set_period_size(pcm, params, config->period_size, 0);
    ret = sunxi_sound_pcm_hw_params_set_periods(pcm, params, config->period_count, 0);
    ret = sunxi_sound_pcm_set_hw_params(pcm, params);
    if (ret < 0) {
        awtinysnd_err("Unable to install hw prams!\n");
        return ret;
    }

    /* get our refined hw_params */
    sunxi_sound_pcm_hw_params_get_period_size(params, &pcm->config.period_size, NULL);
    sunxi_sound_pcm_hw_params_get_periods(params, &pcm->config.period_count, NULL);
    pcm->buffer_size = config->period_count * config->period_size;


    struct sunxi_sound_pcm_sw_params *sparams;

    sunxi_sound_pcm_sw_params_alloca(&sparams);
    sunxi_sound_pcm_sw_params_current(pcm, sparams);

    if (sunxi_pcm_stream(pcm) == SND_PCM_STREAM_CAPTURE) {
        sunxi_sound_pcm_sw_params_set_start_threshold(pcm, sparams, 1);
    } else {
        snd_pcm_uframes_t boundary = 0;
        sunxi_sound_pcm_sw_params_get_boundary(sparams, &boundary);
        sunxi_sound_pcm_sw_params_set_start_threshold(pcm, sparams, config->period_count * config->period_size);
        /* set silence size, in order to fill silence data into ringbuffer */
        sunxi_sound_pcm_sw_params_set_silence_size(pcm, sparams, boundary);
    }
    sunxi_sound_pcm_sw_params_set_stop_threshold(pcm, sparams, config->period_count * config->period_size);
    sunxi_sound_pcm_sw_params_set_avail_min(pcm, sparams, config->period_size);

    ret = sunxi_sound_pcm_set_sw_params(pcm, sparams);
    if (ret < 0) {
        awtinysnd_err("Unable to install sw prams!\n");
        return ret;
    }

    return 0;
}


int sunxi_pcm_close(struct sunxi_pcm *pcm)
{
    int ret = 0, res = 0;

    if (pcm == NULL)
        return 0;

    if (pcm->setup) {
        sunxi_pcm_stop(pcm);
        ret = sunxi_sound_pcm_set_hw_free(pcm);
        if (ret < 0)
            res = ret;
    }

    if (pcm->pcm_dev != NULL) {
        ret = sunxi_sound_device_close(pcm->pcm_dev);
        if (ret < 0)
            res = ret;
    }
    pcm->prepared = 0;
    pcm->running = 0;
    pcm->buffer_size = 0;
    pcm->pcm_dev = NULL;
    sunxi_sound_mutex_destroy(pcm->mutex);
    free(pcm);
    return res;
}

int sunxi_pcm_open(struct sunxi_pcm **pcmp, unsigned int card, unsigned int device,
                     snd_pcm_stream_t stream, int mode)
{
    struct sunxi_pcm *pcm;
    struct sunxi_sound_pcm_info info;
    char fn[64];
    int ret;

    pcm = calloc(1, sizeof(struct sunxi_pcm));
    if (!pcm) {
        awtinysnd_err("no memory!\n");
        return -ENOMEM;
    }

    pcm->stream = stream;

    snprintf(fn, sizeof(fn), "SunxipcmC%uD%u%c", card, device,
             sunxi_pcm_stream(pcm) == SND_PCM_STREAM_CAPTURE ? 'c' : 'p');

    awtinysnd_info("card name %s\n", fn);

    ret = sunxi_sound_device_open(&pcm->pcm_dev, fn, mode);
    if (ret < 0) {
        awtinysnd_err("cannot open device '%s' \n", fn);
        return ret;
    }

    ret = sunxi_sound_device_control(pcm->pcm_dev, SUNXI_SOUND_CTRL_PCM_INFO, (unsigned long)&info);
    if (ret < 0) {
        awtinysnd_err("cannot get info\n");
        goto fail_close;
    }

    pcm->subdevice = info.subdevice;

    pcm->mutex = sunxi_sound_mutex_init();
    if (pcm->mutex == NULL) {
        awtinysnd_err("sunxi_sound_mutex_init failed \n");
        ret = -EINVAL;
        goto fail_close;
    }

    pcm->underruns = 0;
    *pcmp = pcm;

    return 0;

fail_close:
    if (pcm->pcm_dev) {
        sunxi_sound_device_close(pcm->pcm_dev);
        pcm->pcm_dev = NULL;
    }
    return ret;
}

int sunxi_pcm_prepare(struct sunxi_pcm *pcm)
{
    int ret;

    assert(pcm);
    awtinysnd_debug("\n");

    if (pcm->prepared)
        return 0;

    sunxi_sound_pcm_lock(pcm);
    ret = sunxi_sound_device_control(pcm->pcm_dev, SUNXI_SOUND_CTRL_PCM_PREPARE, (unsigned long)NULL);
    if (ret < 0) {
        awtinysnd_err("cannot prepare pcm %d\n", ret);
        sunxi_sound_pcm_unlock(pcm);
        return ret;
    }
    sunxi_sound_pcm_unlock(pcm);

    pcm->prepared = 1;
    return ret;
}


int sunxi_pcm_reset(struct sunxi_pcm *pcm)
{
    int ret;

    assert(pcm);
    awtinysnd_debug("\n");

    if (!pcm->setup) {
        awtinysnd_err("PCM not set up\n");
        return -EIO;
    }

    sunxi_sound_pcm_lock(pcm);
    ret = sunxi_sound_device_control(pcm->pcm_dev, SUNXI_SOUND_CTRL_PCM_RESET, (unsigned long)NULL);
    if (ret < 0) {
        awtinysnd_err("cannot reset pcm %d\n", ret);
        sunxi_sound_pcm_unlock(pcm);
        return ret;
    }
    sunxi_sound_pcm_unlock(pcm);

    return ret;
}

int sunxi_pcm_start(struct sunxi_pcm *pcm)
{
    int ret;

    assert(pcm);
    awtinysnd_debug("\n");

    int prepare_error = sunxi_pcm_prepare(pcm);
    if (prepare_error)
        return prepare_error;

    sunxi_sound_pcm_lock(pcm);
    ret = sunxi_sound_device_control(pcm->pcm_dev, SUNXI_SOUND_CTRL_PCM_START, (unsigned long)NULL);
    if (ret < 0) {
        awtinysnd_err("cannot start pcm %d\n", ret);
        sunxi_sound_pcm_unlock(pcm);
        return ret;
    }
    sunxi_sound_pcm_unlock(pcm);

    pcm->running = 1;
    return ret;
}

int sunxi_pcm_stop(struct sunxi_pcm *pcm)
{
    int ret;

    assert(pcm);
    awtinysnd_debug("\n");

    sunxi_sound_pcm_lock(pcm);
    ret = sunxi_sound_device_control(pcm->pcm_dev, SUNXI_SOUND_CTRL_PCM_DROP, (unsigned long)NULL);
    if (ret < 0) {
        awtinysnd_err("cannot stop pcm %d\n", ret);
        sunxi_sound_pcm_unlock(pcm);
        return ret;
    }
    sunxi_sound_pcm_unlock(pcm);

    pcm->prepared = 0;
    pcm->running = 0;

    return ret;
}

int sunxi_pcm_drain(struct sunxi_pcm *pcm)
{
    int ret;

    assert(pcm);
    awtinysnd_debug("\n");

    sunxi_sound_pcm_lock(pcm);
    ret = sunxi_sound_device_control(pcm->pcm_dev, SUNXI_SOUND_CTRL_PCM_DRAIN, (unsigned long)NULL);
    if (ret < 0) {
        awtinysnd_err("cannot drain pcm %d\n", ret);
        sunxi_sound_pcm_unlock(pcm);
        return ret;
    }
    sunxi_sound_pcm_unlock(pcm);

    return ret;
}

int sunxi_pcm_pause(struct sunxi_pcm *pcm, int enable)
{
    int ret;

    assert(pcm);
    awtinysnd_debug("\n");

    sunxi_sound_pcm_lock(pcm);
    ret = sunxi_sound_device_control(pcm->pcm_dev, SUNXI_SOUND_CTRL_PCM_PAUSE, enable);
    if (ret < 0) {
        awtinysnd_err("cannot pause pcm %d\n", ret);
        sunxi_sound_pcm_unlock(pcm);
        return ret;
    }
    sunxi_sound_pcm_unlock(pcm);

    return ret;
}

int sunxi_pcm_state(struct sunxi_pcm *pcm)
{
    int ret;

    assert(pcm);
    struct sunxi_sound_pcm_sync_ptr sync_ptr;
    awtinysnd_debug("\n");

    memset(&sync_ptr, 0, sizeof(struct sunxi_sound_pcm_sync_ptr));
    sync_ptr.flags = SNDRV_PCM_SYNC_PTR_APPL;

    sunxi_sound_pcm_lock(pcm);
    ret = sunxi_sound_device_control(pcm->pcm_dev, SUNXI_SOUND_CTRL_PCM_SYNC_PTR, (unsigned long)&sync_ptr);
    if (ret < 0) {
        awtinysnd_err("cannot get state pcm %d\n", ret);
        sunxi_sound_pcm_unlock(pcm);
        return ret;
    }
    sunxi_sound_pcm_unlock(pcm);

    return (sunxi_sound_pcm_state_t)sync_ptr.s.status.state;
}

int sunxi_pcm_hwsync(struct sunxi_pcm *pcm)
{
    int ret;

    assert(pcm);
    struct sunxi_sound_pcm_sync_ptr sync_ptr;
    awtinysnd_debug("\n");

    memset(&sync_ptr, 0, sizeof(struct sunxi_sound_pcm_sync_ptr));
    sync_ptr.flags = SNDRV_PCM_SYNC_PTR_HWSYNC;

    sunxi_sound_pcm_lock(pcm);
    ret = sunxi_sound_device_control(pcm->pcm_dev, SUNXI_SOUND_CTRL_PCM_SYNC_PTR, (unsigned long)&sync_ptr);
    if (ret < 0) {
        awtinysnd_err("cannot hwsync pcm %d\n", ret);
        sunxi_sound_pcm_unlock(pcm);
        return ret;
    }
    sunxi_sound_pcm_unlock(pcm);

    return ret;
}

snd_pcm_stream_t sunxi_pcm_stream(struct sunxi_pcm *pcm)
{
	assert(pcm);
	return pcm->stream;
}

snd_pcm_sframes_t sunxi_pcm_writei(struct sunxi_pcm *pcm, const void *buffer, snd_pcm_uframes_t size)
{
	snd_pcm_sframes_t ret = 0;

	assert(pcm);
	assert(size == 0 || buffer);
	/*awtinysnd_debug("\n");*/
	ret = sunxi_sound_device_write(pcm->pcm_dev, buffer, size);
	if (ret < 0) {
		pcm->prepared = 0;
		pcm->running = 0;
	}

	return ret;
}

snd_pcm_sframes_t sunxi_pcm_readi(struct sunxi_pcm *pcm, void *buffer, snd_pcm_uframes_t size)
{

	snd_pcm_sframes_t ret = 0;

	assert(pcm);
	assert(size == 0 || buffer);
	awtinysnd_debug("\n");
	ret = sunxi_sound_device_read(pcm->pcm_dev, buffer, size);
	if (ret < 0) {
		pcm->prepared = 0;
		pcm->running = 0;
	}

	return ret;
}

int sunxi_pcm_resume(struct sunxi_pcm *pcm)
{
	assert(pcm);
	awtinysnd_err("suspend state not supported.\n");
	return -1;
}

int sunxi_pcm_recover(struct sunxi_pcm *pcm, int err, int silent)
{
	assert(pcm);
	if (err > 0)
		err = -err;
	if (err == -EPIPE) {
		const char *s = NULL;
		if (sunxi_pcm_stream(pcm) == SND_PCM_STREAM_PLAYBACK) {
			s = "underrun";
		} else {
			s = "overrun";
		}
		if (silent == 0) {
			awtinysnd_err("%s occured\n", s);
		}
		err = sunxi_pcm_prepare(pcm);
		if (err < 0) {
			awtinysnd_err("cannot recovery from %s, prepare return: %d",
				s, err);
			return err;
		}
		return 0;
	} else if (err == -ESTRPIPE) {
		awtinysnd_info("resume...\n");
		while ((err = sunxi_pcm_resume(pcm)) == -EAGAIN) {
			vTaskDelay(pdMS_TO_TICKS(1));
		}
		if (err < 0) {
			err = sunxi_pcm_prepare(pcm);
			awtinysnd_err("cannot recovery from suspend, prepare return: %d", err);
			return err;
		}
		return 0;
	}
	return err;
}

int sunxi_pcm_delay(struct sunxi_pcm *pcm, snd_pcm_sframes_t *delayp)
{
    int ret = 0;

    assert(pcm);
    assert(delayp);

    sunxi_sound_pcm_lock(pcm);
    ret = sunxi_sound_device_control(pcm->pcm_dev, SUNXI_SOUND_CTRL_PCM_DELAY, (unsigned long)delayp);
    if (ret < 0) {
        awtinysnd_err("cannot delay pcm %d\n", ret);
        sunxi_sound_pcm_unlock(pcm);
        return ret;
    }
    sunxi_sound_pcm_unlock(pcm);

    return ret;
}

#define STATE(v) [SNDRV_PCM_STATE_##v] = #v
#define STREAM(v) [SND_PCM_STREAM_##v] = #v
#define ACCESS(v) [SND_PCM_ACCESS_##v] = #v
#define FORMAT(v) [SND_PCM_FORMAT_##v] = #v

static const char *const sunxi_sound_pcm_stream_names[] = {
	STREAM(PLAYBACK),
	STREAM(CAPTURE),
};

static const char *const sunxi_sound_pcm_state_names[] = {
	STATE(OPEN),
	STATE(SETUP),
	STATE(PREPARED),
	STATE(RUNNING),
	STATE(XRUN),
	STATE(DRAINING),
	STATE(PAUSED),
	STATE(SUSPENDED),
	STATE(DISCONNECTED),
};

static const char *const sunxi_sound_pcm_access_names[] = {
	ACCESS(MMAP_INTERLEAVED),
	ACCESS(MMAP_NONINTERLEAVED),
	ACCESS(MMAP_COMPLEX),
	ACCESS(RW_INTERLEAVED),
	ACCESS(RW_NONINTERLEAVED),
};

static const char *const sunxi_sound_pcm_format_names[] = {
	FORMAT(S8),
	FORMAT(U8),
	FORMAT(S16_LE),
	FORMAT(S16_BE),
	FORMAT(U16_LE),
	FORMAT(U16_BE),
	FORMAT(S24_LE),
	FORMAT(S24_BE),
	FORMAT(U24_LE),
	FORMAT(U24_BE),
	FORMAT(S32_LE),
	FORMAT(S32_BE),
	FORMAT(U32_LE),
	FORMAT(U32_BE),
};

const char *sunxi_sound_pcm_stream_name(snd_pcm_stream_t stream)
{
	if (stream > SND_PCM_STREAM_LAST)
		return NULL;
	return sunxi_sound_pcm_stream_names[stream];
}

const char *sunxi_sound_pcm_access_name(snd_pcm_access_t acc)
{
	if (acc > SND_PCM_ACCESS_LAST)
		return NULL;
	return sunxi_sound_pcm_access_names[acc];
}

const char *sunxi_sound_pcm_format_name(snd_pcm_format_t format)
{
	if (format > SND_PCM_FORMAT_LAST)
		return NULL;
	return sunxi_sound_pcm_format_names[format];
}

int sunxi_pcm_dump_hw_setup(struct sunxi_pcm *pcm)
{
	assert(pcm);
	if (!pcm->setup) {
		awtinysnd_err("PCM not set up\n");
		return -EIO;
	}
	printf("  stream          : %s\n", sunxi_sound_pcm_stream_name(pcm->stream));
	printf("  access          : %s\n", sunxi_sound_pcm_access_name(pcm->config.access));
	printf("  format          : %s\n", sunxi_sound_pcm_format_name(pcm->config.format));
	printf("  channels        : %u\n", pcm->config.channels);
	printf("  rate            : %u\n", pcm->config.rate);
	printf("  buffer_size     : %lu\n", pcm->buffer_size);
	printf("  period_size     : %lu\n", pcm->config.period_size);
	printf("  period_time     : %u\n", pcm->config.period_time);
	return 0;
}

int sunxi_pcm_dump_sw_setup(struct sunxi_pcm *pcm)
{
	assert(pcm);
	if (!pcm->setup) {
		awtinysnd_err("PCM not set up\n");
		return -EIO;
	}
	printf("  avail_min       : %u\n", pcm->config.avail_min);
	printf("  start_threshold : %u\n", pcm->config.start_threshold);
	printf("  stop_threshold  : %u\n", pcm->config.stop_threshold);
	printf("  silence_size    : %u\n", pcm->config.silence_threshold);
	printf("  boundary        : %lu\n", pcm->boundary);
	return 0;
}

int sunxi_pcm_dump_setup(struct sunxi_pcm *pcm)
{
	sunxi_pcm_dump_hw_setup(pcm);
	sunxi_pcm_dump_sw_setup(pcm);
	return 0;
}

ssize_t sunxi_sound_pcm_format_size(snd_pcm_format_t format, size_t samples)
{
	int size = 0;
	size = sunxi_sound_pcm_format_physical_width(format);
	if (size < 0)
		return size;
	return (size * samples / 8);
}

snd_pcm_sframes_t sunxi_sound_pcm_bytes_to_frames(struct sunxi_pcm *pcm, ssize_t bytes)
{
	assert(pcm);
	return bytes * 8 / pcm->frame_bits;
}

ssize_t sunxi_sound_pcm_frames_to_bytes(struct sunxi_pcm *pcm, snd_pcm_sframes_t frames)
{
	assert(pcm);
	return frames * pcm->frame_bits / 8;
}

int sunxi_sound_pcm_hw_params_pause_able(const sunxi_sound_pcm_hw_params_t *params)
{
	assert(params);

	return !!params->can_paused;
}

