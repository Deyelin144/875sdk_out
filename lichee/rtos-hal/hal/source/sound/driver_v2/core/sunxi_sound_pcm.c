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
#include <sound_v2/sunxi_sound_core.h>
#include <sound_v2/sunxi_sound_pcm.h>
#include <sound_v2/sunxi_sound_pcm_common.h>
#include <sound_v2/sunxi_sound_pcm_misc.h>

#include "sunxi_sound_pcm_local.h"

static sunxi_sound_mutex_t sunxi_sound_pcm_mutex;
static LIST_HEAD(gSunxiSoundPcmList);

int sunxi_sound_pcm_create(struct sunxi_sound_card *card, struct sunxi_sound_pcm *pcm);
static int sunxi_sound_pcm_dev_create(struct sunxi_sound_card *card, struct sound_device *pcm_dev);

static int sunxi_sound_pcm_add(struct sunxi_sound_pcm *newpcm)
{
	struct sunxi_sound_pcm *pcm;

	list_for_each_entry(pcm, &gSunxiSoundPcmList, list) {
		if (pcm->card == newpcm->card && pcm->device_num == newpcm->device_num)
			return -EBUSY;
		if (pcm->card->num > newpcm->card->num ||
				(pcm->card == newpcm->card &&
				pcm->device_num > newpcm->device_num)) {
			list_add(&newpcm->list, pcm->list.prev);
			return 0;
		}
	}
	snd_info("pcm add list\n");
	list_add_tail(&newpcm->list, &gSunxiSoundPcmList);
	return 0;
}

int sunxi_sound_create_dataflow(struct sunxi_sound_pcm *pcm, int stream, int dataflow_count)
{
	struct sunxi_sound_pcm_data *pcm_data = &pcm->data[stream];
	struct sunxi_sound_pcm_dataflow *dataflow, *prev;
	char name[32];
	int ret, idx;

	pcm_data->stream = stream;
	pcm_data->pcm = pcm;
	pcm_data->dataflow_count = dataflow_count;

	if (!dataflow_count)
		return 0;

	ret = sunxi_sound_pcm_dev_create(pcm->card, &pcm_data->pcm_dev);
	if (ret != 0) {
		snd_err("sunxi_sound_pcm_dev_create failed !\n");
		return -1;
	}

	snprintf(name, sizeof(name), "SunxipcmC%iD%i%c", pcm->card->num, pcm->device_num,
		stream == SNDRV_PCM_STREAM_PLAYBACK ? 'p' : 'c');

	pcm_data->pcm_dev.name = sunxi_sound_strdup_const(name);
	if (!pcm_data->pcm_dev.name) {
		snd_err("no memory!\n");
		return -ENOMEM;
	}

	prev = NULL;
	for (idx = 0, prev = NULL; idx < dataflow_count; ++idx) {
		dataflow = sound_malloc(sizeof(struct sunxi_sound_pcm_dataflow));
		if (!dataflow) {
			snd_err("alloc dataflow failed !\n");
			return -1;
		}
		dataflow->pcm = pcm;
		dataflow->pcm_data = pcm_data;
		dataflow->stream = stream;
		dataflow->number = idx;
		dataflow->ref_count = 0;
		hal_spin_lock_init(&dataflow->lock);
		dataflow->mutex = sound_mutex_init();
		if (!dataflow->mutex) {
			snd_err("mute create failed !\n");
			return -1;
		}

		if (prev == NULL)
			pcm_data->dataflow = dataflow;
		else
			prev->next = dataflow;

		snprintf(dataflow->name, sizeof(dataflow->name), "%s %i",
			(stream == SNDRV_PCM_STREAM_PLAYBACK)? "Playback dataflow" : "Capture dataflow", idx);

		prev = dataflow;
		snd_debug("new stream\n");
	}
	return 0;
}

static void sunxi_sound_destory_data(struct sunxi_sound_pcm_data * data)
{
	struct sunxi_sound_pcm_dataflow *dataflow, *next;
	snd_debug("\n");

	dataflow = data->dataflow;
	while(dataflow)
	{
		next = dataflow->next;
		sound_mutex_destroy(dataflow->mutex);
		hal_spin_lock_deinit(&dataflow->lock);
		sound_free(dataflow);
		dataflow = next;
	}
}

static int sunxi_sound_free_pcm(struct sunxi_sound_pcm *pcm)
{
	snd_debug("\n");
	if (pcm->priv_free)
		pcm->priv_free(pcm);
	sunxi_sound_destory_data(&pcm->data[SNDRV_PCM_STREAM_PLAYBACK]);
	sunxi_sound_destory_data(&pcm->data[SNDRV_PCM_STREAM_CAPTURE]);
	if (pcm->open_mutex) {
		sound_mutex_destroy(pcm->open_mutex);
		pcm->open_mutex = NULL;
	}
	if (!sunxi_sound_pcm_mutex) {
		sound_mutex_destroy(sunxi_sound_pcm_mutex);
		sunxi_sound_pcm_mutex = NULL;
	}
	sound_free(pcm);
	return 0;
}

int sunxi_sound_new_pcm(struct sunxi_sound_card *card, const char *id, int device,
		int playback_count, int capture_count, struct sunxi_sound_pcm **rpcm)
{
	struct sunxi_sound_pcm *pcm;
	int err;

	if (!card) {
		snd_err("card is NULL");
		return -ENXIO;
	}

	if (rpcm)
		*rpcm = NULL;

	pcm = sound_malloc(sizeof(*pcm));
	if (!pcm)
		return -ENOMEM;

	pcm->card = card;
	pcm->device_num = device;

	pcm->open_mutex = sound_mutex_init();
	if (!pcm->open_mutex) {
		snd_err("create mutex failed");
		goto free_pcm;
	}

	if (!sunxi_sound_pcm_mutex) {
		sunxi_sound_pcm_mutex = sound_mutex_init();
		if (!sunxi_sound_pcm_mutex) {
			snd_err("create mutex failed");
			goto free_pcm;
		}
	}

	INIT_LIST_HEAD(&pcm->list);

	if (id)
		strncpy(pcm->id, id, sizeof(pcm->id));

	err = sunxi_sound_create_dataflow(pcm, SNDRV_PCM_STREAM_PLAYBACK, playback_count);
	if (err < 0)
		goto free_pcm;

	err = sunxi_sound_create_dataflow(pcm, SNDRV_PCM_STREAM_CAPTURE, capture_count);
	if (err < 0)
		goto free_pcm;

	err = sunxi_sound_pcm_create(card, pcm);
	if (err) {
		snd_err("pcm: It can't create sound card for card %s,:%d.\n",
				card->name, err);
		goto free_pcm;
	}

	if (rpcm)
		*rpcm = pcm;
	return 0;

free_pcm:
	sunxi_sound_free_pcm(pcm);
	return err;
}

/* Run PCM ioctl ops */
static int sunxi_sound_pcm_ops_ioctl(struct sunxi_sound_pcm_dataflow *dataflow,
			     unsigned cmd, void *arg)
{
	if (dataflow->ops->ioctl)
		return dataflow->ops->ioctl(dataflow, cmd, arg);
	else
		return sunxi_sound_pcm_lib_ioctl(dataflow, cmd, arg);
}

static int sunxi_sound_pcm_get_info(struct sunxi_sound_pcm_dataflow *dataflow, void *info_params)
{
	struct sunxi_sound_pcm_info *info = (struct sunxi_sound_pcm_info *)info_params;
	struct sunxi_sound_pcm *pcm = dataflow->pcm;
	struct sunxi_sound_pcm_data *pcm_data = dataflow->pcm_data;

	memset(info, 0, sizeof(*info));
	info->card = pcm->card->num;
	info->device = pcm->device_num;
	info->stream = dataflow->stream;
	info->subdevice = dataflow->number;
	strncpy((char*)info->id, pcm->id, sizeof(info->id));
	strncpy((char*)info->name, pcm->name, sizeof(info->name));
	info->subdevices_count = pcm_data->dataflow_count;
	info->subdevices_avail = pcm_data->dataflow_count - pcm_data->dataflow_opened;
	strncpy((char*)info->subname, dataflow->name, sizeof(info->subname));

	return 0;
}

