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

#ifndef _AW_MALLOC_H
#define _AW_MALLOC_H

#if (defined(CONFIG_HEAP_MULTIPLE) || defined(CONFIG_HEAP_MULTIPLE_DYN))

#define CONFIG_AW_MEM_HEAP_COUNT (4)

#define SRAM_HEAP_ID   (0)
#define LPSRAM_HEAP_ID (1)
#define HPSRAM_HEAP_ID (2)
#define DRAM_HEAP_ID   (3)

extern unsigned long __dram_heap_start;
extern unsigned long __sram_heap_start;
extern unsigned long __lpram_heap_start;
extern unsigned long __hpram_heap_start;

size_t aw_xPortGetTotalHeapSize( int heapID );
size_t aw_xPortGetFreeHeapSize( int heapID );
size_t aw_xPortGetMinimumEverFreeHeapSize( int heapID );
void aw_vPortGetHeapStats( int heapID, HeapStats_t * pxHeapStats );

void *aw_pvPortMalloc(int heapID, size_t size);
void aw_vPortFree(int heapID, void *ptr);

void *aw_sram_pvPortMalloc(size_t size);
void aw_sram_vPortFree(void *ptr);

void *aw_dram_pvPortMalloc(size_t size);
void aw_dram_vPortFree(void *ptr);
void *aw_lpsram_pvPortMalloc(size_t size);
void aw_lpsram_vPortFree(void *ptr);
void *aw_hpsram_pvPortMalloc(size_t size);
void aw_hpsram_vPortFree(void *ptr);

#endif
#endif
