/*********************************************************************************
  *Copyright(C),2014-2022,GuRobot
  *Author:  weigang
  *Date:  2022.02.26
  *Description:  DOM 创建和属性设置函数对应表
  * 以下代码是通过python生成的，不要直接修改，
  * 修改 engine_data对应的xml文件，然后运行python3 gen_code.py生成
  *History:  
**********************************************************************************/
#ifndef GU_DOM_WIDGET_MAP_H
#define GU_DOM_WIDGET_MAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gu_func_point_refact.h"

//自定义事件, 图片下载状态变化
#define GU_EVENT_IMG_HTTP_STATUS_CHANGED _LV_EVENT_LAST+1
#define GU_EVENT_FOCUS_CHANGED _LV_EVENT_LAST+2

#define MAX_DOM_COUNT {{MAX_DOM_COUNT}}
#define MAX_DOM_ATTR_COUNT {{MAX_DOM_ATTR_COUNT}}
#define MAX_CSS_STYLE_MAP_COUNT {{MAX_CSS_STYLE_MAP_COUNT}}
#define MAX_ATTR_VALUE_COUNT {{MAX_ATTR_VALUE_COUNT}}

{{DOM_TYPE_INDEX_DEFINE}}
{{DOM_ATTR_TYPE_INDEX_DEFINE}}
{{CSS_ATTR_TYPE_INDEX_DEFINE}}

{{ATTR_VALUE_MAP_H}}

typedef struct {
    char *name;
    int index;
} guAttrLVValue_t;

typedef struct {
    char *name;
    guAttrLVValue_t *map;
    int count;
} guAttrLVMap_t;

extern guFuncPoint_t gCssStyleSetFuncMap[MAX_CSS_STYLE_MAP_COUNT];
extern guFuncPoint_t gLocalStyleSetFuncMap[MAX_CSS_STYLE_MAP_COUNT];
extern guFuncPoint_t gDomCreateFuncMap[MAX_DOM_COUNT];
extern guFuncPoint_t gDomSetAttrFuncMap[MAX_DOM_ATTR_COUNT];
extern guFuncPoint_t gDomGetAttrFuncMap[MAX_DOM_ATTR_COUNT];

extern int getDomTypeIndex(const char *domname);
extern int getDomAttrIndex(int DomTypeIndex, char *attrname);
extern int getStyleTypeIndex(const char *name);
extern char *getAttrValueNameByIndex(int DomTypeIndex, int AttrValueIndex);
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*GU_CSS_H*/
