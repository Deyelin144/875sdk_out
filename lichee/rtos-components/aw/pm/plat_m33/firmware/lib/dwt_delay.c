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

#include "head.h"
#include "delay.h"

#define  DWT_CR      *(volatile uint32_t *)0xE0001000
#define  DWT_CYCCNT  *(volatile uint32_t *)0xE0001004
#define  DEM_CR      *(volatile uint32_t *)0xE000EDFC

#define  DEM_CR_TRCENA                   (1 << 24)
#define  DWT_CR_CYCCNTENA                (1 <<  0)

int hal_dwt_tick_init(void)
{
	DEM_CR |= (uint32_t)DEM_CR_TRCENA;
	DWT_CYCCNT = (uint32_t)0u;
	DWT_CR |= (uint32_t)DWT_CR_CYCCNTENA;

	return 0;
}

uint32_t hal_dwt_tick_read(void)
{
	return ((uint32_t)DWT_CYCCNT);
}

static uint32_t get_cpu_freq(void)
{
	/* CPU Clock Freq */
	return head->cpufreq;
}

/* 
 * Used when processor in high frequency.
 * When the processor is running at low frequency,
 * single instruction is used to delay assignment cycle.
 */
void hal_dwt_delay(uint32_t us)
{
	uint32_t ticks;
	uint32_t told,tnow,tcnt=0;
	uint32_t cpu_freq = get_cpu_freq();

	hal_dwt_tick_init();
	ticks = us * (cpu_freq / 1000000);
	tcnt = 0;
	told = (uint32_t)hal_dwt_tick_read();

	while(1)
	{
		tnow = (uint32_t)hal_dwt_tick_read();
		if(tnow != told)
		{
			if(tnow > told)
				tcnt += tnow - told;
			else
				tcnt += UINT32_MAX - told + tnow;
			told = tnow;
			if(tcnt >= ticks)
				break;
		}
	}
}

