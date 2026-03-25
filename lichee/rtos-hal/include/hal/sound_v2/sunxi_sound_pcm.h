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

#ifndef __SUNXI_SOUND_PCM_H
#define __SUNXI_SOUND_PCM_H

#include <unistd.h>
#include <hal_sem.h>
#include <hal_mutex.h>
#include "hal_atomic.h"
#include <sunxi_hal_common.h>
#include "aw_list.h"

#include <sunxi_hal_sound.h>
#include <sound_v2/sunxi_sound_core.h>

struct sunxi_sound_pcm_hardware {
	uint32_t info;
	uint64_t formats;
	uint32_t rates;
	uint32_t rate_min;
	uint32_t rate_max;
	uint32_t channels_min;
	uint32_t channels_max;
	uint32_t buffer_bytes_max;
	uint32_t period_bytes_min;
	uint32_t period_bytes_max;
	uint32_t periods_min;
	uint32_t periods_max;
};

struct sunxi_sound_pcm;
struct sunxi_sound_pcm_data;
struct sunxi_sound_pcm_dataflow;
typedef void* __iomem;
typedef __iomem dma_addr_t;

#define SUNXI_SOUND_CMA_BLOCK_BYTES	1024
#define SUNXI_SOUND_CMA_MAX_KBYTES	128
#define SUNXI_SOUND_CMA_MIN_KBYTES	4
#define SUNXI_SOUND_CMA_MAX_BYTES	(SUNXI_SOUND_CMA_BLOCK_BYTES * SUNXI_SOUND_CMA_MAX_KBYTES)
#define SUNXI_SOUND_CMA_MIN_BYTES	(SUNXI_SOUND_CMA_BLOCK_BYTES * SUNXI_SOUND_CMA_MIN_KBYTES)

#define SUNXI_SOUND_FIFO_SIZE		128

struct sunxi_dma_params {
	char *name;
	dma_addr_t dma_addr;
	uint32_t src_maxburst;
	uint32_t dst_maxburst;
	uint8_t dma_drq_type_num;

	/* max buffer set (value must be (2^n)Kbyte) */
	size_t cma_kbytes;
};

struct sunxi_sound_dma_buffer {
	dma_addr_t addr;
	size_t bytes;
};

struct sunxi_sound_pcm_priv {
	struct sunxi_sound_pcm *pcm;
	struct sunxi_sound_pcm_dataflow *dataflow;
};

struct sunxi_sound_pcm_hw_rule;
typedef int (*sunxi_sound_pcm_hw_rule_func_t)(struct sunxi_sound_pcm_hw_params *params,
				      struct sunxi_sound_pcm_hw_rule *rule);

struct sunxi_sound_pcm_hw_rule {
	unsigned int cond;
	int var;
	int deps[4];
	sunxi_sound_pcm_hw_rule_func_t func;
	void *private_data;
};

struct sunxi_sound_pcm_hw_constrains {
	union snd_interval intervals[SND_PCM_HW_PARAM_LAST_INTERVAL - SND_PCM_HW_PARAM_FIRST_INTERVAL + 1];
	unsigned int rules_num;
	unsigned int rules_all;
	struct sunxi_sound_pcm_hw_rule *rules;
};

#define is_range_of_hw_constrains(param, cons, type) \
((param->intervals[type].range.min >= cons->intervals[type].range.min) && \
(param->intervals[type].range.min <= cons->intervals[type].range.max))

int snd_pcm_hw_rule_mul(struct sunxi_sound_pcm_hw_params *params, struct sunxi_sound_pcm_hw_rule *rule);
int snd_pcm_hw_rule_div(struct sunxi_sound_pcm_hw_params *params, struct sunxi_sound_pcm_hw_rule *rule);
int snd_pcm_hw_rule_muldivk(struct sunxi_sound_pcm_hw_params *params, struct sunxi_sound_pcm_hw_rule *rule);
int snd_pcm_hw_rule_mulkdiv(struct sunxi_sound_pcm_hw_params *params, struct sunxi_sound_pcm_hw_rule *rule);
int snd_pcm_hw_rule_format(struct sunxi_sound_pcm_hw_params *params, struct sunxi_sound_pcm_hw_rule *rule);
int snd_pcm_hw_rule_sample_bits(struct sunxi_sound_pcm_hw_params *params, struct sunxi_sound_pcm_hw_rule *rule);
int snd_pcm_hw_rule_rate(struct sunxi_sound_pcm_hw_params *params, struct sunxi_sound_pcm_hw_rule *rule);
int snd_pcm_hw_rule_buffer_bytes_max(struct sunxi_sound_pcm_hw_params *params, struct sunxi_sound_pcm_hw_rule *rule);

