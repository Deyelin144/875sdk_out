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

#include <serial.h>
#include <interrupt.h>
#include <stdio.h>
#include <stdint.h>
#include <FreeRTOS.h>
#include <string.h>
#include <task.h>
#include <aw_version.h>
#include <irqs.h>
#include <platform.h>
#include <memory.h>
#include <hal_gpio.h>
#include <hal_uart.h>
#include <hal_msgbox.h>
#include <hal_clk.h>
#include <hal_cache.h>
#ifdef CONFIG_DRIVERS_DMA
#include <hal_dma.h>
#endif

#include "excep.h"
#include "cpufreq.h"

#ifdef CONFIG_COMPONENTS_AW_SYS_CONFIG_SCRIPT
#include <hal_cfg.h>
#endif

#include <compiler.h>

#ifdef CONFIG_DRIVERS_SPINOR
#include <sunxi_hal_spinor.h>
#endif

#ifdef CONFIG_COMPONENTS_PM
#include "pm_init.h"
#endif

#ifdef CONFIG_KASAN
#include "kasan_rtos.h"
#endif

void system_tick_init(void);
void enter_interrupt_handler(void);
void exit_interrupt_handler(void);
int clic_driver_init(void);
void plic_init(void);
void timekeeping_init(void);
void handle_arch_irq(irq_regs_t *regs);

void C906_Default_IRQHandler(void)
{
    enter_interrupt_handler();
    handle_arch_irq(NULL);
    exit_interrupt_handler();
}

__weak void sunxi_dma_init(void)
{
    return;
}

__weak void sunxi_gpio_init(void)
{
    return;
}

__weak int sunxi_soundcard_init(void)
{
    return 0;
}

__weak void heap_init(void)
{
    return;
}

static void prvSetupHardware(void)
{
    timekeeping_init();
    plic_init();
    irq_init();
    hal_clock_init();
    hal_gpio_init();
#ifdef CONFIG_DRIVERS_MSGBOX_AMP
    hal_msgbox_init();
#endif

#if !defined(CONFIG_DISABLE_ALL_UART_LOG)
    serial_init();

#if defined(CONFIG_COMPONENT_CLI)
    hal_uart_init_for_amp_cli(CONFIG_CLI_UART_PORT);
#endif
#endif

    clic_driver_init();
    system_tick_init();
#ifdef CONFIG_HPSRAM_INIT_IN_OS
    hpsram_init();
#endif
#ifdef CONFIG_DRIVERS_DMA
    hal_dma_init();
#endif
}

void systeminit(void)
{
}

#ifdef CONFIG_KASAN
/* Used for contructor calls. */
typedef void (*ctor_fn_t)(void);

/* Call all constructor functions linked into the OS. */
void do_ctors(void)
{
	extern long __ctors_start__, __ctors_end__;
	unsigned long t;
	ctor_fn_t *fn = (ctor_fn_t *) &__ctors_start__;

	for (; fn < (ctor_fn_t *) &__ctors_end__; fn++)
	{
		t = (unsigned long)(*fn);
		/*
		 * Enable CONFIG_XIP allows FLASH to store read-only data,
		 * which generally do not lead to overflow.
		 * Here, filter out the constructors of global variables in FLASH,
		 * init shadow area corresponding to variables in the RAM region only.
		 */
		if ((t >= CONFIG_ARCH_START_ADDRESS) &&
			(t <= CONFIG_ARCH_START_ADDRESS + CONFIG_ARCH_MEM_LENGTH)) {
			(*fn)();
		}
	}
}
#endif

void start_kernel(void)
{
#ifdef CONFIG_KASAN
    kasan_early_init();
#endif
    portBASE_TYPE ret;

    void cache_init(void);
    cache_init();
#ifdef CONFIG_ARCH_HAVE_ICACHE
    hal_icache_init();
#endif

#ifdef CONFIG_ARCH_HAVE_DCACHE
    hal_dcache_init();
#endif

#ifdef CONFIG_DOUBLE_FREE_CHECK
    void memleak_double_free_check_init(void);
    memleak_double_free_check_init();
#endif

#ifdef CONFIG_COMPONENTS_STACK_PROTECTOR
    void stack_protector_init(void);
    stack_protector_init();
#endif
#ifdef CONFIG_KASAN
    kasan_init();
#endif

    systeminit();

    /* Init heap */
    heap_init();

#ifdef CONFIG_COMPONENTS_AW_SYS_CONFIG_SCRIPT
	hal_cfg_init();
#endif

#ifdef CONFIG_COMPONENTS_PM
    pm_riscv_clear_boot_flag();
    pm_wakecnt_init();
    pm_wakesrc_init();
    pm_devops_init();
    pm_syscore_init();
#endif

    cpufreq_vf_init();

    /* Init hardware devices */
    prvSetupHardware();
    setbuf(stdout, 0);
    setbuf(stdin, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

#ifdef CONFIG_COMPONENTS_CPLUSPLUS_UNWIND
    extern char __eh_frame_start[];
    void __register_frame ( void * );
    __register_frame( &__eh_frame_start );
#endif

#ifdef CONFIG_COMPONENT_CPLUSPLUS
    /* It should be called after the stdout is ready, otherwise the std:cout can't work  */
    int cplusplus_system_init(void);
    cplusplus_system_init();
#endif

#ifdef CONFIG_CHECK_ILLEGAL_FUNCTION_USAGE
    void __cyg_profile_func_init(void);
    __cyg_profile_func_init();
#endif

#ifdef CONFIG_KASAN
    do_ctors();
#endif


    /* Setup kernel components */

    void cpu0_app_entry(void *);
    ret = xTaskCreate(cpu0_app_entry, (signed portCHAR *) "init-thread-0", 4096, NULL, 31, NULL);
    if (ret != pdPASS)
    {
        printf("Error creating task, status was %d\n", ret);
        while (1);
    }

    vTaskStartScheduler();
}

