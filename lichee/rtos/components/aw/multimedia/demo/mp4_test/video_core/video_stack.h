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

#ifndef __LHH_STDIO_H__
#define __LHH_STDIO_H__

#include "semphr.h"

typedef struct _MemHandle {
	uint8_t *addr;              /**< memory base address            */
	uint32_t mem_count;         /**< memory count                   */
	uint32_t mem_len;           /**< actual memory length in bytes  */
	uint32_t avail;             /**< avail memory in buff           */
	uint32_t wp;                /**< write point                    */
	uint32_t rp;                /**< read point                     */
	uint32_t pause;             /**< read pause                     */
	SemaphoreHandle_t d_mutex;  /**< opt data mutex                 */
	SemaphoreHandle_t p_sem;    /**< pause sem                      */
	SemaphoreHandle_t r_sem;    /**< wait stack free                */
	SemaphoreHandle_t w_sem;    /**< flag stack is free             */
} MemHandle;

extern int mrb_open(MemHandle *mp, uint32_t period_size, uint32_t period_count);
extern void mrb_close(MemHandle *mp);
/**
 * mrb_write_data and mrb_write_finish must be used in pairs in the same thread
*/
extern void *mrb_write_data(MemHandle *mp);
extern int mrb_write_finish(MemHandle *mp);
/**
 * mrb_read_data and mrb_read_finish must be used in pairs in the same thread
*/
extern void *mrb_read_data(MemHandle *mp);
extern int mrb_read_finish(MemHandle *mp);
extern int mrb_isEmpty(MemHandle *mp);

extern void *mrb_insert_data_lock(MemHandle *mp);
extern void mrb_insert_data_unlock(MemHandle *mp);
extern void mrb_pause(MemHandle *mp);
extern void mrb_continue(MemHandle *mp);
extern void mrb_reset(MemHandle *mp);

#endif
/******************************************** end of file ********************************************/
