#ifndef DUIKIT_COMMON_H
#define DUIKIT_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>

#ifdef DUIKIT_USE_ASSERT
#define DUIKIT_ASSERT(expr) \
    do { \
        if (! (expr)) printf ("error at file %s & line %u " #expr "\n", __FILE__, __LINE__); \
    } while (0)
#else
#include <assert.h>
#define DUIKIT_ASSERT(expr) assert(expr);
#endif

#define DUIKIT_WEAK __attribute__((weak))

//https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/Variable-Attributes.html#Variable-Attributes
#define DUIKIT_ALIGNED(x) __attribute__((aligned(x)))
#define DUIKIT_PACK __attribute__((__packed__))

//https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/Function-Attributes.html
#define DUIKIT_DEPRECATED __attribute__((deprecated))

//https://cplusplus.com/reference/cstddef/offsetof
#ifndef offsetof
#define offsetof(type, member) (size_t)&(((type*)0)->member)
#endif

#ifndef container_of
#define container_of(ptr, type, member) ({ \
     const typeof(((type *)0)->member) *__mptr = (ptr); \
     (type *)((char *)__mptr - offsetof(type,member));}) 
#endif

/*
 * 根据结构体字段的地址获取结构体首地址
 */
#ifndef duikit_container_of
#define duikit_container_of(ptr, type, member) \
	((type *)((char *)(ptr) - (char *)(&((type *)0)->member)))
#endif

#ifndef DUIKIT_EXPORT
#ifdef __GNUC__
#define DUIKIT_EXPORT __attribute ((visibility("default")))
#else
#define DUIKIT_EXPORT
#endif
#endif

#define DUIKIT_MIN(x, y) ({\
            const typeof(x) _x = (x); \
            const typeof(y) _y = (y); \
            (void) (&_x == &_y); \
            _x < _y ? _x : _y; \
        })
#define DUIKIT_MAX(x, y) DUIKIT_MIN(y, x)

#define DUIKIT_ALIGN_UP(size, align) (((size) + (align) - 1) & ~((align) - 1))
#define DUIKIT_ALIGN_DOWN(size, align) ((size) & ~((align) - 1))

typedef enum {
    DUIKIT_ERR_MIN = -7,
    //空间不足
    DUIKIT_ERR_NOSPACE = -6,
    //操作禁止
    DUIKIT_ERR_PROHIBIT = -5,
    //接口未实现
    DUIKIT_ERR_NOSYS = -4,
    //参数非法
    DUIKIT_ERR_INVAL = -3,
    //内存不足
    DUIKIT_ERR_NOMEM = -2,
    //异常错误
    DUIKIT_ERR_ERROR = -1,
    //OK
    DUIKIT_ERR_OK = 0,
    //超时
    DUIKIT_ERR_TIMEOUT,
    //终止
    DUIKIT_ERR_ABORT,
    //空
    DUIKIT_ERR_EMPTY,
    //满
    DUIKIT_ERR_FULL,
    //完成
    DUIKIT_ERR_FINISHED,
    DUIKIT_ERR_MAX,
} duikit_err_t;

/*
 * 多线程间通信用的一些结构内部会记录状态信息
 * 状态以组合方式存在
 * 例如
 *	DUIKIT_STATE_RUNNING | DUIKIT_STATE_WAIT_READABLE 代表当前在运行但是不可读
 *	DUIKIT_STATE_RUNNING | DUIKIT_STATE_ABORT 代表当前在运行并且将要终止
 *	DUIKIT_STATE_RUNNING | DUIKIT_STATE_WAIT_READABLE | DUIKIT_STATE_ABORT 代表当前在运行但是不可读并且将要终止
 */
typedef enum {
    DUIKIT_STATE_IDLE = 0,
    DUIKIT_STATE_RUNNING = 1 << 0,
    DUIKIT_STATE_WAIT_READABLE = 1 << 1,
    DUIKIT_STATE_WAIT_WRITEABLE = 1 << 2,
    DUIKIT_STATE_ABORT = 1 << 3,
    DUIKIT_STATE_FINISHED = 1 << 4,
} duikit_state_t;

#ifdef CONFIG_ADS_COMPONENT_COMMON_MEMERY_MONITOR
#include "as_mem_monitor.h"
#define duikit_malloc(size) as_mem_malloc(size)
#define duikit_free(ptr) as_mem_free(ptr)
#define duikit_calloc(num, size) ({ \
    size_t _n = (num); \
    size_t _s = (size); \
    void *_p = duikit_malloc(_n * _s); \
    if (_p) { \
        memset(_p, 0, _n * _s); \
    } \
    _p; \
})
#define duikit_realloc(ptr, size) as_mem_realloc(ptr, size)
#else
/**
 * @brief 内存管理函数族
 */
typedef struct {
    void *(*malloc)(size_t size);
    void (*free)(void *ptr);
    void *(*calloc)(size_t nmemb, size_t size);
    void *(*realloc)(void *ptr, size_t size);
} duikit_alloc_cfg_t;

/**
 * @brief 替换DUIKIT内部使用的内存管理函数族
 *
 * @note 系统默认使用c库的malloc/calloc/realloc/free, 如需修改，请在使用DUIKIT其他接口之前调用此接口替换
 *
 * @param cfg 内存管理函数族
 */
DUIKIT_EXPORT duikit_err_t duikit_alloc_config(const duikit_alloc_cfg_t *cfg);

/*
 * DUIKIT内部统一使用下列内存管理接口
 */
DUIKIT_EXPORT void *duikit_malloc(size_t size);
DUIKIT_EXPORT void *duikit_calloc(size_t nmemb, size_t size);
DUIKIT_EXPORT void *duikit_realloc(void *ptr, size_t size);
DUIKIT_EXPORT void duikit_free(void *ptr);
#endif

/* Single Linked list management macros
 *
 * e.g.
 * struct foo {
 *   struct foo *next;
 *   int dummy;
 * };
 *
 * struct foo *header = NULL;
 * struct foo *foo = io_calloc(1, sizeof(*foo));
 * DUIKIT_LIST_ADD_HEAD(struct foo, &heder, foo);
 * DUIKIT_LIST_ADD_TAIL(struct foo, &heder, foo);
 * DUIKIT_LIST_DELETE(struct foo, &header, foo);
 */

#define DUIKIT_LIST_ADD_HEAD(type_, head_, elem_) \
  do {                                     \
    (elem_)->next = (*head_);              \
    *(head_) = (elem_);                    \
  } while (0)

#define DUIKIT_LIST_ADD_TAIL(type_, head_, elem_) \
  do {                                     \
    type_ **h = head_;                     \
    while (*h != NULL) h = &(*h)->next;    \
    *h = (elem_);                          \
  } while (0)

#define DUIKIT_LIST_DELETE(type_, head_, elem_)   \
  do {                                     \
    type_ **h = head_;                     \
    while (*h != (elem_)) h = &(*h)->next; \
    *h = (elem_)->next;                    \
  } while (0)

#ifdef __cplusplus
}
#endif

#endif
