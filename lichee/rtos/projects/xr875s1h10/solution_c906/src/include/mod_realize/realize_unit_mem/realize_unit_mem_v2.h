#ifndef __realize_unit_mem_H__
#define __realize_unit_mem_H__

#include <stdint.h>
#include "realize_unit_mem_block_pool.h"
#include <stdio.h>

#if CONFIG_USE_MEM_V2

typedef enum {
    MEM_V2_POOL_TYPE_TLSF = 0,
#ifdef CONFIG_MEM_V2_USE_BLOCK_POOL
    MEM_V2_POOL_TYPE_BLOCK_POOL,
#endif
    MEM_V2_POOL_TYPE_MAX
} mem_pool_type_v2_t;

typedef struct {
	unsigned int mem_size;
} unit_mem_conf_t;

//监控3类内存使用情况
typedef enum {
    MEM_MONITOR_TYPE_ENG = 0,
    MEM_MONITOR_TYPE_JS = 1,
    MEM_MONITOR_TYPE_IMG = 2,
    MEM_MONITOR_TYPE_LVGL = 3,
    MEM_MONITOR_TYPE_MAX,
} guMemMonitorType_t;

typedef struct {
    uint32_t pool_size;
	uint32_t from_size : 28;
    uint32_t pool_type : 4;
	uint32_t to_size;
    void *handle;
	void *mem_pool;
    void *(*malloc)(void *handle, unsigned int size, unsigned int *block_size);
    void *(*realloc)(void *handle, void *ptr, unsigned int size, unsigned int *old_block_size, unsigned int *new_block_size, void *lock_func);
    void *(*align_malloc)(void *handle, unsigned int size, unsigned int *block_size);
    void *(*align_realloc)(void *handle, void *ptr, unsigned int size, unsigned int *old_block_size, unsigned int *new_block_size, void *lock_func);
    void (*free)(void *handle, void *ptr, unsigned int *block_size);
    unsigned int (*get_block_size)(void *handle, void *ptr);
} mem_pool_v2_t;

typedef struct {
    mem_pool_v2_t *pool;
    uint8_t pool_cnt : 6;
    uint8_t is_js_independent : 1;     //是否JS使用独立的内存池 
    uint8_t pool_inited : 1;     //是否已初始化内存池
    mem_block_pool_param_t block_pool;
    void *mutex;
    void *start_addr;
    int mem_use[MEM_MONITOR_TYPE_MAX];
} mem_pool_config_v2_t;

//调试分析使用
typedef struct {
    size_t total_size;      // 总内存
    size_t used_size;       // 已使用内存
    size_t max_free_block;  // 剩余的最大块内存
    size_t fragmentation;    // 碎片化程度
} mem_pool_info_v2_t;


#define realize_unit_mem_malloc(x) realize_unit_mem_malloc_ex(x, __FILE__, __LINE__, MEM_MONITOR_TYPE_ENG, 0)
#define realize_unit_mem_calloc(x, y) realize_unit_mem_calloc_ex(x, y, __FILE__, __LINE__, MEM_MONITOR_TYPE_ENG, 0)
#define realize_unit_mem_free(x) realize_unit_mem_free_ex(x, __FILE__, __LINE__, MEM_MONITOR_TYPE_ENG)
#define realize_unit_mem_realloc(x, y) realize_unit_mem_realloc_ex(x, y, __FILE__, __LINE__, MEM_MONITOR_TYPE_ENG, 0)
#define realize_unit_mem_strdup(x) realize_unit_mem_strdup_ex(x, __FILE__, __LINE__, MEM_MONITOR_TYPE_ENG, 0)

#define realize_unit_mem_align_malloc(x) realize_unit_mem_malloc_ex(x, __FILE__, __LINE__, MEM_MONITOR_TYPE_ENG, 8)
#define realize_unit_mem_align_calloc(x, y) realize_unit_mem_calloc_ex(x, y, __FILE__, __LINE__, MEM_MONITOR_TYPE_ENG, 8)
#define realize_unit_mem_align_realloc(x, y) realize_unit_mem_realloc_ex(x, y, __FILE__, __LINE__, MEM_MONITOR_TYPE_ENG, 8)
#define realize_unit_mem_align_strdup(x) realize_unit_mem_strdup_ex(x, __FILE__, __LINE__, MEM_MONITOR_TYPE_ENG, 8)

