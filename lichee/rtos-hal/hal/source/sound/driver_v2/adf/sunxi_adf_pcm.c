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

#include <aw_common.h>
#include <sound_v2/sunxi_adf_core.h>
#include <sound_v2/sunxi_sound_pcm_common.h>


static inline const char *adf_cpu_dai_name(struct sunxi_sound_adf_pcm_running_param *rtp)
{
	return (rtp)->num_cpus == 1 ? adf_rtp_to_cpu(rtp, 0)->name : "multicpu";
}

static inline const char *adf_codec_dai_name(struct sunxi_sound_adf_pcm_running_param *rtp)
{
	return (rtp)->num_codecs == 1 ? adf_rtp_to_codec(rtp, 0)->name : "multicodec";
}

int sunxi_adf_sound_set_runtime_hwparams(struct sunxi_sound_pcm_dataflow *dataflow,
	const struct sunxi_sound_pcm_hardware *hw)
{
	dataflow->pcm_running->hw = *hw;

	return 0;
}

static void sunxi_adf_sound_pcm_hw_init(struct sunxi_sound_pcm_hardware *hw)
{
	hw->rates		= UINT_MAX;
	hw->rate_min		= 0;
	hw->rate_max		= UINT_MAX;
	hw->channels_min	= 0;
	hw->channels_max	= UINT_MAX;
	hw->formats		= ULLONG_MAX;
}

static void sunxi_adf_sound_pcm_hw_update_rate(struct sunxi_sound_pcm_hardware *hw,
				   struct sunxi_sound_adf_pcm_stream *p)
{
	hw->rates = sunxi_sound_pcm_rate_mask_intersect(hw->rates, p->rates);

	/* setup hw->rate_min/max via hw->rates first */
	sunxi_sound_pcm_limit_hw_rates(hw);

	/* update hw->rate_min/max by snd_soc_pcm_stream */
	hw->rate_min = max(hw->rate_min, p->rate_min);
	hw->rate_max = min_not_zero(hw->rate_max, p->rate_max);
}

static void sunxi_adf_sound_pcm_hw_update_chan(struct sunxi_sound_pcm_hardware *hw,
				   struct sunxi_sound_adf_pcm_stream *p)
{
	hw->channels_min = max(hw->channels_min, p->channels_min);
	hw->channels_max = min(hw->channels_max, p->channels_max);
}

static void sunxi_adf_sound_pcm_hw_update_format(struct sunxi_sound_pcm_hardware *hw,
				     struct sunxi_sound_adf_pcm_stream *p)
{
	hw->formats &= p->formats;
}

static int sunxi_adf_sound_pcm_init_runtime_hw_constrains(struct sunxi_sound_pcm_hardware *hw,
				struct sunxi_sound_pcm_hw_constrains *cons)
{
	union snd_interval *interval_access = &cons->intervals[SND_PCM_HW_PARAM_ACCESS];
	if (hw->info & SNDRV_PCM_INFO_INTERLEAVED)
		interval_access->mask |= 1 << SND_PCM_ACCESS_RW_INTERLEAVED;
	if (hw->info & SNDRV_PCM_INFO_NONINTERLEAVED)
		interval_access->mask |= 1 << SND_PCM_ACCESS_RW_NONINTERLEAVED;
	if (hw->info & SNDRV_PCM_INFO_MMAP) {
		if (hw->info & SNDRV_PCM_INFO_INTERLEAVED)
			interval_access->mask |= 1 << SND_PCM_ACCESS_MMAP_INTERLEAVED;
		if (hw->info & SNDRV_PCM_INFO_NONINTERLEAVED)
			interval_access->mask |= 1 << SND_PCM_ACCESS_MMAP_NONINTERLEAVED;
	}

