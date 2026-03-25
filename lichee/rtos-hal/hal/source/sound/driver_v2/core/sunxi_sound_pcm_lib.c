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
#include <hal_cache.h>

#include <aw_common.h>
#include <sound_v2/sunxi_sound_pcm_common.h>
#include <sound_v2/sunxi_sound_core.h>
#include <sound_v2/sunxi_sound_pcm.h>
#include <sound_v2/sunxi_sound_pcm_misc.h>

#include "sunxi_sound_pcm_local.h"

/* #define XRUN_DEBUG */
#ifdef XRUN_DEBUG
#define sunxi_xrun_debug(fmt, args...) \
	printf(SNDRV_LOG_COLOR_BLUE "[XRUN_DEBUG][%s:%d]" fmt \
		SNDRV_LOG_COLOR_NONE, __FUNCTION__, __LINE__, ##args)
#else
#define sunxi_xrun_debug(fmt,arg...)
#endif


void sunxi_sound_set_pcm_ops(struct sunxi_sound_pcm *pcm, int direction,
				const struct sunxi_sound_pcm_ops *ops)
{
	struct sunxi_sound_pcm_data *pcm_data = &pcm->data[direction];
	struct sunxi_sound_pcm_dataflow *dataflow;

	for (dataflow = pcm_data->dataflow; dataflow != NULL; dataflow = dataflow->next)
		dataflow->ops = ops;
}

static void sunxi_xrun(struct sunxi_sound_pcm_dataflow *dataflow)
{
	struct sunxi_sound_pcm_running_param *pcm_running = dataflow->pcm_running;

	pcm_running->xrun_cnt++;
#ifdef CONFIG_ARCH_DSP
#include <awlog.h>
	printfFromISR("%s occure!!! xrun_cnt=%u\n",
			dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK ?
			"Underrun" : "Overrun",
			pcm_running->xrun_cnt);
#else
	snd_err("%s occure!!! xrun_cnt=%u\n",
			dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK ?
			"Underrun" : "Overrun",
			pcm_running->xrun_cnt);
#endif
	sunxi_sound_pcm_stop(dataflow, SNDRV_PCM_STATE_XRUN);

}

/*
 * mode, 0:clean+invalid, 1:invalid
 *
 */
static void do_align_dcache_control(int mode, unsigned long addr, unsigned int len)
{
	unsigned long align_value = 0;

	align_value = addr & (CACHELINE_LEN - 1);
	if (align_value) {
		addr &= ~(CACHELINE_LEN - 1);
		len += align_value;
	}
	if (len % CACHELINE_LEN != 0)
		len = ((len / CACHELINE_LEN) + 1) * CACHELINE_LEN;
	if (mode == 0)
		hal_dcache_clean_invalidate(addr, len);
	else
		hal_dcache_invalidate(addr, len);
	return;
}

void sunxi_sound_pcm_playback_silence(struct sunxi_sound_pcm_dataflow *dataflow, snd_pcm_uframes_t new_hw_ptr)
{
	struct sunxi_sound_pcm_running_param *pcm_running = dataflow->pcm_running;
	snd_pcm_uframes_t frames, ofs, transfer;

	if (pcm_running->silence_size < pcm_running->boundary) {
		snd_err("silence_size:%lu, boundary:%lu,"
			" not support silence_size < boundary!\n",
			pcm_running->silence_size, pcm_running->boundary);
		return;
	} else {
		if (new_hw_ptr == ULONG_MAX) {
			/* initialization, filled silence data into whole ringbuffer */
			snd_pcm_sframes_t avail = sunxi_sound_pcm_playback_hw_avail(pcm_running);

			if (avail > pcm_running->buffer_size)
				avail = pcm_running->buffer_size;
			pcm_running->silence_filled = avail > 0 ? avail : 0;
			pcm_running->silence_start = (pcm_running->status->hw_ptr +
						pcm_running->silence_filled) %
						pcm_running->boundary;
		} else {
			ofs = pcm_running->status->hw_ptr;
			frames = new_hw_ptr - ofs;
			/* cross boundary */
			if ((snd_pcm_sframes_t)frames < 0)
				frames += pcm_running->boundary;
			pcm_running->silence_filled -= frames;
			if ((snd_pcm_sframes_t)pcm_running->silence_filled < 0) {
				pcm_running->silence_filled = 0;
				pcm_running->silence_start = new_hw_ptr;
			} else {
				pcm_running->silence_start = ofs;
			}
		}
		frames = pcm_running->buffer_size - pcm_running->silence_filled;
	}
	if (frames > pcm_running->buffer_size) {
		sunxi_xrun_debug("frames:%lu,new_hw_ptr:%lu,hw_ptr:%lu,boundary:%lu,buffer_size:%lu\n",
			   frames, new_hw_ptr, ofs, pcm_running->boundary, pcm_running->buffer_size);
		return;
	}
	if (frames == 0)
		return;
	ofs = pcm_running->silence_start % pcm_running->buffer_size;
	while (frames > 0) {
		char *hwbuf = (char *)(pcm_running->dma_addr + frames_to_bytes(pcm_running, ofs));
		/* seperate twice to fill silence if cross ringbuffer */
		transfer = ofs + frames > pcm_running->buffer_size ? pcm_running->buffer_size - ofs : frames;
		if (pcm_running->access != SND_PCM_ACCESS_RW_INTERLEAVED &&
				pcm_running->access != SND_PCM_ACCESS_MMAP_INTERLEAVED) {
			snd_err("unsupport access:%d\n", pcm_running->access);
			return ;
		}

		sunxi_sound_pcm_format_set_silence(pcm_running->format, hwbuf, transfer * pcm_running->channels);
		/* flush cache */
		do_align_dcache_control(0, (unsigned long)hwbuf, frames_to_bytes(pcm_running, transfer));
		pcm_running->silence_filled += transfer;
		frames -= transfer;
		ofs = 0;
	}

	return;
}

int sunxi_sound_pcm_update_state(struct sunxi_sound_pcm_dataflow *dataflow,
			struct sunxi_sound_pcm_running_param *pcm_running)
{
	snd_pcm_uframes_t avail;

	avail = sunxi_sound_pcm_avail(dataflow);

	if (pcm_running->status->state == SNDRV_PCM_STATE_DRAINING) {
		snd_debug("\n");
		if (avail >= pcm_running->buffer_size) {
			sunxi_sound_pcm_stop(dataflow, SNDRV_PCM_STATE_SETUP);
			return -EPIPE;
		}
	} else {
		snd_debug("\n");
		/*
		 * available data larger than stop_thread(buffer_size)
		 * Playback: means pcm data in ringbuffer
		 * Capture:  pcm data full of ringbuffer
		 */
		if (avail >= pcm_running->stop_threshold) {
			snd_debug("avail=0x%lx, stop_threshold=0x%lx\n", avail, pcm_running->stop_threshold);
			sunxi_xrun(dataflow);
			return -EPIPE;
		}
	}
	if (pcm_running->twake) {
		if (avail >= pcm_running->twake)
			sound_schd_wakeup(pcm_running->tsleep);
	} else if (avail >= pcm_running->control->avail_min) {
		sound_schd_wakeup(pcm_running->sleep);
	}

	return 0;
}

static int sunxi_sound_pcm_update_hw_ptr0(struct sunxi_sound_pcm_dataflow *dataflow,
					unsigned int in_interrupt)
{
	struct sunxi_sound_pcm_running_param *pcm_running = dataflow->pcm_running;
	snd_pcm_uframes_t pos;
	snd_pcm_uframes_t old_hw_ptr, new_hw_ptr, hw_base;
	snd_pcm_sframes_t delta;

	snd_debug("\n");
	old_hw_ptr = pcm_running->status->hw_ptr;
	pos = dataflow->ops->pointer(dataflow);

	if (pos >= pcm_running->buffer_size) {
		snd_err("invalid position: pos = %ld, buffer size = %ld, period size = %ld\n",
			pos, pcm_running->buffer_size, pcm_running->period_size);
		pos = 0;
	}
	sunxi_xrun_debug("hw_ptr:0x%lx, appl_ptr:0x%lx, pos:0x%lx\n",
			pcm_running->status->hw_ptr, pcm_running->control->appl_ptr, pos);
	/* frame align */
	pos -= pos % pcm_running->min_align;
	hw_base = pcm_running->hw_ptr_base;
	new_hw_ptr = hw_base + pos;

	/*
	 * new_hw_ptr might be lower than old_hw_ptr in case
	 * when pointer cross the end of the ring buffer
	 */
	sunxi_xrun_debug("in_interrupt:%u, new_hw_ptr:0x%lx, old_hw_ptr:0x%lx, hw_base:0x%lx\n",
			in_interrupt, new_hw_ptr, old_hw_ptr, hw_base);
	if (new_hw_ptr < old_hw_ptr) {
		hw_base += pcm_running->buffer_size;
		if (hw_base >= pcm_running->boundary) {
			hw_base = 0;
		}
		new_hw_ptr = hw_base + pos;
	}
	sunxi_xrun_debug("new_hw_ptr:0x%lx, old_hw_ptr:0x%lx, hw_base:0x%lx\n", new_hw_ptr, old_hw_ptr, hw_base);

	delta = new_hw_ptr - old_hw_ptr;

	/* it means cross the end of boundary */
	if (delta < 0)
		delta += pcm_running->boundary;
	if (pcm_running->no_period_wakeup) {
		/* usually period wakeup(10s) */
	}

	/* something must be really wrong */
	if (delta >= pcm_running->buffer_size + pcm_running->period_size) {
		snd_err("Unexpected hw_ptr, stream:%d, pos:%ld, new_hw_ptr:%ld,old_hw_ptr:%ld\n",
			dataflow->stream, (long)pos, (long)new_hw_ptr, (long)old_hw_ptr);
		return 0;
	}

	if (delta > pcm_running->period_size + pcm_running->period_size / 2) {
		sunxi_xrun_debug("Lost interrupts? stream:%d, pos:%ld, new_hw_ptr:%ld,old_hw_ptr:%ld\n",
			dataflow->stream, (long)pos, (long)new_hw_ptr, (long)old_hw_ptr);
	}

	sunxi_xrun_debug("\n");
	if (pcm_running->status->hw_ptr == new_hw_ptr)
		return 0;

	if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK &&
		pcm_running->silence_size > 0)
		sunxi_sound_pcm_playback_silence(dataflow, new_hw_ptr);

	pcm_running->hw_ptr_base = hw_base;
	pcm_running->status->hw_ptr = new_hw_ptr;
	sunxi_xrun_debug("hw_ptr_base:0x%lx, hw_ptr:0x%lx\n", hw_base, new_hw_ptr);

	return sunxi_sound_pcm_update_state(dataflow, pcm_running);
}