/**
 * @brief 设置内存池参数
 *
 * @param config 内存池参数结构体。
 * @return 
 */
void realize_unit_mem_set_pool_config(mem_pool_config_v2_t *config);

mem_pool_config_v2_t *realize_unit_mem_get_config();


int realize_unit_mem_init(void);

/**
 * @brief 分配内存块
 *
 * @param size 需要分配的内存块的大小，单位为字节。
 * @return 成功则返回分配的内存块地址；失败则返回NULL
 */
void* realize_unit_mem_malloc_ex(size_t size, const char *file, const int line, guMemMonitorType_t type, size_t align);

/**
 * @brief tlsf重新分配内存块
 *
 * @param rmem	 指向已分配的内存块
 * @param newsize 重新分配的内存大小
 * @return 成功则返回分配的内存块地址；失败则返回NULL
 */
void* realize_unit_mem_realloc_ex(void *rmem, size_t newsize, const char *file, const int line, guMemMonitorType_t type, size_t align);

/**
 * @brief tlsf分配多内存块
 *
 * @param nmemb	 分配的空间对象的计数
 * @param size 分配的空间对象的大小
 * @return 成功则指向指向第一个内存块地址的指针，并且所有分配的内存块都被初始化成零；如果失败则返回 NULL。
 */
void* realize_unit_mem_calloc_ex(int nmemb, size_t size, const char *file, const int line, guMemMonitorType_t type, size_t align);

/**
 * @brief 释放内存块
 *
 * @param p 待释放的内存块指针
 * @return void
 */
void realize_unit_mem_free_ex(void* p, const char *file, const int line, guMemMonitorType_t type);


/**
 * @brief 字符串复制
 *
 * @param p 原字符串
 * @return void
 */
char * realize_unit_mem_strdup_ex(const char* p, const char *file, const int line, guMemMonitorType_t type, size_t align);


/**
 * @brief 获取不同类别内存使用情况
 *
 * @param type
 * @return 已经使用多少内存
 */
unsigned int realize_unit_mem_get_mem_used(guMemMonitorType_t type);

int realize_unit_mem_get_pool_info(char *buffer, size_t buffer_size, char disp_block_info);

void *realize_unit_mem_get_start_addr();

int realize_unit_mem_get_block_size(void *p);

void realize_unit_mem_upd_mem_used(guMemMonitorType_t type, int used_size);

void realize_unit_mem_lock();
void realize_unit_mem_unlock();

/*****************************内存监控管理************************************/

#ifdef CONFIG_DEBUG_MEM_LEAK_MONITOR
  #define mem_leak_add mem_leak_add_ex
  #define mem_leak_remove mem_leak_remove_ex

#else
  #define mem_leak_add(p, size, blocksize, file, line) ;
  #define mem_leak_remove(p, file, line) ;
#endif

// mem leak debug 
#define mem_leak_start_monitor(x) mem_leak_start_monitor_ex(x, __FILE__, __LINE__);
#define mem_leak_check_monitor_status(x) mem_leak_check_monitor_status_ex(x, __FILE__, __LINE__);
#define mem_leak_monitor_statics(x) mem_leak_check_report_ex(x, __FILE__, __LINE__);

void mem_leak_add_ex(void *address, int size, int block_size, const char *filename, int line);
void mem_leak_remove_ex(void *address, const char *filename, int line);


void mem_leak_start_monitor_ex(const char *name, const char *file, int line);
void mem_leak_check_monitor_status_ex(const char *name, const char *file, int line);
void mem_leak_check_report_ex(const char *name, const char *file, int line);
#ifdef CONFIG_DEBUG_MEM_LEAK_MONITOR
unsigned short realize_unit_mem_get_user_data(void *p, unsigned int *block_size);
int realize_unit_mem_set_user_data(void *p, unsigned short user_data);
#endif
#endif

#endif