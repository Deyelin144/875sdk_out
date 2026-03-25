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
#include <ctype.h>
#include <errno.h>
#include <aw_common.h>
#include <hal_mutex.h>
#include <FreeRTOS.h>
#include <assert.h>

#include <aw-tiny-sound-lib/sunxi_tiny_sound_mixer.h>

struct sunxi_mixer {
    sound_device_t *mixer_dev;
    /** Card information */
    struct sunxi_sound_control_card_info card_info;
    /** A continuous array of mixer controls */
    struct sunxi_mixer_ctl *ctl;
    /** The number of mixer controls */
    unsigned int count;
};

struct sunxi_mixer_ctl {
    /** The mixer that the mixer control belongs to */
    struct sunxi_mixer *mixer;
    /** Information on the control's value (i.e. type, number of values) */
    struct sunxi_sound_control_info info;
    /** A list of string representations of enumerated values (only valid for enumerated controls) */
    char **ename;
};

static void sunxi_mixer_cleanup_control(struct sunxi_mixer_ctl *ctl)
{
    unsigned int m;

    if (ctl->ename) {
        unsigned int max = ctl->info.items;
        for (m = 0; m < max; m++)
            free(ctl->ename[m]);
        free(ctl->ename);
    }
}

void sunxi_mixer_close(struct sunxi_mixer *mixer)
{
    unsigned int n;

    if (!mixer)
        return;

    if (mixer->mixer_dev)
        sunxi_sound_device_close(mixer->mixer_dev);

    if (mixer->ctl) {
        for (n = 0; n < mixer->count; n++)
            sunxi_mixer_cleanup_control(&mixer->ctl[n]);
        free(mixer->ctl);
    }

    free(mixer);

    /* TODO: verify frees */
}

static int sunxi_mixer_add_controls(struct sunxi_mixer *mixer)
{
    struct sunxi_sound_control_part_list plist;
    unsigned int *pid = NULL;
    struct sunxi_mixer_ctl *ctl;
    const unsigned int old_count = mixer->count;
    unsigned int new_count;
    unsigned int n;
    int ret;

    memset(&plist, 0, sizeof(struct sunxi_sound_control_part_list));

    ret = sunxi_sound_device_control(mixer->mixer_dev, SUNXI_SOUND_CTRL_PART_LIST, (unsigned long)&plist);
    if (ret < 0) {
        awtinymix_err("cannot get part list\n");
        goto fail;
    }

    if (old_count == plist.count)
        return 0; /* no new controls return unchanged */

    if (old_count > plist.count)
        return -1; /* driver has removed controls - this is bad */

    ctl = realloc(mixer->ctl, sizeof(struct sunxi_mixer_ctl) * plist.count);
    if (!ctl)
        goto fail;

    memset(ctl + (old_count * sizeof(struct sunxi_mixer_ctl)), 0, (plist.count - old_count) * sizeof(struct sunxi_mixer_ctl));

    mixer->ctl = ctl;

    new_count = plist.count;
    plist.space = new_count - old_count; /* controls we haven't seen before */
    plist.offset = old_count; /* first control we haven't seen */

    pid = calloc(plist.space, sizeof(unsigned int));
    if (!pid)
        goto fail;

    plist.numid = pid;

    ret = sunxi_sound_device_control(mixer->mixer_dev, SUNXI_SOUND_CTRL_PART_LIST, (unsigned long)&plist);
    if (ret < 0) {
        awtinymix_err("cannot get part list\n");
        goto fail;
    }

    for (n = old_count; n < new_count; n++) {
        struct sunxi_sound_control_info *pi = &mixer->ctl[n].info;
        pi->id = pid[n - old_count];
        ret = sunxi_sound_device_control(mixer->mixer_dev, SUNXI_SOUND_CTRL_PART_INFO, (unsigned long)pi);
        if (ret < 0) {
            awtinymix_err("cannot get info\n");
            goto fail_extend;
        }
        ctl[n].mixer = mixer;
    }

    mixer->count = new_count;
    free(pid);
    return 0;

fail_extend:

    sunxi_mixer_cleanup_control(&ctl[n]);

    mixer->count = n;   /* keep controls we successfully added */
    /* fall through... */
fail:
    free(pid);
    return -1;
}

