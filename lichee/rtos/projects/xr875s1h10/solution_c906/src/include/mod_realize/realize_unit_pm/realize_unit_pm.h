#ifndef __REALIZE_UNIT_PM_H__
#define __REALIZE_UNIT_PM_H__   

#include "../../platform/gulitesf_config.h"
#include "../realize_unit_log/realize_unit_log.h"
#include "../realize_unit_mem/realize_unit_mem.h"

#ifdef CONFIG_PM_SUPPORT
#define PM_ON_DEBUG 0

#if PM_ON_DEBUG
#define pm_log_debug(format, ...)
#define pm_log_info(format, ...)
#else
#define pm_log_debug realize_unit_log_debug
#define pm_log_info realize_unit_log_info
#endif
#define pm_log_warn realize_unit_log_warn
#define pm_log_error realize_unit_log_error

#define pm_malloc realize_unit_mem_malloc
#define pm_calloc realize_unit_mem_calloc
#define pm_realloc realize_unit_mem_realloc
#define pm_free realize_unit_mem_free

typedef enum {
    UNIT_PM_WAKEUP_SRC_NONE = 0,    //No wakeup source  
    UNIT_PM_WAKEUP_SRC_EXIT,         //Key wakeup source  
    UNIT_PM_WAKEUP_SRC_RTC,        // RTC wakeup source
    UNIT_PM_WAKEUP_SRC_USB,         // USB wakeup source    
    UNIT_PM_WAKEUP_SRC_WLAN,       // WLAN wakeup source
    UNIT_PM_WAKEUP_SRC_USER,        // 用户自定义唤醒源    
    UNIT_PM_WAKEUP_SRC_MAX         // Max wakeup source    
} unit_pm_wakeup_type_t;  

typedef enum {
    UNIT_PM_MODE_NONE = -1,    //No limit power mode 
    UNIT_PM_MODE_NO_SLEEP = 0,     //No sleep state 
    UNIT_PM_MODE_LIGHT_SLEEP,      //light Sleep state      (sleep)
    UNIT_PM_MODE_MEDIUM_SLEEP,     //medium Sleep state     (standby)   
    UNIT_PM_MODE_DEEP_SLEEP,       //deep Sleep state       (hibernate) 
    UNIT_PM_MODE_POWER_OFF,        //Power off state    
    UNIT_PM_MODE_MAX   
} unit_pm_mode_t; 

typedef enum {
    UNIT_PM_OPERATOR_NONE = 0,    //No operator  
    UNIT_PM_OPERATOR_DONE,          //
    UNIT_PM_OPERATOR_IMMEDIATELY,    //Immediately enter sleep state    
    UNIT_PM_OPERATOR_MAX           //Max operator   
} unit_pm_operator_state_t;

typedef enum {
    UNIT_PM_ERROR_NONE = 0,    //No error
    UNIT_PM_ERROR_SUPPEND_FAIL = -1,       //Suspend failure
    UNIT_PM_ERROR_UNKOWN_WAKEUP = -2,    //Unknown wakeup source
    UNIT_PM_ERROR_MAX           //Max error
} unit_pm_error_t;

typedef struct {
    unit_pm_wakeup_type_t type;     //Wakeup source type    
    union {
        struct {
            char name[20];    // 具体的唤醒源   
        } exit;
        struct {   
            char type;    //0: delay, 1: specific
            unsigned long delay_time;     // delay time     (ms)    
            char alarm_time[20]; // specific time ("YY-MM-DD HH:MM")
        } rtc;  
        struct {
            char state;    // 0:移除唤醒  1：插入唤醒，2：移除插入都唤醒   
        } usb;
        struct {
            char *data;
        } user;
    } param; 
} unit_pm_wakeup_info_t;  

typedef struct {
    unit_pm_mode_t mode;
    unsigned long time;             //Sleep time (ms)  
    unsigned long report_time;      //Report time (ms)  
    bool enable;
} unit_pm_auto_sleep_t; 

typedef void (*unit_pm_wakeup_cb_t)(unit_pm_wakeup_info_t *info, void *userdata);    //Callback function when wakeup  
typedef void (*unit_pm_auto_sleep_cb_t)(unit_pm_auto_sleep_t *info, void *userdata);    //Callback function when auto sleep 
typedef void (*unit_pm_error_cb_t)(unit_pm_error_t error, void *userdata);    //Callback function when error happen
typedef struct {
    unit_pm_wakeup_cb_t report_wakeup_cb;
    unit_pm_auto_sleep_cb_t report_auto_sleep; 
    unit_pm_error_cb_t error_cb;
} unit_pm_cb_t;

typedef struct _unit_pm_obj {
	void *ctx;
    unit_pm_mode_t cur_mode;

    int (*set_auto_sleep)(struct _unit_pm_obj *obj, unit_pm_auto_sleep_t *info);  //Set power mode      
    int (*get_auto_sleep_info)(struct _unit_pm_obj *obj, unit_pm_mode_t mode, unit_pm_auto_sleep_t *info);   //Get current sleep state    //0：不睡眠 1：轻度睡眠 2：深度睡眠 
    int (*clear_auto_sleep)(struct _unit_pm_obj *obj, unit_pm_mode_t mode);  //Reset power mode   
    int (*clear_wakeup_src)(struct _unit_pm_obj *obj, unit_pm_wakeup_type_t type);  //clear wakeup source   
    int (*set_wakeup_src)(struct _unit_pm_obj *obj, unit_pm_wakeup_info_t *info, int wakeup_src_num);  //Set wakeup source
    int (*get_wakeup_src)(struct _unit_pm_obj *obj, unit_pm_wakeup_type_t type, unit_pm_wakeup_info_t **info, int *wakeup_src_num);  //Get wakeup source
    int (*set_immediately_sleep)(struct _unit_pm_obj *obj, unit_pm_mode_t mode, int state);  //Mark immediately sleep    //state: 1：立即休眠    //0：取消立即休眠      
    int (*get_immediately_sleep_state)(struct _unit_pm_obj *obj);    //1:可以进入休眠状态    //0：不可以进入休眠状态
    unit_pm_mode_t (*get_immediately_sleep_mode)(struct _unit_pm_obj *obj);    //获取要立即进入休眠的模式
    int (*report_ready_entry_mode)(struct _unit_pm_obj *obj, unit_pm_mode_t mode);   //上报即将自动进入的工作模式
    int (*goto_sleep)(struct _unit_pm_obj *obj, unit_pm_mode_t mode);  //Enter sleep state  //0：成功 -1：失败  
} unit_pm_obj_t; 

unit_pm_obj_t *realize_unit_pm_new(void *userdata, unit_pm_cb_t *cb);
void realize_unit_pm_delete(unit_pm_obj_t **pm_obj);

unit_pm_obj_t *realize_unit_pm_get_gloabl_obj(); //Get global object 
int realize_unit_pm_get_state();

#endif 
#endif
