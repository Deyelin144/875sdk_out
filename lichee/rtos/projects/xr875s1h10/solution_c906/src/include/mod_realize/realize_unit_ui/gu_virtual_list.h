/*********************************************************************************
  *Copyright(C),2014-2024,GuRobot
  *Author:  weigang
  *Date:  2024.05.02
  *Description: 虚拟列表实现
  *History:  
**********************************************************************************/
#ifndef GU_VIRTUAL_LIST_H
#define GU_VIRTUAL_LIST_H
#include "lvgl.h"
#include "./base/gu_hashMap.h"
#include "./base/gu_list.h"
#include "gu_data_bind.h"

typedef struct last_postion
{
  int set_y;
  int last_focused_index;  
  int last_focused_offset;  
  int scroll_top;
}virtual_list_last_postion_t;

typedef struct gu_virtal_list_stru {
    lv_obj_t * list_container;    
    guDom_t * containerDom;
    lv_obj_t * list_phantom;
    lv_obj_t * list_body;  
    bool is_focused;
    // guJSON *list_all;  
    guDataBind_t *bindData;
    guJSON *focus_item_data;
    char path[100];
    char show_list_path[100];
    int event_duration; //事件防抖,默认100ms
    int screen_count;
    int buffer_size;
    int last_startIndex;
    int total_count;
    int item_size;
    int container_height;
    int total_size;
    int last_focused_index;
    int last_item_fouced_index;
    int user_focused_index; //用户指定的焦点
    // int max_start_offset;
    int max_start_index;
    int last_focused_offset;
    struct last_postion postion;
}gu_virtual_list_t;


// void set_virtual_list_data_by_script(guHash_t *bindMap, guJSON *data, char *path,guJSON *list, int screen_count, char *focused_item_data, char *init_position, guList_t *cssList, guHash_t *domIdMap, guHash_t *allImgSrc);
void updVirtualBindData(guHash_t *bindMap, guDataBind_t *bindData, const char *path, guJSON *data, guList_t *cssList, guHash_t *domIdMap, guHash_t *allImgSrc);
void deinit_virtual_list();
bool is_virtual_list_path(const char *path);
bool isVforBindToVirtualList(guDom_t *dom);
gu_virtual_list_t *get_virtual_list(guDom_t *dom);
bool isExistVirtualList();
void destory_virtual_list_dom(guDom_t *dom);
#endif
