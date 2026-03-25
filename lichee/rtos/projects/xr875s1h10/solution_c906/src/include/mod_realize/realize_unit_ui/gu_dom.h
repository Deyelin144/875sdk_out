/*********************************************************************************
  *Copyright(C),2014-2022,GuRobot
  *Author:  weigang
  *Date:  2022.02.12
  *Description:  html DOM的存储结构
  *History:  
**********************************************************************************/
#include "gu_css.h"
#include "./base/gu_list.h"
#include "../../components/gu_json/gu_json.h"
#include "gu_data_bind.h"
#include "./base/gu_tree.h"
#include "lv_qrcode.h"
#include "./base/gu_hashMap.h"
#include "gu_attr_type.h"

#ifndef GU_DOM_H
#define GU_DOM_H

#ifdef __cplusplus
extern "C" {
#endif

guDom_t *create_dom_from_layout(guDom_t* parentDom, guJSON *layout);
void setDomOwnLeaf(guDom_t *cur_node, guTreeLeaf_t *leaf);
void setParentDom(guDom_t *cur_node, guDom_t *parent_node);
guDom_t *cloneDom(guDom_t *cur_node, guDom_t *parent_node, void *userdata);
guFuncPoint_t *gu_get_dom_attr_set_method(guDomAttrType_t domAttrTypeIndex);
void printfDom(guDom_t *dom);
char *exportDomTreeXML(guDom_t *dom);
guDom_t* getParentDom(guDom_t *cur_node);
guAttrValue_t *getAttValueByIndex(guDom_t *dom, guDomAttrType_t attrIndex);
guDomAttr_t *addDomAttr(guDom_t *dom, guDomAttrType_t attrIndex, guAttrValue_t *attrValue);
guDom_t* newDom(guDom_t *parentdom, guDomType_t domType, guCssClass_t *default_style, char*vifBindPath, bool isNeedCreateWidget);
void destoryDom(guDom_t **dom, void*page);
void dom_attr_lock();
void dom_attr_unlock();
void delDomLVObject(guDom_t *dom);
void destoryBindGet(guList_t *list_get);
lv_obj_t * createWidget(guDom_t *dom, guDom_t *parentDom);
void hiddenDom(guDom_t *dom);
guAttrValue_t *getAttrValue(guDom_t *dom,int valueint, char *valuestring, guAttrValueType_t valueType, guList_t *css_list, guHash_t* domIdMap, guHash_t*allImgSrc);
int setWidgetAttr(guDom_t *dom, guDomAttrType_t domAttrTypeIndex, guAttrValue_t *attrValue);
char *getDomName(guDomType_t domType);
char *getAttrName(guDomAttrType_t DomAttrType);
int setWidgetHtmlAttr(guDom_t *dom, guDomAttrType_t domAttrTypeIndex,  char *attrValueNam, bool isAddDomAttrList, guList_t *cssList, guHash_t *domIdMap, guHash_t *allImgSrc);
void setWidgetHTMLStyle(guDom_t *dom, char * styleName, char *attrValuestr, char *selectorstr, guHash_t *allImgSrc);
guAttrValueTypeImgSrc_t *getImgSrcValue(guDom_t *dom,  char *value, guHash_t *allImgSrc);
guDom_t *getSysteLayerDom(void);
void createJSDom(guDom_t *dom);
void delJSDom(guDom_t *dom);
bool isJSDomDeleted(guDom_t *dom);
void initJSDomHash(void);

void set_btn_list(guDom_t *dom, guJSON *listData);
void set_bar_max_value(guDom_t *dom, int maxValue);
void set_list_btn_class(lv_obj_t *widget, lv_style_t *sytle, lv_style_selector_t selector );
void set_obj_show(guDom_t *dom, bool b);
void set_obj_center(guDom_t *dom, bool b);
void set_label_symbol(guDom_t *dom, int32_t symbol_unicode);
void set_grid_cell(guDom_t *dom, char *point_list);
void set_gridnav(guDom_t *dom, int flag);
void set_switch_value(guDom_t *dom, bool b);
void set_obj_focus(guDom_t *dom, bool b);
void set_obj_focus_key(guDom_t *dom, bool b);
void set_obj_group_focus(guDom_t *dom, bool b);
void set_slider_max_value(guDom_t *dom, int maxValue);


void gif_restart(guDom_t *dom, bool b);

lv_obj_t * create_virtual_lv_obj(lv_obj_t *parent);
void set_tab_pos(guDom_t *dom, int maxValue);
void set_tab_size(guDom_t *dom, int size);
void show_tab_btns(guDom_t *dom, bool b);
void set_tab_name(guDom_t *dom, char *name);
void tabview_remove_tab(guDom_t *dom);
void set_dom_id(guDom_t *dom, char *id);
void scroll_to_x(guDom_t *dom, int x);
void scroll_to_y(guDom_t *dom, int y);
void set_scroll_anim(guDom_t *dom, bool b);
void tabview_set_act(guDom_t *dom, int index);
void set_scroll_anim_time(guDom_t *dom, char *anim_time);
void set_disable_tabview_scroll(guDom_t *dom, int scroll);

void set_spin_time(guDom_t *dom, int spin_time);
void set_arc_length(guDom_t *dom, int length);

void remove_style(guDom_t *dom, int part);

void set_qr_size(guDom_t *dom, int size);
void set_fg_color(guDom_t *dom, char*color);
void set_bg_color(guDom_t *dom, char*color);
void set_qr_data(guDom_t *dom, char*data);
void gif_pause(guDom_t *dom, bool b);
void label_ins_text(guDom_t *dom, char* text);
void set_canvas_size(guDom_t *dom, int w, int h);
void set_canvas_fill_bg(guDom_t *dom, char* color, int opa);
void set_canvas_set_px(guDom_t *dom, int x, int y, char *color);
void set_disk_cache(guDom_t *dom, bool b);
void set_disk_cache_bin(guDom_t *dom, bool b);

int get_unit_int(const char *source);
bool get_switch_value(lv_obj_t *widget);
int get_slider_value(lv_obj_t *widget);
void set_span_text_color(guDom_t *dom, char *color);
void set_span_text_font(guDom_t *dom, char *font);
void roller_set_selected(guDom_t *dom, int index);
void roller_set_options(guDom_t *dom, char *options);
void set_drag(guDom_t *dom, bool isDrag);
void set_zoom_width(guDom_t *dom, int width);
void set_virtual_list_data(guDom_t *dom, char *data);
void set_virtual_list_position(guDom_t *dom, char *position);
void set_virtual_focused_index(guDom_t *dom, int focused_index);
void set_virtual_list_focused(guDom_t *dom, bool b);
void set_virtual_list_focused_item_data(guDom_t *dom, char *data);
char *get_virtual_list_position(lv_obj_t *widget);
void set_img_is_content_in_mem(guDom_t *dom, bool b);
void set_nav_long_press(guDom_t *dom, bool b);
int get_virtual_focused_index(lv_obj_t *widget);
void roller_set_options_mode(guDom_t *dom, int index);
void set_circular_scroll(guDom_t *dom, bool b);
void create_dom_prop_change_trigger(guDomPropChangedTrigger_t *trigger);
void destroy_dom_prop_change_trigger(guDomPropChangedTrigger_t *trigger);
void set_focus_index(guDom_t *dom, int index);
void set_img_is_load_mem(guDom_t *dom, bool b);
int get_focus_index(lv_obj_t *widget);
void set_obj_bg_color(guDom_t *dom, lv_color_t color);
void set_placeholder_src(guDom_t *dom, char *src);
void set_obj_checked(guDom_t *dom, bool b);
void set_obj_scrolled(guDom_t *dom, bool b);
void set_obj_disabled(guDom_t *dom, bool b);
void set_obj_pressed(guDom_t *dom, bool b);
void set_slider_key_mode(guDom_t *dom, int key_mode);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*GU_DOM_H*/