static int sunxi_sound_pcm_dev_create(struct sunxi_sound_card *card, struct sound_device *pcm_dev)
{
	struct sound_card_device *dev  = &card->dev;

	if (dev->priv_data == NULL || pcm_dev == NULL)
		return -ENXIO;

	INIT_LIST_HEAD(&pcm_dev->child_list);
	pcm_dev->parent = dev;
	return 0;
}

static void sunxi_sound_pcm_dev_destory(struct sound_device *pcm_dev)
{
	if (pcm_dev->name)
		sound_free(pcm_dev->name);
	pcm_dev->parent = NULL;
}

static int sunxi_sound_pcm_hw_constrains_init(struct sunxi_sound_pcm_dataflow *dataflow)
{
	struct sunxi_sound_pcm_running_param *runtime = dataflow->pcm_running;
	struct sunxi_sound_pcm_hw_constrains *constrains = &runtime->hw_constrains;
	int i;

	for (i = SND_PCM_HW_PARAM_FIRST_MASK; i <= SND_PCM_HW_PARAM_LAST_MASK; i++) {
		constrains->intervals[i].mask = 0xff;
	}
	for (i = SND_PCM_HW_PARAM_FIRST_INTERVAL; i <= SND_PCM_HW_PARAM_LAST_INTERVAL; i++) {
		constrains->intervals[i].range.min = 0;
		constrains->intervals[i].range.max = UINT_MAX;
	}
	return 0;
}

static int sunxi_sound_pcm_release_dataflow(struct sunxi_sound_pcm_dataflow *dataflow)
{
	struct sunxi_sound_pcm_running_param *pcm_running;

	dataflow->ref_count--;
	if (dataflow->ref_count > 0)
		return 0;

	if (dataflow->hw_opened) {
		if (dataflow->ops->hw_free &&
		    dataflow->pcm_running->status->state != SNDRV_PCM_STATE_OPEN)
			dataflow->ops->hw_free(dataflow);
		dataflow->ops->close(dataflow);
		dataflow->ref_count = 0;
		dataflow->hw_opened = 0;
	}

	pcm_running = dataflow->pcm_running;

	if (pcm_running->status) {
		sound_free(pcm_running->status);
		pcm_running->status = NULL;
	}
	if (pcm_running->control) {
		sound_free(pcm_running->control);
		pcm_running->control = NULL;
	}
	if (pcm_running->pcm_mutex) {
		sound_mutex_destroy(pcm_running->pcm_mutex);
		pcm_running->pcm_mutex = NULL;
	}
	if (pcm_running->tsleep) {
		sound_schd_destroy(pcm_running->tsleep);
		pcm_running->tsleep = NULL;
	}
	if (pcm_running->sleep) {
		sound_schd_destroy(pcm_running->sleep);
		pcm_running->sleep = NULL;
	}
	if (pcm_running->dsleep) {
		sound_schd_destroy(pcm_running->dsleep);
		pcm_running->dsleep = NULL;
	}
	sound_free(pcm_running);
	dataflow->pcm_running = NULL;
	dataflow->pcm_data->dataflow_opened--;

	return 0;
}

static int sunxi_sound_pcm_hw_refine(void *dataflow_handle, void *params_wrapper)
{
	struct sunxi_sound_pcm_dataflow *dataflow = dataflow_handle;
	struct sunxi_sound_pcm_hw_params *params = params_wrapper;
	struct sunxi_sound_pcm_running_param *pcm_running = dataflow->pcm_running;
	struct sunxi_sound_pcm_hw_constrains *cons = &pcm_running->hw_constrains;

	snd_debug("\n");
	memcpy(params, cons, sizeof(union snd_interval) *
		   (SND_PCM_HW_PARAM_LAST_INTERVAL - SND_PCM_HW_PARAM_FIRST_INTERVAL + 1));

	return 0;
}

static int sunxi_sound_pcm_open_dataflow(struct sunxi_sound_pcm *pcm, struct sunxi_sound_pcm_dataflow *dataflow)
{
	int ret = 0;
	struct sunxi_sound_pcm_running_param *pcm_running;

	snd_debug("\n");

	pcm_running = sound_malloc(sizeof(struct sunxi_sound_pcm_running_param));
	if (!pcm_running) {
		snd_err("no memory\n");
		ret = -ENOMEM;
		goto error;
	}

	pcm_running->tsleep = sound_schd_init();
	if (pcm_running->tsleep == NULL) {
		snd_err("semaphore tsleep create failed\n");
		ret = -1;
		goto error;
	}

	pcm_running->sleep = sound_schd_init();
	if (pcm_running->sleep == NULL) {
		snd_err("semaphore sleep create failed\n");
		ret = -1;
		goto error;
	}

	pcm_running->dsleep = sound_schd_init();
	if (pcm_running->dsleep == NULL) {
		snd_err("semaphore dsleep create failed\n");
		ret = -1;
		goto error;
	}

	memset(pcm_running->dsleep_list, 0, sizeof(pcm_running->dsleep_list));

	pcm_running->status = sound_malloc(sizeof(struct sunxi_sound_pcm_mmap_status));
	if (!pcm_running->status) {
		snd_err("no memory\n");
		ret = -ENOMEM;
		goto error;
	}
	pcm_running->control = sound_malloc(sizeof(struct sunxi_sound_pcm_mmap_control));
	if (!pcm_running->control) {
		snd_err("no memory\n");
		ret = -ENOMEM;
		goto error;
	}

	pcm_running->status->state = SNDRV_PCM_STATE_OPEN;
	pcm_running->pcm_mutex = sound_mutex_init();

	dataflow->pcm_running = pcm_running;
	dataflow->priv_data = pcm->priv_data;
	dataflow->ref_count++;

	ret = sunxi_sound_pcm_hw_constrains_init(dataflow);
	if (ret < 0) {
		snd_err("sunxi_sound_pcm_hw_constrains_init failed\n");
		goto error;
	}

	ret = dataflow->ops->open(dataflow);
	if (ret < 0)
		goto error;

	dataflow->hw_opened = 1;

	return ret;

error:
	sunxi_sound_pcm_release_dataflow(dataflow);
	return ret;
}

static int sunxi_sound_pcm_open(struct sunxi_sound_pcm *pcm, int stream,
		unsigned int mode, struct sunxi_sound_pcm_dataflow **rdataflow)
{
	struct sunxi_sound_pcm_data *pcm_data;
	struct sunxi_sound_pcm_dataflow *dataflow;
	int ret = 0;
	//Todo
	int prefer_subdevice = -1;

	*rdataflow = NULL;

	pcm_data = &pcm->data[stream];
	if (pcm_data->dataflow == NULL || pcm_data->dataflow_count == 0)
		return -ENODEV;

	if (mode & SND_DEV_APPEND) {

		for (dataflow = pcm_data->dataflow; dataflow;
			 dataflow = dataflow->next)
			if (dataflow->number == prefer_subdevice)
				break;

		if (!dataflow) {
			return -ENODEV;
		}
		if (!(dataflow->ref_count > 0)) {
			snd_err("stream:%d, ref_count is %d\n", stream, dataflow->ref_count);
			return -EINVAL;
		}
		dataflow->ref_count++;
		*rdataflow = dataflow;
		return 0;
	}

	for (dataflow = pcm_data->dataflow; dataflow; dataflow = dataflow->next) {
		if (!(dataflow->ref_count > 0) &&
		    (prefer_subdevice == -1 ||
		     dataflow->number == prefer_subdevice))
			break;
	}

	if (dataflow == NULL)
		return -EAGAIN;


	ret = sunxi_sound_pcm_open_dataflow(pcm, dataflow);
	if (ret < 0) {
		snd_err("open dataflow failed,%d\n", ret);
		return ret;
	}
	dataflow->mode = mode;
	pcm_data->dataflow_opened++;
	*rdataflow = dataflow;

	return ret;
}

static int sunxi_sound_pcm_pb_dev_open(struct sound_device *dev, unsigned int mode)
{
	struct sunxi_sound_pcm *pcm = NULL;
	struct sunxi_sound_pcm_priv *pcm_priv = NULL;
	struct sunxi_sound_pcm_dataflow *dataflow = NULL;
	int err = 0;

	if (dev == NULL)
		return -ENXIO;

	pcm_priv = dev->priv_data;
	if (pcm_priv == NULL)
		return -ENXIO;

	pcm = pcm_priv->pcm;
	if (pcm == NULL)
		return -ENXIO;

	sound_mutex_lock(pcm->open_mutex);

	err = sunxi_sound_pcm_open(pcm, SNDRV_PCM_STREAM_PLAYBACK, mode, &dataflow);
	if (err != 0) {
		snd_err("sunxi_sound_pcm_open failed ret %d\n", err);
		goto err;
	}

	if (dataflow)
		pcm_priv->dataflow = dataflow;

err:
	sound_mutex_unlock(pcm->open_mutex);
	return err;
}

