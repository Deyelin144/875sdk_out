/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the People's Republic of China and other countries.
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

#ifndef _SUNXI_HAL_SOUND_H_
#define _SUNXI_HAL_SOUND_H_

#include<stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SND_DEV_APPEND  (1<<8)
#define SND_DEV_NONBLOCK  (1<<9)

#define _SOUNDIOC(type,nr)   ((type << 16)|(nr))

#define SOUND_CTRL       (0x01000) /* control I/O ioctl commands */
#define SOUND_PCM        (0x01100) /* pcm driver ioctl commands */


/*****************************************************************************
 *																			 *
 *			   Digital Audio (PCM) interface - Sunxipcm??				 *
 *																			 *
 *****************************************************************************/

typedef unsigned long snd_pcm_uframes_t;
typedef signed long snd_pcm_sframes_t;

typedef union snd_interval snd_interval_t;
union snd_interval {
	struct {
		uint32_t min;
		uint32_t max;
		int openmin;	/* whether the interval is left-open */
		int openmax;	/* whether the interval is right-open */
		int integer;	/* whether the value is integer or not */
		int empty;
	} range;
	uint32_t mask;
};

typedef long sunxi_sound_pcm_sframes_t;
typedef unsigned long sunxi_sound_pcm_uframes_t;

/** PCM stream (direction) */
typedef enum _snd_pcm_stream {
    /** Playback stream */
    SND_PCM_STREAM_PLAYBACK = 0,
    /** Capture stream */
    SND_PCM_STREAM_CAPTURE,
    SND_PCM_STREAM_LAST = SND_PCM_STREAM_CAPTURE
} snd_pcm_stream_t;

/** PCM access type */
typedef enum _snd_pcm_access {
	/** mmap access with simple interleaved channels */
	SND_PCM_ACCESS_MMAP_INTERLEAVED = 0,
	/** mmap access with simple non interleaved channels */
	SND_PCM_ACCESS_MMAP_NONINTERLEAVED,
	/** mmap access with complex placement */
	SND_PCM_ACCESS_MMAP_COMPLEX,
	/** snd_pcm_readi/snd_pcm_writei access */
	SND_PCM_ACCESS_RW_INTERLEAVED,
	/** snd_pcm_readn/snd_pcm_writen access */
	SND_PCM_ACCESS_RW_NONINTERLEAVED,
	SND_PCM_ACCESS_LAST = SND_PCM_ACCESS_RW_NONINTERLEAVED
} snd_pcm_access_t;

/** PCM sample format */
typedef enum _snd_pcm_format {
	/** Unknown */
	SND_PCM_FORMAT_UNKNOWN = -1,
	/** Signed 8 bit */
	SND_PCM_FORMAT_S8 = 0,
	/** Unsigned 8 bit */
	SND_PCM_FORMAT_U8,
	/** Signed 16 bit Little Endian */
	SND_PCM_FORMAT_S16_LE,
	/** Signed 16 bit Big Endian */
	SND_PCM_FORMAT_S16_BE,
	/** Unsigned 16 bit Little Endian */
	SND_PCM_FORMAT_U16_LE,
	/** Unsigned 16 bit Big Endian */
	SND_PCM_FORMAT_U16_BE,
	/** Signed 24 bit Little Endian using low three bytes in 32-bit word */
	SND_PCM_FORMAT_S24_LE,
	/** Signed 24 bit Big Endian using low three bytes in 32-bit word */
	SND_PCM_FORMAT_S24_BE,
	/** Unsigned 24 bit Little Endian using low three bytes in 32-bit word */
	SND_PCM_FORMAT_U24_LE,
	/** Unsigned 24 bit Big Endian using low three bytes in 32-bit word */
	SND_PCM_FORMAT_U24_BE,
	/** Signed 32 bit Little Endian */
	SND_PCM_FORMAT_S32_LE,
	/** Signed 32 bit Big Endian */
	SND_PCM_FORMAT_S32_BE,
	/** Unsigned 32 bit Little Endian */
	SND_PCM_FORMAT_U32_LE,
	/** Unsigned 32 bit Big Endian */
	SND_PCM_FORMAT_U32_BE,

/* only support little endian */
/** Signed 16 bit CPU endian */
	SND_PCM_FORMAT_S16 = SND_PCM_FORMAT_S16_LE,
	/** Unsigned 16 bit CPU endian */
	SND_PCM_FORMAT_U16 = SND_PCM_FORMAT_U16_LE,
	/** Signed 24 bit CPU endian */
	SND_PCM_FORMAT_S24 = SND_PCM_FORMAT_S24_LE,
	/** Unsigned 24 bit CPU endian */
	SND_PCM_FORMAT_U24 = SND_PCM_FORMAT_U24_LE,
	/** Signed 32 bit CPU endian */
	SND_PCM_FORMAT_S32 = SND_PCM_FORMAT_S32_LE,
	/** Unsigned 32 bit CPU endian */
	SND_PCM_FORMAT_U32 = SND_PCM_FORMAT_U32_LE,

	SND_PCM_FORMAT_LAST = SND_PCM_FORMAT_U32_BE,
} snd_pcm_format_t;

