#ifndef _NATIVE_APP_INPUT_H_
#define _NATIVE_APP_INPUT_H_

#include "../../../mod_realize/realize_unit_input/realize_unit_input.h"

typedef struct {
	input_ctrl_val_t power;
	input_ctrl_val_t test_ag;
	input_ctrl_val_t test_fac;
} button_val_t;

typedef struct {
	char *name;
	union {
		button_val_t button;
	} val;
	input_cb_t input_handle;
} native_input_t;

void *native_app_input_create(void *obj_in, native_input_t *native_input);
void native_app_input_destory(void **input_obj);
void native_input_resume_state(void *m_input);
#endif


