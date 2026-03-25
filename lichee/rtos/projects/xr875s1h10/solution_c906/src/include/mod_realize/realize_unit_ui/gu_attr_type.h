/*********************************************************************************
  *Copyright(C),2014-2022,GuRobot
  *Author:  weigang
  *Date:  2022.02.12
  *Description: lvgl的数据类别处理
  *     将dom和class的属性的数据类别转成 lvgl的类别
  *History:  
**********************************************************************************/

#ifndef GU_ATTR_TYPE_H
#define GU_ATTR_TYPE_H

#include "lvgl.h"
#include "./base/gu_list.h"
#include "./base/gu_hashMap.h"
#include "./base/gu_tree.h"
#include "gu_css.h"
#include "../../components/gu_json/gu_json.h"
#ifdef __cplusplus
extern "C" {
#endif

//属性值类别
typedef enum {
    ATTR_VALUE_TYPE_INTEGER=0, //整数
    ATTR_VALUE_TYPE_STRING=1,  //字符串
    ATTR_VALUE_TYPE_CLASS=2, //lv style dom属性
    ATTR_VALUE_TYPE_V_BIND=3,  //只是绑定数据 dom属性 
    ATTR_VALUE_TYPE_V_EVENT=4, //事件处理 dom属性
    ATTR_VALUE_TYPE_COLOR=5, //颜色 （只用于css）
    ATTR_VALUE_TYPE_FONT = 6, //字体 （只用于css）      
    // ATTR_VALUE_TYPE_DOM_ID = 7, //dom的ID 
    ATTR_VALUE_TYPE_ALIGN_TO = 8, //align_to属性的值  
    ATTR_VALUE_TYPE_ALIGN_POSITION = 9, //object 位置        
    ATTR_VALUE_TYPE_POSITION = 10, //object 位置      
    ATTR_VALUE_TYPE_JSON = 11,
    ATTR_VALUE_TYPE_IMG_SRC = 12,  //IMG的图片源
    ATTR_VALUE_TYPE_GRID_DSC_ARR = 13, //GRID layout表示行列的数组
    ATTR_VALUE_TYPE_GRIDNAV_FLAG = 14, //GRID GRIDNAV 
    ATTR_VALUE_TYPE_GRIDNAV_FOCUS = 15, //focus
    ATTR_VALUE_TYPE_LINE_POINTS = 16, // 线的点列表
    ATTR_VALUE_TYPE_UNIT = 17, // 单位
    ATTR_VALUE_TYPE_STRING_COPY = 18, //复制字符串，只用于 accepted_chars 
    ATTR_VALUE_TYPE_JS_EVENT=19, //事件类别，只用于JS生成的DOM事件处理， 
    ATTR_VALUE_TYPE_CANVAS_BUFF=20,    
}guAttrValueType_t;

typedef enum {
    IMG_SOURCE_TYPE_HTTP,
    IMG_SOURCE_TYPE_DATA,
    IMG_SOURCE_TYPE_FILE,
    IMG_SOURCE_TYPE_MEM,
    IMG_SOURCE_TYPE_UNKNOW
}guImgSourceType_t;
typedef struct alignto_stu{
  void *lv_widget;
  int align;
  int x_ofs;
  int y_ofs;
}guAttrValueTypeAlignTo_t;

typedef unsigned char guDomType_t;

//描述一个DOM
typedef struct gu_dom
{
    guTreeLeaf_t *owerTreeLeaf;  //所属TreeLeaf
    guCssClass_t *default_style; //默认的style
    lv_obj_t *lv_widget; //lv_widget对象 
    guList_t *AttrList;  //属性列表的第一个节点 没有class 属性
    char* v_if_bindPath; //该dom是否绑定了v_if
    void *vforDatabind;  //当前dom是否为vfor节点, 绑定对象  guDataBind_t类别
    guDomType_t domTypeIndex;    
    char is_gridnav;     //是否是gridnav
    char isCloneNode;    //是否是克隆节点
    char isIDAttr;       //是否有ID属性          
    char isVirtualList;  //是否是虚拟列表  
}guDom_t;

typedef uint32_t guDomAttrType_t; // 前16位是style的index,后16位是属性的index，style的第一位是1,表示是style

typedef struct attrValue{
    void *pValue; //属性值 
    char *bindPath; //type = ATTR_VALUE_TYPE_V_BIND bindPath为路径
    int intValue;
    guAttrValueType_t type; //属性值类别; 
}guAttrValue_t;



typedef struct gu_dom_attr_s{
  guDomAttrType_t attrIndex; //属性ID  前16位是style的index,后16位是属性的index，style的第一位是1,表示是style
  guAttrValue_t *attrValue; //属性值
  // guList_t *owerAttrList; //
}guDomAttr_t;

typedef struct imgsrc_str{
  guDom_t *dom;
  char *source_path;
  void *dest_path;
  void *pContent;
  void *ThreadHandle;
  void *Destoryhandle;
  long curPageId; //申请图片的页面ID，用于http请求图片，请求到图片后，如果页面ID发生变化，就不调用设置了lv_image_src
  char isCached;
  guHash_t *allImgSrc;
  guList_t *others_dom_list; //绑定了imgsrc对象的其他dom列表
}guAttrValueTypeImgSrc_t;

typedef struct postion_stu{
  int x;
  int y;
}guAttrValueTypePostion_t;

typedef enum{
    guImgRequestTaskStatus_Idle = 0,
    guImgRequestTaskStatus_Requesting = 1,
    guImgRequestTaskStatus_RequestSuccess = 2,
    guImgRequestTaskStatus_RequestFail = 3,
    guImgRequestTaskStatus_RequestCancel = 4,
}guImgRequestTaskStatus;

typedef struct{
    guImgRequestTaskStatus status;
    guAttrValueTypeImgSrc_t *img_src;
}guImgRequestTask_t;

typedef struct{
    long create_time;   //创建时间
    long last_access_time;  //最后访问时间    
    int size;   //缓存文件大小
    int times;  //使用次数
}guImgDiskCacheInfo_t;

typedef struct{
    int total_size;     //总大小
    int total_block_size;//总块大小
    int count;//缓存个数
    guHash_t *img_disk_hash;    //缓存hash，存放的数据格式为 guImgDiskCacheInfo_t
}guImgDiskCache_t;



typedef enum{
    EVENT_PARA_SOURCE = 0, //原始数据
    EVENT_PARA_BIND = 1, //需要绑定的数据
    EVENT_PARA_KEYCODE = 2, //对与KEY事件生效
    EVENT_PARA_IMG_HTTP_STATUS_CODE = 3, //IMAGE http  
    EVENT_PARA_GESTURE = 4, //对与GESTURE事件生效    
    EVENT_PARA_VALUE_CHANGE = 5,
    EVENT_PARA_PRESS_X = 6,
    EVENT_PARA_PRESS_Y = 7
}guEventParaType_t;

typedef struct eventPara_t{
    char *para; //参数
    char *bindPath; //如果是绑定参数 
    guEventParaType_t EventParatype;
}guEventPara_t;

typedef struct _guEventFunc_t
{
    void *page; //所属页面
    char *funcname; //函数名
    guList_t *paraList; //参数列表
}guEventFunc_t;


typedef struct triggerDomAttr{
    char *dom_id;
    char *attrName;
    char *valueName;
    int attrIndex;
    guDom_t *dom;
    bool isAnim;
    int animaStartValue;
    int animaStopValue;    
    bool isStyle;
    int styleSelector;
}triggerDomAttr_t;
typedef struct triggerAnimation{
    int time;
    void *path;
}triggerAnimation_t;

typedef struct _guDomPropChangedTrigger
{
    guDom_t *trigger_dom;
    lv_event_code_t event_code;
    triggerAnimation_t animation;
    lv_anim_t *lv_anim;
    int source_dom_count;
    triggerDomAttr_t *source_dom_attr;
    int dest_dom_count;
    triggerDomAttr_t *dest_dom_attr;
    guJSON *data;
}guDomPropChangedTrigger_t;

int getAttrOrLocalStyleIndex(guDomAttrType_t domAttrTypeIndex, bool *isStyle);
guAttrValue_t * cloneAttrValue(guAttrValue_t *s);
guAttrValue_t * newAttrValue(guAttrValueType_t type, int value, void *pValue);
void getAttrValueString(guAttrValue_t *s, char *buf);
void destoryAttrValue(guAttrValue_t *s);
void getImgSrcPath(char *sourcePath, void *img_data, int size, char *path);
void destoryImgSrc(guAttrValueTypeImgSrc_t *imgsrc );
guEventFunc_t *getEventPara(const char *para);
guList_t *getParaListFromString(const char *para);
void printEventPara(guList_t *paraList);
void printEventFunction(guEventFunc_t *EvntFunc);
guEventPara_t *newEventPara(char *para, guEventParaType_t EventParatype);
void destoryEventPara(guEventPara_t *p);
void waiting_destory_img(guAttrValueTypeImgSrc_t *img_src_value);
guImgSourceType_t getImgSourceType(const char *source);
//所有类别convert 2个使用场景 主要是 guDom类的setWidgetHtmlAttr， getDomAttrValueByHtmlStr方法调用
// 1. PC 端 xml->转json时使用
// 2. 设备端，如果一个dom的属性绑定了Json变量，设置json变量时，可以使用html的值设置 例如设置 width 直接设置成 80%字符串
//      设备端拿到json的值后，还需要手动转成 guAttrValue_t * 大部分不需要特别处理，少部分需要
//      例如：class align_to
// 3. 如果一个属性值时css的值，在设备端的代码不会使用 convert
// 
guAttrValue_t *UNIT_convert(const char *source);
guAttrValue_t *ALIGN_convert(const char *source);
guAttrValue_t *FLEX_ALIGN_convert(const char *source);
guAttrValue_t *FLEX_FLOW_convert(const char *source);
guAttrValue_t *LAYOUT_convert(const char *source);
guAttrValue_t *STYLE_convert(const void *source);
guAttrValue_t *STRING_convert(const char *source);
guAttrValue_t *STRING_COPY_convert(const char *source);
guAttrValue_t *EVENT_convert(const void *source);
guAttrValue_t *COLOR_convert(const void *source);
guAttrValue_t *LABEL_LONG_MODE_convert(const void *source);
guAttrValue_t *FONT_convert(const void *source);

guAttrValue_t *GRAD_DIR_convert(const void *source);
guAttrValue_t *BG_GRAD_convert(const void *source);
guAttrValue_t *DITHER_convert(const void *source);
guAttrValue_t *BOOL_convert(const void *source);
guAttrValue_t *IMG_SRC_convert(const void *source);
guAttrValue_t *TEXT_DECOR_convert(const void *source);
guAttrValue_t *TEXT_ALIGN_convert(const void *source);
guAttrValue_t *BLEND_MODE_convert(const void *source);
guAttrValue_t *INTEGER_convert(const void *source);
guAttrValue_t *ALIGN_TO_convert(const void *source);
guAttrValue_t *OBJ_FLAG_convert(const void *source);
guAttrValue_t *SIZE_MODE_convert(const void *source);
guAttrValue_t *JSON_convert(void *source);
guAttrValue_t *DOM_ID_convert(const void *source);
guAttrValue_t *ALIGN_POSITION_convert(const void *source);
guAttrValue_t *POSITION_convert(const void *source);
guAttrValue_t *SCROLLBAR_MODE_convert(const void *source);
guAttrValue_t *BASE_DIR_convert(const void *source);
guAttrValue_t *STATE_convert(const void *source);
guAttrValue_t *GRID_DSC_ARR_convert(const void *source);
guAttrValue_t *SLIDER_MODE_convert(const void *source);
guAttrValue_t *GRIDNAV_FLAG_convert(const void *source);
guAttrValue_t *GRIDNAV_FOCUS_convert(const void *source);
guAttrValue_t *LINE_POINTS_convert(const void *source);
guAttrValue_t *BORDER_SIDE_convert(const void *source);
guAttrValue_t *DIR_convert(const void *source);
guAttrValue_t *ARC_MODE_convert(const void *source);
guAttrValue_t *STYLE_PART_convert(const void *source);
guAttrValue_t *SPAN_MODE_convert(const void *source);
guAttrValue_t *SPAN_OVERFLOW_convert(const void *source);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*GU_CSS_H*/