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
#ifndef __SUNXI_ADF_CORE_H
#define __SUNXI_ADF_CORE_H

#include <errno.h>
#include <hal_sem.h>
#include <hal_mutex.h>
#include "aw_list.h"
#include <sound_v2/sunxi_sound_control.h>
#include <sound_v2/sunxi_sound_pcm.h>
#include <sound_v2/sunxi_sound_pcm_misc.h>


#define SUNXI_ERROEPROBE	517	/* Driver requests probe retry */

#define SUNXI_ENOTSUPP		524	/* Operation is not supported */


struct sunxi_sound_adf_pcm_stream {
	const char *stream_name;
	u64 formats;			/* SNDRV_PCM_FMTBIT_* */
	unsigned int rates;		/* SNDRV_PCM_RATE_* */
	unsigned int rate_min;		/* min rate */
	unsigned int rate_max;		/* max rate */
	unsigned int channels_min;	/* min channels */
	unsigned int channels_max;	/* max channels */
	unsigned int sig_bits;		/* number of bits of content */
};

/* Adf audio ops */
struct sunxi_sound_adf_ops {
	int (*startup)(struct sunxi_sound_pcm_dataflow *);
	void (*shutdown)(struct sunxi_sound_pcm_dataflow *);
	int (*hw_params)(struct sunxi_sound_pcm_dataflow *, struct sunxi_sound_pcm_hw_params *);
	int (*hw_free)(struct sunxi_sound_pcm_dataflow *);
	int (*prepare)(struct sunxi_sound_pcm_dataflow *);
	int (*trigger)(struct sunxi_sound_pcm_dataflow *, int);
};

struct sunxi_sound_adf_dai_bind_component {
	const char *name;
	const char *bind_name;
};

struct sunxi_sound_adf_dai_bind {
	const char *name;
	const char *stream_name;
	/* sunxi audio framework operations */
	const struct sunxi_sound_adf_ops *ops;
	struct sunxi_sound_adf_dai_bind_component *cpus;
	unsigned int num_cpus;
	struct sunxi_sound_adf_dai_bind_component *codecs;
	unsigned int num_codecs;
	struct sunxi_sound_adf_dai_bind_component *platforms;
	unsigned int num_platforms;
	unsigned int dai_fmt;           /* format to set on init */
	bool nonatomic; /* whole PCM operations are in non-atomic context */
	/* For unidirectional dai bind */
	unsigned int playback_only:1;
	unsigned int capture_only:1;
	unsigned int stop_first_dma:1;
};

static inline struct sunxi_sound_adf_dai_bind_component*
adf_bind_to_cpu(struct sunxi_sound_adf_dai_bind *bind, int n) {
	return &(bind)->cpus[n];
}

static inline struct sunxi_sound_adf_dai_bind_component*
adf_bind_to_codec(struct sunxi_sound_adf_dai_bind *bind, int n) {
	return &(bind)->codecs[n];
}

static inline struct sunxi_sound_adf_dai_bind_component*
adf_bind_to_platform(struct sunxi_sound_adf_dai_bind *bind, int n) {
	return &(bind)->platforms[n];
}

#define for_each_bind_codecs(bind, i, codec)				\
	for ((i) = 0;							\
	     ((i) < bind->num_codecs) &&				\
		     ((codec) = adf_bind_to_codec(bind, i));		\
	     (i)++)

#define for_each_bind_platforms(bind, i, platform)			\
	for ((i) = 0;							\
	     ((i) < bind->num_platforms) &&				\
		     ((platform) = adf_bind_to_platform(bind, i));	\
	     (i)++)

#define for_each_bind_cpus(bind, i, cpu)				\
	for ((i) = 0;							\
	     ((i) < bind->num_cpus) &&					\
		     ((cpu) = adf_bind_to_cpu(bind, i));		\
	     (i)++)

