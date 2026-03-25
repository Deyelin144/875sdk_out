/*********************************************************************************
  *Copyright(C),2014-2022,GuRobot
  *Author:  weigang
  *Date:  2022.06.22
  *Description:  APP管理
  *History:
**********************************************************************************/
#include "./base/gu_list.h"
#include "gu_attr_type.h"
#include "gu_page.h"
#include "gu_utils.h"
#include "gu_html_lv_map.h"
#include "../realize_unit_mem/realize_unit_mem_statistics.h"
#include "../../platform/gulitesf_config.h"

#ifndef GU_APP_H
#define GU_APP_H

#define MAX_APP_NAME_LEN 30

typedef enum {
    PAGE_LOAD_JS_TYPE_JS_ENG = 0,     //1. 在C语言的引擎不调用add_watch, onload, delete 等JS代码，这些代码由JS引擎完成 2. 根据传入页面参数决定是否加载页面JS
    PAGE_LOAD_JS_TYPE_C_ENG = 1       //在C语言的引擎加载JS代码，包括 add_watch, onload, delete
}guLoadJSType_t;

typedef struct guAPP_stru {
    guHash_t* supportFont;    //APP支持的字体
    guHash_t* allImgSrc;      //APP所有图片内存,如果为NULL，表示APP不缓存图片;
    guLoadJSType_t load_js_type;  //    
    guPage_t* currentPage;    //当前页面
    guList_t* defaultDomCss;  //dom默认的css
    guJSON* resouce;          //资源文件 多语言支持
    guJSON* globalResouce;    //全局资源文件
    char locale [3];           //多语言支持 语言
    char rootPath [APP_MAX_ROOT_PATH_LEN]; //APP的根路径
    guHash_t* sysEvent;      //APP event
    guHash_t* codeCache;     //代码缓存,每个页面保存
    char name [MAX_APP_NAME_LEN];  //APPname      
    bool isLaunchAPP;           //是否是启动APP         
    guMemStatis_t memStatis;    //内存统计    
}guAPP_t;


void guInitApp(const char* appRootPath, char* uuid, exitApp_t funcExit);
guAPP_t* getCurrApp(void);
void suspendAPP();
void activeAPP(const char* appname);
void change_default_locale(char* locale);
void destoryCurAPP(void);
guPage_t* getCurrentPage(void);
void cleanAppImgCache(char* path);
guHash_t* getGlobalImgSrc(void);
//fontName是lite APP里css 里字体名称
void guAddAppFont(const char* fontName, const lv_font_t* font);
void guAddMemImg(const char* mem_name, const void* img_addr);
void *guGetMemImg(const char* mem_name);
lv_font_t* guGetAppFont(const char* fontName);
void guAddAppEventHandle(const char* eventHandleName, void* eventHandleFunc);
void *guGetAPPEventHandle(const char* eventHandleName);  
void clear_last_app_resouce();
void loadAppJs(char* rootPath, char* appName, bool is_send_by_queue);

void LoadInFirstPage(void);


#endif /*GU_CSS_H*/