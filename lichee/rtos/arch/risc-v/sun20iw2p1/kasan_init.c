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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "kasan_rtos.h"

void rt_malloc_small_sethook(void (*hook)(void *ptr, uint32_t size));
void rt_free_small_sethook(void (*hook)(void *ptr, uint32_t size));
extern void kasan_enable_report(void);
/*
 * Initialize hook functions related to KASAN for allocating
 * memory from the heap.
 */
static void kasan_init_nommu(void)
{
	kasan_init_report();
	kasan_enable_report();

	rt_malloc_small_sethook(rt_malloc_small_func_hook);
	rt_free_small_sethook(rt_free_small_func_hook);
}

void kasan_init(void)
{
	kasan_init_nommu();
}

/* Initialize all shadow mem with zero*/
static void kasan_shadow_init_nommu(void)
{
	unsigned long addr;
	unsigned long end;
	unsigned long temp;
	unsigned long len = sizeof(unsigned long);

	end = CONFIG_ARCH_START_ADDRESS + CONFIG_ARCH_MEM_LENGTH;
	addr = end - (CONFIG_ARCH_MEM_LENGTH>>3);

	for (temp = addr ; temp <= end; temp += len) {
		*((unsigned long *)temp) = 0;
	}
}

void kasan_early_init(void)
{
	kasan_shadow_init_nommu();
}
