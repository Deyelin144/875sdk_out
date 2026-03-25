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

#include <aw_common.h>
#include <sound_v2/sunxi_sound_core.h>
#include <sound_v2/sunxi_sound_control.h>
#include <sound_v2/sunxi_sound_io.h>
#include <sound_v2/sunxi_sound_pcm_common.h>


static int sunxi_sound_adf_control_user_get(struct sunxi_sound_adf_control *adf_control, struct sunxi_sound_control_info *info)
{
	if (!adf_control->count || !adf_control->private_data) {
		snd_err("kcontrol count=%d, private_data=%p\n",
			adf_control->count, adf_control->private_data);
		return -1;
	}

	info->value = ((unsigned long *)adf_control->private_data)[0];
	if (info->private_data)
		memcpy(info->private_data, adf_control->private_data, adf_control->count * sizeof(unsigned long));
	else
		info->private_data = (unsigned long *)adf_control->private_data;
	info->id = adf_control->id;
	info->count  = adf_control->count;
	info->name = adf_control->name;
	info->max = adf_control->max;
	info->min = adf_control->min;
	snd_info("count=%d, value:%lu\n", adf_control->count, info->value);

	return 0;
}

static inline int sunxi_sound_adf_control_user_set_value(struct sunxi_sound_adf_control *adf_control,
						unsigned long value, int index)
{
	unsigned long *user_value;
	if (value < adf_control->min || value > adf_control->max) {
		snd_err("invalid range. min:%d, max:%d, value:%lu\n",
			adf_control->min, adf_control->max, value);
		return -1;
	}
	user_value = (unsigned long*)adf_control->private_data;
	user_value[index] = value;

	return 0;
}

static int sunxi_sound_adf_control_user_set(struct sunxi_sound_adf_control *adf_control, struct sunxi_sound_control_info * info)
{
	unsigned long*user_value;
	int i;
	if (!adf_control->count || !adf_control->private_data) {
		snd_err("kcontrol count=%d, private_data=%p\n",
			adf_control->count, adf_control->private_data);
		return -1;
	}

	if (adf_control->count == 1) {
		user_value = (unsigned long *)&info->value;
		return sunxi_sound_adf_control_user_set_value(adf_control, *user_value, 0);
	}

	for (i = 0; i < adf_control->count; i++) {
		user_value = (unsigned long *)&info->value;
		if (sunxi_sound_adf_control_user_set_value(adf_control, user_value[i], i) < 0)
			return -1;
	}

	return 0;
}

static int sunxi_sound_adf_control_get(struct sunxi_sound_adf_control *adf_control, struct sunxi_sound_control_info *info)
{
	struct sunxi_sound_adf_component *component = adf_control->private_data;
	unsigned int val = 0;

	val = sunxi_sound_component_read(component, adf_control->reg);

	sunxi_sound_adf_control_to_sound_ctrl_info(adf_control, info, val);

	return 0;
}

static int sunxi_sound_adf_control_enum_get(struct sunxi_sound_adf_control *adf_control,
			    struct sunxi_sound_control_info *info)
{
	struct sunxi_sound_adf_component *component = adf_control->private_data;
	unsigned int val = 0;

	if (adf_control->type != SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED) {
		snd_err("invalid kcontrol type = %d.\n", adf_control->type);
		return -EINVAL;
	}

	val = sunxi_sound_component_read(component, adf_control->reg);

	sunxi_sound_adf_control_to_sound_ctrl_info(adf_control, info, val);

	return 0;
}

static int sunxi_sound_adf_control_enum_set(struct sunxi_sound_adf_control *adf_control, struct sunxi_sound_control_info * info)
{
	struct sunxi_sound_adf_component *component = adf_control->private_data;

	if (adf_control->type != SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED) {
		snd_err("invalid kcontrol type = %d.\n", adf_control->type);
		return -EINVAL;
	}

	if (info->value >= adf_control->items) {
		snd_err("invalid kcontrol items = %ld.\n", info->value);
		return -EINVAL;
	}

	sunxi_sound_component_update_bits(component, adf_control->reg,
					(adf_control->mask << adf_control->shift),
					((unsigned int)info->value << adf_control->shift));

	adf_control->value = info->value & adf_control->mask;

	snd_info("mask:0x%x, shift:%d, value:%lu\n",
			adf_control->mask, adf_control->shift, adf_control->value);

	return 0;
}