int sunxi_sound_pcm_update_hw_ptr(struct sunxi_sound_pcm_dataflow *dataflow)
{
	return sunxi_sound_pcm_update_hw_ptr0(dataflow, 0);
}

static int sunxi_sound_pcm_lib_do_reset(struct sunxi_sound_pcm_dataflow *dataflow, void *arg)
{
	struct sunxi_sound_pcm_running_param *pcm_running = dataflow->pcm_running;
	hal_irq_state_t flags;

	sound_pcm_stream_lock_irqsave(dataflow, flags);
	if (sunxi_sound_pcm_running(dataflow) &&
		sunxi_sound_pcm_update_hw_ptr(dataflow) >= 0) {
		pcm_running->status->hw_ptr %= pcm_running->buffer_size;
	} else {
		pcm_running->status->hw_ptr = 0;
	}
	sound_pcm_stream_unlock_irqrestore(dataflow, flags);

	return 0;
}

int sunxi_sound_pcm_lib_ioctl(struct sunxi_sound_pcm_dataflow *dataflow,
				unsigned int cmd, void *arg)
{
	switch (cmd) {
	case SNDRV_PCM_IOCTL1_RESET:
		return sunxi_sound_pcm_lib_do_reset(dataflow, arg);
	case SNDRV_PCM_IOCTL1_CHANNEL_INFO:
		snd_err("unsupport SNDRV_PCM_IOCTL1_CHANNEL_INFO.\n");
		break;
	case SNDRV_PCM_IOCTL1_FIFO_SIZE:
		snd_err("unsupport SNDRV_PCM_IOCTL1_FIFO_SIZE.\n");
		break;
	}
	return -ENXIO;
}

