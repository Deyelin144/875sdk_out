/*********************************************************************************
  *Copyright(C),2014-2022,GuRobot
  *Author:  weigang
  *Date:  2022.02.21
  *Description:  数据绑定
  *History:  
**********************************************************************************/
#ifndef GU_DATA_BIND_H
#define GU_DATA_BIND_H
#ifdef __cplusplus
} /*extern "C"*/
#endif
#include "gu_dom.h"
#include "./base/gu_hashMap.h"
#include "../../components/gu_json/gu_json.h"

//绑定类别
typedef enum {
    BIND_TYPE_NO_BIND=0,  
    BIND_TYPE_V_FOR=1,
    BIND_TYPE_V_FOR_ITEM=2,    
    BIND_TYPE_V_IF=3,  
    // BIND_TYPE_V_SHOW=4, //用show属性替代
    BIND_TYPE_V_EVENT=5,
    BIND_TYPE_V_DATA=6,
}guBindType_t;



typedef struct guDataBind
{   
    guDom_t *dom;
    char *path; //json的路径 如果是数组某个元素用数组下标表示 ，例如 data.messageList[1]
    void  *vforBind; // 类别是：guVforBind_t
    char *expression; //表达式    
    guDomAttrType_t domAttrTypeIndex; //属性类别 前16位是style的index,后16位是属性的index
    guBindType_t bindType;    
    char isBindGet;  //是否绑定了读属性，
}guDataBind_t;

typedef struct guVforBind
{
  char *itemname; //vfor item 别名
  guDataBind_t *parentVfor; //父Vfor的绑定对象
  char isChildVfor;   //当前的绑定是否为子vfor节点
  char isVirtualList; //是否绑定了虚拟列表
  unsigned short array_size;//绑定数组的长度
}guVforBind_t;

struct domCloneUserData_stru{
  guJSON *allData;
  guJSON *itemData;
  guHash_t *bindMap;
  guDataBind_t *bindData;
  char *curPath;
  guList_t *cssList;
  guHash_t *domIdMap;
  guHash_t *allImgSrc;
};

typedef struct bind_get_stru{
    int domAttrTypeIndex;
    char *path;
    lv_obj_t *bind_obj;
    guJSON* cur_node;
}bind_get_t;

char *getValueByBindData(char *souceValue, guJSON *full_data, guDataBind_t *bindData, guJSON *item);
//添加绑定
guDataBind_t *addDomAttrBind(guHash_t *bindMap, guDom_t *dom, guDomAttrType_t domAttrTypeIndex, guBindType_t bindType, const char *path);
void updOneBindData(guHash_t *bindMap, guDataBind_t *bindData, const char *path, guJSON *data, guJSON *item, guList_t *cssList, guHash_t *domIdMap, guHash_t *allImgSrc);
void upd_bind_get_value(bind_get_t* bind_get, lv_obj_t *obj);
void updateDomVforData(guDom_t *cur_dom, void *userdata);
guTreeLeaf_t * addVForItem(guHash_t *bindMap, guDataBind_t *bindData ,guJSON *data, guJSON *item, guList_t *cssList, guHash_t *domIdMap, guHash_t *allImgSrc, const char *key, int index);
char *vGetforItemPath(const char *path, int index);
void updateVforDom(guHash_t *bindMap, const char *key, guDataBind_t *bindData, guJSON *data, guJSON *list, guList_t *cssList, guHash_t *domIdMap, guHash_t *allImgSrc);
void destoryBind(guDataBind_t *bindData);
//数据更新操作
void updateString(guHash_t *bindMap, guJSON *data,  const char *path, const char*value, lv_obj_t *ignore_lv_obj, guList_t *cssList, guHash_t *domIdMap, guHash_t *allImgSrc);
void updateObject(guHash_t *bindMap, guJSON *fulldata, guJSON *itemData,  const char *path, lv_obj_t *ignore_lv_obj, guList_t *cssList, guHash_t *domIdMap, guHash_t *allImgSrc);
void replaceArray(guHash_t *bindMap, guJSON *data, const char *path, guJSON*list, guList_t *cssList, guHash_t *domIdMap, guHash_t *allImgSrc);
void deleteArrayItem(guHash_t *bindMap, guJSON *data, const char *path, int index);
void updateArrayItem(guHash_t *bindMap, guJSON *data, const char *path, int index, char *value, guList_t *cssList, guHash_t *domIdMap, guHash_t *allImgSrc);


// void pushItemToArray(guDataBind_t *bindData, guJSON *data, const char *path, guJSON*item, guList_t *cssList);
void joinArray(guHash_t *bindMap, guJSON *data, const char *path, guJSON*value,  guList_t *cssList, guHash_t *domIdMap, guHash_t *allImgSrc, bool isReplace);
// void deleteItemOfArray(guDataBind_t *bindData, guJSON *data, const char *path, int index);
void deleteArray(guHash_t *bindMap, guJSON *data,  const char *path, guHash_t *allImgSrc);
void updateItemArray(guDataBind_t *bindData, guJSON *data,  char *path, int index);

void destoryBindData(guList_t *bindList);
void updateBindValue(guHash_t *bindMap, guJSON *data,  const char *path, guJSON *item, lv_obj_t *ignore_lv_obj, guList_t *cssList, guHash_t *domIdMap, guHash_t *allImgSrc);

// guFuncPoint_t * gu_get_dom_attr_get_method(guDomAttrType_t domAttrTypeIndex);
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*GU_DATA_H*/