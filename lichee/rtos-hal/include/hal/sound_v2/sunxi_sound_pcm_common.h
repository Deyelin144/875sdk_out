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
#ifndef __SUNXI_SOUND_PCM_COMMON_H
#define __SUNXI_SOUND_PCM_COMMON_H

#include <stdint.h>
#include <errno.h>
#include <sunxi_hal_sound.h>

#ifndef EBADFD
#define EBADFD		77
#endif
#ifndef ESTRPIPE
#define ESTRPIPE	86
#endif

#ifndef LONG_MAX
#define LONG_MAX	((long)(~0UL>>1))
#endif
#ifndef ULONG_MAX
#define ULONG_MAX	(~0UL)
#endif
#ifndef UINT_MAX
#define UINT_MAX	(~0U)
#endif
#ifndef ULLONG_MAX
#define ULLONG_MAX	(~0ULL)
#endif


#define SNDRV_PCM_INFO_MMAP		0x00000001	/* hardware supports mmap */
#define SNDRV_PCM_INFO_MMAP_VALID	0x00000002	/* period data are valid during transfer */
#define SNDRV_PCM_INFO_DOUBLE		0x00000004	/* Double buffering needed for PCM start/stop */
#define SNDRV_PCM_INFO_BATCH		0x00000010	/* double buffering */
#define SNDRV_PCM_INFO_INTERLEAVED	0x00000100	/* channels are interleaved */
#define SNDRV_PCM_INFO_NONINTERLEAVED	0x00000200	/* channels are not interleaved */
#define SNDRV_PCM_INFO_COMPLEX		0x00000400	/* complex frame organization (mmap only) */
#define SNDRV_PCM_INFO_BLOCK_TRANSFER	0x00010000	/* hardware transfer block of samples */
#define SNDRV_PCM_INFO_OVERRANGE	0x00020000	/* hardware supports ADC (capture) overrange detection */
#define SNDRV_PCM_INFO_RESUME		0x00040000	/* hardware supports stream resume after suspend */
#define SNDRV_PCM_INFO_PAUSE		0x00080000	/* pause ioctl is supported */
#define SNDRV_PCM_INFO_HALF_DUPLEX	0x00100000	/* only half duplex */
#define SNDRV_PCM_INFO_JOINT_DUPLEX	0x00200000	/* playback and capture stream are somewhat correlated */
#define SNDRV_PCM_INFO_SYNC_START	0x00400000	/* pcm support some kind of sync go */
#define SNDRV_PCM_INFO_NO_PERIOD_WAKEUP	0x00800000	/* period wakeup can be disabled */
#define SNDRV_PCM_INFO_HAS_WALL_CLOCK   0x01000000      /* (Deprecated)has audio wall clock for audio/system time sync */
#define SNDRV_PCM_INFO_HAS_LINK_ATIME              0x01000000  /* report hardware link audio time, reset on startup */
#define SNDRV_PCM_INFO_HAS_LINK_ABSOLUTE_ATIME     0x02000000  /* report absolute hardware link audio time, not reset on startup */
#define SNDRV_PCM_INFO_HAS_LINK_ESTIMATED_ATIME    0x04000000  /* report estimated link audio time */
#define SNDRV_PCM_INFO_HAS_LINK_SYNCHRONIZED_ATIME 0x08000000  /* report synchronized audio/system time */
#define SNDRV_PCM_INFO_DRAIN_TRIGGER	0x40000000		/* internal kernel flag - trigger in drain */
#define SNDRV_PCM_INFO_FIFO_IN_FRAMES	0x80000000	/* internal kernel flag - FIFO size is in frames */


static inline int hw_is_mask(int var)
{
	return var >= SND_PCM_HW_PARAM_FIRST_MASK &&
		var <= SND_PCM_HW_PARAM_LAST_MASK;
}

static inline int hw_is_range(int var)
{
	return var >= SND_PCM_HW_PARAM_FIRST_RANGE &&
		var <= SND_PCM_HW_PARAM_LAST_RANGE;
}

static inline unsigned int div32(unsigned int a, unsigned int b,
				 unsigned int *r)
{
	if (b == 0) {
		*r = 0;
		return UINT_MAX;
	}
	*r = a % b;
	return a / b;
}

static inline uint64_t div_u64_rem(uint64_t dividend, uint32_t divisor, uint32_t *remainder)
{
	*remainder = dividend % divisor;
	return dividend / divisor;
}

static inline unsigned int div_down(unsigned int a, unsigned int b)
{
	if (b == 0)
		return UINT_MAX;
	return a / b;
}

static inline unsigned int div_up(unsigned int a, unsigned int b)
{
	unsigned int r;
	unsigned int q;
	if (b == 0)
		return UINT_MAX;
	q = div32(a, b, &r);
	if (r)
		++q;
	return q;
}