typedef int (*sunxi_sound_pcm_transfer_f)(struct sunxi_sound_pcm_dataflow *dataflow,
			      int channel, unsigned long hwoff,
			      void *buf, unsigned long bytes);

typedef int (*sunxi_sound_pcm_copy_f)(struct sunxi_sound_pcm_dataflow *, snd_pcm_uframes_t, void *,
			  sunxi_sound_pcm_uframes_t, sunxi_sound_pcm_uframes_t, sunxi_sound_pcm_transfer_f);

/* calculate the target DMA-buffer position to be written/read */
static void *sunxi_sound_get_dma_ptr(struct sunxi_sound_pcm_running_param *pcm_running,
			   int channel, unsigned long hwoff)
{
	return pcm_running->dma_addr + hwoff +
		channel * (pcm_running->dma_bytes / pcm_running->channels);
}

static int default_write_copy(struct sunxi_sound_pcm_dataflow *dataflow,
				     int channel, unsigned long hwoff,
				     void *buf, unsigned long bytes)
{
	char *hwbuf = (char *)sunxi_sound_get_dma_ptr(dataflow->pcm_running, channel, hwoff);
	snd_debug("hwbuf:%p, bytes:%lu, frames:%lu\n",
			hwbuf,
			bytes,
			bytes_to_frames(dataflow->pcm_running, bytes));
	memcpy(hwbuf, buf, bytes);
	snd_debug("start:0x%x, end:0x%x, bytes:0x%x\n",
		hwbuf, hwbuf+bytes,
		bytes);
	/* flush cache */
	do_align_dcache_control(0, (unsigned long)hwbuf, bytes);
	return 0;
}

