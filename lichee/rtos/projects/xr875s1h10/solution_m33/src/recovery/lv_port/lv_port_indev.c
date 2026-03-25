/**
 * @file lv_port_indev_templ.c
 *
 */

 /*Copy this file as "lv_port_indev.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include <sunxi-input.h>
#include "evdev.h"
#include "lv_port_indev.h"

/*********************
 *      DEFINES
 *********************/
#define INPUT_DEV_NAME "touchscreen"

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static uint32_t keyboard_last_key = 0;
static lv_indev_state_t state;
static lv_group_t* lv_group_kb = NULL;
static btn_t s_btn;

lv_group_t* get_lv_group_kb() { return lv_group_kb; }

static void touchpad_init(int type);
static void touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static void touchpad_get_xy(lv_coord_t * x, lv_coord_t * y);

static void mouse_init(void);
static void mouse_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static bool mouse_is_pressed(void);
static void mouse_get_xy(lv_coord_t * x, lv_coord_t * y);

static void keypad_init(btn_t *btn);
static void keypad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static uint32_t keypad_get_key(void);

static void encoder_init(void);
static void encoder_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
void encoder_handler(int32_t direct, int32_t key_state);

static void button_init(void);
static void button_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static int8_t button_get_pressed_id(void);
static bool button_is_pressed(uint8_t id);

/**********************
 *  STATIC VARIABLES
 **********************/
lv_indev_t * indev_touchpad;
lv_indev_t * indev_mouse;
lv_indev_t * indev_keypad;
lv_indev_t * indev_encoder;
lv_indev_t * indev_button;

