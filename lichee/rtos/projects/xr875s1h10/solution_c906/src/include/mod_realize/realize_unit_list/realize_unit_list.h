#ifndef _REALIZE_UNIT_LIST_H_
#define _REALIZE_UNIT_LIST_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stdbool.h"
#include "../realize_unit_log/realize_unit_log.h"
#include "../realize_unit_mem/realize_unit_mem.h"

#define LIST_ON_DEBUG 1

#if LIST_ON_DEBUG
#define list_log_debug(format, ...)
#define list_log_info(format, ...)
#else
#define list_log_debug realize_unit_log_debug
#define list_log_info realize_unit_log_info
#endif
#define list_log_wran realize_unit_log_warn
#define list_log_error realize_unit_log_error

#define list_malloc realize_unit_mem_malloc
#define list_calloc realize_unit_mem_calloc
#define list_free realize_unit_mem_free

typedef enum {
	// SINGLE_LIST,		//单链表
	DOUBLE_LIST,		//双向链表
	DOUBLE_CIRCULAR_LIST 	//双向循环链表
} list_type_t;

typedef enum {
	LIST_HEAD_INSERT,		//头插
	LIST_TAIL_INSERT,		//尾插
	LIST_SPECIFY_POS_INSERT 	//指定位置插入
} list_insert_pos_t;

//基类链表的结构体
typedef struct node {
	void  *data;//数据域
	struct node *prev;//前驱节点指针
	struct node *next;//后继节点指针
} list_node_t;

typedef struct {
	void (*free_data)(void *u_data, void *data);
} list_cb_t;

typedef struct _list_obj {
	void *ctx;
	list_cb_t cb;
	void *user_data;
	int (*init)(struct _list_obj *obj);

	//节点插入的位置，头插pos = 0，尾插pos = get_node_num + 1，或者pos = n指定位置插入，但n不超过链表长度，否则插在链表最后一个
	//Success returns this node, failure returns NULL
	list_node_t *(*insert_node)(struct _list_obj *obj, int pos, void *data); 
	int (*delete_node)(struct _list_obj *obj, list_node_t *node);

	//指定位置, pos = 0表示移动到头结点后第一个，pos = get_node_num表示移动到最后一个
	int (*move_to_node)(struct _list_obj *obj, list_node_t *node, int pos);
	//要查找的节点的下标，因头节点供对象内部使用，用户存储的节点下标从1开始
	list_node_t *(*search_node)(struct _list_obj *obj, int idx);
	//获取链表节点个数，只包含用户存储的节点
	int (*get_node_num)(struct _list_obj *obj);
	int (*deinit)(struct _list_obj *obj);
} list_obj_t;

list_obj_t *realize_unit_list_new(list_type_t type, list_cb_t *cb, void *user_data);
void realize_unit_list_delete(list_obj_t **obj);

#endif


