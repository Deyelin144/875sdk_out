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
#include <xtensa/config/core-isa.h>

#include <xtensa_timer.h>

#include <sunxi_hal_common.h>
#include <interrupt.h>
#include <irqs.h>

#ifdef XT_CLOCK_FREQ
#define MS_TO_CYCLE(v)	(XT_CLOCK_FREQ / 1000 * v)
#else
#ifdef XT_BOARD
#include <xtensa/xtbsp.h>
#define MS_TO_CYCLE(v)	(xtbsp_clock_freq_hz() / 1000 * v)
#endif
#endif

typedef enum {
	HAL_ARCH_TIMER_STATUS_FAILED = -3,
	HAL_ARCH_TIMER_STATUS_BUSY = -2,
	HAL_ARCH_TIMER_STATUS_INVALID_PARAM = -1,
	HAL_ARCH_TIMER_STATUS_OK = 0,
} hal_arch_timer_status_t;

struct arch_timer_priv {
	bool used;
	uint32_t irq_no;
	interrupt_handler_t func;
	void *arg;
};

static struct arch_timer_priv g_timers[XCHAL_NUM_TIMERS];

static inline bool timer_is_used(int32_t num)
{
	return g_timers[num].used;
}

static uint32_t timer_irq_no(int32_t num)
{
	switch (num) {
	case 0:
		return SUNXI_DSP_IRQ_DSP_TIMER0;
	case 1:
		return SUNXI_DSP_IRQ_DSP_TIMER1;
	default:
		return 0xffffffff; /* FIXME: invalid timer number */
	}
}

int32_t hal_arch_timer_init(int32_t num, interrupt_handler_t func, void *arg)
{
	struct arch_timer_priv *timer;

	if (num == XT_TIMER_INDEX || num >= XCHAL_NUM_TIMERS)
		return HAL_ARCH_TIMER_STATUS_INVALID_PARAM;

	if (timer_is_used(num))
		return HAL_ARCH_TIMER_STATUS_BUSY;

	g_timers[num].used = true;
	g_timers[num].irq_no = timer_irq_no(num);
	g_timers[num].func = func;
	g_timers[num].arg = arg;

	timer = &g_timers[num];

	if (irq_request(timer->irq_no, timer->func, timer->arg) < 0)
		return HAL_ARCH_TIMER_STATUS_FAILED;

	return HAL_ARCH_TIMER_STATUS_OK;
}

int32_t hal_arch_timer_deinit(int32_t num)
{
	struct arch_timer_priv *timer = &g_timers[num];

	if (num == XT_TIMER_INDEX || num >= XCHAL_NUM_TIMERS)
		return HAL_ARCH_TIMER_STATUS_INVALID_PARAM;

	if (!timer_is_used(num))
		return HAL_ARCH_TIMER_STATUS_BUSY;

	irq_disable(timer->irq_no);
	xthal_set_ccompare(num, 0);
	timer->used = false;

	return HAL_ARCH_TIMER_STATUS_OK;
}

int32_t hal_arch_timer_start(int32_t num, uint32_t interval)
{
	struct arch_timer_priv *timer = &g_timers[num];

	if (num == XT_TIMER_INDEX || num >= XCHAL_NUM_TIMERS)
		return HAL_ARCH_TIMER_STATUS_INVALID_PARAM;

	if (!timer_is_used(num))
		return HAL_ARCH_TIMER_STATUS_BUSY;

	xthal_set_ccompare(num, xthal_get_ccount() + MS_TO_CYCLE(interval));

	if (irq_enable(timer->irq_no) < 0)
		return HAL_ARCH_TIMER_STATUS_FAILED;

	return HAL_ARCH_TIMER_STATUS_OK;
}

int32_t hal_arch_timer_stop(int32_t num)
{
	struct arch_timer_priv *timer = &g_timers[num];

	if (num == XT_TIMER_INDEX || num >= XCHAL_NUM_TIMERS)
		return HAL_ARCH_TIMER_STATUS_INVALID_PARAM;

	if (!timer_is_used(num))
		return HAL_ARCH_TIMER_STATUS_BUSY;

	irq_disable(timer->irq_no);
	xthal_set_ccompare(num, 0);

	return HAL_ARCH_TIMER_STATUS_OK;
}

int32_t hal_arch_timer_update(int32_t num, uint32_t interval)
{
	if (num == XT_TIMER_INDEX || num >= XCHAL_NUM_TIMERS)
		return HAL_ARCH_TIMER_STATUS_INVALID_PARAM;

	if (!timer_is_used(num))
		return HAL_ARCH_TIMER_STATUS_BUSY;

	if (interval)
		xthal_set_ccompare(num, xthal_get_ccount() + MS_TO_CYCLE(interval));
	else /* just to stop */
		xthal_set_ccompare(num, 0);

	return HAL_ARCH_TIMER_STATUS_OK;
}
