/*********************************************************************************
  *Copyright(C),2014-2022,GuRobot
  *Author:  weigang
  *Date:  2022.02.19
  *Description:  页面的处理
  *History:  
**********************************************************************************/
#include "../../components/gu_json/gu_json.h"
#include "gu_dom.h"
#include "gu_virtual_list.h"
#ifndef GU_PAGE_H
#define GU_PAGE_H

#ifdef __cplusplus
extern "C" {
#endif

//APP相关文件
#define PAGE_DEFAULT_PAGE_CSS_FILE "defaultcss.json"
#define PAGE_APP_RESOUCE_FILE "resource.json" //资源文件
#define PAGE_APP_SCRIPT_FILE "app_script.js" //全局函数

//页面相关文件
#define PAGE_DEFAULT_DOM_CSS_FILE "style.json"
#define PAGE_DEFAULT_DOM_LESS_FILE "style.less"
#define PAGE_DEFAULT_LAYOUT_FILE "layout.json"
#define PAGE_DEFAULT_SCRIPT_FILE "script.js"
#define PAGE_DEFAULT_DATA_FILE "data.json"
#define APP_DEFAULT_Page "index"
#define APP_MAX_PATH_LEN 256
// #define APP_MAX_NAME_LEN 20
#define APP_MAX_ROOT_PATH_LEN 50
#define DOM_ATTR_TYPE_INDEX_BIND_UNKONW -1
typedef enum {
  PAGE_STATE_NOMAL,
  PAGE_STATE_PAUSE_EVENT //暂停所有事件处理
}guPageStateType_t;

typedef struct gu_page
{
    char *name;           //页面名称 文件目录名      
    long page_id;          //页面唯一ID
    guDom_t *rootDom;     //顶级dom
    guList_t *css_default; //默认样式表
    guList_t *css_list;   //样式表
    guList_t *img_request_list; //图片请求列表    
    guHash_t *bind_data;  //绑定的数据 写数据
    guHash_t *bind_data_get; //绑定的数据  读数据
    guHash_t *domIdMap;   //Dom和ID绑定表
    guHash_t *allImgSrc;  //当前页面的所有图片信息
    guJSON *data;          //数据   
    guHash_t *sys_event;  //系统事件
    guPageStateType_t state; //当前页面的状态
    bool isNeedCache;     //是否需要缓存
}guPage_t;

typedef struct page_stack_content
{
  char *page_name;
  guJSON *page_para;    
}guPageStack_t;

typedef void (*exitApp_t)(void);

guPage_t *guLoadPage(char *page_name, guJSON *page_para);
void destoryPageStack(guPageStack_t *page);
guList_t * load_css(guJSON *css, guHash_t *AllImgScr);
void destoryPage(guPage_t *page, bool isExitApp);
void destoryCurAPP(void);
int LoadPageJSFromFile(char *rootPath, const char* page_name, char** js_script);
long get_current_page_id(void);
bool getLoadingStatus(void);
guJSON *getJsonDataFromFile(const char *page_name, const char *json_file);
void addBindSync(guDataBind_t *bind);
void syncBindListToJS(bool isRunWithQueue);
void stopLoadingPage(guPage_t *page);
void clear_last_page_lv(void);
bool isPageDestorying(void);
void initWaittingImgSrcList(void);
void addWatingImgSrcDeleteList(guAttrValueTypeImgSrc_t *img_src_value);
void deleteWatingImgSrcList(void);
int savePageDatatoCache(char *key);
void setPageCachePath(char *path);
void start_count_time();
void printPageFlushTime(void);
void set_page_mem_leak_monitor(bool is_mem_leak);
void set_page_data_save_js_only(bool is_save_js_only);
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*GU_CSS_H*/