static int32_t encoder_diff;
static lv_indev_state_t encoder_state;
static int s_input_fd;
static int s_input_last_x;
static int s_input_last_y;
static int s_input_state;
static upgrade_handle_t *s_handle = NULL;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_indev_init(upgrade_handle_t *handle)
{
    /**
     * Here you will find example implementation of input devices supported by LittelvGL:
     *  - Touchpad
     *  - Mouse (with cursor support)
     *  - Keypad (supports GUI usage only with key)
     *  - Encoder (supports GUI usage only with: left, right, push)
     *  - Button (external buttons to press points on the screen)
     *
     *  The `..._read()` function are only examples.
     *  You should shape them according to your hardware
     */

    /*------------------
    * Evdev
    * -----------------*/
#if USE_EVDEV
    static lv_indev_drv_t indev_drv;
    evdev_init();
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = evdev_read;
    lv_indev_t *evdev_indev = lv_indev_drv_register(&indev_drv);
#endif

    s_handle = handle;

#if 0
    /*------------------
     * Touchpad
     * -----------------*/

    /*Initialize your touchpad if you have*/
    if (handle->tp.enable) {
        static lv_indev_drv_t indev_drv_pointer;
        touchpad_init(handle->tp.type);

        /*Register a touchpad input device*/
        lv_indev_drv_init(&indev_drv_pointer);
        indev_drv_pointer.type = LV_INDEV_TYPE_POINTER;
        indev_drv_pointer.read_cb = touchpad_read;
        indev_touchpad = lv_indev_drv_register(&indev_drv_pointer);
    }
#endif

#if 0

    /*------------------
     * Mouse
     * -----------------*/

    /*Initialize your touchpad if you have*/
    mouse_init();

    /*Register a mouse input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = mouse_read;
    indev_mouse = lv_indev_drv_register(&indev_drv);

    /*Set cursor. For simplicity set a HOME symbol now.*/
    lv_obj_t * mouse_cursor = lv_img_create(lv_scr_act());
    lv_img_set_src(mouse_cursor, LV_SYMBOL_HOME);
    lv_indev_set_cursor(indev_mouse, mouse_cursor);
#endif

#if 1
    /*------------------
     * Keypad
     * -----------------*/

    /*Initialize your keypad or keyboard if you have*/

    if (handle->btn.enable) {
        static lv_indev_drv_t indev_drv_btn;
        keypad_init(&handle->btn);

        lv_group_kb = lv_group_create();
        lv_group_set_default(lv_group_kb);

        /*Register a keypad input device*/
        lv_indev_drv_init(&indev_drv_btn);
        indev_drv_btn.type = LV_INDEV_TYPE_KEYPAD;
        indev_drv_btn.read_cb = keypad_read;
        indev_keypad = lv_indev_drv_register(&indev_drv_btn);
        lv_indev_set_group(indev_keypad, lv_group_kb);

    }

    /*Later you should create group(s) with `lv_group_t * group = lv_group_create()`,
     *add objects to the group with `lv_group_add_obj(group, obj)`
     *and assign this input device to group to navigate in it:
     *`lv_indev_set_group(indev_keypad, group);`*/
#endif

#if 0

    /*------------------
     * Encoder
     * -----------------*/

    /*Initialize your encoder if you have*/

    if (handle->encoder.enable) {
        static lv_indev_drv_t indev_drv_encoder;
        encoder_init();
        /*Register a encoder input device*/
        lv_indev_drv_init(&indev_drv_encoder);
        indev_drv_encoder.type = LV_INDEV_TYPE_ENCODER;
        indev_drv_encoder.read_cb = encoder_read;
        indev_encoder = lv_indev_drv_register(&indev_drv_encoder);
        lv_indev_set_group(indev_encoder, lv_group_kb);
        lv_group_set_editing(lv_group_get_default(), true);
    }

    /*Later you should create group(s) with `lv_group_t * group = lv_group_create()`,
     *add objects to the group with `lv_group_add_obj(group, obj)`
     *and assign this input device to group to navigate in it:
     *`lv_indev_set_group(indev_encoder, group);`*/

#endif

#if 0
    /*------------------
     * Button
     * -----------------*/

    /*Initialize your button if you have*/
    button_init();

    /*Register a button input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_BUTTON;
    indev_drv.read_cb = button_read;
    indev_button = lv_indev_drv_register(&indev_drv);

    /*Assign buttons to points on the screen*/
    static const lv_point_t btn_points[2] = {
            {10, 10},   /*Button 0 -> x:10; y:10*/
            {40, 100},  /*Button 1 -> x:40; y:100*/
    };
    lv_indev_set_button_points(indev_button, btn_points);
#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/


/*------------------
 * Touchpad
 * -----------------*/

/*Initialize your touchpad*/
#if 1

static void touchpad_init(int type)
{
    /*Your code comes here*/
    int ret = 0;

    ret = sunxi_input_init();
	if (ret < 0) {
		printf("[lv_indev]sunxi input init fail.\n");
        return;
    }

    s_input_fd = sunxi_input_open(INPUT_DEV_NAME);
    if (-1 == s_input_fd) {
		printf("[lv_indev]sunxi input open fail.\n");
    }

    s_input_last_x = 0;
    s_input_last_y = 0;
    s_input_state = LV_INDEV_STATE_REL;
}

#endif

/*Will be called by the library to read the touchpad*/
static void touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    struct sunxi_input_event event;

    /*Save the pressed coordinates and the state*/
    while (sunxi_input_read(s_input_fd, &event, sizeof(struct sunxi_input_event)) > 0) {
        if (event.type == INPUT_EVENT_ABS) {
            if (INPUT_ABS_MT_POSITION_X == event.code) {
                s_input_last_x = event.value;
            } else if (INPUT_ABS_MT_POSITION_Y == event.code) {
                s_input_last_y = event.value;
            } else if (INPUT_ABS_MT_PRESSURE == event.code) {
                if (0 == event.value) {
                    s_input_state = LV_INDEV_STATE_REL;
                } else {
                    s_input_state = LV_INDEV_STATE_PR;
                }
                // printf("[lv_port_indev] x: %d, y: %d, state: %d.\n", s_input_last_x, s_input_last_y, s_input_state);
                break;
            }
        }
    }

    /*Set the last pressed coordinates*/
    data->point.x = s_input_last_x;
    data->point.y = s_input_last_y;
    data->state = s_input_state;
}

/*------------------
 * Mouse
 * -----------------*/

/*Initialize your mouse*/
static void mouse_init(void)
{
    /*Your code comes here*/
}

