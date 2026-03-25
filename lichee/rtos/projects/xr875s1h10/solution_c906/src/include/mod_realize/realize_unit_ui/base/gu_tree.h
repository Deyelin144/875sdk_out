/*********************************************************************************
  *Copyright(C),2014-2022,GuRobot
  *Author:  weigang
  *Date:  2022.02.21
  *Description:  tree 管理
  *History:  
**********************************************************************************/
#include "gu_list.h"
#ifndef GU_TREE_H
#define GU_TREE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct gu_tree_leaf
{
    struct gu_tree_leaf *pParent; //如果为NULL,表示根结点
    void *pContent; //结点的实际数据
    guListItem_t *owerListItem; 
    guList_t *pListChildLayerleaf;//子叶子列表
}guTreeLeaf_t;


guTreeLeaf_t *guTreeClone(guTreeLeaf_t *rootLeaf, void*CloneFunc,void*setOwnLeafFunc, void *userdata, void*afterCloneFunc);
//给加入一个结点，从树的左边开始加
guTreeLeaf_t * guTreeInsertLeaf(guTreeLeaf_t *parentLeaf, void *pContent);
//删除一个节点，要遍历所有子节点都删除，从叶子开始删
void guDropLeaf(guTreeLeaf_t *deleteLeaf,void*ContentDeteleFunc, void *userData);
//开始遍历一颗树，采用深度优先的算法
void *guTreeStartWalkGetContext(guTreeLeaf_t *parentLeaf, guList_t *stackList);
//遍历一颗树，获得下一个结点，采用深度优先的算法
void *guTreeWalkNextGetContext(guList_t *stackList);
//设置一个结点点父节点
void setParent(guTreeLeaf_t *curLeaf, guTreeLeaf_t *parentLeaf);

void guDebugPrintTree(guTreeLeaf_t *parentLeaf, void *printfFun);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*GU_CSS_H*/