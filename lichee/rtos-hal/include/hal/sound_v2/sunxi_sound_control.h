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
#ifndef __SUNXI_SOUND_CONTROL_H
#define __SUNXI_SOUND_CONTROL_H

//#include <sound_v2/sunxi_sound_core.h>
#include <hal_mutex.h>
#include <sunxi_hal_common.h>
#include <sunxi_hal_sound.h>

#include "aw_list.h"


#define SUNXI_SOUND_CTRL_PART_AUTO_MASK (0)

#define SOUND_CTRL_ADF_CONTROL(xname, xreg, xshift, xmax) \
{ \
	.type = SUNXI_SOUND_CTRL_PART_TYPE_INTEGER, \
	.name	= xname, \
	.reg	= xreg, \
	.shift	= xshift, \
	.max	= xmax, \
	.min	= 0, \
	.get	= NULL, \
	.set	= NULL, \
	.count	= 1, \
}

#define SOUND_CTRL_ADF_CONTROL_EXT(xname, xmax, xmin, xget, xset) \
{ \
	.type = SUNXI_SOUND_CTRL_PART_TYPE_INTEGER, \
	.name	= xname, \
	.reg	= 0, \
	.shift	= 0, \
	.max	= xmax, \
	.min	= xmin, \
	.get	= xget, \
	.set	= xset, \
	.count	= 1, \
}

#define SOUND_CTRL_ADF_CONTROL_EXT_REG(xname, xreg, xshift, xmax, xget, xset) \
{ \
	.type = SUNXI_SOUND_CTRL_PART_TYPE_INTEGER, \
	.name	= xname, \
	.reg	= xreg, \
	.shift	= xshift, \
	.max    = xmax, \
	.min	= 0, \
	.get    = xget, \
	.set    = xset, \
	.count  = 1, \
}

#define SOUND_CTRL_ADF_CONTROL_VALUE_EXT(xname, xreg, xshift, xmax, xmin, xget, xset) \
{ \
	.type = SUNXI_SOUND_CTRL_PART_TYPE_INTEGER, \
	.name	= xname, \
	.reg	= xreg, \
	.shift	= xshift, \
	.max	= xmax, \
	.min	= xmin, \
	.get	= xget, \
	.set	= xset, \
	.count	= 1, \
}

#define SOUND_CTRL_ADF_CONTROL_USER(xname, xmax, xmin, xcur) \
{ \
	.type = SUNXI_SOUND_CTRL_PART_TYPE_INTEGER, \
	.name	= xname, \
	.max	= xmax, \
	.min	= xmin, \
	.reg	= xcur, \
	.get	= NULL, \
	.set	= NULL, \
	.count	= 1, \
	.private_data_type = SND_MODULE_USER, \
}

#define SOUND_CTRL_ENUM(xname, xitems, xtexts, xreg, xshift) \
{ \
	.type = SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED, \
	.name = xname,\
	.items = xitems, \
	.texts = xtexts, \
	.reg	= xreg, \
	.shift	= xshift, \
	.mask	= SUNXI_SOUND_CTRL_PART_AUTO_MASK, \
	.get = NULL, \
	.set = NULL, \
	.count	= 1, \
}

#define SOUND_CTRL_ENUM_EXT(xname, xitems, xtexts, xmask, xget, xset) \
{ \
	.type = SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED, \
	.name = xname,\
	.items = xitems, \
	.texts = xtexts, \
	.mask	= xmask, \
	.get = xget, \
	.set = xset, \
	.count	= 1, \
}

#define SOUND_CTRL_ENUM_VALUE_EXT(xname, xitems, xtexts, xreg, xshift, xmask, xget, xset) \
{ \
	.type = SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED, \
	.name = xname,\
	.items = xitems, \
	.texts = xtexts, \
	.reg	= xreg, \
	.shift	= xshift, \
	.mask	= xmask, \
	.get = xget, \
	.set = xset, \
	.count	= 1, \
}

struct sunxi_sound_adf_control;
struct sunxi_sound_control_info;

typedef int (sound_adf_control_get_t) (struct sunxi_sound_adf_control *, struct sunxi_sound_control_info *);
typedef int (sound_adf_control_set_t) (struct sunxi_sound_adf_control *, struct sunxi_sound_control_info *);

struct sunxi_sound_adf_control_new {
	sound_control_part_type_t type;
	const char *name;		/* ASCII name of item */
	unsigned int count;		/* count of same elements */
	sound_adf_control_get_t *get;
	sound_adf_control_set_t *set;
	int reg;
	unsigned int shift;
	int min;
	int max;
	int mask;
	/* for enum */
	unsigned int items;
	const char * const *texts;
	unsigned long value;
	int private_data_type;
};

struct sunxi_sound_adf_control {
	unsigned int id;
	sound_control_part_type_t type;
	const char *name;
	int reg;
	unsigned int shift;
	int min;
	int max;
	int mask;
	int count;
	/* for enum */
	unsigned int items;
	const char * const *texts;
	unsigned long value;

	sound_adf_control_get_t *get;
	sound_adf_control_set_t *set;
	struct list_head list;
	int dynamic;
	void *private_data;
	int private_data_type;
};

struct sunxi_sound_ctl_private {
	struct sunxi_sound_card *card;
	int ref;
};

int sound_add_control(struct sunxi_sound_card *card, struct sunxi_sound_adf_control *ctrl);

int sunxi_sound_ctrl_add_part(struct sunxi_sound_card *card, struct sunxi_sound_control_info *info);
int sunxi_sound_ctrl_remove_part(struct sunxi_sound_card *card, struct sunxi_sound_adf_control *control);
struct sunxi_sound_card *sunxi_sound_ctrl_find_by_name(const char *name);
struct sunxi_sound_card *sunxi_sound_ctrl_find_by_num(int num);

struct sunxi_sound_adf_control* sunxi_sound_control_create(const struct sunxi_sound_adf_control_new *ncontrol,
				  void *private_data);
void sunxi_sound_control_destory(struct sunxi_sound_adf_control *adf_control);

void sunxi_sound_adf_control_to_sound_ctrl_info(struct sunxi_sound_adf_control *adf_control,
			struct sunxi_sound_control_info *info, unsigned long value);

int sunxi_sound_control_new(struct sunxi_sound_card *card);

#endif /* __SUNXI_SOUND_CONTROL_H */