struct sunxi_sound_pcm_ops {
	int (*open)(struct sunxi_sound_pcm_dataflow *dataflow);
	int (*close)(struct sunxi_sound_pcm_dataflow *dataflow);
	int (*ioctl)(struct sunxi_sound_pcm_dataflow * dataflow,
		     unsigned int cmd, void *arg);
	int (*hw_params)(struct sunxi_sound_pcm_dataflow *dataflow,
			struct sunxi_sound_pcm_hw_params *params);
	int (*hw_free)(struct sunxi_sound_pcm_dataflow *dataflow);
	int (*prepare)(struct sunxi_sound_pcm_dataflow *dataflow);
	int (*trigger)(struct sunxi_sound_pcm_dataflow *dataflow, int cmd);
	snd_pcm_uframes_t (*pointer)(struct sunxi_sound_pcm_dataflow *dataflow);
	int (*copy)(struct sunxi_sound_pcm_dataflow *dataflow, int channel,
			snd_pcm_uframes_t pos,
			void *buf, snd_pcm_uframes_t count);
};

#define SND_SOC_DAIFMT_I2S              1 /* I2S mode */
#define SND_SOC_DAIFMT_RIGHT_J          2 /* Right Justified mode */
#define SND_SOC_DAIFMT_LEFT_J           3 /* Left Justified mode */
#define SND_SOC_DAIFMT_DSP_A            4 /* L data MSB after FRM LRC */
#define SND_SOC_DAIFMT_DSP_B            5 /* L data MSB during FRM LRC */
#define SND_SOC_DAIFMT_AC97             6 /* AC97 */
#define SND_SOC_DAIFMT_PDM              7 /* Pulse density modulation */

										 /* left and right justified also known as MSB and LSB respectively */
#define SND_SOC_DAIFMT_MSB              SND_SOC_DAIFMT_LEFT_J
#define SND_SOC_DAIFMT_LSB              SND_SOC_DAIFMT_RIGHT_J

#define	SND_SOC_DAIFMT_SIG_SHIFT		8
#define	SND_SOC_DAIFMT_MASTER_SHIFT		12

#define SND_SOC_DAIFMT_NB_NF            (1 << 8) /* normal bit clock + frame */
#define SND_SOC_DAIFMT_NB_IF            (2 << 8) /* normal BCLK + inv FRM */
#define SND_SOC_DAIFMT_IB_NF            (3 << 8) /* invert BCLK + nor FRM */
#define SND_SOC_DAIFMT_IB_IF            (4 << 8) /* invert BCLK + FRM */

#define SND_SOC_DAIFMT_CBM_CFM          (1 << 12) /* codec clk & FRM master */
#define SND_SOC_DAIFMT_CBS_CFM          (2 << 12) /* codec clk slave & FRM master */
#define SND_SOC_DAIFMT_CBM_CFS          (3 << 12) /* codec clk master & frame slave */
#define SND_SOC_DAIFMT_CBS_CFS          (4 << 12) /* codec clk & FRM slave */

#define SND_SOC_DAIFMT_FORMAT_MASK      0x000f
#define SND_SOC_DAIFMT_CLOCK_MASK       0x00f0
#define SND_SOC_DAIFMT_INV_MASK         0x0f00
#define SND_SOC_DAIFMT_MASTER_MASK      0xf000


#define SNDRV_PCM_IOCTL1_RESET		0
										 /* 1 is absent slot. */
#define SNDRV_PCM_IOCTL1_CHANNEL_INFO	2
										 /* 3 is absent slot. */
#define SNDRV_PCM_IOCTL1_FIFO_SIZE	4

#define SNDRV_PCM_TRIGGER_STOP          0
#define SNDRV_PCM_TRIGGER_START         1
#define SNDRV_PCM_TRIGGER_PAUSE_PUSH    3
#define SNDRV_PCM_TRIGGER_PAUSE_RELEASE 4
#define SNDRV_PCM_TRIGGER_SUSPEND       5
#define SNDRV_PCM_TRIGGER_RESUME        6
#define SNDRV_PCM_TRIGGER_DRAIN         7