/** PCM state */
typedef enum _sunxi_sound_pcm_state {
	/** Open */
	SNDRV_PCM_STATE_OPEN = 0,
	/** Setup installed */
	SNDRV_PCM_STATE_SETUP,
	/** Ready to start */
	SNDRV_PCM_STATE_PREPARED,
	/** Running */
	SNDRV_PCM_STATE_RUNNING,
	/** Stopped: underrun (playback) or overrun (capture) detected */
	SNDRV_PCM_STATE_XRUN,
	/** Draining: running (playback) or stopped (capture) */
	SNDRV_PCM_STATE_DRAINING,
	/** Paused */
	SNDRV_PCM_STATE_PAUSED,
	/** Hardware is suspended */
	SNDRV_PCM_STATE_SUSPENDED,
	/** Hardware is disconnected */
	SNDRV_PCM_STATE_DISCONNECTED,
	SNDRV_PCM_STATE_LAST = SNDRV_PCM_STATE_DISCONNECTED
} sunxi_sound_pcm_state_t;

struct sunxi_sound_pcm_info {
	unsigned int device;		/* RO/WR (control): device number */
	unsigned int subdevice;		/* RO/WR (control): subdevice number */
	int stream;			/* RO/WR (control): stream direction */
	int card;			/* R: card number */
	unsigned char id[64];		/* ID (user selectable) */
	unsigned char name[64];		/* name of this device */
	unsigned char subname[32];	/* subdevice name */
	unsigned int subdevices_count;
	unsigned int subdevices_avail;
};

typedef int snd_pcm_hw_param_t;
#define SND_PCM_HW_PARAM_ACCESS		0
#define SND_PCM_HW_PARAM_FORMAT		1
#define SND_PCM_HW_PARAM_FIRST_MASK	SND_PCM_HW_PARAM_ACCESS
#define SND_PCM_HW_PARAM_LAST_MASK	SND_PCM_HW_PARAM_FORMAT
#define SND_PCM_HW_PARAM_SAMPLE_BITS	2
#define SND_PCM_HW_PARAM_FRAME_BITS	3
#define SND_PCM_HW_PARAM_CHANNELS	4
#define SND_PCM_HW_PARAM_RATE		5
#define SND_PCM_HW_PARAM_PERIOD_TIME	6
#define SND_PCM_HW_PARAM_PERIOD_SIZE	7
#define SND_PCM_HW_PARAM_PERIOD_BYTES	8
#define SND_PCM_HW_PARAM_PERIODS	9
#define SND_PCM_HW_PARAM_BUFFER_TIME	10
#define SND_PCM_HW_PARAM_BUFFER_SIZE	11
#define SND_PCM_HW_PARAM_BUFFER_BYTES	12
#define SND_PCM_HW_PARAM_FIRST_RANGE SND_PCM_HW_PARAM_SAMPLE_BITS
#define SND_PCM_HW_PARAM_LAST_RANGE SND_PCM_HW_PARAM_BUFFER_BYTES
#define SND_PCM_HW_PARAM_FIRST_INTERVAL SND_PCM_HW_PARAM_ACCESS
#define SND_PCM_HW_PARAM_LAST_INTERVAL SND_PCM_HW_PARAM_BUFFER_BYTES