void sunxi_sound_adf_control_to_sound_ctrl_info(struct sunxi_sound_adf_control *adf_control,
			struct sunxi_sound_control_info *info, unsigned long value)
{
	info->id = adf_control->id;
	info->type = adf_control->type;

	info->value = (unsigned long)((value >> adf_control->shift) & adf_control->mask);
	if (info->type == SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED) {
		if (info->value >= adf_control->items)
			info->value = adf_control->items - 1;
	} else {
		if (info->value > adf_control->max)
			info->value = adf_control->max;
		else if (info->value < adf_control->min)
			info->value = adf_control->min;
	}
	info->name = adf_control->name;
	info->min = adf_control->min;
	info->max = adf_control->max;
	info->count = adf_control->count;
	info->items = adf_control->items;
	info->texts = adf_control->texts;
	snd_debug("id:%d, type:%d, name:%s, value:%lu, min:0x%x, max:0x%x, count:0x%x, items:0x%x\n",
		adf_control->id, adf_control->type, info->name, info->value,
		info->min, info->max, info->count, info->items);
}

static int sunxi_sound_adf_control_set(struct sunxi_sound_adf_control *adf_control, struct sunxi_sound_control_info * info)
{
	struct sunxi_sound_adf_component *component = adf_control->private_data;
	sunxi_sound_component_update_bits(component, adf_control->reg,
				(adf_control->mask << adf_control->shift),
				((unsigned int)info->value << adf_control->shift));
	snd_info("mask:0x%x, shitf:%d, value:%lu\n",
			adf_control->mask, adf_control->shift, info->value);
	return 0;
}

int sunxi_sound_ctrl_remove_part(struct sunxi_sound_card *card, struct sunxi_sound_adf_control *control)
{
	if (!card || !control)
		return -EINVAL;
	snd_info("\n");

	if (control->dynamic != 1)
		return -1;

	list_del(&control->list);
	snd_info("\n");

	card->controls_nums -= control->count;

	sunxi_sound_control_destory(control);
	snd_info("\n");

	return 0;
}

int sunxi_sound_ctrl_add_part(struct sunxi_sound_card *card, struct sunxi_sound_control_info *info)
{
	struct sunxi_sound_adf_control *control;

	control = sound_malloc(sizeof(struct sunxi_sound_adf_control));
	if (!control) {
		snd_err("no memory\n");
		return -ENOMEM;
	}

	if (!info->count) {
		snd_info("unknown count, default 1.\n");
		info->count = 1;
	}
	control->dynamic = 1;
	control->name = sunxi_sound_strdup_const(info->name);
	if (!control->name) {
		snd_err("no memory\n");
		goto err;
	}
	control->private_data_type = SND_MODULE_USER;
	control->private_data = sound_malloc(info->count * sizeof(unsigned long));
	if (!control->private_data) {
		snd_err("no memory\n");
		goto err;
	}
	if (info->private_data) {
		memcpy(control->private_data, info->private_data, info->count * sizeof(unsigned long));
	} else {
		unsigned long *user_value;
		int i;
		user_value = (unsigned long *)control->private_data;
		for (i = 0; i < info->count; i++)
			user_value[i] = info->value;
	}
	control->count = info->count;
	control->max = info->max;
	control->min = info->min;
#if 0
	control->mask = ((1 << __fls(control->max)) - 1);
#else
	control->mask = 0;
#endif
	control->id = card->controls_nums++;
	control->get = sunxi_sound_adf_control_user_get;
	control->set = sunxi_sound_adf_control_user_set;

	list_add_tail(&control->list, &card->controls);
	return 0;
err:
	if (control->private_data)
		sound_free(control->private_data);
	if (control->name)
		sound_free(control->name);
	if (control)
		sound_free(control);
	return -1;
}