	cons->intervals[SND_PCM_HW_PARAM_FORMAT].mask = hw->formats;
	cons->intervals[SND_PCM_HW_PARAM_CHANNELS].range.min = hw->channels_min;
	cons->intervals[SND_PCM_HW_PARAM_CHANNELS].range.max = hw->channels_max;
	cons->intervals[SND_PCM_HW_PARAM_RATE].range.min = hw->rate_min;
	cons->intervals[SND_PCM_HW_PARAM_RATE].range.max = hw->rate_max;
	cons->intervals[SND_PCM_HW_PARAM_PERIOD_BYTES].range.min = hw->period_bytes_min;
	cons->intervals[SND_PCM_HW_PARAM_PERIOD_BYTES].range.max = hw->period_bytes_max;
	cons->intervals[SND_PCM_HW_PARAM_PERIODS].range.min = hw->periods_min;
	cons->intervals[SND_PCM_HW_PARAM_PERIODS].range.max = hw->periods_max;

	cons->intervals[SND_PCM_HW_PARAM_BUFFER_BYTES].range.max = hw->buffer_bytes_max;

	cons->intervals[SND_PCM_HW_PARAM_SAMPLE_BITS].range.min =
			(uint32_t)sunxi_sound_pcm_format_physical_width(__pcm_ffs(hw->formats));
	cons->intervals[SND_PCM_HW_PARAM_SAMPLE_BITS].range.max =
			(uint32_t)sunxi_sound_pcm_format_physical_width(__fls(hw->formats));

	return 0;
}

static void sunxi_adf_sound_pcm_init_runtime_hw(struct sunxi_sound_pcm_dataflow *dataflow)
{
	struct sunxi_sound_pcm_hardware *hw = &dataflow->pcm_running->hw;
	struct sunxi_sound_adf_pcm_running_param *rtp = adf_dataflow_to_rtp(dataflow);
	struct sunxi_sound_adf_dai *codec_dai;
	struct sunxi_sound_adf_dai *cpu_dai;
	struct sunxi_sound_adf_pcm_stream *codec_stream;
	struct sunxi_sound_adf_pcm_stream *cpu_stream;
	unsigned int cpu_chan_min = 0, cpu_chan_max = UINT_MAX;
	uint64_t formats = hw->formats;
	int i;

	sunxi_adf_sound_pcm_hw_init(hw);

	for_each_rtp_cpu_dais(rtp, i, cpu_dai) {

		if (!sunxi_sound_adf_dai_stream_valid(cpu_dai, dataflow->stream))
			continue;

		cpu_stream = sunxi_sound_adf_dai_get_pcm_stream(cpu_dai, dataflow->stream);

		sunxi_adf_sound_pcm_hw_update_chan(hw, cpu_stream);
		sunxi_adf_sound_pcm_hw_update_rate(hw, cpu_stream);
		sunxi_adf_sound_pcm_hw_update_format(hw, cpu_stream);
	}
	cpu_chan_min = hw->channels_min;
	cpu_chan_max = hw->channels_max;

	/* second calculate min/max only for CODECs in the DAI link */
	for_each_rtp_codec_dais(rtp, i, codec_dai) {

		if (!sunxi_sound_adf_dai_stream_valid(codec_dai, dataflow->stream))
			continue;

		codec_stream = sunxi_sound_adf_dai_get_pcm_stream(codec_dai, dataflow->stream);

		sunxi_adf_sound_pcm_hw_update_chan(hw, codec_stream);
		sunxi_adf_sound_pcm_hw_update_rate(hw, codec_stream);
		sunxi_adf_sound_pcm_hw_update_format(hw, codec_stream);
	}

	/* Verify both a valid CPU DAI and a valid CODEC DAI were found */
	if (!hw->channels_min)
		return;

	if (rtp->num_codecs > 1) {
		hw->channels_min = cpu_chan_min;
		hw->channels_max = cpu_chan_max;
	}

	if (formats)
		hw->formats &= formats;

	/* fill in hw constrains */
	sunxi_adf_sound_pcm_init_runtime_hw_constrains(hw, &dataflow->pcm_running->hw_constrains);

	return;
}


