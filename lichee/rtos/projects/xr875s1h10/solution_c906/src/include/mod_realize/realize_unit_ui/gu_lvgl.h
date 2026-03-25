/*********************************************************************************
  *Copyright(C),2014-2022,GuRobot
  *Author:  weigang
  *Date:  2022.07.16
  *Description:  lvgl锁
  *History:
**********************************************************************************/

#ifndef GU_LVGL_H
#define GU_LVGL_H

#include <stdbool.h>


void init_lv_Mutex(void);
void _lv_lock(const char* name, int line);
void _lv_unlock(const char* name, int line);
bool get_lock_status(void);

#define lv_lock() _lv_lock(__func__, __LINE__)
#define lv_unlock() _lv_unlock(__func__, __LINE__)

#endif /*GU_CSS_H*/