static int default_read_copy(struct sunxi_sound_pcm_dataflow *dataflow,
				    int channel, unsigned long hwoff,
				    void *buf, unsigned long bytes)
{
	char *hwbuf = (char *)sunxi_sound_get_dma_ptr(dataflow->pcm_running, channel, hwoff);
	snd_debug("hwbuf:%p, bytes:%lu, frames:%lu\n",
			hwbuf,
			bytes,
			bytes_to_frames(dataflow->pcm_running, bytes));
	do_align_dcache_control(1, (unsigned long)hwbuf, bytes);
	memcpy(buf, hwbuf, bytes);
	return 0;
}

/* fill silence instead of copy data; called as a transfer helper
 * from __snd_pcm_lib_write() or directly from noninterleaved_copy() when
 * a NULL buffer is passed
 */
static int sunxi_sound_fill_silence(struct sunxi_sound_pcm_dataflow *dataflow, int channel,
			unsigned long hwoff, void *buf, unsigned long bytes)
{
	struct sunxi_sound_pcm_running_param *pcm_running = dataflow->pcm_running;

	if (dataflow->stream != SNDRV_PCM_STREAM_PLAYBACK)
		return 0;

	sunxi_sound_pcm_format_set_silence(pcm_running->format,
				   sunxi_sound_get_dma_ptr(pcm_running, channel, hwoff),
				   bytes_to_samples(pcm_running, bytes));
	return 0;
}