static int sunxi_sound_adf_pcm_components_open(struct sunxi_sound_pcm_dataflow *dataflow)
{
	struct sunxi_sound_adf_pcm_running_param *rtp = adf_dataflow_to_rtp(dataflow);
	struct sunxi_sound_adf_component *component;
	int i, ret = 0;

	for_each_rtp_components(rtp, i, component) {

		ret = sunxi_sound_adf_component_open(component, dataflow);
		if (ret < 0)
			break;
	}

	return ret;
}

static int sunxi_sound_adf_pcm_components_close(struct sunxi_sound_pcm_dataflow *dataflow)
{
	struct sunxi_sound_adf_pcm_running_param *rtp = adf_dataflow_to_rtp(dataflow);
	struct sunxi_sound_adf_component *component;
	int i, ret = 0;

	for_each_rtp_components(rtp, i, component) {
		int r = sunxi_sound_adf_component_close(component, dataflow);
		if (r < 0)
			ret = r;
	}

	return ret;
}

static int sunxi_sound_adf_pcm_clean(struct sunxi_sound_pcm_dataflow *dataflow)
{
	struct sunxi_sound_adf_pcm_running_param *rtp = adf_dataflow_to_rtp(dataflow);
	struct sunxi_sound_adf_dai *dai;
	int i;

	sound_mutex_lock(rtp->card->pcm_mutex);

	for_each_rtp_dais(rtp, i, dai)
		sunxi_sound_adf_dai_shutdown(dai, dataflow);

	if (rtp->dai_bind->ops &&
	    rtp->dai_bind->ops->shutdown)
		rtp->dai_bind->ops->shutdown(dataflow);

	sunxi_sound_adf_pcm_components_close(dataflow);

	sound_mutex_unlock(rtp->card->pcm_mutex);

	return 0;
}

static int sunxi_sound_adf_pcm_close(struct sunxi_sound_pcm_dataflow *dataflow)
{
	int ret;

	sunxi_sound_adf_pcm_clean(dataflow);

	/* dapm stop */
	ret = sunxi_sound_adf_pcm_component_dapm_control(dataflow, 0);
	if (ret < 0 && ret != -EBUSY) {
		snd_err("sunxi_sound_adf_pcm_component_dapm_control failed return %d\n", ret);
		return ret;
	}

	return ret;
}

static int sunxi_sound_adf_pcm_open(struct sunxi_sound_pcm_dataflow *dataflow)
{
	struct sunxi_sound_adf_pcm_running_param *rtp = adf_dataflow_to_rtp(dataflow);
	struct sunxi_sound_adf_dai *dai;
	int i, ret = 0;

	snd_debug("\n");

	sound_mutex_lock(rtp->card->pcm_mutex);

	ret = sunxi_sound_adf_pcm_components_open(dataflow);
	if (ret < 0)
		goto err;

	if (rtp->dai_bind->ops &&
	    rtp->dai_bind->ops->startup) {
		ret = rtp->dai_bind->ops->startup(dataflow);
		if (ret < 0) {
			snd_err("Adf: error on %s: %d\n",
					rtp->dai_bind->name, ret);
			goto err;
		}
	}

	/* startup the audio subsystem */
	for_each_rtp_dais(rtp, i, dai) {
		ret = sunxi_sound_adf_dai_startup(dai, dataflow);
		if (ret < 0)
			goto err;
	}

	sunxi_adf_sound_pcm_init_runtime_hw(dataflow);

	snd_debug("open %s stream\n", dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK ?
					"Playback" : "Capture");

err:
	sound_mutex_unlock(rtp->card->pcm_mutex);
	if (ret < 0) {
		sunxi_sound_adf_pcm_clean(dataflow);
		snd_err("failed (%d)", ret);
	}

	return ret;
}

