#ifndef _INPUT_H_
#define _INPUT_H_

#include "./realize_unit_keyboard.h"

#define INPUT_NAME_SIZE 24

#define INPUT_ON_DEBUG 1
#if INPUT_ON_DEBUG
#define input_log_debug realize_unit_log_debug
#define input_log_info realize_unit_log_info
#else
#define input_log_debug(format, ...)
#define input_log_info(format, ...)
#endif
#define input_log_wran realize_unit_log_warn
#define input_log_error realize_unit_log_error

#define input_malloc realize_unit_mem_malloc
#define input_free realize_unit_mem_free

typedef enum {
	TYPE_KEYBOARD,
	TYPE_ENCODER,
} input_type_t;

typedef enum {
	INPUT_ERR = -1,
	INPUT_NORMAL,
	INPUT_INTERCEPT,
} input_action_t;

typedef struct {
	unsigned short which;
	char flag;
	char val_type;
	void *val;
	int action;
	char err;
} input_ctrl_val_t;

typedef struct {
	int (*send_event)(input_type_t type, void *val, void *u_data);
} input_cb_t, input_inner_cb_t;

int realize_unit_input_init(void);
void realize_unit_input_set_inner_cb(char *name, void *u_data, input_inner_cb_t *cb);
void realize_unit_input_deinit(void);

void *realize_unit_input_create(const char *name, input_cb_t *cb, void *u_data);
void realize_unit_input_destory(void **obj);
int realize_unit_input_ctrl(void *obj, input_ctrl_val_t *param_array, int array_len);


#endif


