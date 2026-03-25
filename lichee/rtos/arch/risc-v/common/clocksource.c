/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the People's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECqHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
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

#include <stdint.h>
#include <stddef.h>
#include <ktimer.h>
#include <stdio.h>
#include <timex.h>
#include <csr.h>
#include <sunxi_hal_common.h>

#ifdef CONFIG_ARCH_SUN20IW2P1
#ifdef CONFIG_DRIVERS_CCMU
#include <hal_clk.h>
#endif

#define DEFAULT_CLOCKSOURCE_TIMER_CLK_FREQ 40000000
#define DEFAULT_NS_PER_TICK 25

static uint32_t s_timer_clk_freq = DEFAULT_CLOCKSOURCE_TIMER_CLK_FREQ;
static uint32_t s_ns_per_tick = DEFAULT_NS_PER_TICK;

uint32_t arch_timer_get_cntfrq(void)
{
    return s_timer_clk_freq;
}
#else
#define TIMER_FREQ    (40000000)

static uint32_t arch_timer_rate;
static inline uint32_t arch_timer_get_cntfrq(void)
{
    uint32_t val = TIMER_FREQ;
    return val;
}
#endif

static inline uint64_t arch_counter_get_cntpct(void)
{
    uint64_t cval;

    cval = get_cycles64();
    return cval;
}

uint64_t (*arch_timer_read_counter)(void) = arch_counter_get_cntpct;
static cycle_t arch_counter_read(struct clocksource *cs)
{
    return arch_timer_read_counter();
}

static struct clocksource arch_timer_clocksource =
{
    .name   = "arch_sys_counter",
    .read   = arch_counter_read,
    .mask   = CLOCKSOURCE_MASK(56),
};

int64_t ktime_get(void)
{
    struct clocksource *cs = &arch_timer_clocksource;
    uint64_t arch_counter = cs->read(cs);

#ifdef CONFIG_ARCH_SUN20IW2P1
    return arch_counter * s_ns_per_tick;
#else
    double ns_per_ticks = cs->mult / (1 << cs->shift);

    //~41.6666667 nano seconds per tick.
    ns_per_ticks = NS_PER_TICK;

    return arch_counter * ns_per_ticks;
#endif
}

int do_gettimeofday(struct timespec64 *ts)
{
    hal_assert(ts != NULL);

    int64_t nsecs = ktime_get();

    ts->tv_sec  = nsecs / NSEC_PER_SEC;
    ts->tv_nsec = nsecs % NSEC_PER_SEC;

    return 0;
}

void udelay(unsigned int us)
{
    uint64_t start, target;
    uint64_t frequency;

    frequency = arch_timer_get_cntfrq();
    start = arch_counter_get_cntpct();
    target = frequency / 1000000ULL * us;

    while (arch_counter_get_cntpct() - start <= target) ;
}

void mdelay(uint32_t ms)
{
    udelay(ms * 1000);
}

void sdelay(uint32_t sec)
{
    mdelay(sec * 1000);
}

#ifdef CONFIG_ARCH_SUN20IW2P1
void timekeeping_init(void)
{
    uint32_t timer_clk_freq = 0;
#ifdef CONFIG_DRIVERS_CCMU
    timer_clk_freq = HAL_GetHFClock();
    if (!timer_clk_freq)
    {
        timer_clk_freq = DEFAULT_CLOCKSOURCE_TIMER_CLK_FREQ;
    }
#else
    timer_clk_freq = DEFAULT_CLOCKSOURCE_TIMER_CLK_FREQ;
#endif

    s_timer_clk_freq = timer_clk_freq;
    s_ns_per_tick = 1000000000 / timer_clk_freq;
}
#else
void timekeeping_init(void)
{
    struct clocksource *cs = &arch_timer_clocksource;

    cs->shift = 24;
    cs->mult = 699050667;
    cs->arch_timer_rate = arch_timer_get_cntfrq();
    if (cs->arch_timer_rate == 0)
    {
        cs->arch_timer_rate = TIMER_FREQ;
    }

    return;
}
#endif

void timestamp(char *tag)
{
    struct timespec64 ts;
    do_gettimeofday(&ts);

    printf("[TSM]: %*.*s]:sec %ld, nsec %d.\n", 12, 12, tag, ts.tv_sec, ts.tv_nsec);
}