typedef struct sunxi_sound_pcm_hw_params {
	union snd_interval intervals[SND_PCM_HW_PARAM_LAST_INTERVAL -
				SND_PCM_HW_PARAM_FIRST_INTERVAL + 1];
	unsigned int can_paused;

	uint32_t rmask; 	/* W: requested masks */
	uint32_t cmask; 	/* R: changed masks */
} sunxi_sound_pcm_hw_params_t;

typedef struct sunxi_sound_pcm_sw_params {
	snd_pcm_uframes_t avail_min;			/* min avail frames for wakeup */
	snd_pcm_uframes_t start_threshold;		/* min hw_avail frames for automatic start */
	snd_pcm_uframes_t stop_threshold;		/* min avail frames for automatic stop */
	snd_pcm_uframes_t silence_size; 		/* silence block size */
	snd_pcm_uframes_t boundary; 			/* pointers wrap point */
} sunxi_sound_pcm_sw_params_t;

typedef struct _sunxi_sound_pcm_channel_info {
	unsigned int channel;
	void *addr; 		/* base address of channel samples */
	unsigned int first; 	/* offset to first sample in bits */
	unsigned int step;		/* samples distance in bits */
	enum { SND_PCM_AREA_MMAP, SND_PCM_AREA_LOCAL } type;
} sunxi_sound_pcm_channel_info_t;

struct sunxi_sound_pcm_mmap_status {
	sunxi_sound_pcm_state_t state;
	sunxi_sound_pcm_uframes_t hw_ptr;
	sunxi_sound_pcm_state_t state_suspend;
};

struct sunxi_sound_pcm_mmap_control {
	sunxi_sound_pcm_uframes_t appl_ptr;
	sunxi_sound_pcm_uframes_t avail_min;
};

#define SNDRV_PCM_SYNC_PTR_HWSYNC       (1<<0)  /* execute hwsync */
#define SNDRV_PCM_SYNC_PTR_APPL         (1<<1)  /* get appl_ptr from driver (r/w op) */
#define SNDRV_PCM_SYNC_PTR_AVAIL_MIN    (1<<2)  /* get avail_min from driver */

struct sunxi_sound_pcm_sync_ptr {
	unsigned int flags;
	union {
		struct sunxi_sound_pcm_mmap_status status;
		unsigned char reserved[64];
	} s;
	union {
		struct sunxi_sound_pcm_mmap_control control;
		unsigned char reserved[64];
	} c;
};


#define SUNXI_SOUND_PCM(nr)					_SOUNDIOC(SOUND_PCM,(nr))

#define SUNXI_SOUND_CTRL_PCM_INFO			SUNXI_SOUND_PCM(0)
#define SUNXI_SOUND_CTRL_PCM_HW_REFINE		SUNXI_SOUND_PCM(1)
#define SUNXI_SOUND_CTRL_PCM_HW_PARAMS		SUNXI_SOUND_PCM(2)
#define SUNXI_SOUND_CTRL_PCM_HW_FREE		SUNXI_SOUND_PCM(3)
#define SUNXI_SOUND_CTRL_PCM_SW_PARAMS		SUNXI_SOUND_PCM(4)
#define SUNXI_SOUND_CTRL_PCM_PREPARE		SUNXI_SOUND_PCM(5)
#define SUNXI_SOUND_CTRL_PCM_RESET			SUNXI_SOUND_PCM(6)
#define SUNXI_SOUND_CTRL_PCM_START			SUNXI_SOUND_PCM(7)
#define SUNXI_SOUND_CTRL_PCM_RESUME			SUNXI_SOUND_PCM(8)
#define SUNXI_SOUND_CTRL_PCM_HWSYNC			SUNXI_SOUND_PCM(9)
#define SUNXI_SOUND_CTRL_PCM_DELAY			SUNXI_SOUND_PCM(10)
#define SUNXI_SOUND_CTRL_PCM_SYNC_PTR		SUNXI_SOUND_PCM(11)
#define SUNXI_SOUND_CTRL_PCM_DRAIN			SUNXI_SOUND_PCM(12)
#define SUNXI_SOUND_CTRL_PCM_DROP			SUNXI_SOUND_PCM(13)
#define SUNXI_SOUND_CTRL_PCM_PAUSE			SUNXI_SOUND_PCM(14)


