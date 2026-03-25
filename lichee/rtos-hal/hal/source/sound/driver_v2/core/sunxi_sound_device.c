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
#include "aw_list.h"

#include <aw_common.h>
#include <sound_v2/sunxi_sound_core.h>
#include <sound_v2/sunxi_sound_pcm.h>
#include <sound_v2/sunxi_sound_control.h>

static struct sunxi_sound_card *sunxi_sound_cards[SUNXI_SOUND_CARDS];
static sunxi_sound_mutex_t sunxi_sound_card_mutex;

int sunxi_sound_device_create(struct sunxi_sound_card *card, enum sunxi_sound_device_type type,
		   void *dev_data, const struct sunxi_sound_device_ops *ops)
{
	struct sunxi_sound_device *dev;
	struct list_head *p;

	if (!card || !dev_data || !ops)
		return -ENXIO;

	dev = sound_malloc(sizeof(*dev));
	if (!dev)
		return -ENOMEM;

	INIT_LIST_HEAD(&dev->list);
	dev->card = card;
	dev->type = type;
	dev->state = SNDRV_DEV_BUILD;
	dev->device_data = dev_data;
	dev->ops = ops;

	/* insert the entry in an incrementally sorted list */
	list_for_each_prev(p, &card->devices) {
		struct sunxi_sound_device *pdev = list_entry(p, struct sunxi_sound_device, list);
		if ((unsigned int)pdev->type <= (unsigned int)type)
			break;
	}

	list_add(&dev->list, p);
	return 0;
}

static void __sunxi_sound_device_destory(struct sunxi_sound_device *dev)
{
	/* unlink */
	list_del(&dev->list);

	if (dev->ops->dev_unregiter) {
		if (dev->ops->dev_unregiter(dev))
			snd_err("sound device free failure\n");
	}
	sound_free(dev);
}

static int __sunxi_sound_device_register(struct sunxi_sound_device *dev)
{
	if (dev->state == SNDRV_DEV_BUILD) {
		if (dev->ops->dev_register) {
			int err = dev->ops->dev_register(dev);
			if (err < 0)
				return err;
		}
		dev->state = SNDRV_DEV_REGISTERED;
	}
	return 0;
}

static struct sunxi_sound_device *sunxi_sound_look_for_dev(struct sunxi_sound_card *card, void *device_data)
{
	struct sunxi_sound_device *dev;

	list_for_each_entry(dev, &card->devices, list)
		if (dev->device_data == device_data)
			return dev;

	return NULL;
}

void sunxi_sound_device_destory(struct sunxi_sound_card *card, void *device_data)
{
	struct sunxi_sound_device *dev;

	if (!card || !device_data)
		return;
	dev = sunxi_sound_look_for_dev(card, device_data);
	if (dev)
		__sunxi_sound_device_destory(dev);
	else
		snd_debug("device free %p, not found\n",device_data);
}

int sunxi_sound_device_register(struct sunxi_sound_card *card, void *device_data)
{
	struct sunxi_sound_device *dev;

	if (!card || !device_data)
		return -ENXIO;
	dev = sunxi_sound_look_for_dev(card, device_data);
	if (dev)
		return __sunxi_sound_device_register(dev);

	snd_err("sound device register failure\n");
	return -ENXIO;
}

int sunxi_sound_device_register_all(struct sunxi_sound_card *card)
{
	struct sunxi_sound_device *dev;
	int err;

	if (!card)
		return -ENXIO;

	list_for_each_entry(dev, &card->devices, list) {
		err = __sunxi_sound_device_register(dev);
		if (err < 0)
			return err;
	}

	sound_mutex_lock(sunxi_sound_card_mutex);
	if (sunxi_sound_cards[card->num]) {
		/* already registered */
		sound_mutex_unlock(sunxi_sound_card_mutex);
		return 0;
	}
	sunxi_sound_cards[card->num] = card;
	sound_mutex_unlock(sunxi_sound_card_mutex);
	return 0;
}

