#ifndef __HEAP_WRAPPER_H__
#define __HEAP_WRAPPER_H__

#include "stdio.h"

////////////////////////////////////////////////////////////////////////////
/*****************************mem api************************************/
////////////////////////////////////////////////////////////////////////////
/**
 * @brief 分配内存块
 * @param size 需要分配的内存块的大小，单位为字节。
 * @return 成功则返回分配的内存块地址；失败则返回NULL
 */
typedef void* (*malloc_t)(size_t size);
/**
 * @brief 重新分配内存块
 * @param rmem	 指向已分配的内存块
 * @param newsize 重新分配的内存大小
 * @return 成功则返回分配的内存块地址；失败则返回NULL
 */
typedef void* (*realloc_t)(void *rmem, size_t newsize);
/**
 * @brief 分配多内存块
 * @param nmemb  分配的空间对象的计数
 * @param size 分配的空间对象的大小
 * @return 成功则指向指向第一个内存块地址的指针，并且所有分配的内存块都被初始化成零；如果失败则返回 NULL。
 */
typedef void* (*calloc_t)(size_t nmemb, size_t size);
/**
 * @brief 释放内存块
 * @param size 待释放的内存块指针
 * @return void
 */
typedef void (*free_t)(void* p);

/**
 * @brief 获取heap的剩余内存大小
 * @param 
 * @return size 剩余内存大小
 */
typedef size_t (*get_free_size_t)(void);

typedef struct {
	malloc_t malloc;
	realloc_t realloc;
	calloc_t calloc;
	free_t free;
	get_free_size_t free_size;
} heap_wrapper_t;

#endif

