#ifndef _REALIZE_MCU_H_
#define _REALIZE_MCU_H_

typedef enum {
    MCU_EVENT_TOUCHKEY = 0,     //触摸按键
    MCU_EVENT_ACTION   = 1,     //动作回复
    MCU_EVENT_GYRO     = 2,     //陀螺仪数据
    MCU_EVENT_MAX,
} mcu_event_type_t;

typedef enum {
    MCU_TOUCHKEY_EAR_R     = 0,    // 左耳感应区触摸检测
    MCU_TOUCHKEY_EAR_L     = 1,    // 右耳感应区触摸检测
    MCU_TOUCHKEY_CROWN_R   = 2,    // 头顶右感应区触摸检测
    MCU_TOUCHKEY_CROWN_L   = 3,    // 头顶左感应区触摸检测
    MCU_TOUCHKEY_ID_MAX,
} mcu_touchkey_id_t;

typedef enum {
    MCU_TOUCHKEY_PRESS_DOWN     = 0,    //按键按下瞬间触发的事件
    MCU_TOUCHKEY_PRESS_UP       = 1,    //按键释放瞬间触发的事件
    MCU_TOUCHKEY_PRESS_HOLD     = 2,    //按键持续按住期间周期性触发的事件
    MCU_TOUCHKEY_PRESS_REPEAT   = 3,    //用于处理连击事件
    MCU_TOUCHKEY_LONG_PRESS     = 4,    //当按键按下时间超过预设阈值时触发
    MCU_TOUCHKEY_SHORT_CLICK    = 5,    //快速单击事件
    MCU_TOUCHKEY_DOUBLE_CLICK   = 6,    //两次单击时触发
    MCU_TOUCHKEY_EVENT_MAX,
} mcu_touchkey_event_t;

typedef enum {
    MCU_ACTION_LEG_LEFT     = 0,    //左腿
    MCU_ACTION_LEG_RIGHT    = 1,    //右腿
    MCU_ACTION_FOOT_LEFT    = 2,    //左脚
    MCU_ACTION_FOOT_RIGHT   = 3,    //右脚
    MCU_ACTION_ID_MAX,
} mcu_action_id_t;

typedef enum { 
    MCU_ACTION_SUCCESS = 0,         //动作被停止
    MCU_ACTION_FAILED = 1,          //动作执行失败
    MCU_ACTION_RUNNING = 2,         //动作正在执行
    MCU_ACTION_STOP = 3,            //动作执行成功
    MCU_ACTION_EVENT_MAX,
} mcu_action_event_t;

typedef struct {
    int x_data;
    int y_data;
    int z_data;
} mcu_gyro_data_t;

typedef struct {
    mcu_event_type_t event_type;
	unsigned short id;
	unsigned short data;
	void  *data_st;
} mcu_event_param_t;

typedef void (*realize_mcu_callback_t)(char* str);

typedef void (*realize_mcu_event_callback_t)(void *event_st);

int realize_mcu_init(realize_mcu_callback_t callback);

void realize_mcu_send_app_msg(char *type, int target, int describe);

void realize_mcu_send_app_msg_gyro(char *type, int x_data, int y_data, int z_data);

void realize_register_mcu_event_cb(realize_mcu_event_callback_t cb);

void mcu_robot_trim_info(int target, void *trim_st);

#endif //_REALIZE_MCU_H_