/* call transfer function with the converted pointers and sizes;
 * for interleaved mode, it's one shot for all samples
 */
static int sunxi_sound_interleaved_copy(struct sunxi_sound_pcm_dataflow *dataflow,
			    snd_pcm_uframes_t hwoff, void *data,
			    snd_pcm_uframes_t off,
			    snd_pcm_uframes_t frames,
			    sunxi_sound_pcm_transfer_f transfer)
{
	struct sunxi_sound_pcm_running_param *pcm_running = dataflow->pcm_running;

	/* convert to bytes */
	hwoff = frames_to_bytes(pcm_running, hwoff);
	off = frames_to_bytes(pcm_running, off);
	frames = frames_to_bytes(pcm_running, frames);
	return transfer(dataflow, 0, hwoff, data + off, frames);
}

/* call transfer function with the converted pointers and sizes for each
 * non-interleaved channel; when buffer is NULL, silencing instead of copying
 */
static int sunxi_sound_noninterleaved_copy(struct sunxi_sound_pcm_dataflow *dataflow,
			       snd_pcm_uframes_t hwoff, void *data,
			       snd_pcm_uframes_t off,
			       snd_pcm_uframes_t frames,
			       sunxi_sound_pcm_transfer_f transfer)
{
	struct sunxi_sound_pcm_running_param *pcm_running = dataflow->pcm_running;
	int channels = pcm_running->channels;
	void **bufs = data;
	int c, err;

	/* convert to bytes; note that it's not frames_to_bytes() here.
	 * in non-interleaved mode, we copy for each channel, thus
	 * each copy is n_samples bytes x channels = whole frames.
	 */
	off = samples_to_bytes(pcm_running, off);
	frames = samples_to_bytes(pcm_running, frames);
	hwoff = samples_to_bytes(pcm_running, hwoff);
	for (c = 0; c < channels; ++c, ++bufs) {
		if (!data || !*bufs)
			err = sunxi_sound_fill_silence(dataflow, c, hwoff, NULL, frames);
		else
			err = transfer(dataflow, c, hwoff, *bufs + off,
				       frames);
		if (err < 0)
			return err;
	}
	return 0;
}

static void sunxi_sound_schd_wakeup_all(struct sunxi_sound_pcm_running_param *pcm_running)
{
	int i;

	sound_schd_wakeup(pcm_running->dsleep);
	for (i = 0; i < ARRAY_SIZE(pcm_running->dsleep_list); i++) {
		if (pcm_running->dsleep_list[i] != NULL)
			sound_schd_wakeup(pcm_running->dsleep_list[i]);
	}
}

void sunxi_sound_pcm_period_elapsed(struct sunxi_sound_pcm_dataflow *dataflow)
{
	struct sunxi_sound_pcm_running_param *pcm_running;
	hal_irq_state_t flags = 0;

	snd_debug("\n");
	pcm_running = dataflow->pcm_running;
	sound_pcm_stream_lock_irqsave(dataflow, flags);
	if (!sunxi_sound_pcm_running(dataflow) ||
		sunxi_sound_pcm_update_hw_ptr0(dataflow, 1) < 0)
		goto _end;
	/* TODO:timer interrupt for dmix */
	sunxi_sound_schd_wakeup_all(pcm_running);
_end:
	/* kill_fasync(&runtime->fasync, SIGIO, POLL_IN) */
	sound_pcm_stream_unlock_irqrestore(dataflow, flags);
	return;
}