static int sunxi_sound_pcm_cap_dev_open(struct sound_device *dev, unsigned int mode)
{
	struct sunxi_sound_pcm *pcm = NULL;
	struct sunxi_sound_pcm_priv *pcm_priv = NULL;
	struct sunxi_sound_pcm_dataflow *dataflow = NULL;
	int err = 0;

	if (dev == NULL)
		return -ENXIO;

	pcm_priv = dev->priv_data;
	if (pcm_priv == NULL)
		return -ENXIO;

	pcm = pcm_priv->pcm;
	if (pcm == NULL)
		return -ENXIO;

	sound_mutex_lock(pcm->open_mutex);

	err = sunxi_sound_pcm_open(pcm, SNDRV_PCM_STREAM_CAPTURE, mode, &dataflow);
	if (err != 0) {
		snd_err("sunxi_sound_pcm_open failed ret %d\n", err);
		goto err;
	}

	if (dataflow)
		pcm_priv->dataflow = dataflow;

err:
	sound_mutex_unlock(pcm->open_mutex);
	return err;
}

static int sunxi_sound_pcm_dev_close(struct sound_device *dev)
{
	struct sunxi_sound_pcm *pcm = NULL;
	struct sunxi_sound_pcm_priv *pcm_priv = NULL;
	struct sunxi_sound_pcm_dataflow *dataflow = NULL;

	if (dev == NULL)
		return -ENXIO;

	pcm_priv = dev->priv_data;
	if (pcm_priv == NULL)
		return -ENXIO;

	pcm = pcm_priv->pcm;
	if (pcm == NULL)
		return -ENXIO;

	dataflow = pcm_priv->dataflow;
	if (dataflow == NULL)
		return -ENXIO;

	sound_mutex_lock(pcm->open_mutex);
	sunxi_sound_pcm_release_dataflow(dataflow);
	sound_mutex_unlock(pcm->open_mutex);
	return 0;

}

static int sunxi_sound_hw_constrains_check(struct sunxi_sound_pcm_dataflow *dataflow,
		struct sunxi_sound_pcm_hw_params *params)
{
	struct sunxi_sound_pcm_hw_constrains *cons = &dataflow->pcm_running->hw_constrains;
	int i;

	if (!(params->intervals[SND_PCM_HW_PARAM_FORMAT].mask &
		cons->intervals[SND_PCM_HW_PARAM_FORMAT].mask)) {
		snd_err("hw_params format invalid."
			"params mask:0x%x, hw_cons mask:0x%x\n",
			params->intervals[SND_PCM_HW_PARAM_FORMAT].mask,
			cons->intervals[SND_PCM_HW_PARAM_FORMAT].mask);
		return -EFAULT;
	}

	params->intervals[SND_PCM_HW_PARAM_FRAME_BITS].range.min =
		params->intervals[SND_PCM_HW_PARAM_SAMPLE_BITS].range.min *
		params->intervals[SND_PCM_HW_PARAM_CHANNELS].range.min;
	snd_info("sample bits:%u, frame bits:%u\n",
		params->intervals[SND_PCM_HW_PARAM_SAMPLE_BITS].range.min,
		params->intervals[SND_PCM_HW_PARAM_FRAME_BITS].range.min);
	/*
	 * update period_time,period_size,period_bytes
	 * priority:
	 * period_time
	 * period_size
	 * period_bytes
	 */
	snd_info("period_time:%u, period_size:%u, period_bytes:%u\n",
		params->intervals[SND_PCM_HW_PARAM_PERIOD_TIME].range.min,
		params->intervals[SND_PCM_HW_PARAM_PERIOD_SIZE].range.min,
		params->intervals[SND_PCM_HW_PARAM_PERIOD_BYTES].range.min);
	if (params->intervals[SND_PCM_HW_PARAM_PERIOD_TIME].range.min != 0) {
		/* period_size */
		params->intervals[SND_PCM_HW_PARAM_PERIOD_SIZE].range.min =
			params->intervals[SND_PCM_HW_PARAM_RATE].range.min *
			params->intervals[SND_PCM_HW_PARAM_PERIOD_TIME].range.min / 1000000;
		/* period_bytes */
		params->intervals[SND_PCM_HW_PARAM_PERIOD_BYTES].range.min =
			params->intervals[SND_PCM_HW_PARAM_PERIOD_SIZE].range.min *
			params->intervals[SND_PCM_HW_PARAM_FRAME_BITS].range.min / 8;
	} else if (params->intervals[SND_PCM_HW_PARAM_PERIOD_SIZE].range.min != 0) {
		/* period_time */
		params->intervals[SND_PCM_HW_PARAM_PERIOD_TIME].range.min =
			params->intervals[SND_PCM_HW_PARAM_PERIOD_SIZE].range.min *
			1000000 / params->intervals[SND_PCM_HW_PARAM_RATE].range.min;
		/* period_bytes */
		params->intervals[SND_PCM_HW_PARAM_PERIOD_BYTES].range.min =
			params->intervals[SND_PCM_HW_PARAM_PERIOD_SIZE].range.min *
			params->intervals[SND_PCM_HW_PARAM_FRAME_BITS].range.min / 8;
	} else {
		/* period_size */
		params->intervals[SND_PCM_HW_PARAM_PERIOD_SIZE].range.min =
			params->intervals[SND_PCM_HW_PARAM_PERIOD_BYTES].range.min * 8 /
			params->intervals[SND_PCM_HW_PARAM_FRAME_BITS].range.min;
		/* period_time */
		params->intervals[SND_PCM_HW_PARAM_PERIOD_TIME].range.min =
			1000000 * params->intervals[SND_PCM_HW_PARAM_RATE].range.min /
			params->intervals[SND_PCM_HW_PARAM_PERIOD_SIZE].range.min;
	}
	snd_info("period_time:%u, period_size:%u, period_bytes:%u\n",
		params->intervals[SND_PCM_HW_PARAM_PERIOD_TIME].range.min,
		params->intervals[SND_PCM_HW_PARAM_PERIOD_SIZE].range.min,
		params->intervals[SND_PCM_HW_PARAM_PERIOD_BYTES].range.min);