#define SNDRV_PCM_RATE_5512             (1<<0)          /* 5512Hz */
#define SNDRV_PCM_RATE_8000             (1<<1)          /* 8000Hz */
#define SNDRV_PCM_RATE_11025            (1<<2)          /* 11025Hz */
#define SNDRV_PCM_RATE_16000            (1<<3)          /* 16000Hz */
#define SNDRV_PCM_RATE_22050            (1<<4)          /* 22050Hz */
#define SNDRV_PCM_RATE_32000            (1<<5)          /* 32000Hz */
#define SNDRV_PCM_RATE_44100            (1<<6)          /* 44100Hz */
#define SNDRV_PCM_RATE_48000            (1<<7)          /* 48000Hz */
#define SNDRV_PCM_RATE_64000            (1<<8)          /* 64000Hz */
#define SNDRV_PCM_RATE_88200            (1<<9)          /* 88200Hz */
#define SNDRV_PCM_RATE_96000            (1<<10)         /* 96000Hz */
#define SNDRV_PCM_RATE_176400           (1<<11)         /* 176400Hz */
#define SNDRV_PCM_RATE_192000           (1<<12)         /* 192000Hz */

#define SNDRV_PCM_RATE_CONTINUOUS       (1<<30)         /* continuous range */
#define SNDRV_PCM_RATE_KNOT             (1<<31)         /* supports more non-continuos rates */

#define SNDRV_PCM_RATE_8000_44100       (SNDRV_PCM_RATE_8000|SNDRV_PCM_RATE_11025|\
                                         SNDRV_PCM_RATE_16000|SNDRV_PCM_RATE_22050|\
                                         SNDRV_PCM_RATE_32000|SNDRV_PCM_RATE_44100)
#define SNDRV_PCM_RATE_8000_48000       (SNDRV_PCM_RATE_8000_44100|SNDRV_PCM_RATE_48000)
#define SNDRV_PCM_RATE_8000_96000       (SNDRV_PCM_RATE_8000_48000|SNDRV_PCM_RATE_64000|\
                                         SNDRV_PCM_RATE_88200|SNDRV_PCM_RATE_96000)
#define SNDRV_PCM_RATE_8000_192000      (SNDRV_PCM_RATE_8000_96000|SNDRV_PCM_RATE_176400|\
                                         SNDRV_PCM_RATE_192000)

#define	SNDRV_PCM_FORMAT_S8	((snd_pcm_format_t) 0)
#define	SNDRV_PCM_FORMAT_U8	((snd_pcm_format_t) 1)
#define	SNDRV_PCM_FORMAT_S16_LE	((snd_pcm_format_t) 2)
#define	SNDRV_PCM_FORMAT_S16_BE	((snd_pcm_format_t) 3)
#define	SNDRV_PCM_FORMAT_U16_LE	((snd_pcm_format_t) 4)
#define	SNDRV_PCM_FORMAT_U16_BE	((snd_pcm_format_t) 5)
#define	SNDRV_PCM_FORMAT_S24_LE	((snd_pcm_format_t) 6)
#define	SNDRV_PCM_FORMAT_S24_BE	((snd_pcm_format_t) 7)
#define	SNDRV_PCM_FORMAT_U24_LE	((snd_pcm_format_t) 8)
#define	SNDRV_PCM_FORMAT_U24_BE	((snd_pcm_format_t) 9)
#define	SNDRV_PCM_FORMAT_S32_LE	((snd_pcm_format_t) 10)
#define	SNDRV_PCM_FORMAT_S32_BE	((snd_pcm_format_t) 11)
#define	SNDRV_PCM_FORMAT_U32_LE	((snd_pcm_format_t) 12)
#define	SNDRV_PCM_FORMAT_U32_BE	((snd_pcm_format_t) 13)

