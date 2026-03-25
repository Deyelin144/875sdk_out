/*********************************************************************************
  *Copyright(C),2014-2022,GuRobot
  *Author:  weigang
  *Date:  2022.02.14
  *Description:  list管理，简单的链表操作
  *History:  
**********************************************************************************/

#ifndef GU_LIST_H
#define GU_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*pFunHandleItem)(void *, void *);

typedef struct guListItem
{
    struct guListItem *pNext; 
    struct guListItem *pPrev; 
    void *pContent; //结点真实的内容
}guListItem_t;

typedef struct gu_list
{
    int count;  //结点数
    // guListItem_t *pxIndex; //用于遍历 用于 guListStartGetNextContent 和 guListGetNextContent
    guListItem_t *pFirstItem; //第一个指针
    guListItem_t *pListEnd;//最后节点的指针
}guList_t;



guList_t *guListInit(void);
guListItem_t * guListInsertEnd(guList_t *list, void *newnode);
void guListDestory(guList_t *list, void *pFunItemContentDestory);
void guListDelItem(guList_t *list, guListItem_t *listItem);
void guListWalk(guList_t *list, pFunHandleItem pFunHandleItem, void *para);
guList_t *guNewStack(void);//新建一个堆栈
guListItem_t *guPushStack(guList_t *stackList, void *Content);
void *guPopStack(guList_t *stackList);
void *guGetTopStackContent(guList_t *stackList);

void *guListStartGetNextContent(guList_t *list, guListItem_t **pxIndex);
void *guListGetNextContent(guList_t *list, guListItem_t **pxIndex); 

void *guListStartGetPrev(guList_t *list, guListItem_t **pxIndex);
void *guListGetPrevContent(guList_t *list, guListItem_t **pxIndex); 

void *guListStartGetNext(guList_t *list, guListItem_t **pxIndex);
void *guListGetNextItem(guList_t *list, guListItem_t **pxIndex);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*GU_LIST_H*/