struct sunxi_sound_adf_card {
	const char *name;
	struct sunxi_sound_card *sound_card;
	sunxi_sound_mutex_t pcm_mutex;
	int (*probe)(struct sunxi_sound_adf_card *card);
	int (*remove)(struct sunxi_sound_adf_card *card);
	/* CPU <--> Codec DAI binds */
	struct sunxi_sound_adf_dai_bind *dai_bind;
	int num_binds;
	struct list_head rtp_list;
	int num_rtp;
	/* bit field */
	unsigned int inited:1;
	unsigned int probed:1;
	void *drvdata;
	void *pm_dev;
};


#define for_each_card_rtps(card, rtp)			\
	list_for_each_entry(rtp, &(card)->rtp_list, list)
#define for_each_card_rtps_safe(card, rtp, _rtp)	\
	list_for_each_entry_safe(rtp, _rtp, &(card)->rtp_list, list)

struct sunxi_sound_adf_pcm_running_param {
	struct sunxi_sound_adf_card *card;
	struct sunxi_sound_adf_dai_bind *dai_bind;
	struct sunxi_sound_pcm_ops ops;
	struct sunxi_sound_pcm *pcm;

	struct sunxi_sound_adf_dai **dais;
	unsigned int num_codecs;
	unsigned int num_cpus;

	unsigned int num; /* 0-based and monotonic increasing */
	struct list_head list; /* rtp list of the sound adf card */
	int num_components;

	struct sunxi_sound_adf_component *components[]; /* CPU/Codec/Platform */
};

#define adf_rtp_to_cpu(rtp, n)   (rtp)->dais[n]
#define adf_rtp_to_codec(rtp, n) (rtp)->dais[n + (rtp)->num_cpus]
#define adf_dataflow_to_rtp(dataflow) \
				(struct sunxi_sound_adf_pcm_running_param *)((dataflow)->priv_data)

#define for_each_rtp_components(rtp, i, component)			\
			for ((i) = 0, component = NULL; 				\
				 ((i) < rtp->num_components) && ((component) = rtp->components[i]);\
				 (i)++)
#define for_each_rtp_cpu_dais(rtp, i, dai)				\
			for ((i) = 0;							\
				 ((i) < rtp->num_cpus) && ((dai) = adf_rtp_to_cpu(rtp, i)); \
				 (i)++)
#define for_each_rtp_codec_dais(rtp, i, dai)				\
			for ((i) = 0;							\
				 ((i) < rtp->num_codecs) && ((dai) = adf_rtp_to_codec(rtp, i)); \
				 (i)++)
#define for_each_rtp_dais(rtp, i, dai)					\
			for ((i) = 0;							\
				 ((i) < (rtp)->num_cpus + (rtp)->num_codecs) && 	\
					 ((dai) = (rtp)->dais[i]);				\
				 (i)++)

#define for_each_card_prebinds(card, i, bind)				\
	for ((i) = 0;							\
	     ((i) < (card)->num_binds) && ((bind) = &(card)->dai_bind[i]); \
	     (i)++)

#include <sound_v2/sunxi_adf_component.h>
#include <sound_v2/sunxi_adf_dai.h>
#include <sound_v2/sunxi_adf_pcm.h>

int sunxi_sound_adf_register_card(struct sunxi_sound_adf_card *card);
int sunxi_sound_adf_unregister_card(struct sunxi_sound_adf_card *card);
int sunxi_sound_adf_register_component(const struct sunxi_sound_adf_component_driver *component_driver,
				struct sunxi_sound_adf_dai_driver *dai_drv,
				int num_dai);
void sunxi_sound_adf_unregister_component(const struct sunxi_sound_adf_component_driver *component_driver);
int sunxi_sound_adf_find_dai_name(const char *name, int id, const char **dai_name);
int sunxi_sound_adf_get_dai_bind_codecs(struct sunxi_sound_adf_dai_bind *dai_bind, int *id);

#endif /* __SUNXI_ADF_CORE_H */