static int sunxi_sound_adf_pcm_prepare(struct sunxi_sound_pcm_dataflow *dataflow)
{
	struct sunxi_sound_adf_pcm_running_param *rtp = adf_dataflow_to_rtp(dataflow);

	int ret = 0;

	sound_mutex_lock(rtp->card->pcm_mutex);

	if (rtp->dai_bind->ops &&
	    rtp->dai_bind->ops->prepare) {
		ret = rtp->dai_bind->ops->prepare(dataflow);
		if (ret < 0) {
			snd_err("Adf: error on %s: %d\n",
					rtp->dai_bind->name, ret);
			goto err;
		}
	}

	ret = sunxi_sound_adf_pcm_component_prepare(dataflow);
	if (ret < 0)
		goto err;

	ret = sunxi_sound_adf_pcm_dai_prepare(dataflow);
	if (ret < 0)
		goto err;

	ret = sunxi_sound_adf_pcm_component_dapm_control(dataflow, 1);
	if (ret < 0 && ret != -EBUSY)
		goto err;

err:
	sound_mutex_unlock(rtp->card->pcm_mutex);

	if (ret < 0)
		snd_err("Adf: failed (%d)\n", ret);

	return ret;
}

static int sunxi_sound_adf_pcm_hw_clean(struct sunxi_sound_pcm_dataflow *dataflow)
{
	struct sunxi_sound_adf_pcm_running_param *rtp = adf_dataflow_to_rtp(dataflow);
	struct sunxi_sound_adf_dai *dai;
	int i;

	sound_mutex_lock(rtp->card->pcm_mutex);

	if (rtp->dai_bind->ops &&
	    rtp->dai_bind->ops->hw_free)
		rtp->dai_bind->ops->hw_free(dataflow);

	sunxi_sound_adf_pcm_component_hw_free(dataflow);

	for_each_rtp_dais(rtp, i, dai) {
	if (!sunxi_sound_adf_dai_stream_valid(dai, dataflow->stream))
			continue;

		sunxi_sound_adf_dai_hw_free(dai, dataflow);
	}

	sound_mutex_unlock(rtp->card->pcm_mutex);

	return 0;
}

static int sunxi_sound_adf_pcm_hw_free(struct sunxi_sound_pcm_dataflow *dataflow)
{
	return sunxi_sound_adf_pcm_hw_clean(dataflow);
}

static int sunxi_sound_adf_pcm_hw_params(struct sunxi_sound_pcm_dataflow *dataflow,
				struct sunxi_sound_pcm_hw_params *params)
{
	struct sunxi_sound_adf_pcm_running_param *rtp = adf_dataflow_to_rtp(dataflow);
	struct sunxi_sound_adf_dai *cpu_dai;
	struct sunxi_sound_adf_dai *codec_dai;
	int i, ret = 0;

	sound_mutex_lock(rtp->card->pcm_mutex);

	if (rtp->dai_bind->ops &&
		rtp->dai_bind->ops->hw_params) {
		ret = rtp->dai_bind->ops->hw_params(dataflow, params);
		if (ret < 0) {
			snd_err("Adf: error on %s: %d\n",
					rtp->dai_bind->name, ret);
			goto err;
		}
	}

	for_each_rtp_codec_dais(rtp, i, codec_dai) {
		struct sunxi_sound_pcm_hw_params codec_params;

		if (!sunxi_sound_adf_dai_stream_valid(codec_dai, dataflow->stream))
			continue;

		/* copy params for each codec */
		codec_params = *params;

		ret = sunxi_sound_adf_dai_hw_params(codec_dai, dataflow,
						&codec_params);
		if(ret < 0)
			goto err;
	}

	for_each_rtp_cpu_dais(rtp, i, cpu_dai) {

		if (!sunxi_sound_adf_dai_stream_valid(cpu_dai, dataflow->stream))
			continue;

		ret = sunxi_sound_adf_dai_hw_params(cpu_dai, dataflow, params);
		if (ret < 0)
			goto err;

	}

