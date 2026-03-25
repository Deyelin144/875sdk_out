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

#include "type.h"
#include "arch.h"
#include "standby_scb.h"

volatile uint32_t scb_bak[SCB_RW_REGS_NUM];

void scb_save(void)
{
	volatile uint32_t *pdata;
	uint32_t start;
	int i;

	scb_bak[0] = *(volatile uint32_t *)SCB_ACTLR_REG;

	pdata = (volatile uint32_t *)SCB_ICSR_REG;
	start = (uint32_t)pdata;
	i = 1;
	while (pdata <= (uint32_t *)(start + 0x38))
	{
		scb_bak[i] = *pdata;
		/* 32bit point */
		pdata++;
		i++;
	}

	scb_bak[i] = *(volatile uint32_t *)SCB_CPACR_REG;
	i++;
	scb_bak[i] = *(volatile uint32_t *)SCB_NSACR_REG;
}

void scb_restore(void)
{
	volatile uint32_t *pdata;
	uint32_t start;
	int i;

	*(volatile uint32_t *)SCB_ACTLR_REG = scb_bak[0];

	pdata = (volatile uint32_t *)SCB_ICSR_REG;
	start = (uint32_t)pdata;
	i = 1;
	while (pdata <= (uint32_t *)(start + 0x38))
	{
		*(volatile uint32_t *)pdata = scb_bak[i];
		/* 32bit point */
		pdata++;
		i++;
	}

	*(volatile uint32_t *)SCB_CPACR_REG = scb_bak[i];
	i++;
	*(volatile uint32_t *)SCB_NSACR_REG = scb_bak[i];
}
