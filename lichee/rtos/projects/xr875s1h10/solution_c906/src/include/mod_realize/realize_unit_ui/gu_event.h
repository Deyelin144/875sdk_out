/*********************************************************************************
  *Copyright(C),2014-2022,GuRobot
  *Author:  weigang
  *Date:  2022.03.06
  *Description:  event 处理
  *History:  
**********************************************************************************/
#include "./base/gu_list.h"
#include "gu_dom.h"

#ifndef GU_EVENT_H
#define GU_EVENT_H

#ifdef __cplusplus
extern "C" {
#endif

void guEventHandle(lv_event_t * event);
void guEventHandleForJS(lv_event_t *event);
void guEvent_bind_get(lv_event_t * event);
void guEvent_set_event_duration(int duration);
void guEvent_set_scroll_end_event_duration(int duration);
void gu_event_set_scrolling(bool is_scrolling);
bool gu_event_is_scrolling();
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*GU_CSS_H*/