static inline void _sound_add_control(struct sunxi_sound_card *card, struct sunxi_sound_adf_control *control)
{
	if (control->private_data_type == SND_MODULE_USER) {
		struct sunxi_sound_control_info info;

		memset(&info, 0, sizeof(struct sunxi_sound_control_info));
		info.type = control->type;
		info.count = control->count;
		info.name = control->name;
		info.max = control->max;
		info.min = control->min;
		/* initialize value, use reg instead */
		info.value = control->reg;
		if (sunxi_sound_ctrl_add_part(card, &info) < 0)
			snd_err("add user control(%s) failed\n", control->name);
	} else {
		switch (control->type) {
		case SUNXI_SOUND_CTRL_PART_TYPE_INTEGER:
			control->mask = ((1 << __fls(control->max)) - 1);
			if (!control->get)
				control->get = sunxi_sound_adf_control_get;
			if (!control->set)
				control->set = sunxi_sound_adf_control_set;
			break;
		case SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED:
			if (control->mask == SUNXI_SOUND_CTRL_PART_AUTO_MASK)
				control->mask = ((1 << __fls(control->items-1)) - 1);
			if (!control->get)
				control->get = sunxi_sound_adf_control_enum_get;
			if (!control->set)
				control->set = sunxi_sound_adf_control_enum_set;
			break;
		default:
			snd_err("kcontrol(%s) type invalid.\n", control->name);
			break;
		}
		control->id = card->controls_nums++;
		list_add_tail(&control->list, &card->controls);
	}
}

static struct sunxi_sound_adf_control * sunxi_sound_ctl_find_adf_control(struct sunxi_sound_card *card,
			struct sunxi_sound_control_info *info)
{
	struct sunxi_sound_adf_control *adfctl;

	list_for_each_entry(adfctl, &card->controls, list) {
		if (!strlen(adfctl->name))
			continue;
		if ((info->name && !strncmp(adfctl->name, info->name, strlen(adfctl->name))) ||
			adfctl->id == info->id)
			return adfctl;
	}
	return NULL;
}

static int sunxi_sound_ctl_card_info(struct sunxi_sound_card *card, void *card_info)
{
	struct sunxi_sound_control_card_info *info = (struct sunxi_sound_control_card_info *)card_info;

	sound_mutex_lock(card->ctrl_mutex);
	info->card = card->num;
	strncpy(info->name, card->name, sizeof(info->name));
	sound_mutex_unlock(card->ctrl_mutex);
	return 0;
}

static int sunxi_sound_ctl_part_list(struct sunxi_sound_card *card,
			     struct sunxi_sound_control_part_list *list)
{
	struct sunxi_sound_adf_control *adfctl;
	unsigned int offset, space, jidx;
	int used = 0;

	offset = list->offset;
	space = list->space;

	sound_mutex_lock(card->ctrl_mutex);
	list->count = card->controls_nums;
	if (space > 0) {
		list_for_each_entry(adfctl, &card->controls, list) {
			if (offset >= adfctl->count) {
				offset -= adfctl->count;
				continue;
			}
			for (jidx = offset; jidx < adfctl->count; jidx++) {
				list->numid[jidx + used] = adfctl->id;
				used++;
				if (!--space)
					goto out;
			}
			offset = 0;
		}
	}
out:
	sound_mutex_unlock(card->ctrl_mutex);
	return 0;
}

