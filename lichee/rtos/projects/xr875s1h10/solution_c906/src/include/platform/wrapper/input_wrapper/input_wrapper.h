#ifndef _INPUT_WRAPPER_H_
#define _INPUT_WRAPPER_H_

#include "../../../mod_realize/realize_unit_input/realize_unit_input.h"

typedef void (*get_state_t)(unsigned int input_id, unsigned short id, unsigned char state);

typedef struct _keyboard_node {
	char *name;
	int btn_num;
	keyboard_btn_ctrl_t *btn;
	int (*init)(unsigned int i_id, char *json_str, get_state_t cb);
	void (*deinit)(struct _keyboard_node *keyboard);
} keyboard_wrapper_node_t;

typedef struct {
    int num;
    keyboard_wrapper_node_t *node;
} keyboard_wrapper_t;

typedef struct _encoder_node {
	char *name;
	int (*init)(int, get_state_t);
	void (*deinit)(struct _encoder_node *encode);
} encoder_wrapper_node_t;

typedef struct {
    int num;
    encoder_wrapper_node_t *node;
} encoder_wrapper_t;

typedef struct {
	keyboard_wrapper_t keyboard;
	encoder_wrapper_t encoder;
} input_wrapper_t;

input_wrapper_t *input_wrapper_handle(void);


#endif