void sunxi_sound_device_destory_all(struct sunxi_sound_card *card)
{
	struct sunxi_sound_device *dev, *next;

	if (!card)
		return;

	list_for_each_entry_safe_reverse(dev, next, &card->devices, list) {
		/* exception: free ctl and lowlevel stuff later */
		if (dev->type == SNDRV_DEV_CONTROL ||
		    dev->type == SNDRV_DEV_LOWLEVEL)
			continue;
		__sunxi_sound_device_destory(dev);
	}

	/* free all */
	list_for_each_entry_safe_reverse(dev, next, &card->devices, list)
		__sunxi_sound_device_destory(dev);

	sound_mutex_lock(sunxi_sound_card_mutex);
	sunxi_sound_cards[card->num] = NULL;
	sound_mutex_unlock(sunxi_sound_card_mutex);
}

void sunxi_sound_cards_info(struct sunxi_sound_info_buffer *buf)
{
	int idx, count;
	struct sunxi_sound_card *card;
	char tmp[32];

	for (idx = count = 0; idx < SUNXI_SOUND_CARDS; idx++) {
		sound_mutex_lock(sunxi_sound_card_mutex);
		card = sunxi_sound_cards[idx];
		if (card) {
			count++;
			snprintf(tmp, sizeof(tmp), "%2i : %s\n",
					idx,
					card->name);
			strncat(buf->buffer, tmp, strlen(tmp) + 1);
		}
		sound_mutex_unlock(sunxi_sound_card_mutex);
	}
	if (!count)
		snprintf(buf->buffer, buf->len, "--- no sound cards ---\n");
}


static struct sound_card_device *g_sunxi_sound_dev_first = NULL;
static sunxi_sound_mutex_t sunxi_sound_mutex, sunxi_sound_dev_mutex;
static int g_sound_init_count;

void sunxi_sound_dev_init(struct sound_card_device* dev)
{
	INIT_LIST_HEAD(&dev->child_head);
	INIT_LIST_HEAD(&dev->sibling_node);
}

static int sunxi_sound_card_init()
{
	if (!g_sunxi_sound_dev_first) {
		g_sunxi_sound_dev_first = sound_malloc(sizeof(struct sound_card_device));
		if (!g_sunxi_sound_dev_first)
			return -ENOMEM;
		g_sunxi_sound_dev_first->name = "sunxi_sound_dev_first";
		sunxi_sound_dev_init(g_sunxi_sound_dev_first);
	}

	if (!sunxi_sound_mutex) {
		sunxi_sound_mutex = sound_mutex_init();
		if (!sunxi_sound_mutex) {
			snd_err("hal_mutex_create failed\n");
			return -EINVAL;
		}
	}

	if (!sunxi_sound_card_mutex) {
		sunxi_sound_card_mutex = sound_mutex_init();
		if (!sunxi_sound_card_mutex) {
			snd_err("hal_mutex_create failed\n");
			return -EINVAL;
		}
	}

	if (!sunxi_sound_dev_mutex) {
		sunxi_sound_dev_mutex = sound_mutex_init();
		if (!sunxi_sound_dev_mutex) {
			snd_err("hal_mutex_create failed\n");
			return -EINVAL;
		}
	}
	g_sound_init_count++;

	return 0;
}

static void sunxi_sound_card_deinit()
{
	g_sound_init_count--;
	if (g_sound_init_count == 0) {
		if (g_sunxi_sound_dev_first) {
			sound_free(g_sunxi_sound_dev_first);
			g_sunxi_sound_dev_first = NULL;
		}

		if (sunxi_sound_mutex) {
			sound_mutex_destroy(sunxi_sound_mutex);
			sunxi_sound_mutex = NULL;
		}

		if (sunxi_sound_card_mutex) {
			sound_mutex_destroy(sunxi_sound_card_mutex);
			sunxi_sound_card_mutex = NULL;
		}

		if (sunxi_sound_dev_mutex) {
			sound_mutex_destroy(sunxi_sound_dev_mutex);
			sunxi_sound_dev_mutex = NULL;
		}
	}
}