#define _SNDRV_PCM_FMTBIT(fmt)          (1ULL << (int)SND_PCM_FORMAT_##fmt)
#define SNDRV_PCM_FMTBIT_S8             _SNDRV_PCM_FMTBIT(S8)
#define SNDRV_PCM_FMTBIT_U8             _SNDRV_PCM_FMTBIT(U8)
#define SNDRV_PCM_FMTBIT_S16_LE         _SNDRV_PCM_FMTBIT(S16_LE)
#define SNDRV_PCM_FMTBIT_S16_BE         _SNDRV_PCM_FMTBIT(S16_BE)
#define SNDRV_PCM_FMTBIT_U16_LE         _SNDRV_PCM_FMTBIT(U16_LE)
#define SNDRV_PCM_FMTBIT_U16_BE         _SNDRV_PCM_FMTBIT(U16_BE)
#define SNDRV_PCM_FMTBIT_S24_LE         _SNDRV_PCM_FMTBIT(S24_LE)
#define SNDRV_PCM_FMTBIT_S24_BE         _SNDRV_PCM_FMTBIT(S24_BE)
#define SNDRV_PCM_FMTBIT_U24_LE         _SNDRV_PCM_FMTBIT(U24_LE)
#define SNDRV_PCM_FMTBIT_U24_BE         _SNDRV_PCM_FMTBIT(U24_BE)
#define SNDRV_PCM_FMTBIT_S32_LE         _SNDRV_PCM_FMTBIT(S32_LE)
#define SNDRV_PCM_FMTBIT_S32_BE         _SNDRV_PCM_FMTBIT(S32_BE)
#define SNDRV_PCM_FMTBIT_U32_LE         _SNDRV_PCM_FMTBIT(U32_LE)
#define SNDRV_PCM_FMTBIT_U32_BE         _SNDRV_PCM_FMTBIT(U32_BE)

struct sunxi_sound_pcm_running_param {
	snd_pcm_uframes_t hw_ptr_base;
	snd_pcm_access_t access;
	snd_pcm_format_t format;
	unsigned int rate;
	unsigned int channels;
	unsigned int can_paused;
	snd_pcm_uframes_t period_size;
	unsigned int periods;
	snd_pcm_uframes_t buffer_size;
	snd_pcm_uframes_t min_align;		/* Min alignment for the format, frame align */
	unsigned int frame_bits;
	unsigned int sample_bits;
	unsigned int no_period_wakeup: 1;

	dma_addr_t dma_addr;
	size_t dma_bytes;

	struct sunxi_sound_dma_buffer *dma_buffer_p;

	/* private section */
	void *private_data;

	struct sunxi_sound_pcm_mmap_status *status;
	struct sunxi_sound_pcm_mmap_control *control;

	/* HW constrains */
	struct sunxi_sound_pcm_hardware hw;
	struct sunxi_sound_pcm_hw_constrains hw_constrains;

	/* Other params */
	snd_pcm_uframes_t start_threshold;
	snd_pcm_uframes_t stop_threshold;
	snd_pcm_uframes_t silence_size;

	snd_pcm_uframes_t silence_start;
	snd_pcm_uframes_t silence_filled;

	snd_pcm_uframes_t boundary;

	/* locking / scheduling */
	snd_pcm_uframes_t twake;
	sunxi_sound_schd_t tsleep;   /* transfer sleep */
	sunxi_sound_schd_t sleep;    /* poll sleep (drain...) */
	sunxi_sound_schd_t dsleep;	/* direct access sleep */

	sunxi_sound_schd_t dsleep_list[32];

	unsigned int xrun_cnt;

	sunxi_sound_mutex_t pcm_mutex;
};

struct sunxi_sound_pcm_dataflow {
	struct sunxi_sound_pcm *pcm;
	struct sunxi_sound_pcm_data *pcm_data;
	void *priv_data;		/* copied from pcm->priv_data */
	char name[32];
	int stream;
	int number;
	struct sunxi_sound_pcm_running_param *pcm_running;
	const struct sunxi_sound_pcm_ops *ops;
	struct sunxi_sound_dma_buffer dma_buffer;
	hal_spinlock_t lock;
	hal_irq_state_t irqstate;
	sunxi_sound_mutex_t mutex;
	/* -- next dataflow -- */
	struct sunxi_sound_pcm_dataflow *next;
	int ref_count;
	unsigned int mode;
	int dapm_state;
	int hw_opened;
};

struct sunxi_sound_pcm_data {
	int stream;
	struct sunxi_sound_pcm *pcm;
	unsigned int dataflow_count;
	unsigned int dataflow_opened;
	struct sunxi_sound_pcm_dataflow *dataflow;
	struct sound_device pcm_dev;
};

