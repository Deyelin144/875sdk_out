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

#ifndef __SUNXI_SOUND_CORE_H
#define __SUNXI_SOUND_CORE_H

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <hal_sem.h>
#include <hal_mutex.h>
#include <hal_atomic.h>
#include <hal_interrupt.h>
#include <sunxi_hal_common.h>
#include "aw_list.h"
#include <sunxi_hal_sound.h>

//#include <sound_v2/sunxi_sound_pcm_common.h>
#define SUNXI_SOUND_CARDS 8

typedef unsigned int hal_irq_state_t;

#define hal_local_irq_enable()  hal_interrupt_enable()
#define hal_local_irq_disable() hal_interrupt_disable()
#define hal_local_irq_save(flags) \
	do { \
		flags = hal_interrupt_disable_irqsave(); \
	} while (0)
#define hal_local_irq_restore(flags) \
	do { \
		hal_interrupt_enable_irqrestore(flags); \
	} while (0)


typedef hal_mutex_t sunxi_sound_mutex_t;
sunxi_sound_mutex_t sound_mutex_init(void);
int sound_mutex_lock_timeout(sunxi_sound_mutex_t mutex, long ms);
int sound_mutex_lock(sunxi_sound_mutex_t mutex);
void sound_mutex_unlock(sunxi_sound_mutex_t mutex);
void sound_mutex_destroy(sunxi_sound_mutex_t mutex);
const char *sunxi_sound_strdup_const(const char *s);

typedef struct {
	hal_sem_t sem;
	int waiting;
} *sunxi_sound_schd_t;

sunxi_sound_schd_t sound_schd_init(void);
int sound_schd_timeout(sunxi_sound_schd_t schd, long ms);
void sound_schd_wakeup(sunxi_sound_schd_t schd);
void sound_schd_destroy(sunxi_sound_schd_t schd);

#define sunxi_sound_readb(reg)          (*(volatile uint8_t  *)(reg))
#define sunxi_sound_readw(reg)          (*(volatile uint16_t *)(reg))
#define sunxi_sound_readl(reg)          (*(volatile uint32_t *)(reg))
#define sunxi_sound_writeb(value,reg)   (*(volatile uint8_t  *)(reg) = (value))
#define sunxi_sound_writew(value,reg)   (*(volatile uint16_t *)(reg) = (value))
#define sunxi_sound_writel(value,reg)   (*(volatile uint32_t *)(reg) = (value))


#define sound_malloc(size)	calloc(1, size)
#define sound_strdup(ptr)		strdup(ptr)
#define sound_free(ptr)		free((void *)ptr)

#define SNDRV_LOG_COLOR_NONE		"\e[0m"
#define SNDRV_LOG_COLOR_RED		"\e[31m"
#define SNDRV_LOG_COLOR_GREEN	"\e[32m"
#define SNDRV_LOG_COLOR_YELLOW		"\e[33m"
#define SNDRV_LOG_COLOR_BLUE		"\e[34m"