int sunxi_mixer_add_ctls(struct sunxi_mixer *mixer, struct sunxi_sound_control_info *info)
{
    int ret;

    if (!mixer)
        return 0;

    ret = sunxi_sound_device_control(mixer->mixer_dev, SUNXI_SOUND_CTRL_PART_ADD, (unsigned long)info);
    if (ret < 0) {
        awtinymix_err("cannot add ctl\n");
        return ret;
    }
    return 0;
}

int sunxi_mixer_remove_ctls(struct sunxi_mixer *mixer, unsigned int id)
{
    int ret;
    struct sunxi_sound_control_info info;

    if (!mixer)
        return 0;

    memset(&info, 0, sizeof(info));
    info.id = id;
    ret = sunxi_sound_device_control(mixer->mixer_dev, SUNXI_SOUND_CTRL_PART_REMOVE, (unsigned long)&info);
    if (ret < 0) {
        awtinymix_err("cannot remove ctl\n");
        return ret;
    }
    return 0;
}

struct sunxi_mixer *sunxi_mixer_open(unsigned int card)
{
    struct sunxi_mixer *mixer = NULL;
    char fn[64];
    int ret;

    mixer = calloc(1, sizeof(struct sunxi_mixer));
    if (!mixer) {
        awtinymix_err("no memory!\n");
        goto fail;
    }

    snprintf(fn, sizeof(fn), "SunxiControlC%u", card);
    ret = sunxi_sound_device_open(&mixer->mixer_dev, fn, 0);
    if (ret < 0) {
        awtinymix_err("cannot open device '%s' \n", fn);
        goto fail;
    }

    ret = sunxi_sound_device_control(mixer->mixer_dev, SUNXI_SOUND_CTRL_CARD_INFO, (unsigned long)&mixer->card_info);
    if (ret < 0) {
        awtinymix_err("cannot get info\n");
        goto fail;
    }

    if (sunxi_mixer_add_controls(mixer) != 0)
        goto fail;

    return mixer;

fail:
    if (mixer)
        sunxi_mixer_close(mixer);
    return NULL;
}

unsigned int sunxi_mixer_get_num_ctls(const struct sunxi_mixer *mixer)
{
    if (!mixer)
        return 0;

    return mixer->count;
}

struct sunxi_mixer_ctl *sunxi_mixer_get_ctl(struct sunxi_mixer *mixer, unsigned int id)
{
    if (mixer && (id < mixer->count))
        return mixer->ctl + id;

    return NULL;
}

struct sunxi_mixer_ctl *sunxi_mixer_get_ctl_by_name(struct sunxi_mixer *mixer, const char *name)
{
    return sunxi_mixer_get_ctl_by_name_and_index(mixer, name, 0);
}

struct sunxi_mixer_ctl *sunxi_mixer_get_ctl_by_name_and_index(struct sunxi_mixer *mixer,
                                                  const char *name,
                                                  unsigned int index)
{
    unsigned int n;
    struct sunxi_mixer_ctl *ctl;

    if (!mixer)
        return NULL;

    ctl = mixer->ctl;

    for (n = 0; n < mixer->count; n++)
        if (!strcmp(name, (char*) ctl[n].info.name))
            if (index-- == 0)
                return mixer->ctl + n;

    return NULL;
}

const char *sunxi_mixer_ctl_get_name(const struct sunxi_mixer_ctl *ctl)
{
    if (!ctl)
        return NULL;

    return (const char *)ctl->info.name;
}

enum sunxi_mixer_ctl_type sunxi_mixer_ctl_get_type(const struct sunxi_mixer_ctl *ctl)
{
    if (!ctl)
        return SUNXI_MIXER_CTL_TYPE_UNKNOWN;

    switch (ctl->info.type) {
    case SUNXI_SOUND_CTRL_PART_TYPE_INTEGER:    return SUNXI_MIXER_CTL_TYPE_INT;
    case SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED: return SUNXI_MIXER_CTL_TYPE_ENUM;
    default:                                    return SUNXI_MIXER_CTL_TYPE_UNKNOWN;
    };
}

