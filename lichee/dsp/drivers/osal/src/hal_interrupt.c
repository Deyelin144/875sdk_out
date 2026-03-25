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

#include <stdio.h>
#include <interrupt.h>
#include <hal_interrupt.h>
#include <hal_status.h>
#include <sunxi_hal_common.h>

#include <portmacro.h>

#include <xtensa/hal.h>

#include <FreeRTOS.h>
#include <task.h>

int32_t hal_request_irq(int32_t irq, hal_irq_handler_t handler, const char *name, void *data)
{
	if (irq_request(irq, (interrupt_handler_t)handler, data) == FAIL) {
		return HAL_ERROR;
	}

	return HAL_OK;
}

void hal_free_irq(int32_t irq)
{
	if (irq_free(irq) == FAIL) {
		printf("irq free failure.\n");
	}
}

int hal_enable_irq(int32_t irq)
{
	return irq_enable(irq);
}

void hal_disable_irq(int32_t irq)
{
	irq_disable(irq);
}

extern unsigned port_interruptNesting;	/* defined in port.c */
uint32_t hal_interrupt_get_nest(void)
{
	return port_interruptNesting;
}

void hal_interrupt_enable(void)
{
    if (hal_interrupt_get_nest() == 0) {
        taskEXIT_CRITICAL();
    } else {
        taskEXIT_CRITICAL_FROM_ISR(0);
    }
}

void hal_interrupt_disable(void)
{
    if (hal_interrupt_get_nest() == 0) {
        taskENTER_CRITICAL();
    } else {
        taskENTER_CRITICAL_FROM_ISR();
    }
}

unsigned long hal_interrupt_disable_irqsave(void)
{
    unsigned long flag = 0;
    if (hal_interrupt_get_nest() == 0) {
        taskENTER_CRITICAL();
    } else {
        flag = taskENTER_CRITICAL_FROM_ISR();
    }
    return flag;
}

void hal_interrupt_enable_irqrestore(unsigned long flag)
{
    if (hal_interrupt_get_nest() == 0) {
        taskEXIT_CRITICAL();
    } else {
        taskEXIT_CRITICAL_FROM_ISR(flag);
    }
}

unsigned long hal_interrupt_is_disable(void)
{
    unsigned long ps;
    __asm__ volatile("rsr.ps %0\n" : "=r"(ps));
    if ((ps & 0xf) >= XCHAL_EXCM_LEVEL)
        return 1;
    return 0;
}