	ret = sunxi_sound_adf_pcm_component_hw_params(dataflow, params);

err:
	sound_mutex_unlock(rtp->card->pcm_mutex);

	if (ret < 0) {
		sunxi_sound_adf_pcm_hw_clean(dataflow);
		snd_err("Adf: failed (%d)\n", ret);
	}
	return ret;
}


static int sunxi_sound_adf_pcm_trigger(struct sunxi_sound_pcm_dataflow *dataflow, int cmd)
{
	struct sunxi_sound_adf_pcm_running_param *rtp = adf_dataflow_to_rtp(dataflow);
	int ret = -EINVAL, _ret = 0;
	int err = 0;

	switch (cmd) {
		case SNDRV_PCM_TRIGGER_START:
		case SNDRV_PCM_TRIGGER_RESUME:
		case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
			if (rtp->dai_bind->ops &&
				rtp->dai_bind->ops->trigger) {
				ret = rtp->dai_bind->ops->trigger(dataflow, cmd);
				if (ret < 0) {
					snd_err("Adf: error on %s: %d\n",
							rtp->dai_bind->name, ret);
					goto _err;
				}
			}
			ret = sunxi_sound_adf_pcm_component_trigger(dataflow, cmd);
			if (ret < 0)
				goto _err;

			ret = sunxi_sound_adf_pcm_dai_trigger(dataflow, cmd);
_err:
			if (ret < 0)
				err = 1;
		}

	if (err) {
		_ret = ret;
		switch (cmd) {
		case SNDRV_PCM_TRIGGER_START:
			cmd = SNDRV_PCM_TRIGGER_STOP;
			break;
		case SNDRV_PCM_TRIGGER_RESUME:
			cmd = SNDRV_PCM_TRIGGER_SUSPEND;
			break;
		case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
			cmd = SNDRV_PCM_TRIGGER_PAUSE_PUSH;
			break;
		}
	}

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		if (rtp->dai_bind->stop_first_dma) {
			ret = sunxi_sound_adf_pcm_component_trigger(dataflow, cmd);
			if (ret < 0)
				break;

			ret = sunxi_sound_adf_pcm_dai_trigger(dataflow, cmd);
			if (ret < 0)
				break;
		} else {
			ret = sunxi_sound_adf_pcm_dai_trigger(dataflow, cmd);
			if (ret < 0)
				break;

			ret = sunxi_sound_adf_pcm_component_trigger(dataflow, cmd);
			if (ret < 0)
				break;
		}
		if (rtp->dai_bind->ops &&
				rtp->dai_bind->ops->trigger)
			ret = rtp->dai_bind->ops->trigger(dataflow, cmd);
		break;
	}

	if (_ret)
		ret = _ret;

	return ret;
}

static snd_pcm_uframes_t sunxi_sound_adf_pcm_pointer(struct sunxi_sound_pcm_dataflow *dataflow)
{
	snd_pcm_uframes_t offset = 0;

	offset = sunxi_sound_adf_pcm_component_pointer(dataflow);

	return offset;
}

int sunxi_sound_adf_pcm_dapm_control_total(struct sunxi_sound_pcm *pcm, int onoff)
{
	struct sunxi_sound_pcm_dataflow *dataflow;
	int i, err = 0;

	if (!pcm)
		return 0;

	for_each_pcm_dataflow(pcm, i, dataflow) {

		if (!dataflow->pcm_running)
			continue;

		if (!dataflow->ops)
			continue;

		err = sunxi_sound_adf_pcm_component_dapm_control(dataflow, onoff);
		if (err < 0 && err != -EBUSY)
			return err;

	}
	return 0;
}

