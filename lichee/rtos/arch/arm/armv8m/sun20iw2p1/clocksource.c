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
#include <stdio.h>

#include <barrier.h>
#include <ktimer.h>
#include <io.h>

#include <sunxi_hal_rcosc_cali.h>

#include <hal_status.h>

#define SUNXI_RTC_BASE (0x40051000)
#define SUNXI_RTC_FREE_COUNTER_LOW_OFFSET (0x60)
#define SUNXI_RTC_FREE_COUNTER_HIGH_OFFSET (0x64)

#define DEFAULT_CLOCKSOURCE_TIMER_CLK_FREQ 32000
#define DEFAULT_NS_PER_TICK 31250

static uint32_t s_timer_clk_freq = DEFAULT_CLOCKSOURCE_TIMER_CLK_FREQ;
static uint32_t s_ns_per_tick = DEFAULT_NS_PER_TICK;

static inline uint32_t arch_timer_get_cntfrq(void)
{
    return s_timer_clk_freq;
}

static inline uint64_t arch_counter_get_cntpct(void)
{
    uint64_t cval = 0ULL;

    isb();
    uint64_t lval = readl(SUNXI_RTC_BASE + SUNXI_RTC_FREE_COUNTER_LOW_OFFSET);
    uint64_t hval = readl(SUNXI_RTC_BASE + SUNXI_RTC_FREE_COUNTER_HIGH_OFFSET);
    cval = lval | (hval << 32);
    isb();
    return cval;
}

static cycle_t arch_counter_read(struct clocksource *cs)
{
    return arch_counter_get_cntpct();
}

static struct clocksource arch_timer_clocksource =
{
    .name   = "arch_sys_counter",
    .read   = arch_counter_read,
};

int64_t ktime_get(void)
{
    struct clocksource *cs = &arch_timer_clocksource;

    uint64_t arch_counter = cs->read(cs);

    return arch_counter * s_ns_per_tick;
}

int do_gettimeofday(struct timespec64 *ts)
{
    if (ts == NULL)
        return 0;

    int64_t nsecs = ktime_get();

    ts->tv_sec  = nsecs / NSEC_PER_SEC;
    ts->tv_nsec = nsecs % NSEC_PER_SEC;

    return 0;
}

void timekeeping_init(void)
{
    uint32_t rccal_clk_freq = 0;
    hal_status_t ret = hal_get_rccal_output_freq(&rccal_clk_freq);
    if (ret != HAL_OK)
    {
        rccal_clk_freq = DEFAULT_CLOCKSOURCE_TIMER_CLK_FREQ;
    }

    s_timer_clk_freq = rccal_clk_freq;
    s_ns_per_tick = 1000000000 / rccal_clk_freq;
    return;
}