static inline unsigned int mul(unsigned int a, unsigned int b)
{
	if (a == 0)
		return 0;
	if (div_down(UINT_MAX, a) < b)
		return UINT_MAX;
	return a * b;
}

static inline unsigned int add(unsigned int a, unsigned int b)
{
	if (a >= UINT_MAX - b)
		return UINT_MAX;
	return a + b;
}

static inline unsigned int sub(unsigned int a, unsigned int b)
{
	if (a > b)
		return a - b;
	return 0;
}

static inline unsigned int muldiv32(unsigned int a, unsigned int b,
				    unsigned int c, unsigned int *r)
{
	uint64_t n = (uint64_t)a * (uint64_t)b;
	uint32_t rem;
	if (c == 0) {
		*r = 0;
		return UINT_MAX;
	}
	n = div_u64_rem(n, c, &rem);
	if (n >= UINT_MAX) {
		*r = 0;
		return UINT_MAX;
	}
	*r = rem;
	return n;
}

static inline int __pcm_ffs(uint32_t value)
{
	uint32_t offset;

	for (offset = 0; offset < sizeof(value)*8; offset++) {
		if (value & (1<<offset))
			return offset;
	}
	return -1;
}

static inline int __ffs(uint32_t value)
{
	uint32_t offset;

	for (offset = 0; offset < sizeof(value)*8; offset++) {
		if (value & (1<<offset))
			return offset;
	}
	return -1;
}

static inline int __fls(uint32_t value)
{
	uint32_t offset;

	for (offset = sizeof(value)*8; offset > 0; offset--) {
		if (value & (1<<(offset - 1)))
			return offset;
	}
	return -1;
}

static inline snd_pcm_access_t params_access(const struct sunxi_sound_pcm_hw_params *p)
{
	const union snd_interval *interval = NULL;
	interval = &p->intervals[SND_PCM_HW_PARAM_ACCESS -
				SND_PCM_HW_PARAM_FIRST_INTERVAL];

	if (interval->mask != 0)
		return (snd_pcm_access_t)__ffs(interval->mask);
	return (snd_pcm_access_t)-1;
}

static inline snd_pcm_format_t params_format(const struct sunxi_sound_pcm_hw_params *p)
{
	const union snd_interval *interval = NULL;
	interval = &p->intervals[SND_PCM_HW_PARAM_FORMAT -
				SND_PCM_HW_PARAM_FIRST_INTERVAL];

	if (interval->mask != 0)
		return (snd_pcm_format_t)__ffs(interval->mask);
	return SND_PCM_FORMAT_UNKNOWN;
}

static inline snd_interval_t *hw_param_interval(struct sunxi_sound_pcm_hw_params *params,
						snd_pcm_hw_param_t var)
{
	return &params->intervals[var];
}

static inline const union snd_interval *hw_param_interval_c(const struct sunxi_sound_pcm_hw_params *params,
							int var)
{
	return &params->intervals[var - SND_PCM_HW_PARAM_FIRST_INTERVAL];
}

static inline unsigned int params_channels(const struct sunxi_sound_pcm_hw_params *p)
{
	return hw_param_interval_c(p, SND_PCM_HW_PARAM_CHANNELS)->range.min;
}

static inline unsigned int params_rate(const struct sunxi_sound_pcm_hw_params *p)
{
	return hw_param_interval_c(p, SND_PCM_HW_PARAM_RATE)->range.min;
}

static inline unsigned int params_period_size(const struct sunxi_sound_pcm_hw_params *p)
{
	return hw_param_interval_c(p, SND_PCM_HW_PARAM_PERIOD_SIZE)->range.min;
}

static inline unsigned int params_period_time(const struct sunxi_sound_pcm_hw_params *p)
{
	return hw_param_interval_c(p, SND_PCM_HW_PARAM_PERIOD_TIME)->range.min;
}

static inline unsigned int params_periods(const struct sunxi_sound_pcm_hw_params *p)
{
	return hw_param_interval_c(p, SND_PCM_HW_PARAM_PERIODS)->range.min;
}

static inline unsigned int params_buffer_size(const struct sunxi_sound_pcm_hw_params *p)
{
	return hw_param_interval_c(p, SND_PCM_HW_PARAM_BUFFER_SIZE)->range.min;
}

static inline unsigned int params_buffer_time(const struct sunxi_sound_pcm_hw_params *p)
{
	return hw_param_interval_c(p, SND_PCM_HW_PARAM_BUFFER_TIME)->range.min;
}


int sunxi_sound_pcm_format_physical_width(snd_pcm_format_t format);


#endif /* __SUNXI_SOUND_PCM_COMMON_H */