int sunxi_sound_card_get_idx(struct sunxi_sound_card *card)
{
	struct sound_card_device *dev = NULL, *_dev, *tmp;
	int idx = 0;

	if (card->dev.sibling == NULL)
		dev = g_sunxi_sound_dev_first;
	else
		dev = card->dev.sibling;

	if (list_empty(&dev->sibling_node))
		return 0;

	list_for_each_entry_safe(_dev, tmp, &dev->sibling_node, sibling_node)
		idx++;

	return idx;
}

int sunxi_sound_card_create(const char* name, struct sunxi_sound_card **card_ret)
{
	struct sunxi_sound_card *card;
	int ret;

	if (!card_ret || !name)
		return -EINVAL;

	*card_ret = NULL;

	sunxi_sound_card_init();

	card = sound_malloc(sizeof(*card));
	if (!card)
		return -ENOMEM;

	card->name = sunxi_sound_strdup_const(name);
	if (!card->name)
		return -ENOMEM;

	card->ctrl_mutex = sound_mutex_init();
	if (!card->ctrl_mutex) {
		snd_err("ctrl_mutex create failed !\n");
		return -1;
	}

	sunxi_sound_dev_init(&card->dev);
	card->dev.name = card->name;
	card->dev.sibling = g_sunxi_sound_dev_first;
	card->dev.priv_data = card;

	sound_mutex_lock(sunxi_sound_card_mutex);

	card->num = sunxi_sound_card_get_idx(card);

	list_add_tail(&card->dev.sibling_node, &g_sunxi_sound_dev_first->sibling_node);

	sound_mutex_unlock(sunxi_sound_card_mutex);

	INIT_LIST_HEAD(&card->devices);
	INIT_LIST_HEAD(&card->controls);

	ret = sunxi_sound_control_new(card);
	if (ret < 0) {
		snd_err("sunxi_sound_control new  failed !\n");
		return ret;
	}

	*card_ret = card;
	return 0;

}

int sunxi_sound_card_destory(struct sunxi_sound_card *card)
{
	if (card == NULL)
		return 0;

	sunxi_sound_device_destory_all(card);

	if (card->ctrl_mutex)
		sound_mutex_destroy(card->ctrl_mutex);

	list_del(&card->dev.sibling_node);
	card->dev.sibling = NULL;
	card->dev.priv_data = NULL;

	if (card->name) {
		sound_free(card->name);
	}

	sound_free(card);

	sunxi_sound_card_deinit();
	snd_info("\n");

	return 0;
}

int sunxi_sound_register_device(struct sunxi_sound_card *card, const struct sound_device_ops *sdev_ops,
			void *private_data, struct sound_device *dev)
{

	if (!dev)
		return -EINVAL;

	sound_mutex_lock(sunxi_sound_mutex);
	dev->sdev_ops = sdev_ops;
	if (private_data)
		dev->priv_data = private_data;
	list_add_tail(&dev->child_list, &card->dev.child_head);
	sound_mutex_unlock(sunxi_sound_mutex);

	return 0;
}

int sunxi_sound_unregister_device(struct sound_card_device *card_dev, struct sound_device *dev)
{
	struct sound_device *sdev = NULL, *tmp;

	sound_mutex_lock(sunxi_sound_mutex);
	list_for_each_entry_safe(sdev, tmp, &card_dev->child_head, child_list) {
		if (sdev && sdev == dev) {
			list_del(&dev->child_list);
			dev->priv_data = NULL;
			break;
		}
	}
	sound_mutex_unlock(sunxi_sound_mutex);
	return 0;
}

