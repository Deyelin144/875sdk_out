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
#include <hal_osal.h>
#include <interrupt.h>
#include <hal_sem.h>
#include <hal_mem.h>
#include <hal_mutex.h>

#include <aw_common.h>
#include <sound_v2/sunxi_sound_core.h>
#include <sound_v2/sunxi_sound_pcm.h>
#include <sound_v2/sunxi_sound_pcm_common.h>
#include "sunxi_sound_pcm_local.h"

sunxi_sound_mutex_t sound_mutex_init(void)
{
	sunxi_sound_mutex_t hal_mutex = NULL;

	hal_mutex = hal_mutex_create();
	if (hal_mutex == NULL) {
		snd_err("hal_mutex_create err.\n");
	}
	return hal_mutex;
}

int sound_mutex_lock_timeout(sunxi_sound_mutex_t mutex, long ms)
{
	long ret;
#ifdef CONFIG_KERNEL_FREERTOS
		const uint32_t timeout = ms / (1000 / CONFIG_HZ);
#elif defined(CONFIG_OS_NUTTX)
		const uint32_t timeout = MSEC2TICK(ms);
#endif
	if (!mutex)
		return -1;
	snd_debug("\n");
	ret = hal_mutex_timedwait(mutex, timeout);

	return ret;
}

int sound_mutex_lock(sunxi_sound_mutex_t mutex)
{
	int ret;
	if (!mutex)
		return -1;
	snd_debug("\n");
	//ret = hal_mutex_timedwait(mutex, 100);
	ret = hal_mutex_lock(mutex);

	return ret;
}

void sound_mutex_unlock(sunxi_sound_mutex_t mutex)
{
	if (!mutex)
		return;
	snd_debug("\n");
	hal_mutex_unlock(mutex);
}

void sound_mutex_destroy(sunxi_sound_mutex_t mutex)
{
	if (!mutex)
		return;
	hal_mutex_delete(mutex);
}

sunxi_sound_schd_t sound_schd_init(void)
{
	sunxi_sound_schd_t schd;

	schd = sound_malloc(sizeof(*schd));
	if (!schd)
		return NULL;
	schd->waiting = 0;
	schd->sem = hal_sem_create(0);
	if (schd->sem == NULL) {
		sound_free(schd);
		snd_err("sem init err.\n");
		return NULL;
	}
	return schd;
}

int sound_schd_timeout(sunxi_sound_schd_t schd, long ms)
{
	int ret;
#ifdef CONFIG_KERNEL_FREERTOS
	const uint32_t timeout = ms / (1000 / CONFIG_HZ);
#elif defined(CONFIG_OS_NUTTX)
	const uint32_t timeout = MSEC2TICK(ms);
#endif
	if (!schd)
		return -1;
	snd_debug("\n");
	schd->waiting = 1;
	ret = hal_sem_timedwait(schd->sem, timeout);
	schd->waiting = 0;
	if (ret == 0)
		return 0;
	return -1;
}

void sound_schd_wakeup(sunxi_sound_schd_t schd)
{
	int ret;
	/* may call with irq disabled, so use xSemaphoreGiveFromISR */
	if (!schd || !schd->waiting)
		return;
	snd_debug("\n");
	ret = hal_sem_post(schd->sem);
	if (ret == 0) {
		return;
	}
}

void sound_schd_destroy(sunxi_sound_schd_t schd)
{
	if (!schd)
		return;
	snd_debug("\n");
	hal_sem_delete(schd->sem);
	sound_free(schd);
}

static void sound_pcm_stream_lock(struct sunxi_sound_pcm_dataflow *dataflow)
{
	if (dataflow->pcm->nonatomic)
		hal_mutex_lock(dataflow->mutex);
	else
		hal_spin_lock(&dataflow->lock);
}

static void sound_pcm_stream_unlock(struct sunxi_sound_pcm_dataflow *dataflow)
{
	if (dataflow->pcm->nonatomic)
		hal_mutex_unlock(dataflow->mutex);
	else
		hal_spin_unlock(&dataflow->lock);
}

void sound_pcm_stream_lock_irq(struct sunxi_sound_pcm_dataflow *dataflow)
{
	snd_lock_debug("\n");
	if (dataflow->pcm->nonatomic)
		sound_pcm_stream_lock(dataflow);
	else {
		hal_local_irq_save(dataflow->irqstate);
		sound_pcm_stream_lock(dataflow);
	}
}

void sound_pcm_stream_unlock_irq(struct sunxi_sound_pcm_dataflow *dataflow)
{
	snd_lock_debug("\n");
	if (dataflow->pcm->nonatomic)
		sound_pcm_stream_unlock(dataflow);
	else {
		sound_pcm_stream_unlock(dataflow);
		hal_local_irq_restore(dataflow->irqstate);
	}
}