static int sunxi_sound_ctl_part_read(struct sunxi_sound_card *card, void *info_params)
{
	int ret;
	struct sunxi_sound_adf_control *adfctl;
	struct sunxi_sound_control_info *info = (struct sunxi_sound_control_info *)info_params;


	sound_mutex_lock(card->ctrl_mutex);

	adfctl = sunxi_sound_ctl_find_adf_control(card, info);
	if (adfctl == NULL) {
		ret = -ENOENT;
		goto unlock;
	}

	ret = adfctl->get(adfctl, info);
	if (ret != 0) {
		goto unlock;
	}

unlock:
	sound_mutex_unlock(card->ctrl_mutex);
	return ret;
}

static int sunxi_sound_ctl_part_write(struct sunxi_sound_card *card, void *info_params)
{
	int ret;
	struct sunxi_sound_adf_control *adfctl;
	struct sunxi_sound_control_info *info = (struct sunxi_sound_control_info *)info_params;

	sound_mutex_lock(card->ctrl_mutex);

	adfctl = sunxi_sound_ctl_find_adf_control(card, info);
	if (adfctl == NULL) {
		ret = -ENOENT;
		goto unlock;
	}

	ret = adfctl->set(adfctl, info);
	if (ret != 0) {
		goto unlock;
	}

unlock:
	sound_mutex_unlock(card->ctrl_mutex);
	return ret;
}

static int sunxi_sound_ctl_add_part(struct sunxi_sound_card *card, void *info_params)
{
	int ret;
	struct sunxi_sound_adf_control *adfctl;
	struct sunxi_sound_control_info *info = (struct sunxi_sound_control_info *)info_params;

	sound_mutex_lock(card->ctrl_mutex);

	adfctl = sunxi_sound_ctl_find_adf_control(card, info);
	if (adfctl != NULL) {
		ret = -EEXIST;
		snd_info("card:%s,adf ctl[%u] \"%s\" is already exist\n", card->name, adfctl->id, adfctl->name);
		goto unlock;
	}

	ret = sunxi_sound_ctrl_add_part(card, info);
	if (ret != 0) {
		goto unlock;
	}

unlock:
	sound_mutex_unlock(card->ctrl_mutex);
	return ret;
}

static int sunxi_sound_ctl_del_part(struct sunxi_sound_card *card, void *info_params)
{
	int ret;
	struct sunxi_sound_adf_control *adfctl;
	struct sunxi_sound_control_info *info = (struct sunxi_sound_control_info *)info_params;

	sound_mutex_lock(card->ctrl_mutex);

	adfctl = sunxi_sound_ctl_find_adf_control(card, info);
	if (adfctl == NULL) {
		ret = -ENOENT;
		goto unlock;
	}

	ret = sunxi_sound_ctrl_remove_part(card, adfctl);
	if (ret != 0) {
		goto unlock;
	}

unlock:
	sound_mutex_unlock(card->ctrl_mutex);
	return ret;
}

struct sunxi_sound_adf_control* sunxi_sound_control_create(const struct sunxi_sound_adf_control_new *ncontrol,
				  void *private_data)
{
	struct sunxi_sound_adf_control *adf_ctl;
	unsigned int count;

	if (!ncontrol)
		return NULL;

	count = ncontrol->count;
	if (count == 0)
		count = 1;

	adf_ctl = sound_malloc(sizeof(struct sunxi_sound_adf_control));
	if (!adf_ctl) {
		snd_err("no memory\n");
		goto err;
	}

	adf_ctl->type = ncontrol->type;

	if (ncontrol->name) {
		adf_ctl->name = sunxi_sound_strdup_const(ncontrol->name);
		if (!adf_ctl->name) {
			snd_err("no memory\n");
			goto err;
		}
	}
	adf_ctl->reg = ncontrol->reg;
	adf_ctl->shift = ncontrol->shift;

	adf_ctl->max = ncontrol->max;
	adf_ctl->min = ncontrol->min;
	adf_ctl->mask = ncontrol->mask;
	adf_ctl->count = count;

	adf_ctl->items = ncontrol->items;
	adf_ctl->texts = ncontrol->texts;
	adf_ctl->value = ncontrol->value;

	adf_ctl->get = ncontrol->get;
	adf_ctl->set = ncontrol->set;

	adf_ctl->private_data = private_data;
	adf_ctl->private_data_type = ncontrol->private_data_type;

	return adf_ctl;
err:
	if (adf_ctl->name) {
		sound_free(adf_ctl->name);
		adf_ctl->name = NULL;
	}
	if (adf_ctl) {
		sound_free(adf_ctl);
		adf_ctl = NULL;
	}
	return NULL;
}