const char *sunxi_mixer_ctl_get_type_string(const struct sunxi_mixer_ctl *ctl)
{
    if (!ctl)
        return "";

    switch (ctl->info.type) {
    case SUNXI_SOUND_CTRL_PART_TYPE_INTEGER:    return "INT";
    case SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED: return "ENUM";
    default:                             return "Unknown";
    };
}

unsigned int sunxi_mixer_ctl_get_num_values(const struct sunxi_mixer_ctl *ctl)
{
    if (!ctl)
        return 0;

    return ctl->info.count;
}

int sunxi_mixer_ctl_get_value(const struct sunxi_mixer_ctl *ctl, unsigned int id)
{
    struct sunxi_sound_control_info info;
    int ret;

    if (!ctl || (id >= ctl->info.count))
        return -EINVAL;

    memset(&info, 0, sizeof(struct sunxi_sound_control_info));
    info.id = ctl->info.id;

    ret = sunxi_sound_device_control(ctl->mixer->mixer_dev, SUNXI_SOUND_CTRL_PART_READ, (unsigned long)&info);
    if (ret < 0) {
        awtinymix_err("cannot read part\n");
        return ret;
    }

    switch (ctl->info.type) {

    case SUNXI_SOUND_CTRL_PART_TYPE_INTEGER:
        return info.value;

    case SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED:
        return info.value;

    default:
        return -EINVAL;
    }

    return 0;
}

int sunxi_mixer_ctl_set_value(struct sunxi_mixer_ctl *ctl, unsigned int id, int value)
{
    struct sunxi_sound_control_info info;
    int ret;

    if (!ctl || (id >= ctl->info.count))
        return -EINVAL;

    memset(&info, 0, sizeof(struct sunxi_sound_control_info));
    info.id = ctl->info.id;

    ret = sunxi_sound_device_control(ctl->mixer->mixer_dev, SUNXI_SOUND_CTRL_PART_READ, (unsigned long)&info);
    if (ret < 0) {
        awtinymix_err("cannot read part\n");
        return ret;
    }

    switch (ctl->info.type) {

    case SUNXI_SOUND_CTRL_PART_TYPE_INTEGER:
        if ((value < sunxi_mixer_ctl_get_range_min(ctl)) ||
            (value > sunxi_mixer_ctl_get_range_max(ctl))) {
            return -EINVAL;
        }

        info.value = value;
        break;

    case SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED:
        info.value = value;
        break;

    default:
        return -EINVAL;
    }

    ret = sunxi_sound_device_control(ctl->mixer->mixer_dev, SUNXI_SOUND_CTRL_PART_WRITE, (unsigned long)&info);
    if (ret < 0) {
        awtinymix_err("cannot write part\n");
        return ret;
    }
    return ret;
}

int sunxi_mixer_ctl_get_range_min(const struct sunxi_mixer_ctl *ctl)
{
    if (!ctl || (ctl->info.type != SUNXI_SOUND_CTRL_PART_TYPE_INTEGER))
        return -EINVAL;

    return ctl->info.min;
}

int sunxi_mixer_ctl_get_range_max(const struct sunxi_mixer_ctl *ctl)
{
    if (!ctl || (ctl->info.type != SUNXI_SOUND_CTRL_PART_TYPE_INTEGER))
        return -EINVAL;

    return ctl->info.max;
}

unsigned int sunxi_mixer_ctl_get_num_enums(const struct sunxi_mixer_ctl *ctl)
{
    if (!ctl)
        return 0;

    return ctl->info.items;
}

int sunxi_mixer_ctl_fill_enum_string(struct sunxi_mixer_ctl *ctl)
{
    struct sunxi_sound_control_info tmp;
    unsigned int m;
    int ret;
    char **enames;

    if (ctl->ename) {
        return 0;
    }

    enames = calloc(ctl->info.items, sizeof(char*));
    if (!enames)
        goto fail;

    memset(&tmp, 0, sizeof(tmp));
    tmp.id = ctl->info.id;
    ret = sunxi_sound_device_control(ctl->mixer->mixer_dev, SUNXI_SOUND_CTRL_PART_INFO, (unsigned long)&tmp);
    if (ret < 0) {
        awtinymix_err("cannot get info\n");
        goto fail;
    }

    for (m = 0; m < ctl->info.items; m++) {
        enames[m] = strdup(tmp.texts[m]);
        if (!enames[m])
            goto fail;
    }
    ctl->ename = enames;
    return 0;

fail:
    if (enames) {
        for (m = 0; m < ctl->info.items; m++) {
            if (enames[m]) {
                free(enames[m]);
            }
        }
        free(enames);
    }
    return -1;
}

