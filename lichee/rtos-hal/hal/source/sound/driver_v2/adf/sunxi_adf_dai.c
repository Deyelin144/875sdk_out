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
#include <sound_v2/sunxi_adf_dai.h>


/* Config DAI master clock (MCLK) or system clock (SYSCLK) */
int sunxi_sound_adf_dai_set_sysclk(struct sunxi_sound_adf_dai *dai, int clk_id,
			   unsigned int freq, int dir)
{
	int ret = -SUNXI_ENOTSUPP;

	if (dai->driver->ops &&
		dai->driver->ops->set_sysclk) {
		ret = dai->driver->ops->set_sysclk(dai, clk_id, freq, dir);
		if (ret < 0) {
			snd_err("Adf:error on %s,:%d.\n",
				dai->name, ret);
			return ret;
		}
	}
	return 0;
}

/* Configures the clock dividers. */
int sunxi_sound_adf_dai_set_clkdiv(struct sunxi_sound_adf_dai *dai,
			   int div_id, int div)
{
	int ret = -EINVAL;

	if (dai->driver->ops &&
		dai->driver->ops->set_clkdiv) {
		ret = dai->driver->ops->set_clkdiv(dai, div_id, div);
		if (ret < 0) {
			snd_err("Adf:error on %s,:%d.\n",
				dai->name, ret);
			return ret;
		}
	}
	return 0;
}

/* Configures and enables PLL to generate output clock based on input clock. */
int sunxi_sound_adf_dai_set_pll(struct sunxi_sound_adf_dai *dai, int pll_id, int source,
			unsigned int freq_in, unsigned int freq_out)
{
	int ret = -1;

	if (dai->driver->ops &&
	    dai->driver->ops->set_pll) {
		ret = dai->driver->ops->set_pll(dai, pll_id, source,
						freq_in, freq_out);
		if (ret < 0) {
			snd_err("Adf:error on %s,:%d.\n",
				dai->name, ret);
			return ret;
		}
	}
	return 0;
}

/* Configures the DAI for a preset BCLK to sample rate ratio.*/
int sunxi_sound_adf_dai_set_bclk_ratio(struct sunxi_sound_adf_dai *dai, unsigned int ratio)
{
	int ret = -EINVAL;

	if (dai->driver->ops &&
	    dai->driver->ops->set_bclk_ratio) {
		ret = dai->driver->ops->set_bclk_ratio(dai, ratio);
		if (ret < 0) {
			snd_err("Adf:error on %s,:%d.\n",
				dai->name, ret);
			return ret;
		}
	}
	return 0;
}

int sunxi_sound_adf_dai_set_fmt(struct sunxi_sound_adf_dai *dai, unsigned int fmt)
{
	int ret = -SUNXI_ENOTSUPP;

	if (dai->driver->ops &&
	    dai->driver->ops->set_fmt) {
		ret = dai->driver->ops->set_fmt(dai, fmt);
		if (ret < 0) {
			snd_err("Adf:error on %s,:%d.\n",
				dai->name, ret);
			return ret;
		}
	}
	return 0;
}

int sunxi_sound_adf_set_tdm_slot(struct sunxi_sound_adf_dai *dai,
			     unsigned int tx_mask, unsigned int rx_mask,
			     int slots, int slot_width)
{
	int ret = -SUNXI_ENOTSUPP;

	if (dai->driver->ops &&
	    dai->driver->ops->set_tdm_slot) {
		ret = dai->driver->ops->set_tdm_slot(dai, tx_mask, rx_mask,
						      slots, slot_width);
		if (ret < 0) {
			snd_err("Adf:error on %s,:%d.\n",
				dai->name, ret);
			return ret;
		}
	}
	return 0;
}


int sunxi_sound_adf_dai_hw_params(struct sunxi_sound_adf_dai *dai,
			  struct sunxi_sound_pcm_dataflow *dataflow,
			  struct sunxi_sound_pcm_hw_params *params)
{
	int ret = 0;

	if (dai->driver->ops &&
	    dai->driver->ops->hw_params) {
		ret = dai->driver->ops->hw_params(dataflow, params, dai);
		if (ret < 0) {
			snd_err("Adf:error on %s,:%d.\n",
				dai->name, ret);
			return ret;
		}
	}
	return 0;

}