	/*
	 * update periods,buffer_time,buffer_size,buffer_bytes
	 * priority:
	 * buffer_time
	 * buffer_size
	 * periods
	 */
	snd_info("periods:%u, buffer_time:%u, buffer_size:%u, buffer_bytes:%u\n",
		params->intervals[SND_PCM_HW_PARAM_PERIODS].range.min,
		params->intervals[SND_PCM_HW_PARAM_BUFFER_TIME].range.min,
		params->intervals[SND_PCM_HW_PARAM_BUFFER_SIZE].range.min,
		params->intervals[SND_PCM_HW_PARAM_BUFFER_BYTES].range.min);
	if (params->intervals[SND_PCM_HW_PARAM_BUFFER_TIME].range.min != 0) {
		/* periods */
		params->intervals[SND_PCM_HW_PARAM_PERIODS].range.min =
			params->intervals[SND_PCM_HW_PARAM_BUFFER_TIME].range.min /
			params->intervals[SND_PCM_HW_PARAM_PERIOD_TIME].range.min;
		/* buffer_time */
		params->intervals[SND_PCM_HW_PARAM_BUFFER_TIME].range.min =
			params->intervals[SND_PCM_HW_PARAM_PERIODS].range.min *
			params->intervals[SND_PCM_HW_PARAM_PERIOD_TIME].range.min;
		/* buffer_size */
		params->intervals[SND_PCM_HW_PARAM_BUFFER_SIZE].range.min =
			params->intervals[SND_PCM_HW_PARAM_PERIODS].range.min *
			params->intervals[SND_PCM_HW_PARAM_PERIOD_SIZE].range.min;
		/* buffer_bytes */
		params->intervals[SND_PCM_HW_PARAM_BUFFER_BYTES].range.min =
			params->intervals[SND_PCM_HW_PARAM_PERIODS].range.min *
			params->intervals[SND_PCM_HW_PARAM_PERIOD_BYTES].range.min;
	} else if (params->intervals[SND_PCM_HW_PARAM_BUFFER_SIZE].range.min != 0) {
		/* periods */
		params->intervals[SND_PCM_HW_PARAM_PERIODS].range.min =
			params->intervals[SND_PCM_HW_PARAM_BUFFER_SIZE].range.min /
			params->intervals[SND_PCM_HW_PARAM_PERIOD_SIZE].range.min;
		/* buffer_time */
		params->intervals[SND_PCM_HW_PARAM_BUFFER_TIME].range.min =
			params->intervals[SND_PCM_HW_PARAM_PERIODS].range.min *
			params->intervals[SND_PCM_HW_PARAM_PERIOD_TIME].range.min;
		/* buffer_size */
		params->intervals[SND_PCM_HW_PARAM_BUFFER_SIZE].range.min =
			params->intervals[SND_PCM_HW_PARAM_PERIODS].range.min *
			params->intervals[SND_PCM_HW_PARAM_PERIOD_SIZE].range.min;
		/* buffer_bytes */
		params->intervals[SND_PCM_HW_PARAM_BUFFER_BYTES].range.min =
			params->intervals[SND_PCM_HW_PARAM_PERIODS].range.min *
			params->intervals[SND_PCM_HW_PARAM_PERIOD_BYTES].range.min;
	} else {
		/* use periods 4 */
		if (params->intervals[SND_PCM_HW_PARAM_PERIODS].range.min == 0)
			params->intervals[SND_PCM_HW_PARAM_PERIODS].range.min = 4;
		/* buffer_time */
		params->intervals[SND_PCM_HW_PARAM_BUFFER_TIME].range.min =
			params->intervals[SND_PCM_HW_PARAM_PERIODS].range.min *
			params->intervals[SND_PCM_HW_PARAM_PERIOD_TIME].range.min;
		/* buffer_size */
		params->intervals[SND_PCM_HW_PARAM_BUFFER_SIZE].range.min =
			params->intervals[SND_PCM_HW_PARAM_PERIODS].range.min *
			params->intervals[SND_PCM_HW_PARAM_PERIOD_SIZE].range.min;
		/* buffer_bytes */
		params->intervals[SND_PCM_HW_PARAM_BUFFER_BYTES].range.min =
			params->intervals[SND_PCM_HW_PARAM_PERIODS].range.min *
			params->intervals[SND_PCM_HW_PARAM_PERIOD_BYTES].range.min;
	}
	snd_info("periods:%u, buffer_time:%u, buffer_size:%u, buffer_bytes:%u\n",
		params->intervals[SND_PCM_HW_PARAM_PERIODS].range.min,
		params->intervals[SND_PCM_HW_PARAM_BUFFER_TIME].range.min,
		params->intervals[SND_PCM_HW_PARAM_BUFFER_SIZE].range.min,
		params->intervals[SND_PCM_HW_PARAM_BUFFER_BYTES].range.min);
	for (i = SND_PCM_HW_PARAM_CHANNELS; i <= SND_PCM_HW_PARAM_LAST_INTERVAL; i++) {
		if (!is_range_of_hw_constrains(params, cons, i)) {
			snd_err("hw_params %d type invalid."
				"params min:%u,max:%u, hw_cons min:%u,max:%u\n",
				i, params->intervals[i].range.min,
				params->intervals[i].range.max,
				cons->intervals[i].range.min,
				cons->intervals[i].range.max);
			return -EINVAL;
		}
	}

	return 0;
}

static inline void sunxi_sound_pcm_set_state(struct sunxi_sound_pcm_dataflow *dataflow, sunxi_sound_pcm_state_t state)
{
	snd_debug("set state:%u\n", state);
	sound_pcm_stream_lock_irq(dataflow);
	if (dataflow->pcm_running->status->state != SNDRV_PCM_STATE_DISCONNECTED)
		dataflow->pcm_running->status->state = state;
	sound_pcm_stream_unlock_irq(dataflow);
}

static void sunxi_sound_pcm_dump_params(struct sunxi_sound_pcm_running_param *pcm_running)
{
	snd_debug("HW params:--------------\n");
	snd_debug("format:            %u\n", pcm_running->format);
	snd_debug("rate:              %d\n", pcm_running->rate);
	snd_debug("channels:          %u\n", pcm_running->channels);
	snd_debug("period_size:       %lu\n", pcm_running->period_size);
	snd_debug("periods:           %u\n", pcm_running->periods);
	snd_debug("buffer_size:       %lu\n", pcm_running->buffer_size);
	snd_debug("frame_bits:        %u\n", pcm_running->frame_bits);
	snd_debug("sample_bits:       %u\n", pcm_running->sample_bits);
	snd_debug("SW params:--------------\n");
	snd_debug("avail_min:         %lu\n", pcm_running->control->avail_min);
	snd_debug("start_threshold:   %lu\n", pcm_running->start_threshold);
	snd_debug("stop_threshold:    %lu\n", pcm_running->stop_threshold);
	snd_debug("silence_size:      %lu\n", pcm_running->silence_size);
	snd_debug("boundary:          %lu\n", pcm_running->boundary);
	snd_debug("------------------------\n");
	return ;
}

static int sunxi_sound_pcm_hw_params_set(void *dataflow_handle, void *params_wrapper)
{
	int ret = 0;
	struct sunxi_sound_pcm_dataflow *dataflow = dataflow_handle;
	struct sunxi_sound_pcm_running_param *pcm_running = dataflow->pcm_running;
	struct sunxi_sound_pcm_hw_params *params = params_wrapper;
	unsigned int bits;
	snd_pcm_uframes_t frames;

	snd_lock_debug("\n");
	sound_pcm_stream_lock_irq(dataflow);
	switch (pcm_running->status->state) {
	case SNDRV_PCM_STATE_OPEN:
	case SNDRV_PCM_STATE_SETUP:
	case SNDRV_PCM_STATE_PREPARED:
		break;
	default:
		sound_pcm_stream_unlock_irq(dataflow);
		snd_err("unsupport state transform.\n");
		return -EBADFD;
	}
	sound_pcm_stream_unlock_irq(dataflow);

	ret = sunxi_sound_hw_constrains_check(dataflow, params);
	if (ret < 0) {
		snd_err("hw params invalid\n");
		goto err1;
	}

	if (dataflow->ops->hw_params) {
		ret = dataflow->ops->hw_params(dataflow, params);
		if (ret < 0)
			goto err1;
	}

	pcm_running->access = params_access(params);
	pcm_running->format = params_format(params);
	pcm_running->rate = params_rate(params);
	pcm_running->channels = params_channels(params);
	pcm_running->period_size = params_period_size(params);
	pcm_running->periods = params_periods(params);
	pcm_running->buffer_size = params_buffer_size(params);
	pcm_running->can_paused = params->can_paused;

	bits = sunxi_sound_pcm_format_physical_width(pcm_running->format);
	pcm_running->sample_bits = bits;
	bits *= pcm_running->channels;
	pcm_running->frame_bits = bits;
	/* bits should be 8bit align */
	frames = 1;
	while (bits % 8 != 0) {
		bits *= 2;
		frames *= 2;
	}
	pcm_running->min_align = frames;

	/* default sw params */
	pcm_running->control->avail_min = pcm_running->period_size;
	if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK)
		pcm_running->start_threshold = pcm_running->buffer_size;
	else
		pcm_running->start_threshold = 1;
	pcm_running->stop_threshold = pcm_running->buffer_size;
	pcm_running->silence_size = 0;
	pcm_running->boundary = pcm_running->buffer_size;
	while (pcm_running->boundary * 2 <= LONG_MAX - pcm_running->buffer_size)
		pcm_running->boundary *= 2;

	sunxi_sound_pcm_dump_params(pcm_running);

	sunxi_sound_pcm_set_state(dataflow, SNDRV_PCM_STATE_SETUP);

	return 0;
err1:
	sunxi_sound_pcm_set_state(dataflow, SNDRV_PCM_STATE_OPEN);
	if (dataflow->ops->hw_free)
		dataflow->ops->hw_free(dataflow);

	return ret;
}