static struct sound_device *sunxi_sound_find_sound_dev(
	const struct sound_card_device *dev, const char *devname)
{
	struct sound_card_device *_dev, *tmp;
	struct sound_device *sdev = NULL;

	/* Find sound device from registered sound_card_device */
	list_for_each_entry_safe(_dev, tmp, &dev->sibling_node, sibling_node) {
		list_for_each_entry(sdev, &_dev->child_head, child_list) {
			if (strcmp(sdev->name, devname))
				continue;

			return sdev;
		}
	}

	return NULL;
}

int sunxi_sound_device_open(sound_device_t **sound_dev, const char *devname, int mode)
{
	struct sound_card_device *dev = g_sunxi_sound_dev_first;
	sound_device_t *sdev = NULL;
	int ret = 0;

	*sound_dev = NULL;

	if (dev == NULL) {
		snd_err("sunxi sound open dev failed\n");
		return -EINVAL;
	}

	sound_mutex_lock(sunxi_sound_dev_mutex);

	sdev = sunxi_sound_find_sound_dev(dev, devname);
	if (!sdev) {
		snd_err("%s is not exit in sound device \n", devname);
		ret = -ENOENT;
		goto err;
	}

	if (sdev->sdev_ops->open) {
		ret = sdev->sdev_ops->open(sdev, mode);
		if (ret < 0) {
			snd_err("sdev %s:open error on :%d.\n",
				sdev->name, ret);
			goto err;
		}
	}

	*sound_dev = sdev;

err:
	sound_mutex_unlock(sunxi_sound_dev_mutex);

	return ret;
}

int sunxi_sound_device_close(sound_device_t *sound_dev)
{
	int ret = 0;

	if (sound_dev == NULL) {
		snd_info("sunxi sound dev already close\n");
		return 0;
	}

	sound_mutex_lock(sunxi_sound_dev_mutex);

	if (sound_dev->sdev_ops->close) {
		ret = sound_dev->sdev_ops->close(sound_dev);
		if (ret < 0) {
			snd_err("sdev %s:open error on :%d.\n",
				sound_dev->name, ret);
			goto err;
		}
	}
err:
	sound_mutex_unlock(sunxi_sound_dev_mutex);

	return ret;
}

int sunxi_sound_device_control(sound_device_t *sound_dev, unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	if (sound_dev == NULL) {
		snd_err("sunxi sound dev is NULL\n");
		return -EINVAL;
	}

	sound_mutex_lock(sunxi_sound_dev_mutex);

	if (sound_dev->sdev_ops->control) {
		ret = sound_dev->sdev_ops->control(sound_dev, cmd, arg);
		if (ret < 0) {
			snd_err("sdev %s:control error on :%d.\n",
				sound_dev->name, ret);
			goto err;
		}
	}
err:
	sound_mutex_unlock(sunxi_sound_dev_mutex);

	return ret;
}

int sunxi_sound_device_write(sound_device_t *sound_dev, const void *buffer, snd_pcm_uframes_t size)
{
	int ret = 0;

	if (sound_dev == NULL) {
		snd_err("sunxi sound dev is NULL\n");
		return -EINVAL;
	}

	if (sound_dev->sdev_ops->write) {
		ret = sound_dev->sdev_ops->write(sound_dev, buffer, size);
		if (ret < 0) {
			snd_err("sdev %s:write error on :%d.\n",
				sound_dev->name, ret);
			goto err;
		}
	}
err:
	return ret;
}

int sunxi_sound_device_read(sound_device_t *sound_dev, void *buffer, snd_pcm_uframes_t size)
{
	int ret = 0;

	if (sound_dev == NULL) {
		snd_err("sunxi sound dev is NULL\n");
		return -EINVAL;
	}

	if (sound_dev->sdev_ops->read) {
		ret = sound_dev->sdev_ops->read(sound_dev, buffer, size);
		if (ret < 0) {
			snd_err("sdev %s:read error on :%d.\n",
				sound_dev->name, ret);
			goto err;
		}
	}
err:
	return ret;
}