int sunxi_sound_adf_dai_hw_free(struct sunxi_sound_adf_dai *dai,
			  struct sunxi_sound_pcm_dataflow *dataflow)
{
	int ret = 0;

	if (dai->driver->ops &&
	    dai->driver->ops->hw_free) {
		ret = dai->driver->ops->hw_free(dataflow, dai);
		if (ret < 0) {
			snd_err("Adf:error on %s,:%d.\n",
				dai->name, ret);
			return ret;
		}
	}
	return 0;

}

int sunxi_sound_adf_dai_startup(struct sunxi_sound_adf_dai *dai, struct sunxi_sound_pcm_dataflow *dataflow)
{
	int ret = 0;

	if (dai->driver->ops &&
	    dai->driver->ops->startup) {
		ret = dai->driver->ops->startup(dataflow, dai);
		if (ret < 0) {
			snd_err("Adf:error on %s,:%d.\n",
				dai->name, ret);
			return ret;
		}
	}
	return 0;
}

void sunxi_sound_adf_dai_shutdown(struct sunxi_sound_adf_dai *dai, struct sunxi_sound_pcm_dataflow *dataflow)
{

	if (dai->driver->ops && dai->driver->ops->shutdown)
		dai->driver->ops->shutdown(dataflow, dai);
}

bool sunxi_sound_adf_dai_stream_valid(struct sunxi_sound_adf_dai *dai, int dir)
{
	struct sunxi_sound_adf_pcm_stream *stream = sunxi_sound_adf_dai_get_pcm_stream(dai, dir);

	/* If the codec specifies any channels at all, it supports the stream */
	return stream->channels_min;
}

int sunxi_sound_adf_pcm_dai_probe(struct sunxi_sound_adf_pcm_running_param *rtp)
{
	struct sunxi_sound_adf_dai *dai;
	int i;
	int ret;

	for_each_rtp_dais(rtp, i, dai) {

		if (dai->driver->probe) {
			ret = dai->driver->probe(dai);
			if (ret < 0) {
				snd_err("Adf:error on %s,:%d.\n",
					dai->name, ret);
				return ret;
			}
		}

		dai->probed = 1;
	}

	return 0;
}

int sunxi_sound_adf_pcm_dai_remove(struct sunxi_sound_adf_pcm_running_param *rtp)
{
	struct sunxi_sound_adf_dai *dai;
	int i, r, ret = 0;

	for_each_rtp_dais(rtp, i, dai) {

		if (dai->probed && dai->driver->remove) {
			r = dai->driver->remove(dai);
			if (r < 0)
				ret = r; /* use last error */
		}

		dai->probed = 0;
	}

	return ret;
}


int sunxi_sound_adf_pcm_dai_prepare(struct sunxi_sound_pcm_dataflow *dataflow)
{
	struct sunxi_sound_adf_pcm_running_param *rtp = adf_dataflow_to_rtp(dataflow);
	struct sunxi_sound_adf_dai *dai;
	int i, ret;

	for_each_rtp_dais(rtp, i, dai) {
		if (dai->driver->ops &&
		    dai->driver->ops->prepare) {
			ret = dai->driver->ops->prepare(dataflow, dai);
			if (ret < 0) {
				snd_err("Adf:error on %s,:%d.\n",
					dai->name, ret);
				return ret;
			}
		}
	}

	return 0;
}

int sunxi_sound_adf_pcm_dai_trigger(struct sunxi_sound_pcm_dataflow *dataflow, int cmd)
{
	struct sunxi_sound_adf_pcm_running_param *rtp = adf_dataflow_to_rtp(dataflow);
	struct sunxi_sound_adf_dai *dai;
	int i, ret;

	for_each_rtp_dais(rtp, i, dai) {
		if (dai->driver->ops &&
		    dai->driver->ops->trigger) {
			ret = dai->driver->ops->trigger(dataflow, cmd, dai);
			if (ret < 0) {
				snd_err("Adf:error on %s,:%d.\n",
					dai->name, ret);
				return ret;
			}
		}
	}

	return 0;
}