static int sunxi_sound_pcm_hw_free(void *dataflow_handle)
{
	int ret = 0;
	struct sunxi_sound_pcm_dataflow *dataflow = dataflow_handle;
	struct sunxi_sound_pcm_running_param *pcm_running = dataflow->pcm_running;

	sound_pcm_stream_lock_irq(dataflow);
	switch (pcm_running->status->state) {
	case SNDRV_PCM_STATE_SETUP:
	case SNDRV_PCM_STATE_PREPARED:
		break;
	default:
		sound_pcm_stream_unlock_irq(dataflow);
		snd_err("unsupport state transform.\n");
		return -EBADFD;
	}
	sound_pcm_stream_unlock_irq(dataflow);
	if (dataflow->ops->hw_free)
		ret = dataflow->ops->hw_free(dataflow);
	sunxi_sound_pcm_set_state(dataflow, SNDRV_PCM_STATE_OPEN);

	return ret;
}

static int sunxi_sound_pcm_sw_params(void *dataflow_handle, void *params_wrapper)
{
	int ret = 0;
	struct sunxi_sound_pcm_dataflow *dataflow = dataflow_handle;
	struct sunxi_sound_pcm_running_param *pcm_running = dataflow->pcm_running;
	struct sunxi_sound_pcm_sw_params *params = params_wrapper;

	snd_lock_debug("\n");
	sound_pcm_stream_lock_irq(dataflow);
	if (pcm_running->status->state == SNDRV_PCM_STATE_OPEN) {
		sound_pcm_stream_unlock_irq(dataflow);
		snd_err("unsupport state transform.\n");
		return -EBADFD;
	}
	sound_pcm_stream_unlock_irq(dataflow);

	if (params->avail_min == 0)
		return -EINVAL;
	if (params->silence_size != 0 &&
		params->silence_size != pcm_running->boundary) {
		snd_err("silence_size only support 0 or boundary.\n");
		return -EINVAL;
	}

	snd_lock_debug("\n");
	sound_pcm_stream_lock_irq(dataflow);
	pcm_running->control->avail_min = params->avail_min;
	if (params->start_threshold > pcm_running->buffer_size)
		params->start_threshold = pcm_running->buffer_size;
	pcm_running->start_threshold = params->start_threshold;
	pcm_running->stop_threshold = params->stop_threshold;
	pcm_running->silence_size = params->silence_size;
	params->boundary = pcm_running->boundary;
	if (sunxi_sound_pcm_running(dataflow)) {
		if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK &&
			pcm_running->silence_size > 0)
			sunxi_sound_pcm_playback_silence(dataflow, ULONG_MAX);
		ret = sunxi_sound_pcm_update_state(dataflow, pcm_running);
	}
	sunxi_sound_pcm_dump_params(pcm_running);
	sound_pcm_stream_unlock_irq(dataflow);

	return ret;
}

static int sunxi_sound_do_reset(struct sunxi_sound_pcm_dataflow *dataflow,
			    sunxi_sound_pcm_state_t state)
{
	struct sunxi_sound_pcm_running_param *pcm_running = dataflow->pcm_running;
	int err = 0;

	err = sunxi_sound_pcm_ops_ioctl(dataflow, SNDRV_PCM_IOCTL1_RESET, NULL);
	if (err < 0)
		return err;
	sound_pcm_stream_lock_irq(dataflow);
	pcm_running->hw_ptr_base = 0;
	pcm_running->silence_start = pcm_running->status->hw_ptr;
	pcm_running->silence_filled = 0;
	sound_pcm_stream_unlock_irq(dataflow);
	return 0;
}

/* can't lock in this function */
static int sunxi_sound_pcm_do_start(struct sunxi_sound_pcm_dataflow *dataflow,
			sunxi_sound_pcm_state_t state)
{
	int ret = 0;
	struct sunxi_sound_pcm_running_param *pcm_running = dataflow->pcm_running;

	snd_debug("\n");
	if (pcm_running->status->state != SNDRV_PCM_STATE_PREPARED) {
		snd_err("unsupport state transform.\n");
		return -EBADFD;
	}
	if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK &&
		!sunxi_sound_pcm_playback_data(dataflow))
		return -EPIPE;
	snd_debug("\n");
	if (dataflow->ops->trigger) {
		ret = dataflow->ops->trigger(dataflow, SNDRV_PCM_TRIGGER_START);
		if (ret < 0) {
			dataflow->ops->trigger(dataflow, SNDRV_PCM_TRIGGER_STOP);
			snd_err("trigger failed.\n");
			return ret;
		}
	}
	/* can't lock */
	/*snd_pcm_set_state(substream, state);*/
	dataflow->pcm_running->status->state = state;
	if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK &&
		pcm_running->silence_size > 0)
		sunxi_sound_pcm_playback_silence(dataflow, ULONG_MAX);
	/* timer start */

	return ret;
}

int sunxi_sound_pcm_start(void *dataflow_handle)
{
	int ret = 0;
	struct sunxi_sound_pcm_dataflow *dataflow = dataflow_handle;
	ret = sunxi_sound_pcm_do_start(dataflow, SNDRV_PCM_STATE_RUNNING);
	return ret;
}

static int sunxi_sound_pcm_start_lock_irq(void *dataflow_handle)
{
	int ret = 0;
	struct sunxi_sound_pcm_dataflow *dataflow = dataflow_handle;
	sound_pcm_stream_lock_irq(dataflow);
	ret = sunxi_sound_pcm_do_start(dataflow, SNDRV_PCM_STATE_RUNNING);
	sound_pcm_stream_unlock_irq(dataflow);
	return ret;
}

/* can't lock in this function */
int sunxi_sound_pcm_stop(struct sunxi_sound_pcm_dataflow *dataflow, sunxi_sound_pcm_state_t state)
{
	struct sunxi_sound_pcm_running_param *pcm_running = dataflow->pcm_running;

	if (pcm_running->status->state == SNDRV_PCM_STATE_OPEN) {
		snd_err("unsupport state transform.\n");
		return -EBADFD;
	}
	if (sunxi_sound_pcm_running(dataflow))
		dataflow->ops->trigger(dataflow, SNDRV_PCM_TRIGGER_STOP);
	if (pcm_running->status->state != state) {
		/* can't lock */
		/*snd_pcm_set_state(substream, state);*/
		pcm_running->status->state = state;
	}
	sound_schd_wakeup(pcm_running->sleep);
	sound_schd_wakeup(pcm_running->tsleep);
	return 0;
}

static int sunxi_sound_pcm_pause(struct sunxi_sound_pcm_dataflow *dataflow, int push)
{
	struct sunxi_sound_pcm_running_param *pcm_running = dataflow->pcm_running;
	int ret = 0;

	snd_debug("\n");
	if (push) {
		if (pcm_running->status->state != SNDRV_PCM_STATE_RUNNING) {
			snd_err("unsupport state transform.\n");
			return -EBADFD;
		}
		sunxi_sound_pcm_update_hw_ptr(dataflow);
		ret = dataflow->ops->trigger(dataflow, SNDRV_PCM_TRIGGER_PAUSE_PUSH);
		if (ret < 0) {
			dataflow->ops->trigger(dataflow, SNDRV_PCM_TRIGGER_PAUSE_RELEASE);
			snd_err("trigger failed.\n");
			return ret;
		}
		pcm_running->status->state = SNDRV_PCM_STATE_PAUSED;
		/* ensure perform without irq */
		sound_schd_wakeup(pcm_running->sleep);
		sound_schd_wakeup(pcm_running->tsleep);
	} else {
		if (pcm_running->status->state != SNDRV_PCM_STATE_PAUSED) {
			snd_err("unsupport state transform.\n");
			return -EBADFD;
		}
		ret = dataflow->ops->trigger(dataflow, SNDRV_PCM_TRIGGER_PAUSE_RELEASE);
		if (ret < 0) {
			dataflow->ops->trigger(dataflow, SNDRV_PCM_TRIGGER_PAUSE_PUSH);
			snd_err("trigger failed.\n");
			return ret;
		}
		pcm_running->status->state = SNDRV_PCM_STATE_RUNNING;
	}

	return ret;
}

static int sunxi_sound_pcm_pause_lock_irq(void *dataflow_handle, int push)
{
	int ret = 0;
	struct sunxi_sound_pcm_dataflow *dataflow = dataflow_handle;
	sound_pcm_stream_lock_irq(dataflow);
	ret = sunxi_sound_pcm_pause(dataflow, push);
	sound_pcm_stream_unlock_irq(dataflow);
	return ret;
}