void sunxi_sound_control_destory(struct sunxi_sound_adf_control *adf_control)
{
	if (adf_control->name)
		sound_free((void *)adf_control->name);

	if (adf_control->private_data_type == SND_MODULE_USER && adf_control->private_data)
		sound_free(adf_control->private_data);

	if (adf_control)
		sound_free(adf_control);
}

int sound_add_control(struct sunxi_sound_card *card, struct sunxi_sound_adf_control *ctrl)
{
	int ret = -1;

	if (ctrl) {
		_sound_add_control(card, ctrl);
	} else {
		snd_err("control is null.\n");
		return ret;
	}

	return 0;
}

static int sunxi_sound_control_dev_create(struct sunxi_sound_card *card, struct sound_device *ctrl_dev)
{
	struct sound_card_device *dev = &card->dev;
	char name[32];

	if (dev->priv_data == NULL || ctrl_dev == NULL)
		return -ENXIO;

	snprintf(name, sizeof(name), "SunxiControlC%d", card->num);
	ctrl_dev->name = sunxi_sound_strdup_const(name);
	if (!ctrl_dev->name)
		return -ENOMEM;

	INIT_LIST_HEAD(&ctrl_dev->child_list);
	ctrl_dev->parent = dev;

	return 0;
}

static void sunxi_sound_control_dev_destory(struct sound_device *ctr_dev)
{
	if (ctr_dev->name)
		sound_free(ctr_dev->name);
	ctr_dev->parent = NULL;
}

int sunxi_sound_ctl_dev_open(struct sound_device *dev, unsigned int mode)
{
	struct sound_card_device *card_dev = NULL;
	struct sunxi_sound_card *card = NULL;
	struct sunxi_sound_ctl_private *priv = NULL;

	if (dev == NULL)
		return -ENXIO;

	card_dev = dev->parent;
	if (card_dev == NULL)
		return -ENXIO;

	card = (struct sunxi_sound_card *)card_dev->priv_data;
	if (card == NULL)
		return -ENXIO;

	/* control dev maybe open multi times */
	if (dev->priv_data) {
		sound_mutex_lock(card->ctrl_mutex);
		priv = (struct sunxi_sound_ctl_private *)dev->priv_data;
		if (priv->ref == 0) {
			snd_err("sound_ctl:%s, ref_count is %d\n", dev->name, priv->ref);
			sound_mutex_unlock(card->ctrl_mutex);
			return -EINVAL;
		}
		priv->ref++;
		sound_mutex_unlock(card->ctrl_mutex);
		return 0;
	}

	sound_mutex_lock(card->ctrl_mutex);
	priv = sound_malloc(sizeof(struct sunxi_sound_ctl_private));
	if (priv == NULL) {
		snd_err("malloc failed\n");
		return -ENOMEM;
	}
	priv->card = card;
	priv->ref++;
	dev->priv_data = priv;
	sound_mutex_unlock(card->ctrl_mutex);

	return 0;
}

int sunxi_sound_ctl_dev_close(struct sound_device *dev)
{
	struct sound_card_device *card_dev = NULL;
	struct sunxi_sound_card *card = NULL;
	struct sunxi_sound_ctl_private *priv = NULL;

	if (dev == NULL)
		return -ENXIO;

	if (dev->priv_data == NULL) {
		snd_info("sound_ctl is already closed\n");
		return 0;
	}

	card_dev = dev->parent;
	if (card_dev == NULL)
		return -ENXIO;

	card = (struct sunxi_sound_card *)card_dev->priv_data;
	if (card == NULL)
		return -ENXIO;

	sound_mutex_lock(card->ctrl_mutex);
	priv = dev->priv_data;
	priv->ref--;
	if (priv->ref == 0) {
		sound_free(priv);
		dev->priv_data = NULL;
	}
	sound_mutex_unlock(card->ctrl_mutex);
	return 0;

}

