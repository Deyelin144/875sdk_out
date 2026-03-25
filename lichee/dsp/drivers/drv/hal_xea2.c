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

#include <xtensa_api.h>

#include <sunxi_hal_common.h>
#include <irqs.h>
#include <interrupt.h>
#include <hal_intc.h>

#ifdef CONFIG_DRIVERS_INTC

s32 irq_request(u32 irq_no, interrupt_handler_t hdle, void *arg)
{
	xt_handler old;
	s32 ret;

	if ((irq_no & RINTC_IRQ_MASK) == RINTC_IRQ_MASK) { /* intc interrupt */
		ret = install_isr(irq_no ^ RINTC_IRQ_MASK, (__pISR_hdle_t)hdle, arg);
	} else { /* dsp interrupt */
		old = xt_set_interrupt_handler(irq_no, (xt_handler)hdle, arg);
		ret = old ? OK : FAIL;
	}

	return ret;
}

s32 irq_free(u32 irq_no)
{
	xt_handler old;
	s32 ret;

	if ((irq_no & RINTC_IRQ_MASK) == RINTC_IRQ_MASK) { /* intc interrupt */
		ret = uninstall_isr(irq_no ^ RINTC_IRQ_MASK, NULL);
	} else { /* dsp interrupt */
		old = xt_set_interrupt_handler(irq_no, (xt_handler)NULL, NULL);
		ret = old ? OK : FAIL;
	}
	return ret;
}

s32 irq_enable(u32 irq_no)
{
	s32 ret = OK;

	if ((irq_no & RINTC_IRQ_MASK) == RINTC_IRQ_MASK) /* intc interrupt */
		ret = interrupt_enable(irq_no ^ RINTC_IRQ_MASK);
	else /* dsp interrupt */
		xt_ints_on(1 << irq_no);

	return ret;
}

s32 irq_disable(u32 irq_no)
{
	s32 ret = OK;

	if ((irq_no & RINTC_IRQ_MASK) == RINTC_IRQ_MASK) /* intc interrupt */
		ret = interrupt_disable(irq_no ^ RINTC_IRQ_MASK);
	else /* dsp interrupt */
		xt_ints_off(1 << irq_no);

	return ret;
}

#else /* not defined CONFIG_DRIVERS_INTC */

s32 irq_request(u32 irq_no, interrupt_handler_t hdle, void *arg)
{
	xt_handler old;

	old = xt_set_interrupt_handler(irq_no, (xt_handler)hdle, arg);

	return old ? OK : FAIL;
}

s32 irq_free(u32 irq_no)
{
	xt_handler old;

	old = xt_set_interrupt_handler(irq_no, (xt_handler)NULL, NULL);

	return old ? OK : FAIL;
}

s32 irq_enable(u32 irq_no)
{
	xt_ints_on(1 << irq_no);

	return OK;
}

s32 irq_disable(u32 irq_no)
{
	xt_ints_off(1 << irq_no);

	return OK;
}

#endif /* CONFIG_DRIVERS_INTC */
