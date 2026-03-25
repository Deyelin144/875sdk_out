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

#include <xtensa/hal.h>
#include <xtensa/tie/xt_externalregisters.h>
#include <xtensa/config/core.h>
#include <xtensa/config/core-matmap.h>
#include <xtensa/xtruntime.h>
#include <string.h>
#include "spinlock.h"
#if defined(CONFIG_DRIVERS_CCMU) || defined(CONFIG_DRIVERS_CCU)
#include <hal_clk.h>
#endif
#ifdef CONFIG_DRIVERS_INTC
#include <hal_intc.h>
#endif
#ifdef CONFIG_DRIVERS_GPIO
#include <hal_gpio.h>
#endif
#ifdef CONFIG_DRIVERS_HWSPINLOCK
#include <hal_hwspinlock.h>
#endif
#include <hal_uart.h>
#include <console.h>
#ifdef CONFIG_COMPONENTS_PM
#include <pm_init.h>
#endif
#ifdef CONFIG_COMPONENTS_AW_SYS_CONFIG_SCRIPT
#include <hal_cfg.h>
#endif
extern void cache_config(void);
extern void _xt_tick_divisor_init(void);
extern void console_uart_init(void);
extern unsigned int xtbsp_clock_freq_hz(void);
extern void set_arch_timer_ticks_per_us(uint32_t dsp_clk_freq);

__attribute__((weak)) void heap_init(void)
{

}

static void set_rtos_tick(void)
{
	unsigned int lock;

	/* system tick my use glabal var */
	spin_lock_irqsave(lock);
	_xt_tick_divisor_init();
	spin_unlock_irqrestore(lock);
}

void board_init(void)
{
	heap_init();

	/* cache configuration */
	cache_config();

#ifdef CONFIG_COMPONENTS_AW_SYS_CONFIG_SCRIPT
	hal_cfg_init();
#endif

#ifdef CONFIG_ARCH_SUN20IW2
	extern void cpufreq_vf_init(void);
	cpufreq_vf_init();
#endif

#ifdef CONFIG_COMPONENTS_PM
	pm_wakecnt_init();
	pm_wakesrc_init();
	pm_syscore_init();
	pm_devops_init();
#endif

#if defined(CONFIG_DRIVERS_CCMU) || defined(CONFIG_DRIVERS_CCU)
	/* ccmu init */
	hal_clock_init();
#endif

#ifdef CONFIG_DRIVERS_INTC
	/* intc init */
	hal_intc_init(SUNXI_DSP_IRQ_R_INTC);
#endif

#if defined(CONFIG_DRIVERS_GPIO) && !defined(CONFIG_DRIVER_BOOT_DTS)
	/* gpio init */
	hal_gpio_init();
	hal_gpio_r_all_irq_disable();
#endif

#ifdef CONFIG_DRIVERS_HWSPINLOCK
	hal_hwspinlock_init();
#endif

#if defined(CONFIG_ARCH_SUN20IW2)
	console_uart = CONSOLE_UART;
#if defined(CONFIG_COMPONENTS_FREERTOS_CLI)
	hal_uart_init_for_amp_cli(CONSOLE_UART);
#endif
#else
#if defined(CONFIG_COMPONENTS_FREERTOS_CLI) && !defined(CONFIG_DRIVER_BOOT_DTS)
	console_uart_init();
#endif
#endif

	/* prepare for time API and rtos tick */
	set_arch_timer_ticks_per_us(xtbsp_clock_freq_hz());

	/* set rtos tick */
	set_rtos_tick();
}

