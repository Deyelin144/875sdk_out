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

#include <stdint.h>
#include <type.h>
#include <wakeup.h>
#include <platform.h>
#include <delay.h>

#define isb(option)             __asm__ __volatile__ ("isb " #option : : : "memory")

#define RTC_WUPTIMER_CTRL_REG	(0x40051508)
#define RTC_WUPTIMER_VAL_REG	(0x4005150c)

#define RTC_FREE_COUNTER_LOW_OFFSET (0x60)
#define RTC_FREE_COUNTER_HIGH_OFFSET (0x64)

#define PRCM_SYS_LFCLK_CTL          (0x80)
#define PRCM_SYS_RCOSC_FRQ_DET      (0x140)

#define MSEC_PER_SEC    1000L
#define USEC_PER_MSEC   1000L
#define NSEC_PER_SEC    1000000000L

#define DEFAULT_NS_PER_TICK 31250

#define WDT_BASE	(0x40020400)
#define WDT_CTL_REG    (WDT_BASE + 0x10)
#define WDT_CFG_REG	(WDT_BASE + 0x14)
#define WDT_MODE_REG	(WDT_BASE + 0x18)
#define WDT_KEY_FIELD	(0x16aa << 16)
#define WDT_CFG_RESET           (0x1)
#define WDT_MODE_EN             (0x1)
#define KEY_FIELD_MAGIC         (0x16AA0000)
#define WDT_TIMEOUT_OFFSET      (4)
#define WDT_CTRL_RESTART        (0x1 << 0)
#define WDT_CTRL_KEY            (0x0a57 << 1)

#define DSP_BOOT_FLAG_REG	(0x400501cc)
#define RV_BOOT_FLAG_REG	(0x400501d8)

static uint32_t s_ns_per_tick = DEFAULT_NS_PER_TICK;

struct timespec64
{
    uint64_t  tv_sec;                 /* seconds */
    uint32_t  tv_nsec;                /* nanoseconds */
};

int wakeup_check_callback(void)
{
	return 0;
}

void enable_wuptimer(unsigned int time_ms)
{
    unsigned int val;
    unsigned char lfclk_src_sel;
    lfclk_src_sel = (readl(GPRCM_BASE + PRCM_SYS_LFCLK_CTL) >> 24) & 0x01;

    if(lfclk_src_sel)
        val = (unsigned int)((time_ms / 1000) * 32768);
    else
        val = (unsigned int)((time_ms / 100) * ((readl(GPRCM_BASE + PRCM_SYS_RCOSC_FRQ_DET) >> 8) & 0xfffff));

    if(val > 0x7fffffff)
        val = 0x7fffffff;

    writel(val, RTC_WUPTIMER_VAL_REG);
    writel((0x1 << 31), RTC_WUPTIMER_CTRL_REG);
}

void disable_wuptimer(void)
{
	/*clear pending*/
	unsigned wup_timer = 0;

	wup_timer = readl(RTC_WUPTIMER_VAL_REG);
	wup_timer &= 0x80000000;
	if (wup_timer != 0) {
		// never witre zero if no pending!!!
		writel(0x80000000, RTC_WUPTIMER_VAL_REG);
		while(readl(RTC_WUPTIMER_VAL_REG) != 0);
	}
	/*disable wuptimer*/
	writel(0x0, RTC_WUPTIMER_CTRL_REG);
}

static inline uint64_t pm_arch_counter_get_cntpct(void)
{
    uint64_t cval = 0ULL;

    isb();
    uint64_t lval = readl(RTC_BASE + RTC_FREE_COUNTER_LOW_OFFSET);
    uint64_t hval = readl(RTC_BASE + RTC_FREE_COUNTER_HIGH_OFFSET);
    cval = lval | (hval << 32);
    isb();
    return cval;
}

static uint64_t pm_ktime_get(void)
{
    uint64_t arch_counter = 0ULL;

    arch_counter = pm_arch_counter_get_cntpct();

    return arch_counter * s_ns_per_tick;
}

uint64_t pm_gettime_ns(void)
{
    return pm_ktime_get();
}

void pm_wdt_restart(void)
{
	writel(0x429b0000, DSP_BOOT_FLAG_REG);
	writel(0x429b0000, RV_BOOT_FLAG_REG);

	writel((readl(WDT_MODE_REG) | WDT_KEY_FIELD) & ~(0x1 << 0), WDT_MODE_REG);
	isb();
	hal_dwt_delay(500);
	writel(WDT_KEY_FIELD | 0x1, WDT_CFG_REG);
	isb();
	writel((readl(WDT_MODE_REG) | WDT_KEY_FIELD) | 0x1, WDT_MODE_REG);
	while(1);
}

void pm_wdt_stop(void)
{
#ifdef CONFIG_COMPONENTS_PM_WATCHDOG
    unsigned int wtmode;
    wtmode = readl(WDT_MODE_REG);
    wtmode &= ~WDT_MODE_EN;
    wtmode |= KEY_FIELD_MAGIC;
    writel(wtmode, WDT_MODE_REG);

    isb();
#endif
}

void pm_wdt_start(void)
{
#ifdef CONFIG_COMPONENTS_PM_WATCHDOG
    pm_wdt_stop();
    unsigned int wtmode;
    wtmode = KEY_FIELD_MAGIC | (0x8 << WDT_TIMEOUT_OFFSET) | WDT_MODE_EN;
    writel(KEY_FIELD_MAGIC | WDT_CFG_RESET, WDT_CFG_REG);
    writel(wtmode, WDT_MODE_REG);

    isb();
    writel(WDT_CTRL_KEY | WDT_CTRL_RESTART, WDT_CTL_REG);
#endif
}