struct sunxi_sound_pcm {
	struct sunxi_sound_card *card;
	char name[48];
	char id[64];
	int device_num;
	struct sunxi_sound_pcm_data data[2];
	struct list_head list;
	void *priv_data;
	void (*priv_free) (struct sunxi_sound_pcm *pcm);
	sunxi_sound_mutex_t open_mutex;
	bool nonatomic; /* whole PCM operations are in non-atomic context */
	bool nonatomic_irqsave; /* whole PCM operations are in non-atomic irq context */
};

int sunxi_sound_pcm_start(void *dataflow_handle);
int sunxi_sound_pcm_stop(struct sunxi_sound_pcm_dataflow *dataflow, sunxi_sound_pcm_state_t state);


int sunxi_sound_new_pcm(struct sunxi_sound_card *card, const char *id, int device,
		int playback_count, int capture_count, struct sunxi_sound_pcm **rpcm);

void sound_pcm_stream_lock_irq(struct sunxi_sound_pcm_dataflow *dataflow);
void sound_pcm_stream_unlock_irq(struct sunxi_sound_pcm_dataflow *dataflow);

hal_irq_state_t _sound_pcm_stream_lock_irqsave(struct sunxi_sound_pcm_dataflow *dataflow);
void sound_pcm_stream_unlock_irqrestore(struct sunxi_sound_pcm_dataflow *dataflow,
					unsigned long flags);

#define sound_pcm_stream_lock_irqsave(dataflow, flags) \
do { \
	flags = _sound_pcm_stream_lock_irqsave(dataflow); \
} while (0)

static inline int sunxi_sound_pcm_running(struct sunxi_sound_pcm_dataflow *dataflow)
{
	return (dataflow->pcm_running->status->state == SNDRV_PCM_STATE_RUNNING ||
		(dataflow->pcm_running->status->state == SNDRV_PCM_STATE_DRAINING &&
		 dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK));
}

/**
 * bytes_to_samples - Unit conversion of the size from bytes to samples
 * @runtime: PCM runtime instance
 * @size: size in bytes
 */
static inline ssize_t bytes_to_samples(struct sunxi_sound_pcm_running_param *runtime, ssize_t size)
{
	return size * 8 / runtime->sample_bits;
}

static inline snd_pcm_sframes_t bytes_to_frames(struct sunxi_sound_pcm_running_param *runtime, ssize_t size)
{
	return size * 8 / runtime->frame_bits;
}

/**
 * samples_to_bytes - Unit conversion of the size from samples to bytes
 * @runtime: PCM runtime instance
 * @size: size in samples
 */
static inline ssize_t samples_to_bytes(struct sunxi_sound_pcm_running_param *runtime, ssize_t size)
{
	return size * runtime->sample_bits / 8;
}

static inline ssize_t frames_to_bytes(struct sunxi_sound_pcm_running_param *runtime, snd_pcm_sframes_t size)
{
	return size * runtime->frame_bits / 8;
}

/* Get the available(readable) space for capture */
static inline snd_pcm_uframes_t sunxi_sound_pcm_capture_avail(struct sunxi_sound_pcm_running_param *pcm_running)
{
	snd_pcm_sframes_t avail = pcm_running->status->hw_ptr - pcm_running->control->appl_ptr;
	if (avail < 0)
		avail += pcm_running->boundary;
	return avail;
}

static inline size_t sunxi_sound_pcm_lib_buffer_bytes(struct sunxi_sound_pcm_dataflow *dataflow)
{
	struct sunxi_sound_pcm_running_param *pcm_running = dataflow->pcm_running;
	return frames_to_bytes(pcm_running, pcm_running->buffer_size);
}

static inline size_t sunxi_sound_pcm_lib_period_bytes(struct sunxi_sound_pcm_dataflow *dataflow)
{
	struct sunxi_sound_pcm_running_param *pcm_running = dataflow->pcm_running;
	return frames_to_bytes(pcm_running, pcm_running->period_size);
}