static int sunxi_sound_pcm_reset(void *dataflow_handle)
{
	int ret = 0;
	struct sunxi_sound_pcm_dataflow *dataflow = dataflow_handle;
	struct sunxi_sound_pcm_running_param *pcm_running = dataflow->pcm_running;

	switch (pcm_running->status->state) {
		case SNDRV_PCM_STATE_RUNNING:
		case SNDRV_PCM_STATE_PREPARED:
		case SNDRV_PCM_STATE_PAUSED:
		case SNDRV_PCM_STATE_SUSPENDED:
			break;
		default:
			snd_err("unsupport state transform.\n");
			return -EBADFD;
	}

	sunxi_sound_do_reset(dataflow, pcm_running->status->state);

	//sound_pcm_stream_lock_irq(dataflow);
	pcm_running->control->appl_ptr = pcm_running->status->hw_ptr;
	if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK &&
		pcm_running->silence_size > 0)
		sunxi_sound_pcm_playback_silence(dataflow, ULONG_MAX);
	//sound_pcm_stream_unlock_irq(dataflow);

	return ret;
}

static int sunxi_sound_pcm_prepare(void *dataflow_handle)
{
	int ret = 0;
	struct sunxi_sound_pcm_dataflow *dataflow = dataflow_handle;
	struct sunxi_sound_pcm_running_param *pcm_running = dataflow->pcm_running;

	sound_pcm_stream_lock_irq(dataflow);
	switch (dataflow->pcm_running->status->state) {
		case SNDRV_PCM_STATE_PAUSED:
			sunxi_sound_pcm_pause(dataflow, false);
		case SNDRV_PCM_STATE_SUSPENDED:
			sunxi_sound_pcm_stop(dataflow, SNDRV_PCM_STATE_SETUP);
			break;
		default:
			break;
	}
	sound_pcm_stream_unlock_irq(dataflow);

	if (pcm_running->status->state == SNDRV_PCM_STATE_OPEN ||
		pcm_running->status->state == SNDRV_PCM_STATE_DISCONNECTED) {
		snd_err("unsupport state transform.\n");
		return -EBADFD;
	}
	if (sunxi_sound_pcm_running(dataflow)) {
		snd_err("pcm already running. state %u\n", pcm_running->status->state);
		return -EBUSY;
	}
	ret = dataflow->ops->prepare(dataflow);
	if (ret < 0)
		return ret;
	ret = sunxi_sound_do_reset(dataflow, pcm_running->status->state);
	if (ret < 0)
		return ret;

	pcm_running->control->appl_ptr = pcm_running->status->hw_ptr;
	sunxi_sound_pcm_set_state(dataflow, SNDRV_PCM_STATE_PREPARED);
	return ret;
}

static int sunxi_sound_pcm_drain(void *dataflow_handle)
{
	int ret = 0;
	struct sunxi_sound_pcm_dataflow *dataflow = dataflow_handle;
	struct sunxi_sound_pcm_running_param *pcm_running = dataflow->pcm_running;

	snd_lock_debug("\n");
	sound_pcm_stream_lock_irq(dataflow);
	if (pcm_running->status->state == SNDRV_PCM_STATE_PAUSED)
		sunxi_sound_pcm_pause(dataflow, 0);

	switch (pcm_running->status->state) {
	case SNDRV_PCM_STATE_OPEN:
	case SNDRV_PCM_STATE_DISCONNECTED:
	case SNDRV_PCM_STATE_SUSPENDED:
		snd_err("unsupport state transform.\n");
		sound_pcm_stream_unlock_irq(dataflow);
		return -EBADFD;
	default:
	    break;
	}

	if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		switch (pcm_running->status->state) {
		case SNDRV_PCM_STATE_PREPARED:
			if (!sunxi_sound_pcm_playback_empty(dataflow)) {
				/* ringbuffer has data */
				sunxi_sound_pcm_do_start(dataflow, SNDRV_PCM_STATE_DRAINING);
			} else {
				/* ringbuffer empty */
				/* can't lock */
				pcm_running->status->state = SNDRV_PCM_STATE_SETUP;
			}
			break;
		case SNDRV_PCM_STATE_RUNNING:
			pcm_running->status->state = SNDRV_PCM_STATE_DRAINING;
			break;
		case SNDRV_PCM_STATE_XRUN:
			pcm_running->status->state = SNDRV_PCM_STATE_SETUP;
			break;
		default:
		    break;
		}
	} else {
		if (pcm_running->status->state == SNDRV_PCM_STATE_RUNNING) {
			int new_state = sunxi_sound_pcm_capture_avail(pcm_running) > 0 ?
				SNDRV_PCM_STATE_DRAINING : SNDRV_PCM_STATE_SETUP;
			ret = sunxi_sound_pcm_stop(dataflow, new_state);
		}
	}

	for (;;) {
		long tout = 10;
		snd_debug("during drain process, state:%u\n", pcm_running->status->state);
		if (dataflow->stream != SNDRV_PCM_STREAM_PLAYBACK)
			break;
		if (pcm_running->status->state != SNDRV_PCM_STATE_DRAINING )
			break;
		if (pcm_running->rate) {
			long t = pcm_running->period_size * 2 / pcm_running->rate;
			tout = max(t, tout);
		}
		sound_pcm_stream_unlock_irq(dataflow);
		tout = sound_schd_timeout(pcm_running->sleep, tout*1000);
		snd_lock_debug("\n");
		sound_pcm_stream_lock_irq(dataflow);
		snd_debug("after snd_schd_timeout, tout=%ld\n", tout);
		if (tout != 0) {
			snd_err("playback drain error (DMA transfer error?) tout %ld\n", tout);
			ret = -EIO;
			break;
		}
	}

	sound_pcm_stream_unlock_irq(dataflow);

	return ret;
}

static int sunxi_sound_pcm_drop(void *dataflow_handle)
{
	int ret = 0;
	struct sunxi_sound_pcm_dataflow *dataflow = dataflow_handle;
	struct sunxi_sound_pcm_running_param *pcm_running = dataflow->pcm_running;

	snd_debug("\n");
	if (pcm_running->status->state == SNDRV_PCM_STATE_OPEN ||
            pcm_running->status->state == SNDRV_PCM_STATE_DISCONNECTED ||
            pcm_running->status->state == SNDRV_PCM_STATE_SUSPENDED) {
		snd_err("unsupport state transform.\n");
            return -EBADFD;
	}
	snd_lock_debug("\n");
	sound_pcm_stream_lock_irq(dataflow);
	if (pcm_running->status->state == SNDRV_PCM_STATE_PAUSED)
		sunxi_sound_pcm_pause(dataflow, false);
	ret = sunxi_sound_pcm_stop(dataflow, SNDRV_PCM_STATE_SETUP);
	sound_pcm_stream_unlock_irq(dataflow);

	return ret;
}

static int sunxi_sound_do_pcm_hwsync(struct sunxi_sound_pcm_dataflow *dataflow)
{
	switch (dataflow->pcm_running->status->state) {
	case SNDRV_PCM_STATE_DRAINING:
		if (dataflow->stream == SNDRV_PCM_STREAM_CAPTURE)
			return -EBADFD;
	case SNDRV_PCM_STATE_RUNNING:
		return sunxi_sound_pcm_update_hw_ptr(dataflow);
	case SNDRV_PCM_STATE_PREPARED:
	case SNDRV_PCM_STATE_PAUSED:
		return 0;
	case SNDRV_PCM_STATE_SUSPENDED:
		return -ESTRPIPE;
	case SNDRV_PCM_STATE_XRUN:
		return -EPIPE;
	default:
		return -EBADFD;
	}
}

static int sunxi_sound_pcm_hw_sync(struct sunxi_sound_pcm_dataflow *dataflow)
{
	int ret = 0;

	sound_pcm_stream_lock_irq(dataflow);
	ret = sunxi_sound_do_pcm_hwsync(dataflow);
	sound_pcm_stream_unlock_irq(dataflow);

	return ret;
}