static int sunxi_sound_wait_for_avail(struct sunxi_sound_pcm_dataflow *dataflow,
					snd_pcm_uframes_t *availp)
{
	struct sunxi_sound_pcm_running_param *pcm_running = dataflow->pcm_running;
	int is_playback = dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK;
	int err = 0;
	snd_pcm_uframes_t avail = 0;
	long wait_time, tout = 0;

	/* usually period wakeup(10s) */
	if (pcm_running->no_period_wakeup) {
		wait_time = LONG_MAX;
	} else {
		/* 10ms at lease */
		wait_time = 10;
		if (pcm_running->rate) {
			long t = pcm_running->period_size * 2 / pcm_running->rate;
			wait_time = max(t, wait_time);
		}
	}
	for (;;) {
		/* TODO: break by signal */
		avail = sunxi_sound_pcm_avail(dataflow);
		if (avail >= pcm_running->twake)
			break;
		snd_lock_debug("\n");
		/* lock by write/read process */
		sound_pcm_stream_unlock_irq(dataflow);
		tout = sound_schd_timeout(pcm_running->tsleep, wait_time*1000);
		snd_lock_debug("\n");
		sound_pcm_stream_lock_irq(dataflow);
		switch (pcm_running->status->state) {
		case SNDRV_PCM_STATE_SUSPENDED:
			err = -ESTRPIPE;
			goto _endloop;
		case SNDRV_PCM_STATE_XRUN:
			err = -EPIPE;
			goto _endloop;
		case SNDRV_PCM_STATE_DRAINING:
			if (is_playback)
				err = -EPIPE;
			else
				avail = 0;
			goto _endloop;
		case SNDRV_PCM_STATE_PAUSED:
			continue;
		case SNDRV_PCM_STATE_OPEN:
		case SNDRV_PCM_STATE_SETUP:
		case SNDRV_PCM_STATE_DISCONNECTED:
			snd_err("unsupport state transform.\n");
			err = -EBADFD;
			goto _endloop;
		default:
			break;
		}
		if (tout < 0) {
			snd_err("%s write error (DMA transfer data error?) tout %ld\n",
				is_playback ? "Playback" : "Capture", tout);
			err = -EIO;
			break;
		}
	}
_endloop:
	*availp = avail;
	return err;
}

static int sunxi_sound_pcm_accessible_state(struct sunxi_sound_pcm_running_param *pcm_running)
{
	switch (pcm_running->status->state) {
	case SNDRV_PCM_STATE_PREPARED:
	case SNDRV_PCM_STATE_RUNNING:
	case SNDRV_PCM_STATE_PAUSED:
		return 0;
	case SNDRV_PCM_STATE_XRUN:
		return -EPIPE;
	case SNDRV_PCM_STATE_SUSPENDED:
		return -ESTRPIPE;
	default:
		return -EBADFD;
	}
}

