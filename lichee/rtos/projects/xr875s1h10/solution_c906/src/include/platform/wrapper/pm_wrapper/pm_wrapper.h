#ifndef __PM_WRAPPER_H__
#define __PM_WRAPPER_H__    

#include "../../gulitesf_config.h"
#include "../../../mod_realize/realize_unit_pm/realize_unit_pm.h"
#ifdef CONFIG_PM_SUPPORT

//通知上层设备唤醒了
typedef void (*on_wakeup_cb)(void *userdata);

//初始化
typedef void *(*init_t)(void *cb, void *userdata);

/**
 * @brief set wakeup sources
 * @param handle: Pointer to a handle
 * @param info: Pointer to an array of wakeup source information structures
 * @param wakeup_src_num: Number of wakeup source information structures in the array
 * @return Returns 0 on success, and -1 on failure
 */
typedef int (*set_wakeup_src_t)(void *handle, unit_pm_mode_t mode, unit_pm_wakeup_info_t *info, int wakeup_src_num);

/**
 * @brief set wakeup sources
 * @param handle: Pointer to a handle
 * @param info: Pointer to an array of wakeup source information structures
 * @param wakeup_src_num: Number of wakeup source information structures in the array
 * @return Returns 0 on success, and -1 on failure
 */
typedef int (*get_wakeup_src_t)(void *handle, unit_pm_wakeup_info_t *info); 

//进入休眠
typedef int (*goto_sleep_t)(void *handle, unit_pm_mode_t mode);  

//清除某个已设置的唤醒源
typedef int (*clear_wakeup_src_t)(void *handle, unit_pm_wakeup_type_t type);    

//反初始化
typedef int (*deinit_t)(void **handle);  

typedef struct {
    init_t init;
    set_wakeup_src_t set_wakeup_src;
    get_wakeup_src_t get_wakeup_src;    
    goto_sleep_t goto_sleep;
    clear_wakeup_src_t clear_wakeup_src;    
    deinit_t deinit;    
} pm_wrapper_t;
#endif
#endif