static int sunxi_sound_pcm_get_sync_ptr(void *dataflow_handle,
					void *ptr_wrap)
{
	int ret = 0;
	struct sunxi_sound_pcm_dataflow *dataflow = dataflow_handle;
	struct sunxi_sound_pcm_sync_ptr *sync_ptr = ptr_wrap;
	struct sunxi_sound_pcm_running_param *pcm_running;
	volatile struct sunxi_sound_pcm_mmap_status *status;
	volatile struct sunxi_sound_pcm_mmap_control *control;

	if (!dataflow) {
		snd_err("substream is NULL\n");
		return -EINVAL;
	}
	pcm_running = dataflow->pcm_running;
	if (!pcm_running) {
		snd_err("pcm_running is NULL\n");
		return -EINVAL;
	}
	status = pcm_running->status;
	control = pcm_running->control;

	if (sync_ptr->flags & SNDRV_PCM_SYNC_PTR_HWSYNC) {
		ret = sunxi_sound_pcm_hw_sync(dataflow);
		if (ret < 0)
			return ret;
	}

	sound_pcm_stream_lock_irq(dataflow);
	if (!(sync_ptr->flags & SNDRV_PCM_SYNC_PTR_APPL))
		control->appl_ptr = sync_ptr->c.control.appl_ptr;
	else
		sync_ptr->c.control.appl_ptr = control->appl_ptr;

	if (!(sync_ptr->flags & SNDRV_PCM_SYNC_PTR_AVAIL_MIN))
		control->avail_min = sync_ptr->c.control.avail_min;
	else
		sync_ptr->c.control.avail_min = control->avail_min;

	sync_ptr->s.status.state = status->state;
	sync_ptr->s.status.hw_ptr = status->hw_ptr;
	sound_pcm_stream_unlock_irq(dataflow);

	return ret;
}

static int sunxi_sound_pcm_delay(void *dataflow_handle, void *delay_wrap)
{
	int ret = 0;
	struct sunxi_sound_pcm_dataflow *dataflow = dataflow_handle;
	struct sunxi_sound_pcm_running_param *pcm_running = dataflow->pcm_running;
	snd_pcm_sframes_t n = -1, *delay = delay_wrap;

	sound_pcm_stream_lock_irq(dataflow);
	ret = sunxi_sound_do_pcm_hwsync(dataflow);
	if (!ret) {
		if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK)
			n = sunxi_sound_pcm_playback_hw_avail(pcm_running);
		else
			n = sunxi_sound_pcm_capture_avail(pcm_running);
	}
	sound_pcm_stream_unlock_irq(dataflow);
	if (!ret)
		*delay = n;

	snd_info("ret %d, n %ld, delay %ld.\n", ret, n, *delay);

	return ret;
}

int sunxi_sound_pcm_dev_control(struct sound_device *dev, unsigned int cmd, unsigned long arg)
{
	struct sunxi_sound_pcm_priv *pcm_priv = NULL;
	struct sunxi_sound_pcm_dataflow *dataflow = NULL;
	void *argp = (void *)arg;
	int ret;

	if (dev == NULL || dev->priv_data == NULL)
		return -ENXIO;

	pcm_priv = dev->priv_data;
	dataflow = pcm_priv->dataflow;
	if (dataflow == NULL)
		return -ENXIO;

	switch (cmd) {
		case SUNXI_SOUND_CTRL_PCM_INFO:
			ret = sunxi_sound_pcm_get_info(dataflow, argp);
			if (ret != 0) {
				snd_err("sunxi_sound_pcm_info failed.\n");
			}
			break;
		case SUNXI_SOUND_CTRL_PCM_HW_REFINE:
			ret = sunxi_sound_pcm_hw_refine(dataflow, argp);
			if (ret != 0) {
				snd_err("sunxi_sound_pcm_hw_params_set failed.\n");
			}
			break;
		case SUNXI_SOUND_CTRL_PCM_HW_PARAMS:
			ret = sunxi_sound_pcm_hw_params_set(dataflow, argp);
			if (ret != 0) {
				snd_err("sunxi_sound_pcm_hw_params_set failed.\n");
			}
			break;
		case SUNXI_SOUND_CTRL_PCM_HW_FREE:
			ret = sunxi_sound_pcm_hw_free(dataflow);
			if (ret != 0) {
				snd_err("sunxi_sound_pcm_hw_free failed.\n");
			}
			break;
		case SUNXI_SOUND_CTRL_PCM_SW_PARAMS:
			ret = sunxi_sound_pcm_sw_params(dataflow, argp);
			if (ret != 0) {
				snd_err("sunxi_sound_pcm_sw_params failed.\n");
			}
			break;
		case SUNXI_SOUND_CTRL_PCM_PREPARE:
			ret = sunxi_sound_pcm_prepare(dataflow);
			if (ret != 0) {
				snd_err("sunxi_sound_pcm_prepare failed.\n");
			}
			break;
		case SUNXI_SOUND_CTRL_PCM_RESET:
			ret = sunxi_sound_pcm_reset(dataflow);
			if (ret != 0) {
				snd_err("sunxi_sound_pcm_reset failed.\n");
			}
			break;
		case SUNXI_SOUND_CTRL_PCM_START:
			ret = sunxi_sound_pcm_start_lock_irq(dataflow);
			if (ret != 0) {
				snd_err("sunxi_sound_pcm_start failed.\n");
			}
			break;
		case SUNXI_SOUND_CTRL_PCM_RESUME:
			snd_err("sunxi_sound_pcm don't support resume.\n");
			break;
		case SUNXI_SOUND_CTRL_PCM_HWSYNC:
			ret = sunxi_sound_pcm_hw_sync(dataflow);
			if (ret != 0) {
				snd_err("sunxi_sound_pcm_hw_sync failed.\n");
			}
			break;
		case SUNXI_SOUND_CTRL_PCM_DELAY:
		{
			snd_pcm_sframes_t delay;
			ret = sunxi_sound_pcm_delay(dataflow, &delay);
			if (ret != 0){
				snd_err("sunxi_sound_pcm_delay failed.\n");
				return ret;
			}
			snd_info("delay %ld.\n", delay);
			*(snd_pcm_sframes_t *)argp = delay;
			break;
		}
		case SUNXI_SOUND_CTRL_PCM_SYNC_PTR:
			ret = sunxi_sound_pcm_get_sync_ptr(dataflow, argp);
			if (ret != 0){
				snd_err("sunxi_sound_pcm_get_sync_ptr failed.\n");
			}
			break;
		case SUNXI_SOUND_CTRL_PCM_DRAIN:
			ret = sunxi_sound_pcm_drain(dataflow);
			if (ret != 0){
				snd_err("sunxi_sound_pcm_drain failed.\n");
			}
			break;
		case SUNXI_SOUND_CTRL_PCM_DROP:
			ret = sunxi_sound_pcm_drop(dataflow);
			if (ret != 0){
				snd_err("sunxi_sound_pcm_drop failed.\n");
			}
			break;
		case SUNXI_SOUND_CTRL_PCM_PAUSE:
			ret = sunxi_sound_pcm_pause_lock_irq(dataflow, arg);
			if (ret != 0){
				snd_err("sunxi_sound_pcm_pause_lock_irq failed.\n");
			}
			break;
		default:
			snd_err("pcm(%s) type %d invalid.\n", dev->name, cmd);
			ret = -EINVAL;
			break;
	}

	return ret;
}

int sunxi_sound_pcm_dev_write(struct sound_device *dev, const void *buffer, snd_pcm_uframes_t size)
{
	struct sunxi_sound_pcm_priv *pcm_priv = NULL;
	struct sunxi_sound_pcm_dataflow *dataflow = NULL;
	struct sunxi_sound_pcm_running_param *pcm_running = NULL;
	snd_pcm_sframes_t ret;

	if (dev == NULL || dev->priv_data == NULL)
		return -ENXIO;

	pcm_priv = dev->priv_data;
	dataflow = pcm_priv->dataflow;
	if (dataflow == NULL || dataflow->pcm_running == NULL)
		return -ENXIO;

	pcm_running = dataflow->pcm_running;

	if (pcm_running->status->state == SNDRV_PCM_STATE_OPEN)
		return -EBADFD;

	ret = sunxi_sound_pcm_lib_write(dataflow, buffer, size);
	if (ret < 0) {
		snd_err("pcm(%s) write error ret %ld \n", dev->name, ret);
	}
	return ret;
}