const char *sunxi_mixer_ctl_get_enum_string(struct sunxi_mixer_ctl *ctl,
                                      unsigned int enum_id)
{
    if (!ctl || (ctl->info.type != SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED) ||
        (enum_id >= ctl->info.items) ||
        sunxi_mixer_ctl_fill_enum_string(ctl) != 0)
        return NULL;

    return (const char *)ctl->ename[enum_id];
}

int sunxi_mixer_ctl_set_enum_by_string(struct sunxi_mixer_ctl *ctl, const char *string)
{
    unsigned int i, num_enums;
    struct sunxi_sound_control_info info;
    int ret;

    if (!ctl || (ctl->info.type != SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED) ||
        sunxi_mixer_ctl_fill_enum_string(ctl) != 0)
        return -EINVAL;

    num_enums = ctl->info.items;
    for (i = 0; i < num_enums; i++) {
        if (!strcmp(string, ctl->ename[i])) {
            memset(&info, 0, sizeof(info));
            info.value = i;
            info.id = ctl->info.id;
            ret = sunxi_sound_device_control(ctl->mixer->mixer_dev, SUNXI_SOUND_CTRL_PART_WRITE, (unsigned long)&info);
            if (ret < 0) {
                awtinymix_err("cannot write part\n");
                return ret;
            }
            return 0;
        }
    }

    return -EINVAL;
}

int is_number(const char *str)
{
	char *end;

	if (str == NULL || strlen(str) == 0)
		return 0;

	strtol(str, &end, 0);
	return strlen(end) == 0;
}

int sunxi_mixer_ctl_set_ctl_value(unsigned int card, const char *control,
                              char **values, unsigned int num_values)
{
	struct sunxi_mixer *mixer;
	struct sunxi_mixer_ctl *ctl;
	enum sunxi_mixer_ctl_type type;
	unsigned int num_ctl_values;
	unsigned int i;
	int ret = 0;

	mixer = sunxi_mixer_open(card);
	if (!mixer) {
		awtinymix_err("Failed to open mixer\n");
		return -ENOENT;
	}

	if (isdigit(control[0]))
		ctl = sunxi_mixer_get_ctl(mixer, atoi(control));
	else
		ctl = sunxi_mixer_get_ctl_by_name(mixer, control);

	if (!ctl) {
		awtinymix_err("Invalid mixer control\n");
		ret = -EINVAL;
		goto err;
	}

	type = sunxi_mixer_ctl_get_type(ctl);
	num_ctl_values = sunxi_mixer_ctl_get_num_values(ctl);

	if (is_number(values[0])) {
		if (num_values == 1) {
			/* Set all values the same */
			int value = atoi(values[0]);

			for (i = 0; i < num_ctl_values; i++) {
				if ((ret = sunxi_mixer_ctl_set_value(ctl, i, value)) < 0) {
					awtinymix_err("Error: invalid value\n");
					goto err;
				}
			}
		} else {
			/* Set multiple values */
			if (num_values > num_ctl_values) {
				awtinymix_err("Error: %u values given, but control only takes %u\n",
						num_values, num_ctl_values);
				ret = -EINVAL;
				goto err;
			}
			for (i = 0; i < num_values; i++) {
				if ((ret = sunxi_mixer_ctl_set_value(ctl, i, atoi(values[i]))) < 0) {
					awtinymix_err("Error: invalid value for index %u\n", i);
					goto err;
				}
			}
		}
	} else {
		if (type == SUNXI_MIXER_CTL_TYPE_ENUM) {
			if (num_values != 1) {
				awtinymix_err("Enclose strings in quotes and try again\n");
				goto err;
			}
			if ((ret = sunxi_mixer_ctl_set_enum_by_string(ctl, values[0])) < 0)
				awtinymix_err("Error: invalid enum value\n");
		} else {
			awtinymix_err("Error: only enum types can be set with strings\n");
		}
	}

err:
	sunxi_mixer_close(mixer);

	return ret;
}

