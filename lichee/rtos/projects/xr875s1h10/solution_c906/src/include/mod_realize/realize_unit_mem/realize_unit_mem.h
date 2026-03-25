#ifndef _REALIZE_UNIT_MEM_H_
#define _REALIZE_UNIT_MEM_H_

#include "stdio.h"
#include "../../platform/gulitesf_config.h"
#include "realize_unit_mem_v2.h"

#if !CONFIG_USE_MEM_V2

#ifdef CONFIG_LV_MEM_SIZE
#define UNIT_MEM_MONITOR_OUTSIDE_SIZE CONFIG_LV_MEM_SIZE
#else
#define UNIT_MEM_MONITOR_OUTSIDE_SIZE 0
#endif

typedef struct mem_pool_t
{
	unsigned int pool_size;
	void* tlsf_handle;
	void *mem_pool;
	unsigned int from_size;
	unsigned int to_size;
}mem_pool_t;

typedef struct mem_pool_config
{
    mem_pool_t *mem_pool;
    int mem_pool_cnt;
    char is_js_independent;     //是否JS使用独立的内存池   
    int heap_max_mem_block_count;     //监控heap内存, 如果heap_mem_count > 0 统计内存不够时申请heap内存的内存块，最大统计的内存块
}mem_pool_config_t;


//扩展内存结点数，一个结点8个字节，最多占 8K ,如果设置为0，则不监控扩展内存 
//如果不监控注释掉下面的define
// #define MONITOR_EXTEN_MEM_BLOCK 1600
#ifdef CONFIG_PLATFORM_RTOS
//扩展内存地址长度
#define MEM_ADDR_LEN 4
#else
#define MEM_ADDR_LEN 8 
#endif
//监控3类内存使用情况
typedef enum {
  MEM_MONITOR_TYPE_ENG = 0,
  MEM_MONITOR_TYPE_JS = 1,
  MEM_MONITOR_TYPE_IMG = 2,
  MEM_MONITOR_TYPE_LVGL = 3,
} guMemMonitorType_t;

typedef struct {
	unsigned int mem_size;
} unit_mem_conf_t;


#define realize_unit_mem_malloc(x) realize_unit_mem_malloc_ex(x, __FILE__, __LINE__, MEM_MONITOR_TYPE_ENG)
#define realize_unit_mem_calloc(x, y) realize_unit_mem_calloc_ex(x, y, __FILE__, __LINE__, MEM_MONITOR_TYPE_ENG)
#define realize_unit_mem_free(x) realize_unit_mem_free_ex(x, __FILE__, __LINE__, MEM_MONITOR_TYPE_ENG)
#define realize_unit_mem_realloc(x, y) realize_unit_mem_realloc_ex(x, y, __FILE__, __LINE__, MEM_MONITOR_TYPE_ENG)
#define realize_unit_mem_strdup(x) realize_unit_mem_strdup_ex(x,__FILE__, __LINE__, MEM_MONITOR_TYPE_ENG)


void realize_unit_mem_init(unsigned int mem_size);

/**
 * @brief tlsf分配内存块
 *
 * @param size 需要分配的内存块的大小，单位为字节。
 * @return 成功则返回分配的内存块地址；失败则返回NULL
 */
void* realize_unit_mem_malloc_ex(size_t size, const char *file, const int line, guMemMonitorType_t type);

/**
 * @brief tlsf重新分配内存块
 *
 * @param rmem	 指向已分配的内存块
 * @param newsize 重新分配的内存大小
 * @return 成功则返回分配的内存块地址；失败则返回NULL
 */
void* realize_unit_mem_realloc_ex(void *rmem, size_t newsize, const char *file, const int line, guMemMonitorType_t type);

/**
 * @brief tlsf分配多内存块
 *
 * @param nmemb	 分配的空间对象的计数
 * @param size 分配的空间对象的大小
 * @return 成功则指向指向第一个内存块地址的指针，并且所有分配的内存块都被初始化成零；如果失败则返回 NULL。
 */
void* realize_unit_mem_calloc_ex(int nmemb, size_t size, const char *file, const int line, guMemMonitorType_t type);

/**
 * @brief tlsf释放内存块
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
char * realize_unit_mem_strdup_ex(const char* p, const char *file, const int line, guMemMonitorType_t type);

/**
 * @brief 获取tlsf的内存池的句柄
 *
 * @param void
 * @return void
 */
void *realize_unit_mem_get_handle(void);

/**
 * @brief 得到tlsf的内存块的大小
 *
 * @param void
 * @return void
 */
int realize_unit_mem_get_block_size(void *p);

/**
 * @brief 销毁内存tlsf池
 *
 * @param void
 * @return void
 */
void realize_unit_mem_deinit(void);

/**
 * @brief 获取内存池大小
 *
 * @param void
 * @return unsigned int
 */
unsigned int realize_unit_mem_get_size(void);

/**
 * @brief 统计不同类别内存使用情况
 *
 * @param type
 *         used_size，本次使用的内存数
 * @return void
 */
void realize_unit_mem_upd_mem_used(guMemMonitorType_t type, int used_size);

/**
 * @brief 获取不同类别内存使用情况
 *
 * @param type
 * @return 已经使用多少内存
 */
unsigned int realize_unit_mem_get_mem_used(guMemMonitorType_t type);

int is_pool_mem(void *p);
int realize_unit_mem_check(void);
void realize_unit_mem_set_pool_config(mem_pool_config_t *config);
mem_pool_config_t *realize_unit_mem_get_pool_config();
void realize_unit_mem_lock();
void realize_unit_mem_unlock();
void set_warning_mem_alloc_size(int size);

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
#endif

#endif