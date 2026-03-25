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


#ifndef SUNXI_TINY_SOUND_MIXER_H
#define SUNXI_TINY_SOUND_MIXER_H

#include <stddef.h>
#include <sunxi_hal_sound.h>
#include <sound_v2/sunxi_sound_pcm_common.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define AWTINYMIX_LOG_COLOR_NONE		"\e[0m"
#define AWTINYMIX_LOG_COLOR_RED		"\e[31m"
#define AWTINYMIX_LOG_COLOR_GREEN	"\e[32m"
#define AWTINYMIX_LOG_COLOR_YELLOW		"\e[33m"
#define AWTINYMIX_LOG_COLOR_BLUE		"\e[34m"

#ifdef AWTINYMIX_DEBUG
#define awtinymix_debug(fmt, args...) \
	printf(AWTINYMIX_LOG_COLOR_GREEN "[AWTINYMIX_DEBUG][%s:%d]" fmt \
		AWTINYMIX_LOG_COLOR_NONE, __FUNCTION__, __LINE__, ##args)
#else
#define awtinymix_debug(fmt, args...)
#endif

#if 0
#define awtinymix_info(fmt, args...) \
	printf(AWTINYMIX_LOG_COLOR_BLUE "[AWTINYMIX_INFO][%s:%d]" fmt \
		AWTINYMIX_LOG_COLOR_NONE, __FUNCTION__, __LINE__, ##args)
#else
#define awtinymix_info(fmt, args...)
#endif

#define awtinymix_err(fmt, args...) \
	printf(AWTINYMIX_LOG_COLOR_RED "[AWTINYMIX_ERR][%s:%d]" fmt \
		AWTINYMIX_LOG_COLOR_NONE, __FUNCTION__, __LINE__, ##args)


struct sunxi_mixer;

struct sunxi_mixer_ctl;

/** Mixer control type.
 * @ingroup libtinyalsa-mixer
 */
enum sunxi_mixer_ctl_type {
    /** integer control type */
    SUNXI_MIXER_CTL_TYPE_INT,
    /** an enumerated control type */
    SUNXI_MIXER_CTL_TYPE_ENUM,
    /** unknown control type */
    SUNXI_MIXER_CTL_TYPE_UNKNOWN,
    /** end of the enumeration (not a control type) */
    SUNXI_MIXER_CTL_TYPE_MAX,
};

struct sunxi_mixer *sunxi_mixer_open(unsigned int card);

void sunxi_mixer_close(struct sunxi_mixer *mixer);

int sunxi_mixer_add_ctls(struct sunxi_mixer *mixer, struct sunxi_sound_control_info *info);

int sunxi_mixer_remove_ctls(struct sunxi_mixer *mixer, unsigned int id);

const char *sunxi_mixer_ctl_get_name(const struct sunxi_mixer_ctl *ctl);

unsigned int sunxi_mixer_get_num_ctls(const struct sunxi_mixer *mixer);

struct sunxi_mixer_ctl *sunxi_mixer_get_ctl(struct sunxi_mixer *mixer, unsigned int id);

struct sunxi_mixer_ctl *sunxi_mixer_get_ctl_by_name(struct sunxi_mixer *mixer, const char *name);

struct sunxi_mixer_ctl *sunxi_mixer_get_ctl_by_name_and_index(struct sunxi_mixer *mixer,
                                                  const char *name,
                                                  unsigned int index);

const char *sunxi_mixer_ctl_get_name(const struct sunxi_mixer_ctl *ctl);

enum sunxi_mixer_ctl_type sunxi_mixer_ctl_get_type(const struct sunxi_mixer_ctl *ctl);

const char *sunxi_mixer_ctl_get_type_string(const struct sunxi_mixer_ctl *ctl);

unsigned int sunxi_mixer_ctl_get_num_values(const struct sunxi_mixer_ctl *ctl);

unsigned int sunxi_mixer_ctl_get_num_enums(const struct sunxi_mixer_ctl *ctl);

const char *sunxi_mixer_ctl_get_enum_string(struct sunxi_mixer_ctl *ctl, unsigned int enum_id);

int sunxi_mixer_ctl_get_value(const struct sunxi_mixer_ctl *ctl, unsigned int id);

int sunxi_mixer_ctl_set_value(struct sunxi_mixer_ctl *ctl, unsigned int id, int value);

int sunxi_mixer_ctl_set_enum_by_string(struct sunxi_mixer_ctl *ctl, const char *string);

/* Determe range of integer mixer controls */
int sunxi_mixer_ctl_get_range_min(const struct sunxi_mixer_ctl *ctl);

int sunxi_mixer_ctl_get_range_max(const struct sunxi_mixer_ctl *ctl);

int is_number(const char *str);

int sunxi_mixer_ctl_set_ctl_value(unsigned int card, const char *control,
								  char **values, unsigned int num_values);

#if defined(__cplusplus)
}  /* extern "C" */
#endif

#endif