/*Will be called by the library to read the mouse*/
static void mouse_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    /*Get the current x and y coordinates*/
    mouse_get_xy(&data->point.x, &data->point.y);

    /*Get whether the mouse button is pressed or released*/
    if(mouse_is_pressed()) {
        data->state = LV_INDEV_STATE_PR;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

/*Return true is the mouse button is pressed*/
static bool mouse_is_pressed(void)
{
    /*Your code comes here*/

    return false;
}

/*Get the x and y coordinates if the mouse is pressed*/
static void mouse_get_xy(lv_coord_t * x, lv_coord_t * y)
{
    /*Your code comes here*/

    (*x) = 0;
    (*y) = 0;
}

/*------------------
 * Keypad
 * -----------------*/

/*Initialize your keypad*/
static void keypad_init(btn_t *btn)
{
    if (NULL == btn || !btn->enable) {
        printf("use default\n");
        s_btn.left = 37;
        s_btn.up = 38;
        s_btn.right = 39;
        s_btn.down = 40;
        s_btn.ok = 36;
        s_btn.back = 27;
    } else {
        s_btn.left = btn->left;
        s_btn.up = btn->up;
        s_btn.right = btn->right;
        s_btn.down = btn->down;
        s_btn.ok = btn->ok;
        s_btn.back = btn->back;
    }
}

void recovery_keyboard_handler(uint8_t key_state, uint32_t key_data)
{
	if (LV_INDEV_STATE_REL == key_state) {
		//state = key_state;          /*Save the key is pressed now*/
		keyboard_last_key = 0;
		//printf("state, %d, keyboard_last_key = %u.\n", state, keyboard_last_key);
	} else {
		keyboard_last_key = key_data;
	}
}

/*Will be called by the library to read the mouse*/
static void keypad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    static uint32_t last_key = 0;

	// printf("data->state = %d.\n", data->state);
	
	data->state = state;

    /*Get the current x and y coordinates*/
    //mouse_get_xy(&data->point.x, &data->point.y);

    /*Get whether the a key is pressed and save the pressed key*/
    uint32_t act_key = keypad_get_key();
    // printf("data->state = %d-->%d\n", data->state, act_key);
    if(act_key > 0) {
		data->state = LV_INDEV_STATE_PR;
        /*Translate the keys to LVGL control characters according to your key definitions*/
        if (act_key == s_btn.down) {
            act_key = LV_KEY_DOWN;
        } else if (act_key == s_btn.up) {
            act_key = LV_KEY_UP;
        } else if (act_key == s_btn.left) {
            act_key = LV_KEY_LEFT;
        } else if (act_key == s_btn.right) {
            act_key = s_handle->encoder.enable == 1? LV_KEY_HOME:LV_KEY_RIGHT;
        } else if (act_key == s_btn.ok) {
            act_key = LV_KEY_ENTER;
        } else if (act_key == s_btn.back) {
            act_key = LV_KEY_BACKSPACE;
        }

/*         switch(act_key) {
            case 40:
                act_key = LV_KEY_DOWN;
                break;
            case 38:
                act_key = LV_KEY_UP;
                break;
            case 37:
                act_key = LV_KEY_LEFT;
                break;
            case 39:
                act_key = LV_KEY_RIGHT;
                break;
            case 36:
                act_key = LV_KEY_ENTER;
                break;
            case 27:
                act_key = LV_KEY_BACKSPACE;
                break;
        } */
        last_key = act_key;
		printf("last_key = %u.\n", last_key);
    } else {
        data->state = LV_INDEV_STATE_REL;
		keyboard_last_key = 0;
    }

    data->key = last_key;
}

/*Get the currently being pressed key.  0 if no key is pressed*/
static uint32_t keypad_get_key(void)
{
    /*Your code comes here*/

    return keyboard_last_key;
}

/*------------------
 * Encoder
 * -----------------*/

/*Initialize your keypad*/

/*------------------
 * Encoder
 * -----------------*/

/*Initialize your keypad*/
static void encoder_init(void)
{
    dev_encoder_init();
}

/*Will be called by the library to read the encoder*/
static void encoder_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{

    data->enc_diff = encoder_diff;
    data->state = encoder_state;
    data->key = LV_KEY_ENTER;
    encoder_diff = 0;
}

/*Call this function in an interrupt to process encoder events (turn, press)*/
void recovery_encoder_handler(int32_t direct, int32_t key_state)
{

     if (direct == 1) { //顺时针
        encoder_diff = 1;
    } else if (direct == 0) { //逆时针
        encoder_diff = -1;
    }

    if (key_state != -1) {
        encoder_state = key_state;
    }


    // printf("enc_diff  %d  state %d \n", encoder_diff, encoder_state);
}
/*------------------
 * Button
 * -----------------*/

/*Initialize your buttons*/
static void button_init(void)
{
    /*Your code comes here*/
}

/*Will be called by the library to read the button*/
static void button_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{

    static uint8_t last_btn = 0;

    /*Get the pressed button's ID*/
    int8_t btn_act = button_get_pressed_id();

    if(btn_act >= 0) {
        data->state = LV_INDEV_STATE_PR;
        last_btn = btn_act;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }

    /*Save the last pressed button's ID*/
    data->btn_id = last_btn;
}

/*Get ID  (0, 1, 2 ..) of the pressed button*/
static int8_t button_get_pressed_id(void)
{
    uint8_t i;

    /*Check to buttons see which is being pressed (assume there are 2 buttons)*/
    for(i = 0; i < 2; i++) {
        /*Return the pressed button's ID*/
        if(button_is_pressed(i)) {
            return i;
        }
    }

    /*No button pressed*/
    return -1;
}

/*Test if `id` button is pressed or not*/
static bool button_is_pressed(uint8_t id)
{

    /*Your code comes here*/

    return false;
}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