#ifdef SNDRV_DEBUG
#define snd_debug(fmt, args...) \
	printf(SNDRV_LOG_COLOR_GREEN "[SND_DEBUG][%s:%d]" fmt \
		SNDRV_LOG_COLOR_NONE, __FUNCTION__, __LINE__, ##args)
#else
#define snd_debug(fmt, args...)
#endif

#if 0
#define snd_info(fmt, args...) \
	printf(SNDRV_LOG_COLOR_BLUE "[SND_INFO][%s:%d]" fmt \
		SNDRV_LOG_COLOR_NONE, __FUNCTION__, __LINE__, ##args)
#else
#define snd_info(fmt, args...)
#endif

#define snd_err(fmt, args...) \
	printf(SNDRV_LOG_COLOR_RED "[SND_ERR][%s:%d]" fmt \
		SNDRV_LOG_COLOR_NONE, __FUNCTION__, __LINE__, ##args)
#if 0
#define snd_lock_debug(fmt, args...) \
	printf(SNDRV_LOG_COLOR_RED "[SND_LOCK_DEBUG][%s:%u]" fmt \
		SNDRV_LOG_COLOR_NONE, __FUNCTION__, __LINE__, ##args)
#else
#define snd_lock_debug(fmt, args...)
#endif


enum snd_platform_type {
	SND_PLATFORM_TYPE_CPUDAI = 0,
	SND_PLATFORM_TYPE_CPUDAI_DAC,
	SND_PLATFORM_TYPE_CPUDAI_ADC,
	SND_PLATFORM_TYPE_INTERNAL_I2S,
	SND_PLATFORM_TYPE_I2S0 = 5,
	SND_PLATFORM_TYPE_I2S1,
	SND_PLATFORM_TYPE_I2S2,
	SND_PLATFORM_TYPE_I2S3,
	SND_PLATFORM_TYPE_I2S_MAX,
	SND_PLATFORM_TYPE_DMIC = 10,
	SND_PLATFORM_TYPE_OWA = 12,
	SND_PLATFORM_TYPE_MAX,
};

enum {
	SNDRV_PCM_STREAM_PLAYBACK = 0,
	SNDRV_PCM_STREAM_CAPTURE,
	SNDRV_PCM_STREAM_LAST = SNDRV_PCM_STREAM_CAPTURE,
};

enum snd_module_type {
	SND_MODULE_UNKNOWN = 0,
	SND_MODULE_CODEC,
	SND_MODULE_PLATFORM,
	SND_MODULE_USER,
};

/* number of supported soundcards */
#ifdef CONFIG_SND_DYNAMIC_MINORS
#define SOUNDDRV_CARDS CONFIG_SND_MAX_CARDS
#else
#define SOUNDDRV_CARDS 8
#endif

/* type of the object used in snd_device_*()
 * this also defines the calling order
 */
enum sunxi_sound_device_type {
	SNDRV_DEV_LOWLEVEL,
	SNDRV_DEV_PCM,
	SNDRV_DEV_TIMER,
	SNDRV_DEV_JACK,
	SNDRV_DEV_CONTROL,	/* NOTE: this must be the last one */
};

enum sunxi_sound_device_state {
	SNDRV_DEV_BUILD,
	SNDRV_DEV_REGISTERED,
};

struct sunxi_sound_device;

struct sunxi_sound_device_ops {
	int (*dev_unregiter)(struct sunxi_sound_device *dev);
	int (*dev_register)(struct sunxi_sound_device *dev);
};

struct sunxi_sound_card;

/* For sound device manager, such as pcm, control */
struct sunxi_sound_device {
	struct list_head list;		/* list of registered devices */
	struct sunxi_sound_card *card;		/* card which holds this device */
	enum sunxi_sound_device_state state;	/* state of the device */
	enum sunxi_sound_device_type type;	/* device type */
	void *device_data;		/* device structure */
	const struct sunxi_sound_device_ops *ops;	/* operations */
};

#define sunxi_sound_device(n) list_entry(n, struct sunxi_sound_device, list)

struct sound_device;

struct sound_device_ops {
	int (*open)(struct sound_device *dev, unsigned int mode);
	int (*close)(struct sound_device *dev);
	int (*control)(struct sound_device *dev, unsigned int cmd, unsigned long arg);
	int (*write)(struct sound_device *dev, const void *buffer, sunxi_sound_pcm_uframes_t size);
	int (*read)(struct sound_device *dev, void *buffer, sunxi_sound_pcm_uframes_t size);
};

struct sound_card_device;

/* For sound device, such as pcm, control */
struct sound_device {
	const char *name;
	struct sound_card_device *parent;
	const struct sound_device_ops *sdev_ops;	/* sound dev operations */
	void *priv_data;
	struct list_head child_list;
};

/* For sound card device */
struct sound_card_device {
	const char *name;
	struct sound_card_device *sibling;
	void *priv_data;
	struct list_head child_head;    /* List of children of this device */
	struct list_head sibling_node;  /* Next device in list of all devices */
};

struct sunxi_sound_card {
	const char *name; /* soundcard name */
	int num; /* number of soundcard */
	struct list_head devices; /* device list */
	struct sound_card_device dev; /* card device */
	struct sound_device control_dev;
	sunxi_sound_mutex_t ctrl_mutex;
	struct list_head controls;	/* all controls for this card */
	int controls_nums;		/* nums of all controls */
};

int sunxi_sound_device_create(struct sunxi_sound_card *card, enum sunxi_sound_device_type type,
		   void *dev_data, const struct sunxi_sound_device_ops *ops);

int sunxi_sound_device_register(struct sunxi_sound_card *card, void *device_data);
int sunxi_sound_device_register_all(struct sunxi_sound_card *card);
void sunxi_sound_device_destory(struct sunxi_sound_card *card, void *device_data);
void sunxi_sound_device_destory_all(struct sunxi_sound_card *card);
int sunxi_sound_card_create(const char* name, struct sunxi_sound_card **card_ret);
int sunxi_sound_card_destory(struct sunxi_sound_card *card);
int sunxi_sound_register_device(struct sunxi_sound_card *card, const struct sound_device_ops *sdev_ops,
			void *private_data, struct sound_device *dev);
int sunxi_sound_unregister_device(struct sound_card_device *card_dev, struct sound_device *dev);


#endif /* __SUNXI_SOUND_CORE_H */
