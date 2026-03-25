/**
  *****************************************************************************
  * @file    lhh_stdio.h
  * @author  LHH
  * @version V1.0.0
  * @date    2023-11-4
  * @brief   build a special stack for mp4
  *****************************************************************************
  * @history
  *
  * 1. Date:2023-11-4
  *    Author:lhh
  *    Modification:build
  *
  *****************************************************************************
  */
#ifndef __LHH_STDIO_H__
#define __LHH_STDIO_H__

#include "semphr.h"

typedef struct _MemHandle {
    uint8_t *addr;             /**< memory base address            */
    uint32_t mem_count;        /**< memory count                   */
    uint32_t mem_len;          /**< actual memory length in bytes  */
    uint32_t avail;            /**< avail memory in buff           */
    uint32_t wp;               /**< write point                    */
    uint32_t rp;               /**< read point                     */
    SemaphoreHandle_t d_mutex; /**< opt data mutex                 */
    SemaphoreHandle_t r_sem;   /**< wait stack free                */
    SemaphoreHandle_t w_sem;   /**< flag stack is free             */
} MemHandle;

extern int mrb_open(MemHandle *mp, uint32_t period_size, uint32_t period_count);
extern void mrb_close(MemHandle *mp);
/**
 * mrb_write_data and mrb_write_finish must be used in pairs in the same thread
*/
extern void *mrb_write_data(MemHandle *mp);
extern int mrb_write_finish(MemHandle *mp);
extern int mrb_write_drop(MemHandle *mp);
/**
 * mrb_read_data and mrb_read_finish must be used in pairs in the same thread
*/
extern void *mrb_read_data(MemHandle *mp);
extern int mrb_read_finish(MemHandle *mp);
extern int mrb_isEmpty(MemHandle *mp);

#endif
/******************************************** end of file ********************************************/
