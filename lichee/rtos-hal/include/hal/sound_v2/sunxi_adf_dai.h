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
#ifndef __SUNXI_ADF_DAI_H
#define __SUNXI_ADF_DAI_H

#include "aw_list.h"
//#include <sound_v2/sunxi_adf_core.h>

struct sunxi_sound_adf_dai;
struct sunxi_sound_adf_dai_driver;

struct sunxi_sound_adf_dai_ops {
	/*
	 * DAI clocking configuration, all optional.
	 * Called by soc_card drivers, normally in their hw_params.
	 */
	int (*set_sysclk)(struct sunxi_sound_adf_dai *dai,
		int clk_id, unsigned int freq, int dir);
	int (*set_pll)(struct sunxi_sound_adf_dai *dai, int pll_id, int source,
		unsigned int freq_in, unsigned int freq_out);
	int (*set_clkdiv)(struct sunxi_sound_adf_dai *dai, int div_id, int div);
	int (*set_bclk_ratio)(struct sunxi_sound_adf_dai *dai, unsigned int ratio);

	/*
	 * DAI format configuration
	 * Called by soc_card drivers, normally in their hw_params.
	 */
	int (*set_fmt)(struct sunxi_sound_adf_dai *dai, unsigned int fmt);
	int (*set_tdm_slot)(struct sunxi_sound_adf_dai *dai,
		unsigned int tx_mask, unsigned int rx_mask,
		int slots, int slot_width);

	/*
	 * PCM audio operations - all optional.
	 * Called by adf during audio PCM operations.
	 */
	int (*startup)(struct sunxi_sound_pcm_dataflow *,
		struct sunxi_sound_adf_dai *);
	void (*shutdown)(struct sunxi_sound_pcm_dataflow *,
		struct sunxi_sound_adf_dai *);
	int (*hw_params)(struct sunxi_sound_pcm_dataflow *,
		struct sunxi_sound_pcm_hw_params *, struct sunxi_sound_adf_dai *);
	int (*hw_free)(struct sunxi_sound_pcm_dataflow *,
		struct sunxi_sound_adf_dai *);
	int (*prepare)(struct sunxi_sound_pcm_dataflow *,
		struct sunxi_sound_adf_dai *);

	int (*trigger)(struct sunxi_sound_pcm_dataflow *, int,
		struct sunxi_sound_adf_dai *);

};


struct sunxi_sound_adf_dai_driver {
	const char *name;
	int id;
	/* DAI driver callbacks */
	int (*probe)(struct sunxi_sound_adf_dai *dai);
	int (*remove)(struct sunxi_sound_adf_dai *dai);
	const struct sunxi_sound_adf_dai_ops *ops;
	/* DAI capabilities */
	struct sunxi_sound_adf_pcm_stream capture;
	struct sunxi_sound_adf_pcm_stream playback;
	int probe_rank;
	int remove_rank;
};

struct sunxi_sound_adf_dai {
	const char *name;
	int id;
	/* driver ops */
	struct sunxi_sound_adf_dai_driver *driver;

	/* DAI DMA data */
	void *playback_dma_data;
	void *capture_dma_data;

	/* parent platform/codec */
	struct sunxi_sound_adf_component *component;

	struct list_head list;
	/* bit field */
	unsigned int probed:1;

};

/*
 * Master Clock Directions
 */
#define SUNXI_SOUND_ADF_CLOCK_IN		0
#define SUNXI_SOUND_ADF_CLOCK_OUT		1

int sunxi_sound_adf_dai_set_sysclk(struct sunxi_sound_adf_dai *dai, int clk_id,
			   unsigned int freq, int dir);

int sunxi_sound_adf_dai_set_clkdiv(struct sunxi_sound_adf_dai *dai,
			   int div_id, int div);

int sunxi_sound_adf_dai_set_pll(struct sunxi_sound_adf_dai *dai, int pll_id, int source,
			unsigned int freq_in, unsigned int freq_out);

int sunxi_sound_adf_dai_set_bclk_ratio(struct sunxi_sound_adf_dai *dai, unsigned int ratio);

int sunxi_sound_adf_set_tdm_slot(struct sunxi_sound_adf_dai *dai,
			     unsigned int tx_mask, unsigned int rx_mask,
			     int slots, int slot_width);

int sunxi_sound_adf_dai_set_fmt(struct sunxi_sound_adf_dai *dai, unsigned int fmt);

int sunxi_sound_adf_dai_hw_params(struct sunxi_sound_adf_dai *dai,
			  struct sunxi_sound_pcm_dataflow *dataflow,
			  struct sunxi_sound_pcm_hw_params *params);

int sunxi_sound_adf_dai_hw_free(struct sunxi_sound_adf_dai *dai,
			  struct sunxi_sound_pcm_dataflow *dataflow);

int sunxi_sound_adf_dai_startup(struct sunxi_sound_adf_dai *dai,
			struct sunxi_sound_pcm_dataflow *dataflow);


void sunxi_sound_adf_dai_shutdown(struct sunxi_sound_adf_dai *dai,
			struct sunxi_sound_pcm_dataflow *dataflow);

bool sunxi_sound_adf_dai_stream_valid(struct sunxi_sound_adf_dai *dai, int dir);


int sunxi_sound_adf_pcm_dai_probe(struct sunxi_sound_adf_pcm_running_param *rtp);
int sunxi_sound_adf_pcm_dai_remove(struct sunxi_sound_adf_pcm_running_param *rtp);

int sunxi_sound_adf_pcm_dai_prepare(struct sunxi_sound_pcm_dataflow *dataflow);

int sunxi_sound_adf_pcm_dai_trigger(struct sunxi_sound_pcm_dataflow *dataflow, int cmd);

static inline struct sunxi_sound_adf_pcm_stream *
sunxi_sound_adf_dai_get_pcm_stream(const struct sunxi_sound_adf_dai *dai, int stream)
{
	return (stream == SNDRV_PCM_STREAM_PLAYBACK) ?
		&dai->driver->playback : &dai->driver->capture;
}

static inline void *sunxi_sound_adf_dai_get_dma_data(const struct sunxi_sound_adf_dai *dai,
					     const struct sunxi_sound_pcm_dataflow *ss)
{
	return (ss->stream == SNDRV_PCM_STREAM_PLAYBACK) ?
		dai->playback_dma_data : dai->capture_dma_data;
}

static inline void sunxi_sound_adf_dai_set_dma_data(struct sunxi_sound_adf_dai *dai,
					    const struct sunxi_sound_pcm_dataflow *ss,
					    void *data)
{
	if (ss->stream == SNDRV_PCM_STREAM_PLAYBACK)
		dai->playback_dma_data = data;
	else
		dai->capture_dma_data = data;
}

static inline void sunxi_sound_adf_dai_init_dma_data(struct sunxi_sound_adf_dai *dai,
					     void *playback, void *capture)
{
	dai->playback_dma_data = playback;
	dai->capture_dma_data = capture;
}

#endif /* __SUNXI_ADF_CORE_H */