/****************************************************************************
 *																			*
 *		  Section for driver control interface - control?			*
 *																			*
 ****************************************************************************/

struct sunxi_sound_control_card_info {
	int card;			/* card number */
	char name[32];		/* Short name of soundcard */
};

typedef int sound_control_part_type_t;

#define SUNXI_SOUND_CTRL_PART_TYPE_INTEGER ((sound_control_part_type_t)0)
#define SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED ((sound_control_part_type_t)1)
#define SUNXI_SOUND_CTRL_PART_TYPE_LAST SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED

struct sunxi_sound_control_part_list {
	unsigned int offset;	/* W: first element ID to get */
	unsigned int space;		/* W: count of element IDs to get */
	unsigned int count;		/* R: count of all elements */
	unsigned int *numid; 	/* R: IDs */
};


struct sunxi_sound_control_info {
	unsigned int id;
	sound_control_part_type_t type;
	const char *name;
	unsigned long value;
	int min;
	int max;
	int count;
	/* for enum */
	unsigned int items;
	const char *const *texts;
	unsigned long *private_data;
};

#define SUNXI_SOUND_CTRL(nr)			_SOUNDIOC(SOUND_CTRL,(nr))
#define SUNXI_SOUND_CTRL_CARD_INFO		SUNXI_SOUND_CTRL(0)
#define SUNXI_SOUND_CTRL_PART_LIST		SUNXI_SOUND_CTRL(1)
#define SUNXI_SOUND_CTRL_PART_INFO		SUNXI_SOUND_CTRL(2)
#define SUNXI_SOUND_CTRL_PART_READ		SUNXI_SOUND_CTRL(3)
#define SUNXI_SOUND_CTRL_PART_WRITE		SUNXI_SOUND_CTRL(4)
#define SUNXI_SOUND_CTRL_PART_ADD		SUNXI_SOUND_CTRL(5)
#define SUNXI_SOUND_CTRL_PART_REMOVE	SUNXI_SOUND_CTRL(6)

typedef struct sound_device sound_device_t;

int sunxi_sound_device_open(sound_device_t **sound_dev, const char *devname, int mode);
int sunxi_sound_device_close(sound_device_t *sound_dev);
int sunxi_sound_device_control(sound_device_t *sound_dev, unsigned int cmd, unsigned long arg);
int sunxi_sound_device_write(sound_device_t *sound_dev, const void *buffer, snd_pcm_uframes_t size);
int sunxi_sound_device_read(sound_device_t *sound_dev, void *buffer, snd_pcm_uframes_t size);

struct sunxi_sound_info_buffer {
	char *buffer;		/* pointer to begin of buffer */
	unsigned int len;	/* total length of buffer */
	int error;		/* error code */
};

void sunxi_sound_cards_info(struct sunxi_sound_info_buffer *buf);
void sunxi_sound_pcm_read(struct sunxi_sound_info_buffer *buf);
int sunxi_sound_dataflow_info_read(int card_num, int device, int subdevice, int stream,
							 struct sunxi_sound_info_buffer *buf);

#ifdef __cplusplus
}
#endif

#endif /* _SUNXI_HAL_SOUND_H_ */