static int sunxi_sound_adf_get_playback_capture(struct sunxi_sound_adf_pcm_running_param *rtp,
				    int *playback, int *capture)
{
	struct sunxi_sound_adf_dai *cpu_dai;
	struct sunxi_sound_adf_dai *codec_dai;
	int i;

	for_each_rtp_codec_dais(rtp, i, codec_dai) {
		if (rtp->num_cpus == 1) {
			cpu_dai = adf_rtp_to_cpu(rtp, 0);
		} else if (rtp->num_cpus == rtp->num_codecs) {
			cpu_dai = adf_rtp_to_cpu(rtp, i);
		} else {
			snd_err("N cpus to M codecs bind is not supported yet\n");
			return -EINVAL;
		}

		if (sunxi_sound_adf_dai_stream_valid(codec_dai, SNDRV_PCM_STREAM_PLAYBACK))
			*playback = 1;
		if (sunxi_sound_adf_dai_stream_valid(codec_dai, SNDRV_PCM_STREAM_CAPTURE))
			*capture = 1;
	}

	if (rtp->dai_bind->playback_only) {
		*playback = 1;
		*capture = 0;
	}

	if (rtp->dai_bind->capture_only) {
		*playback = 0;
		*capture = 1;
	}

	return 0;
}

static int sunxi_sound_adf_create_pcm(struct sunxi_sound_pcm **pcm,
			  struct sunxi_sound_adf_pcm_running_param *rtp,
			  int playback, int capture, int num)
{
	char new_name[64];
	int ret;

	snprintf(new_name, sizeof(new_name), "%s %s-%d",
				rtp->dai_bind->stream_name,
				adf_codec_dai_name(rtp), num);

	ret = sunxi_sound_new_pcm(rtp->card->sound_card, new_name, num, playback,
		capture, pcm);
	if (ret < 0) {
		snd_err("Adf: can't create pcm %s for dailink %s: %d\n",
			new_name, rtp->dai_bind->name, ret);
		return ret;
	}

	snd_debug("Adf: registered pcm #%d %s\n",num, new_name);

	return 0;

}

int sunxi_sound_adf_pcm_create(struct sunxi_sound_adf_pcm_running_param *rtp, int num)
{
	struct sunxi_sound_adf_component *component;
	struct sunxi_sound_pcm *pcm;
	int ret = 0, playback = 0, capture = 0;
	int i;

	ret = sunxi_sound_adf_get_playback_capture(rtp, &playback, &capture);
	if (ret < 0)
		return ret;

	ret = sunxi_sound_adf_create_pcm(&pcm, rtp, playback, capture, num);
	if (ret < 0)
		return ret;

	rtp->pcm = pcm;
	pcm->nonatomic = rtp->dai_bind->nonatomic;
	pcm->priv_data = rtp;

	rtp->ops.open		= sunxi_sound_adf_pcm_open;
	rtp->ops.hw_params	= sunxi_sound_adf_pcm_hw_params;
	rtp->ops.prepare	= sunxi_sound_adf_pcm_prepare;
	rtp->ops.trigger	= sunxi_sound_adf_pcm_trigger;
	rtp->ops.hw_free	= sunxi_sound_adf_pcm_hw_free;
	rtp->ops.close		= sunxi_sound_adf_pcm_close;
	rtp->ops.pointer	= sunxi_sound_adf_pcm_pointer;

	for_each_rtp_components(rtp, i, component) {
		const struct sunxi_sound_adf_component_driver *drv = component->driver;
		if (drv->ioctl)
			rtp->ops.ioctl	=sunxi_sound_adf_pcm_component_ioctl;
	}

	if (playback)
		sunxi_sound_set_pcm_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK, &rtp->ops);

	if (capture)
		sunxi_sound_set_pcm_ops(pcm, SNDRV_PCM_STREAM_CAPTURE, &rtp->ops);

	ret = sunxi_sound_adf_pcm_component_new(rtp);
	if (ret < 0)
		return ret;

	snd_info("%s <-> %s mapping ok\n",
		adf_codec_dai_name(rtp), adf_cpu_dai_name(rtp));
	return ret;
}