hal_irq_state_t _sound_pcm_stream_lock_irqsave(struct sunxi_sound_pcm_dataflow *dataflow)
{
	hal_irq_state_t flags = 0;

	snd_lock_debug("\n");
	if (dataflow->pcm->nonatomic_irqsave)
		hal_mutex_lock(dataflow->mutex);
	else {
		hal_local_irq_save(flags);
		hal_spin_lock(&dataflow->lock);
	}
	return flags;
}

void sound_pcm_stream_unlock_irqrestore(struct sunxi_sound_pcm_dataflow *dataflow,
					unsigned long flags)
{
	snd_lock_debug("\n");
	if (dataflow->pcm->nonatomic_irqsave)
		hal_mutex_unlock(dataflow->mutex);
	else {
		hal_spin_unlock(&dataflow->lock);
		hal_local_irq_restore(flags);
	}
}

char *sunxi_sound_strdup(const char *s)
{
	size_t len;
	char *buf;

	if (!s)
		return NULL;

	len = strlen(s) + 1;
	buf = sound_malloc(len);
	if (buf)
		memcpy(buf, s, len);
	return buf;
}

const char *sunxi_sound_strdup_const(const char *s)
{
	return sunxi_sound_strdup(s);
}

#ifdef CONFIG_COMPONENTS_PM

static int sunxi_sound_pcm_suspend(struct sunxi_sound_pcm_dataflow *dataflow)
{
	int err;
	hal_irq_state_t flags;
	struct sunxi_sound_pcm_running_param *pcm_running = dataflow->pcm_running;

	sound_pcm_stream_lock_irqsave(dataflow, flags);

	switch (pcm_running->status->state) {
		case SNDRV_PCM_STATE_SUSPENDED:
			err  = -EBUSY;
			goto err;
		/* unresumable PCM state; return -EBUSY for skipping suspend */
		case SNDRV_PCM_STATE_OPEN:
		case SNDRV_PCM_STATE_SETUP:
		case SNDRV_PCM_STATE_DISCONNECTED:
			err  = -EBUSY;
			goto err;
		default:
			break;
	}

	if (!sunxi_sound_pcm_running(dataflow)) {
		err  = 0;
		goto err;
	}
	err = dataflow->ops->trigger(dataflow, SNDRV_PCM_TRIGGER_SUSPEND);
	if (err) {
		snd_err("pcm %s: trigger suspend failed ret:%d.\n",
				dataflow->name, err);
	}

	pcm_running->status->state_suspend = pcm_running->status->state;
	pcm_running->status->state = SNDRV_PCM_STATE_SUSPENDED;
err:
	sound_pcm_stream_unlock_irqrestore(dataflow, flags);

	return err;
}

int sunxi_sound_pcm_suspend_total(struct sunxi_sound_pcm *pcm)
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

		err = sunxi_sound_pcm_suspend(dataflow);
		if (err < 0 && err != -EBUSY)
			return err;

	}
	return 0;
}

static int sunxi_sound_pcm_resume(struct sunxi_sound_pcm_dataflow *dataflow)
{
	int res;
	struct sunxi_sound_pcm_running_param *pcm_running = dataflow->pcm_running;

	sound_pcm_stream_lock_irq(dataflow);

	if (pcm_running->status->state_suspend != SNDRV_PCM_STATE_RUNNING &&
	    (pcm_running->status->state_suspend != SNDRV_PCM_STATE_DRAINING ||
	     dataflow->stream != SNDRV_PCM_STREAM_PLAYBACK)) {
		res  = 0;
		goto err;
	}

	res = dataflow->ops->trigger(dataflow, SNDRV_PCM_TRIGGER_RESUME);
	if (res) {
		snd_err("pcm %s: trigger resumw failed ret:%d.\n",
				dataflow->name, res);
		if (sunxi_sound_pcm_running(dataflow)) {
			dataflow->ops->trigger(dataflow, SNDRV_PCM_STATE_SUSPENDED);
		}
		goto err;
	}
	pcm_running->status->state = pcm_running->status->state_suspend;

err:
	sound_pcm_stream_unlock_irq(dataflow);

	return res;
}

int sunxi_sound_pcm_resume_total(struct sunxi_sound_pcm *pcm)
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

		err = sunxi_sound_pcm_resume(dataflow);
		if (err < 0 && err != -EBUSY)
			return err;

	}
	return 0;
}
#else
int sunxi_sound_pcm_suspend_total(struct sunxi_sound_pcm *pcm)
{
	return 0;
}
int sunxi_sound_pcm_resume_total(struct sunxi_sound_pcm *pcm)
{
	return 0;
}

#endif



