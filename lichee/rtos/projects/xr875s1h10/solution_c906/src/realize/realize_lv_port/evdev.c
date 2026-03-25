/**
 * @file evdev.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "evdev.h"
#if USE_EVDEV != 0 || USE_BSD_EVDEV

#include "../../drivers/key/dev_touch_area.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#if USE_BSD_EVDEV
#include <dev/evdev/input.h>
#else
#include <sunxi_hal_twi.h>
#ifdef CONFIG_KERNEL_FREERTOS
#include <sunxi-input.h>
#else
#include <sunxi_input.h>
#endif
#endif

#if USE_XKB
#include "xkb.h"
#endif /* USE_XKB */

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
int map(int x, int in_min, int in_max, int out_min, int out_max);

/**********************
 *  STATIC VARIABLES
 **********************/
int evdev_fd;
int evdev_root_x;
int evdev_root_y;
int evdev_button;

int evdev_key_val;
extern int sunxi_input_init(void);


/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the evdev interface
 */
void evdev_init(void)
{
	if (sunxi_input_init() < 0)
		perror("touchscreen init fail\n");

#if USE_BSD_EVDEV
    evdev_fd = open(EVDEV_NAME, O_RDWR | O_NOCTTY);
#else
	evdev_fd = sunxi_input_open(EVDEV_NAME);
#endif
    if(evdev_fd == -1) {
        perror("unable open evdev interface:");
        return;
    }

#if 0
#if USE_BSD_EVDEV
    fcntl(evdev_fd, F_SETFL, O_NONBLOCK);
#else
    fcntl(evdev_fd, F_SETFL, O_ASYNC | O_NONBLOCK);
#endif
#endif

    evdev_root_x = 0;
    evdev_root_y = 0;
    evdev_key_val = 0;
    evdev_button = LV_INDEV_STATE_REL;

#if USE_XKB
    xkb_init();
#endif
}
/**
 * reconfigure the device file for evdev
 * @param dev_name set the evdev device filename
 * @return true: the device file set complete
 *         false: the device file doesn't exist current system
 */
#if 0
bool evdev_set_file(char* dev_name)
{
     if(evdev_fd != -1) {
        close(evdev_fd);
     }
#if USE_BSD_EVDEV
     evdev_fd = open(dev_name, O_RDWR | O_NOCTTY);
#else
     evdev_fd = open(dev_name, O_RDWR | O_NOCTTY | O_NDELAY);
#endif

     if(evdev_fd == -1) {
        perror("unable open evdev interface:");
        return false;
     }

#if USE_BSD_EVDEV
     fcntl(evdev_fd, F_SETFL, O_NONBLOCK);
#else
     fcntl(evdev_fd, F_SETFL, O_ASYNC | O_NONBLOCK);
#endif

     evdev_root_x = 0;
     evdev_root_y = 0;
     evdev_key_val = 0;
     evdev_button = LV_INDEV_STATE_REL;

     return true;
}
#endif
/**
 * Get the current position and state of the evdev
 * @param data store the evdev data here
 */