int sunxi_sound_pcm_dev_read(struct sound_device *dev, void *buffer, snd_pcm_uframes_t size)
{
	struct sunxi_sound_pcm_priv *pcm_priv = NULL;
	struct sunxi_sound_pcm_dataflow *dataflow = NULL;
	struct sunxi_sound_pcm_running_param *pcm_running = NULL;
	snd_pcm_sframes_t ret;

	if (dev == NULL || dev->priv_data == NULL)
		return -ENXIO;

	pcm_priv = dev->priv_data;
	dataflow = pcm_priv->dataflow;
	if (dataflow == NULL || dataflow->pcm_running == NULL)
		return -ENXIO;

	pcm_running = dataflow->pcm_running;

	if (pcm_running->status->state == SNDRV_PCM_STATE_OPEN)
		return -EBADFD;

	ret = sunxi_sound_pcm_lib_read(dataflow, buffer, size);
	if (ret < 0) {
		snd_err("pcm(%s) read error ret %ld \n", dev->name, ret);
	}
	return ret;
}

static const struct sound_device_ops sunxi_sound_pcm_dev_ops[2] =
{
	{
		.open =		sunxi_sound_pcm_pb_dev_open,
		.close =	sunxi_sound_pcm_dev_close,
		.control =	sunxi_sound_pcm_dev_control,
		.write =	sunxi_sound_pcm_dev_write,
	},
	{
		.open =		sunxi_sound_pcm_cap_dev_open,
		.close =	sunxi_sound_pcm_dev_close,
		.control =	sunxi_sound_pcm_dev_control,
		.read	 =	sunxi_sound_pcm_dev_read,
	},
};

static int sunxi_sound_pcm_dev_register(struct sunxi_sound_device *device)
{
	struct sunxi_sound_pcm_priv *pcm_priv  = NULL;
	struct sunxi_sound_pcm *pcm = NULL;
	int idx, err;

	if (!device || !device->device_data)
		return -ENXIO;

	pcm_priv  = device->device_data;
	pcm = pcm_priv->pcm;

	sound_mutex_lock(sunxi_sound_pcm_mutex);
	err = sunxi_sound_pcm_add(pcm);
	if (err)
		goto err;
	for (idx = 0; idx < 2; idx++) {
		if (pcm->data[idx].dataflow == NULL)
			continue;
		err = sunxi_sound_register_device(pcm->card, &sunxi_sound_pcm_dev_ops[idx],
					pcm_priv, &pcm->data[idx].pcm_dev);
		if (err < 0) {
			list_del_init(&pcm->list);
			snd_err("sunxi sound register device %s failed.\n", pcm->data[idx].pcm_dev.name);
			goto err;
		}
	}
err:
	sound_mutex_unlock(sunxi_sound_pcm_mutex);
	return err;
}

static int sunxi_sound_pcm_dev_free(struct sunxi_sound_device *device)
{
	struct sunxi_sound_pcm_priv *pcm_priv = device->device_data;
	struct sunxi_sound_pcm *pcm  = pcm_priv->pcm;
	int idx, err;

	sound_mutex_lock(sunxi_sound_pcm_mutex);
	sound_mutex_lock(pcm->open_mutex);

	list_del_init(&pcm->list);

	for (idx = 0; idx < 2; idx++) {
		err = sunxi_sound_unregister_device(&pcm->card->dev, &pcm->data[idx].pcm_dev);
		if (err < 0)
			return err;
		sunxi_sound_pcm_dev_destory(&pcm->data[idx].pcm_dev);
	}

	sound_free(pcm_priv);

	sound_mutex_unlock(pcm->open_mutex);
	sound_mutex_unlock(sunxi_sound_pcm_mutex);

	sunxi_sound_free_pcm(pcm);
	snd_info("\n");

	return 0;
}

static int sunxi_sound_pcm_info_read(struct sunxi_sound_pcm_dataflow *dataflow,
		struct sunxi_sound_info_buffer *buf)
{
	struct sunxi_sound_pcm_info *info;
	int err;

	info = sound_malloc(sizeof(*info));
	if (!info)
		return -ENOMEM;

	err = sunxi_sound_pcm_get_info(dataflow, info);
	if (err < 0) {
		snprintf(buf->buffer, buf->len , "error %d\n", err);
		sound_free(info);
		return err;
	}

	snprintf(buf->buffer, buf->len ,
			" card: %d\n device: %d\n subdevice: %d\n stream: %s\n id: %s\n name: %s\n subname: %s\n subdevices_count: %d\n subdevices_avail: %d\n",
			info->card, info->device, info->subdevice, \
			(info->stream == SNDRV_PCM_STREAM_PLAYBACK) ? "Playback" : "Capture", \
			info->id, info->name, info->subname, info->subdevices_count, info->subdevices_avail);

	sound_free(info);

	return 0;
}

int sunxi_sound_dataflow_info_read(int card_num, int device, int subdevice, int stream,
					     struct sunxi_sound_info_buffer *buf)
{
	struct sunxi_sound_pcm *pcm;
	struct sunxi_sound_pcm_data *pcm_data;
	struct sunxi_sound_pcm_dataflow *dataflow;
	int err = 0;

	sound_mutex_lock(sunxi_sound_pcm_mutex);
	list_for_each_entry(pcm, &gSunxiSoundPcmList, list) {

		if (pcm->card->num == card_num &&
			pcm->device_num == device) {

			pcm_data = &pcm->data[stream];

			if (pcm_data->dataflow_count == 0) {
				err = -ENOENT;
				goto _error;
			}
			if (subdevice >= pcm_data->dataflow_count) {
				err = -ENXIO;
				goto _error;
			}

			for (dataflow = pcm_data->dataflow; dataflow;
				 dataflow = dataflow->next)
				if (dataflow->number == subdevice)
					break;

			if (dataflow == NULL) {
				err = -ENXIO;
				goto _error;
			}

			sound_mutex_lock(pcm->open_mutex);
			err = sunxi_sound_pcm_info_read(dataflow, buf);
			sound_mutex_unlock(pcm->open_mutex);

			break;
		}
	}

_error:

	sound_mutex_unlock(sunxi_sound_pcm_mutex);

	return err;

}

void sunxi_sound_pcm_read(struct sunxi_sound_info_buffer *buf)
{
	struct sunxi_sound_pcm *pcm;
	char tmp[80];

	sound_mutex_lock(sunxi_sound_pcm_mutex);
	list_for_each_entry(pcm, &gSunxiSoundPcmList, list) {
		snprintf(tmp, sizeof(tmp), "%02i-%02i: %s : %s",
			    pcm->card->num, pcm->device_num, pcm->id, pcm->name);
		strncat(buf->buffer, tmp, strlen(tmp) + 1);
		if (pcm->data[SNDRV_PCM_STREAM_PLAYBACK].dataflow) {
			snprintf(tmp, sizeof(tmp), " : playback %i",
				    pcm->data[SNDRV_PCM_STREAM_PLAYBACK].dataflow_count);
			strncat(buf->buffer, tmp, strlen(tmp) + 1);
		}
		if (pcm->data[SNDRV_PCM_STREAM_CAPTURE].dataflow) {
			snprintf(tmp, sizeof(tmp), " : capture %i",
				    pcm->data[SNDRV_PCM_STREAM_CAPTURE].dataflow_count);
			strncat(buf->buffer, tmp, strlen(tmp) + 1);
		}
		strncat(buf->buffer, "\n", 2);
	}
	sound_mutex_unlock(sunxi_sound_pcm_mutex);
}

int sunxi_sound_pcm_create(struct sunxi_sound_card *card, struct sunxi_sound_pcm *pcm)
{
	struct sunxi_sound_pcm_priv *pcm_priv = NULL;
	int ret;

	static const struct sunxi_sound_device_ops ops = {
		.dev_unregiter = sunxi_sound_pcm_dev_free,
		.dev_register =	sunxi_sound_pcm_dev_register,
	};

	if (!card || !pcm)
		return -ENXIO;

	if ((card->num < 0 || card->num >= SOUNDDRV_CARDS))
		return -ENXIO;

	pcm_priv = sound_malloc(sizeof(struct sunxi_sound_pcm_priv));
	if (!pcm_priv) {
		snd_err("pcm_priv malloc failed.\n");
		return -ENOMEM;
	}
	pcm_priv->pcm = pcm;

	ret = sunxi_sound_device_create(card, SNDRV_DEV_PCM, pcm_priv, &ops);
	if (ret  < 0) {
		snd_err("sunxi_sound_device_create failed card %s,:%d.\n",
				card->name, ret);
	}

	return ret;
}

