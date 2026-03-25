#ifndef __REALIZE_UNIT_SLEEP_H__
#define __REALIZE_UNIT_SLEEP_H__

typedef struct {
    unsigned int auto_screen_off_time;  //自动熄屏时间
    unsigned int auto_power_off_time;   //自动关机时间
    unsigned int screen_off_brightness; //熄屏亮度
    unsigned int screen_on_brightness;  //亮屏亮度
    unsigned int auto_deep_sleep_time;  //自动深度休眠的时间
    unsigned int sleep_poweroff_sign;  //是否需要深度休眠唤醒关机 1：需要，0：不需要
    unsigned int sleep_poweroff_time;  //深度休眠到关机的时间
    unsigned int last_active_time;             //屏幕上一次活动时间
} unit_sleep_param_t;

typedef enum {
	SLEEP_SCREEN_OFF,
	SLEEP_SCREEN_ON,
	SLEEP_POWER_OFF,
	SLEEP_DEEP,
    SLEEP_WAKE,
} unit_sleep_state_t;

typedef void (* sleep_cb_t)(unit_sleep_state_t state);

typedef struct {
    unit_sleep_param_t param;
    unit_sleep_state_t state;
    sleep_cb_t sleep_cb;
    unsigned char report_flag;
    int callback_id;
} unit_sleep_cfg_t;

//唤醒屏幕
void realize_unit_sleep_awaken_disp();

//立即熄屏
void realize_unit_sleep_set_sleep_immediately(int state);

//更新屏幕状态
unit_sleep_state_t realize_unit_sleep_update_state();

//设置屏幕休眠管理参数
int realize_unit_sleep_set_param(unit_sleep_param_t* param);

//获取屏幕休眠管理参数
unit_sleep_cfg_t *realize_unit_sleep_get_cfg_param(void);

#endif