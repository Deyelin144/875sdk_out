/*********************************************************************************
  *Copyright(C),2014-2022,GuRobot
  *Author:  weigang
  *Date:  2023.12.01
  *Description:  内容统计和显示
  *History:  
**********************************************************************************/

#ifndef __GU_MEM_STATIS_H__
#define __GU_MEM_STATIS_H__
#define MEM_POOL_MAX_CNT 2
typedef struct{
    unsigned int total;
    unsigned int free;
    int used_pct;
    int frag[MEM_POOL_MAX_CNT];
    unsigned int max_free[MEM_POOL_MAX_CNT];
    unsigned int pool_total[MEM_POOL_MAX_CNT];
    unsigned int pool_used[MEM_POOL_MAX_CNT];    
    unsigned int image_size;
    unsigned int eng_size;
    unsigned int js_size;
    unsigned int lvgl_size;
    unsigned int lvgl_total_size;
    unsigned int use_heap_size; //使用了外部SDK的内存
    unsigned int heap_free;     //SDK heap内存还剩下多少
}guMemStatis_t;

typedef struct {
    unsigned int total_size; /**< Total heap size*/
    unsigned int free_cnt;
    unsigned int free_size; /**< Size of available memory*/
    unsigned int free_biggest_size;
    unsigned int used_cnt;
    unsigned int max_used; /**< Max size of Heap memory used*/
    unsigned char used_pct; /**< Percentage used*/
    unsigned char frag_pct; /**< Amount of fragmentation*/
} gu_mem_monitor_t;

void realize_unit_mem_statistics_print(const char *desc);
void realize_unit_mem_statistics_get_statis(guMemStatis_t *statis);
void realize_unit_mem_statistics_close_mem_Win(void);
void realize_unit_mem_statistics_show_mem(void);
void show_current_mem_dis(void);
void add_mem_statis_and_compare_last(const char *name);
void print_all_mem_statis();
void start_mem_statis_monitor();
#endif