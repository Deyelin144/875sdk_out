#ifndef _REALIZE_UNIT_KEYBOARD_H_
#define _REALIZE_UNIT_KEYBOARD_H_

#include "../realize_unit_log/realize_unit_log.h"
#include "../realize_unit_mem/realize_unit_mem.h"

#define BTN_ON_DEBUG 1
#if BTN_ON_DEBUG
#define btn_log_debug realize_unit_log_debug
#define btn_log_info realize_unit_log_info
#else
#define btn_log_debug(format, ...)
#define btn_log_info(format, ...)
#endif
#define btn_log_wran realize_unit_log_warn
#define btn_log_error realize_unit_log_error

#define btn_malloc realize_unit_mem_malloc
#define btn_free realize_unit_mem_free

typedef enum {
	PRESS_DOWN = 0,
	PRESS_UP,
	PRESS_HOLD,
	PRESS_REPEAT,
	LONG_PRESS,
	SHORT_CLICK,
	DOUBLE_CLICK,
	NONE_PRESS
} keyboard_btn_event_t;

typedef struct {
	unsigned short id;
	unsigned char state;
	keyboard_btn_event_t event;
} keyboard_btn_event_param_t;

typedef enum {
	SHORT_TRIGGER_TIME,
	LONG_TRIGGER_TIME,
	PRESS_HOLD_TRIGGER_TIME,
	PRESS_HOLD_INTEVAL_TIME,
	ACTIVE_STATE,
	NEED_UP_DOWN,
	ENUM_TOP = 0XFe,
} keyboard_btn_ctrl_flag_t;

typedef struct {
	unsigned short id;
	short short_start; //负数表示不支持，不上报事件
	short long_start;
	short press_hold_start;
	short press_hold_inteval;
	unsigned char active_state; //激活状态
	unsigned char need_up_down; //0表示不需要按下弹起事件，取值0-1
} keyboard_btn_ctrl_t;

typedef enum {
	BTN_CTRL_GET,
	BTN_CTRL_SET
} keyboard_btn_ctrl_act_t;

typedef struct {
	unsigned short id;
	char flag;
	char val_type;
	void *val;
	keyboard_btn_ctrl_act_t action;
} keyboard_btn_ctrl_val_t;

typedef struct {
	int input_id;
	int btn_num;
	keyboard_btn_ctrl_t *btn;
} keyboard_param_t;

typedef struct {
	int (*send_event)(keyboard_btn_event_param_t *event_param, void *userdata, void *inner_udata);
	long (*get_sys_runtime)(void);
} keyboard_cb_t;

typedef struct _keyboard_obj {
	void *ctx;
	void *user_data;
	void *inner_udata;
	int device_id;
	keyboard_cb_t keyboard_cb;
	int (*ctrl)(struct _keyboard_obj *obj, keyboard_btn_ctrl_val_t *ctrl_val);
	int (*exec)(struct _keyboard_obj *obj, unsigned short btn_id, unsigned char state);
} keyboard_obj_t;

keyboard_obj_t *realize_unit_keyboard_new(keyboard_param_t *param, void *user_data, keyboard_cb_t *cb);
void realize_unit_keyboard_delete(keyboard_obj_t **obj);

#endif