void evdev_read(lv_indev_drv_t * drv, lv_indev_data_t * data)
{
    struct sunxi_input_event in;
    static uint8_t last_event_code = 0;

    while(sunxi_input_read(evdev_fd, &in, sizeof(struct sunxi_input_event)) > 0) {
        // printf("type:%d code:%d value:%d last:%d\n", in.type, in.code, in.value,last_event_code);
        if(in.type == INPUT_EVENT_REL) {
            if(in.code == INPUT_RELA_X)
				#if EVDEV_SWAP_AXES
					evdev_root_y += in.value;
				#else
					evdev_root_x += in.value;
				#endif
            else if(in.code == INPUT_RELA_Y)
				#if EVDEV_SWAP_AXES
					evdev_root_x += in.value;
				#else
					evdev_root_y += in.value;
				#endif
        } else if(in.type == INPUT_EVENT_ABS) {
            if(in.code == INPUT_ABS_X)
				#if EVDEV_SWAP_AXES
					evdev_root_y = in.value;
				#else
					evdev_root_x = in.value;
				#endif
            else if(in.code == INPUT_ABS_Y)
				#if EVDEV_SWAP_AXES
					evdev_root_x = in.value;
				#else
					evdev_root_y = in.value;
				#endif
            else if(in.code == INPUT_ABS_MT_POSITION_X)
                                #if EVDEV_SWAP_AXES
                                        evdev_root_y = in.value;
                                #else
                                        evdev_root_x = in.value;
                                #endif
            else if(in.code == INPUT_ABS_MT_POSITION_Y)
                                #if EVDEV_SWAP_AXES
                                        evdev_root_x = in.value;
                                #else
                                        evdev_root_y = in.value;
                                #endif
            else if(in.code == INPUT_ABS_MT_TRACKING_ID) {
                                if(in.value == -1)
                                    evdev_button = LV_INDEV_STATE_REL;
                                else if(in.value == 0)
                                    evdev_button = LV_INDEV_STATE_PR;
            } else if(in.code == INPUT_ABS_MT_PRESSURE) {
                                if(in.value == 0) 
                                    evdev_button = LV_INDEV_STATE_REL;
                                else if(in.value > 0)
                                    evdev_button = LV_INDEV_STATE_PR;
            }
        } else if(in.type == INPUT_EVENT_KEY) {
            if(in.code == INPUT_BTN_MOUSE || in.code == INPUT_BTN_TOUCH) {
                if(in.value == 0)
                    evdev_button = LV_INDEV_STATE_REL;
                else if(in.value == 1)
                    evdev_button = LV_INDEV_STATE_PR;
            } else if(drv->type == LV_INDEV_TYPE_KEYPAD) {
#if USE_XKB
                data->key = xkb_process_key(in.code, in.value != 0);
#else
                switch(in.code) {
                    case INPUT_KEY_BACKSPACE:
                        data->key = LV_KEY_BACKSPACE;
                        break;
                    case INPUT_KEY_ENTER:
                        data->key = LV_KEY_ENTER;
                        break;
                    case INPUT_KEY_PREVIOUS:
                        data->key = LV_KEY_PREV;
                        break;
                    case INPUT_KEY_NEXT:
                        data->key = LV_KEY_NEXT;
                        break;
                    case INPUT_KEY_UP:
                        data->key = LV_KEY_UP;
                        break;
                    case INPUT_KEY_LEFT:
                        data->key = LV_KEY_LEFT;
                        break;
                    case INPUT_KEY_RIGHT:
                        data->key = LV_KEY_RIGHT;
                        break;
                    case INPUT_KEY_DOWN:
                        data->key = LV_KEY_DOWN;
                        break;
                    case INPUT_KEY_TAB:
                        data->key = LV_KEY_NEXT;
                        break;
                    default:
                        data->key = 0;
                        break;
                }
#endif /* USE_XKB */
                if (data->key != 0) {
                    /* Only record button state when actual output is produced to prevent widgets from refreshing */
                    data->state = (in.value) ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
                }
                evdev_key_val = data->key;
                evdev_button = data->state;
                return;
            }
        }
        if (in.type == INPUT_EVENT_ABS && in.value == 0) {
            dev_touch_area_update_state(evdev_root_x, evdev_root_y, LV_INDEV_STATE_REL);
        } else if (in.type == INPUT_EVENT_ABS
                   && (in.code == INPUT_ABS_MT_POSITION_X || in.code == INPUT_ABS_MT_POSITION_Y)
                   && (evdev_root_x != 0 && evdev_root_y != 0)) {
            dev_touch_area_update_state(evdev_root_x, evdev_root_y, LV_INDEV_STATE_PR);
        }

        //偶现触摸连续事件，press被过滤掉，导致无按压事件，这里针对 tp 021 序列特殊处理 
        static int s_filter_num = -1;
        // printf("sfilter_num :%d\n",s_filter_num);
        if(last_event_code == 54 && in.code == 0) {        //0 事件判断
            s_filter_num = 0;
        }

        last_event_code = in.code;

        if (s_filter_num == 0 && in.type == 0 && in.code == 0 && in.value == 0) {
            s_filter_num = 1;
        }else if (s_filter_num == 1 && in.type == 3 && in.code == 58 )  {
            if (in.value == 15) {
                s_filter_num = 2;
            }else {
                s_filter_num = -1;
            }
        }else if (s_filter_num == 2 && in.type == 3 && in.code == 58 )  {   
            if (in.value == 0) {        // 符合 0 2 1事件序列
                s_filter_num = 0;
                printf("process 021 event\n");
                break;
            }else {
                s_filter_num = -1;
            }
        }
    }

    if(drv->type == LV_INDEV_TYPE_KEYPAD) {
        /* No data retrieved */
        data->key = evdev_key_val;
        data->state = evdev_button;
        return;
    }
    if(drv->type != LV_INDEV_TYPE_POINTER)
        return ;
    /*Store the collected data*/

#if EVDEV_CALIBRATE
    data->point.x = map(evdev_root_x, EVDEV_HOR_MIN, EVDEV_HOR_MAX, 0, drv->disp->driver->hor_res);
    data->point.y = map(evdev_root_y, EVDEV_VER_MIN, EVDEV_VER_MAX, 0, drv->disp->driver->ver_res);
#else
    data->point.x = evdev_root_x;
    data->point.y = evdev_root_y;
    // data->point.x = evdev_root_y;
    // data->point.y = drv->disp->driver->ver_res - 1 - evdev_root_x;

#endif

    data->state = evdev_button;

    if(data->point.x < 0)
      data->point.x = 0;
    if(data->point.y < 0)
      data->point.y = 0;
    if(data->point.x >= drv->disp->driver->hor_res)
      data->point.x = drv->disp->driver->hor_res - 1;
    if(data->point.y >= drv->disp->driver->ver_res)
      data->point.y = drv->disp->driver->ver_res - 1;

    return ;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
int map(int x, int in_min, int in_max, int out_min, int out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

#endif
