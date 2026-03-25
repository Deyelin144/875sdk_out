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
#ifndef __IRQSOFF_H
#define __IRQSOFF_H

#include "config.h"

#if DYNAMIC_FTRACE && DYNAMIC_FTRACE_IRQSOFF
#include "port_time.h"

#define IRQS_ON  0
#define IRQS_OFF 1

#define sync_traceinfo_to_ringbuf(latency) \
do { \
} while(0)

#define TRACE_IRQS_OFF() \
do { \
	extern uint8_t  g_irqsoff_flag; \
	extern uint64_t g_irqs_off_time; \
	if (g_irqsoff_flag == IRQS_ON) { \
		g_irqsoff_flag = IRQS_OFF; \
		g_irqs_off_time = ftrace_get_time(); \
	} \
} while(0)

#define TRACE_IRQS_ON() \
do { \
	uint64_t delta = 0; \
	static uint64_t latency = 0; \
	extern uint8_t  g_irqsoff_flag; \
	extern uint64_t g_irqs_on_time; \
	extern uint64_t g_irqs_off_time; \
	if (g_irqsoff_flag == IRQS_OFF) { \
		g_irqsoff_flag = IRQS_ON; \
		g_irqs_on_time = ftrace_get_time(); \
		delta = g_irqs_on_time - g_irqs_off_time; \
		if (delta > latency) { \
			latency = delta; \
			sync_traceinfo_to_ringbuf(latency); \
		} \
	} \
} while(0)

#define C906_SAVE_IRQ(irq) \
({ \
	extern uint32_t g_irq; \
	g_irq = irq; \
})

#endif /* DYNAMIC_FTRACE_IRQSOFF */
#endif /* __IRQSOFF_H */
