/*********************************************************************************
  *Copyright(C),2014-2022,GuRobot
  *Author:  weigang
  *Date:  2022.02.12
  *Description:  html CSS 的存储结构
  *History:  
**********************************************************************************/
#include "./base/gu_list.h"
#include "gu_attr_type.h"
#include "gu_html_lv_map.h"
#include "gu_func_point_refact.h"
#ifndef GU_CSS_H
#define GU_CSS_H

#ifdef __cplusplus
extern "C" {
#endif

//描述一个cssClass
typedef struct guCssClass
{
    char *name; //css名称
    lv_style_t *plvStyle; //lvgl对应的style     
    lv_style_selector_t selector; //
    struct guCssClass *next; //
}guCssClass_t;

typedef int cssAttrType_t;
guCssClass_t * newCssClass( guList_t *cssList, const char *name);
int destory_css_list(struct gu_list *css_list);
// void setCssAttr(lv_style_t *lv_style,  cssAttrType_t cssTypeIndex, char *value);
guCssClass_t *get_css_by_name(struct gu_list *css_list, const char *name);
guFuncPoint_t *get_lv_set_fun(cssAttrType_t cssAttrType);
int get_style_selector(char *selectorstr);
int call_set_css_attr(void *lv_style, cssAttrType_t cssAttrType, ...);
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*GU_CSS_H*/