/* the common loop for read/write data */
snd_pcm_sframes_t __sunxi_sound_pcm_lib_xfer(struct sunxi_sound_pcm_dataflow *dataflow,
				     void *data, int interleaved,
				     snd_pcm_uframes_t size)
{
	struct sunxi_sound_pcm_running_param *pcm_running = dataflow->pcm_running;
	snd_pcm_uframes_t xfer = 0;
	snd_pcm_uframes_t offset = 0;
	snd_pcm_uframes_t avail;
	sunxi_sound_pcm_copy_f writer;
	sunxi_sound_pcm_transfer_f transfer;
	int nonblock;
	int is_playback;
	int err;

	if (dataflow == NULL || pcm_running == NULL)
		return -ENXIO;

	if (pcm_running->status->state == SNDRV_PCM_STATE_OPEN) {
		snd_err("unsupport state transform.\n");
		return -EBADFD;
	}

	is_playback = dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK;
	if (interleaved) {
		if (pcm_running->access != SND_PCM_ACCESS_RW_INTERLEAVED &&
		    pcm_running->channels > 1)
			return -EINVAL;
		writer = sunxi_sound_interleaved_copy;
	} else {
		if (pcm_running->access != SND_PCM_ACCESS_RW_NONINTERLEAVED)
			return -EINVAL;
		writer = sunxi_sound_noninterleaved_copy;
	}

	if (!data) {
		if (is_playback)
			transfer = sunxi_sound_fill_silence;
		else
			return -EINVAL;
	} else {
		if (dataflow->ops->copy)
			transfer = dataflow->ops->copy;
		else
			transfer = is_playback ?
				default_write_copy : default_read_copy;
	}

	if (size == 0)
		return 0;

	nonblock = !!(dataflow->mode & SND_DEV_NONBLOCK);

	sound_pcm_stream_lock_irq(dataflow);
	err = sunxi_sound_pcm_accessible_state(pcm_running);
	if (err < 0)
		goto _end_unlock;

	pcm_running->twake = pcm_running->control->avail_min ? : 1;
	if (pcm_running->status->state == SNDRV_PCM_STATE_RUNNING)
		sunxi_sound_pcm_update_hw_ptr(dataflow);

	/*
	 * If size < start_threshold, wait indefinitely. Another
	 * thread may start capture
	 */
	if (!is_playback &&
	    pcm_running->status->state == SNDRV_PCM_STATE_PREPARED &&
	    size >= pcm_running->start_threshold) {
		err = sunxi_sound_pcm_start(dataflow);
		if (err < 0)
			goto _end_unlock;
	}

	avail = sunxi_sound_pcm_avail(dataflow);

	while (size > 0) {
		snd_pcm_uframes_t frames, appl_ptr, appl_ofs;
		snd_pcm_uframes_t cont;
		if (!avail) {
			if (!is_playback &&
			    pcm_running->status->state == SNDRV_PCM_STATE_DRAINING) {
				sunxi_sound_pcm_stop(dataflow, SNDRV_PCM_STATE_SETUP);
				goto _end_unlock;
			}
			if (nonblock) {
				err = -EAGAIN;
				goto _end_unlock;
			}
			pcm_running->twake = min(size, pcm_running->control->avail_min);
			err = sunxi_sound_wait_for_avail(dataflow, &avail);
			if (err < 0)
				goto _end_unlock;
			if (!avail)
				continue; /* draining */
		}
		frames = size > avail ? avail : size;
		appl_ptr = pcm_running->control->appl_ptr;
		appl_ofs = appl_ptr % pcm_running->buffer_size;
		cont = pcm_running->buffer_size - appl_ofs;
		if (frames > cont)
			frames = cont;
		if (!frames) {
			err = -EINVAL;
			goto _end_unlock;
		}

		sound_pcm_stream_unlock_irq(dataflow);
		err = writer(dataflow, appl_ofs, data, offset, frames,
			     transfer);
		sound_pcm_stream_lock_irq(dataflow);

		err = sunxi_sound_pcm_accessible_state(pcm_running);
		if (err < 0)
			goto _end_unlock;
		appl_ptr += frames;
		if (appl_ptr >= pcm_running->boundary)
			appl_ptr -= pcm_running->boundary;

		pcm_running->control->appl_ptr = appl_ptr;

		offset += frames;
		size -= frames;
		xfer += frames;
		avail -= frames;
		if (is_playback &&
		    pcm_running->status->state == SNDRV_PCM_STATE_PREPARED &&
		    sunxi_sound_pcm_playback_hw_avail(pcm_running) >= (snd_pcm_sframes_t)pcm_running->start_threshold) {
			err = sunxi_sound_pcm_start(dataflow);
			if (err < 0)
				goto _end_unlock;
		}
	}
 _end_unlock:
	pcm_running->twake = 0;
	if (xfer > 0 && err >= 0)
		sunxi_sound_pcm_update_state(dataflow, pcm_running);
	sound_pcm_stream_unlock_irq(dataflow);
	return xfer > 0 ? (snd_pcm_sframes_t)xfer : err;
}