/* Get the available(writeable) space for playback */
static inline snd_pcm_uframes_t sunxi_sound_pcm_playback_avail(struct sunxi_sound_pcm_running_param *pcm_running)
{
	snd_pcm_sframes_t avail = pcm_running->status->hw_ptr + pcm_running->buffer_size - pcm_running->control->appl_ptr;

	if (avail < 0)
		avail += pcm_running->boundary;
	else if ((snd_pcm_uframes_t) avail >= pcm_running->boundary)
		avail -= pcm_running->boundary;

	return avail;
}

/* Get the queued space(has been written) for playback */
static inline snd_pcm_sframes_t sunxi_sound_pcm_playback_hw_avail(struct sunxi_sound_pcm_running_param *pcm_running)
{
	return pcm_running->buffer_size - sunxi_sound_pcm_playback_avail(pcm_running);
}

static inline int sunxi_sound_pcm_playback_data(struct sunxi_sound_pcm_dataflow *dataflow)
{
	struct sunxi_sound_pcm_running_param *pcm_running = dataflow->pcm_running;
/*
	snd_debug("st %lu b %lu hw_p %lu bz %lu ap %lu av %lu\n", pcm_running->stop_threshold, pcm_running->boundary,
	    pcm_running->status->hw_ptr, pcm_running->buffer_size, pcm_running->control->appl_ptr, sunxi_sound_pcm_playback_avail(pcm_running));
*/
	if (pcm_running->stop_threshold >= pcm_running->boundary)
		return 1;

	return sunxi_sound_pcm_playback_avail(pcm_running) < pcm_running->buffer_size;
}

static inline int sunxi_sound_pcm_playback_empty(struct sunxi_sound_pcm_dataflow *dataflow)
{
	struct sunxi_sound_pcm_running_param *pcm_running = dataflow->pcm_running;
	return sunxi_sound_pcm_playback_avail(pcm_running) >= pcm_running->buffer_size;
}


void sunxi_sound_set_pcm_ops(struct sunxi_sound_pcm *pcm, int direction,
				const struct sunxi_sound_pcm_ops *ops);

int sunxi_sound_pcm_lib_ioctl(struct sunxi_sound_pcm_dataflow *dataflow,
				unsigned int cmd, void *arg);
void sunxi_sound_pcm_period_elapsed(struct sunxi_sound_pcm_dataflow *dataflow);
snd_pcm_sframes_t __sunxi_sound_pcm_lib_xfer(struct sunxi_sound_pcm_dataflow *dataflow,
				     void *data, int interleaved,
				     snd_pcm_uframes_t size);


static inline snd_pcm_sframes_t
sunxi_sound_pcm_lib_write(struct sunxi_sound_pcm_dataflow *dataflow,
		  const void  *buf, snd_pcm_uframes_t frames)
{
	return __sunxi_sound_pcm_lib_xfer(dataflow, (void *)buf, true, frames);
}

static inline snd_pcm_sframes_t
sunxi_sound_pcm_lib_read(struct sunxi_sound_pcm_dataflow *dataflow,
		 void *buf, snd_pcm_uframes_t frames)
{
	return __sunxi_sound_pcm_lib_xfer(dataflow, (void *)buf, true, frames);
}


static inline void sunxi_sound_pcm_set_runtime_buffer(struct sunxi_sound_pcm_dataflow *dataflow,
						struct sunxi_sound_dma_buffer *bufp)
{
	struct sunxi_sound_pcm_running_param *pcm_running = dataflow->pcm_running;

	if (bufp) {
		pcm_running->dma_buffer_p = bufp;
		pcm_running->dma_addr = bufp->addr;
		pcm_running->dma_bytes = bufp->bytes;
	} else {
		pcm_running->dma_buffer_p = NULL;
		pcm_running->dma_addr = 0;
		pcm_running->dma_bytes = 0;
	}

	snd_debug("runtime dma buffer addr %p size %lu\n", pcm_running->dma_addr, pcm_running->dma_bytes);
}

#define for_each_pcm_dataflow(pcm, str, dataflow) \
	for ((str) = 0; (str) < 2; (str)++) \
		for ((dataflow) = (pcm)->data[str].dataflow; (dataflow); \
		     (dataflow) = (dataflow)->next)


int sunxi_sound_pcm_suspend_total(struct sunxi_sound_pcm *pcm);
int sunxi_sound_pcm_resume_total(struct sunxi_sound_pcm *pcm);

#endif /* __SUNXI_SOUND_PCM_H */