int sunxi_sound_ctl_dev_control(struct sound_device *dev, unsigned int cmd, unsigned long arg)
{
	struct sunxi_sound_card *card = NULL;
	struct sunxi_sound_ctl_private *priv = NULL;
	void *argp = (void *)arg;
	int ret;

	if (dev == NULL || dev->priv_data == NULL)
		return -ENXIO;

	priv = dev->priv_data;
	card = priv->card;
	if (card == NULL)
		return -ENXIO;

	switch (cmd) {
		case SUNXI_SOUND_CTRL_CARD_INFO:
			ret = sunxi_sound_ctl_card_info(card, argp);
			break;
		case SUNXI_SOUND_CTRL_PART_LIST:
			ret = sunxi_sound_ctl_part_list(card, argp);
			break;
		case SUNXI_SOUND_CTRL_PART_INFO:
		case SUNXI_SOUND_CTRL_PART_READ:
			ret = sunxi_sound_ctl_part_read(card, argp);
			break;
		case SUNXI_SOUND_CTRL_PART_WRITE:
			ret = sunxi_sound_ctl_part_write(card, argp);
			break;
		case SUNXI_SOUND_CTRL_PART_ADD:
			ret = sunxi_sound_ctl_add_part(card, argp);
			break;
		case SUNXI_SOUND_CTRL_PART_REMOVE:
			ret = sunxi_sound_ctl_del_part(card, argp);
			break;
		default:
			snd_err("control(%s) type %d invalid.\n", dev->name, cmd);
			ret = -EINVAL;
			break;
	}

	return ret;
}

static const struct sound_device_ops sunxi_sound_ctl_dev_ops =
{
	.open =		sunxi_sound_ctl_dev_open,
	.close =	sunxi_sound_ctl_dev_close,
	.control =	sunxi_sound_ctl_dev_control,
};

static int sunxi_sound_control_dev_register(struct sunxi_sound_device *device)
{
	struct sunxi_sound_card *card = device->device_data;
	int err;

	err = sunxi_sound_register_device(card, &sunxi_sound_ctl_dev_ops, NULL, &card->control_dev);
	if (err < 0)
		return err;

	return 0;
}

static int sunxi_sound_control_dev_free(struct sunxi_sound_device *device)
{
	struct sunxi_sound_card *card = device->device_data;
	struct sunxi_sound_adf_control *control, *tmp;
	int err;

	err = sunxi_sound_unregister_device(&card->dev, &card->control_dev);
	if (err < 0)
		return err;

	sunxi_sound_control_dev_destory(&card->control_dev);

	list_for_each_entry_safe(control, tmp, &card->controls, list) {
		sunxi_sound_ctrl_remove_part(card, control);
	}
	snd_info("\n");

	return 0;
}

int sunxi_sound_control_new(struct sunxi_sound_card *card)
{
	int ret;

	static const struct sunxi_sound_device_ops ops = {
		.dev_unregiter = sunxi_sound_control_dev_free,
		.dev_register =	sunxi_sound_control_dev_register,
	};

	if (!card)
		return -ENXIO;

	if ((card->num < 0 || card->num >= SOUNDDRV_CARDS))
		return -ENXIO;

	ret = sunxi_sound_control_dev_create(card, &card->control_dev);
	if (ret  < 0) {
		snd_err("control %s: create failed ret:%d.\n",
				card->control_dev.name, ret);
		return ret;
	}

	ret = sunxi_sound_device_create(card, SNDRV_DEV_CONTROL, card, &ops);
	if (ret  < 0) {
		sunxi_sound_control_dev_destory(&card->control_dev);
	}
	return ret;

}

