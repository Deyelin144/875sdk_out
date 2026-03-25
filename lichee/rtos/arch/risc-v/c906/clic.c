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

#include <stdio.h>
#include <stdlib.h>
#include <excep.h>
#include <csr.h>
#include <rv_io.h>
#include <irqflags.h>
#include <timex.h>

#ifdef CONFIG_COMPONENTS_PM
#include "pm_syscore.h"
#endif

#define CSR_PLIC_BASE            0xfc1
#define C910_PLIC_CLINT_OFFSET   0x04000000

static uint64_t mmode_peripheral_addr_base;
static uint64_t clic_int_control_addr_base;
static uint64_t clic_int_control_addr_comp;
static uint64_t clic_int_control_addr_mtime;

int clic_driver_init(void)
{
    set_csr(CSR_MIE, MIE_MSIE);

    mmode_peripheral_addr_base = read_csr(CSR_PLIC_BASE);
    if (mmode_peripheral_addr_base == 0)
    {
        printf("%s line %d, fatal error, peripheral address is null!\n", __func__, __LINE__);
        return -1;
    }

    clic_int_control_addr_base = mmode_peripheral_addr_base + C910_PLIC_CLINT_OFFSET;

    clic_int_control_addr_mtime = clic_int_control_addr_base + 0xbff8;
    clic_int_control_addr_comp = clic_int_control_addr_base + 0x4000;

    return 0;
}

uint64_t sunxi_clic_read_mtime(void)
{
    uint32_t lo, hi;
    unsigned long addr = clic_int_control_addr_mtime;

    do
    {
        hi = readl_relaxed((uint32_t *)addr + 1);
        lo = readl_relaxed((uint32_t *)addr);
    } while (hi != readl_relaxed((uint32_t *)addr + 1));

    return ((uint64_t)hi << 32) | (uint64_t)lo;
}

void sunxi_program_timer_next_event(unsigned long next_compare)
{
    unsigned int mask = -1U;
    unsigned long value = next_compare;
    unsigned long addr = clic_int_control_addr_comp;

    writel_relaxed(value & mask, (void *)(addr));
    writel_relaxed(value >> 32, (void *)(addr) + 0x04);
}

uint64_t sunxi_get_timer_next_event(void)
{
	uint64_t low, high;
	unsigned long addr = clic_int_control_addr_comp;

	low = readl_relaxed((void *)(addr));
	high = readl_relaxed((void *)(addr) + 0x04);

	return low | (high << 32);
}

#define DEFAULT_ARCH_TIMER_CLK_FREQ (40000000UL)
static unsigned long delta = DEFAULT_ARCH_TIMER_CLK_FREQ / CONFIG_HZ;
static uint64_t s_delta_after_wake_up = 0;

#ifdef CONFIG_ARCH_SUN20IW2P1
extern uint32_t arch_timer_get_cntfrq(void);
#endif

#ifdef CONFIG_COMPONENTS_PM
static int clic_suspend(void *data, suspend_mode_t mode)
{
	int ret = 0;

	clear_csr(CSR_MIE, MIE_MTIE);

	uint64_t current = get_cycles64();
	uint64_t target_count_value = sunxi_get_timer_next_event();

	if (current < target_count_value)
	{
		s_delta_after_wake_up = target_count_value - current;
	}
	else
	{
		s_delta_after_wake_up = delta;
	}
	return ret;
}

static void clic_resume(void *data, suspend_mode_t mode)
{
	set_csr(CSR_MIE, MIE_MTIE);
	sunxi_program_timer_next_event(get_cycles64() + s_delta_after_wake_up);
}

static struct syscore_ops clic_syscore_ops = {
	.name = "clic_syscore_ops",
	.suspend = clic_suspend,
	.resume = clic_resume,
};
#endif /* CONFIG_COMPONENTS_PM */

void system_tick_init(void)
{
#ifdef CONFIG_ARCH_SUN20IW2P1
    delta = arch_timer_get_cntfrq() / CONFIG_HZ;
#endif

    set_csr(CSR_MIE, MIE_MTIE);
    sunxi_program_timer_next_event(get_cycles64() + delta);

#ifdef CONFIG_COMPONENTS_PM
	int ret;
	ret = pm_syscore_register(&clic_syscore_ops);
	if (ret)
		printf("WARNING: clic syscore ops registers failed\n");
#endif

}

void riscv_timer_interrupt(void)
{
    clear_csr(CSR_MIE, MIE_MTIE);

    sunxi_program_timer_next_event(get_cycles64() + delta);
    set_csr(CSR_MIE, MIE_MTIE);
}

void rv_trigger_soft_interrupt(void)
{
	volatile uint32_t *msip_reg = (volatile uint32_t *)0x54000000;

	*msip_reg = 1;
	__asm__ volatile("fence iorw,iorw");
	__asm__ volatile("sync");
}

void rv_soft_irq_handler_non_vec_mode(void)
{
	volatile uint32_t *msip_reg = (volatile uint32_t *)0x54000000;

	*msip_reg = 0;
	__asm__ volatile("fence iorw,iorw");
	__asm__ volatile("sync");

	vTaskSwitchContext();
}
