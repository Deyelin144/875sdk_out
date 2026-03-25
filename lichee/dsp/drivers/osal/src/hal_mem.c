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
#include <stdlib.h>

#include <FreeRTOS.h>

void *hal_malloc(uint32_t size)
{
	/* return malloc(size); */
	return pvPortMalloc(size);
}

void hal_free(void *p)
{
	/* free(p); */
	vPortFree(p);
}

void *hal_malloc_align(uint32_t size, int align)
{
       void *ptr;
       void *align_ptr;
       int uintptr_size;
       int align_size;

       /* sizeof pointer */
       uintptr_size = sizeof(void*);
       uintptr_size -= 1;

       /* align the alignment size to uintptr size byte */
       align = ((align + uintptr_size) & ~uintptr_size);

       /* get total aligned size */
       align_size = ((size + uintptr_size) & ~uintptr_size) + align;
       /* allocate memory block from heap */
       ptr = hal_malloc(align_size);
       if (ptr != NULL)
       {
               /* the allocated memory block is aligned */
               if (((uint32_t)ptr & (align - 1)) == 0)
               {
                       align_ptr = (void *)((uint32_t)ptr + align);
               }
               else
               {
                       align_ptr = (void *)(((uint32_t)ptr + (align - 1)) & ~(align - 1));
               }

               /* set the pointer before alignment pointer to the real pointer */
               *((uint32_t *)((uint32_t)align_ptr - sizeof(void *))) = (uint32_t)ptr;

               ptr = align_ptr;
       }

       return ptr;
}

void hal_free_align(void *p)
{
       void *real_ptr;

       real_ptr = (void *) * (uint32_t *)((uint32_t)p - sizeof(void *));
       hal_free(real_ptr);
}


unsigned long hal_virt_to_phys(unsigned long virtaddr)
{
	return virtaddr;
}

unsigned long hal_phys_to_virt(unsigned long phyaddr)
{
	return phyaddr;
}
