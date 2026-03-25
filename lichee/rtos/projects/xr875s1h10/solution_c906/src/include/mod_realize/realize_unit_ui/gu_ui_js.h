/*********************************************************************************
  *Copyright(C),2014-2022,GuRobot
  *Author:  weigang
  *Date:  2022.07.10
  *Description:  ui层 调用JS
  *History:  
**********************************************************************************/
#ifndef GU_JS_H
#define GU_JS_H
#include "./base/gu_list.h"
#include "gu_page.h"
#include "gu_app.h"
#include "../../components/gu_json/gu_json.h"

//使用LV_EVENT处理js的的event，解决JS线程不安全问题
#ifdef __cplusplus
extern "C" {
#endif
void getFuncState(const char *pageName, const char *funcName, guList_t *paraList, char *func_buff);
void guRunScriptFunc(const char *funcName, guList_t *paraList, guPage_t *curPage, bool isRunWithQueue);
void runAppOrPageScript(const char *pageOrAppName, const char *function, const char *para, long page_id, bool isRunWithQueue);
void deletePageScript(const char *pageName, bool isRunWithQueue);
void deleteAppScript(guAPP_t *curApp);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*GU_LIST_H*/