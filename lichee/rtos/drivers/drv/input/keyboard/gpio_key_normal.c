/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the People's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#include <stdio.h>
#include <string.h>
#include "sunxi-input.h"
#include "gpio_key_normal.h"
#include <hal_gpio.h>
#include "script.h"
#include "osal/hal_cfg.h"

/*
***** sys_config ************
[normal-key]
nk_used = 1
; how many key io?
nk_num = 3
nk_gpio_0 = port:PA13<0><1><3><1>
nk_gpio_1 = port:PA27<0><1><3><1>
nk_gpio_2 = port:PA28<0><1><3><1>
*/

//#define SUNXIKBD_DEBUG
#ifdef SUNXIKBD_DEBUG
#define sunxiknl_info(fmt, args...) printf("%s()%d - " fmt, __func__, __LINE__, ##args)
#else
#define sunxiknl_info(fmt, args...)
#endif
#define sunxiknl_err(fmt, args...)   printf("[err]%s()%d - " fmt, __func__, __LINE__, ##args)
#define sunxiknl_alway(fmt, args...) printf("[alway]%s()%d - " fmt, __func__, __LINE__, ##args)

#define NORKEY_SCAN_CYCLE       10                         //ms
#define NORKEY_LONG_PRESS_TIME  (2000 / NORKEY_SCAN_CYCLE) //2s
#define NORKEY_LONG_PRESS_STILL (200 / NORKEY_SCAN_CYCLE)  //200ms
#define NORKEY_PRESS_JITTER     (20 / NORKEY_SCAN_CYCLE)   //20ms
#define NORKEY_LONG_PRESS_RE    1
#define NORKEY_SHORT_PRESS_RE   0

typedef struct _gpio_key_normal gpio_key_normal;
typedef void *(*fsm_key)(int /* value*/);

struct _gpio_key_normal {
    unsigned char key_num;

    unsigned char idle_value;
    unsigned char current_value;
    unsigned char income_value;
    unsigned char change_cnt;
    unsigned char long_cnt;

    fsm_key keystate;

    gpio_pin_t key[0]; // must by last
};

static struct sunxi_input_dev *dev_keynol = NULL;
static gpio_key_normal *g_key_context = NULL;

/**********************
 *   core static FUNCTIONS
 **********************/

static inline void input_report_nkey(struct sunxi_input_dev *dev, unsigned int code, int value)
{
    sunxi_input_event(dev, EV_KEY, code, value);
}

static inline int key_value_change(int value)
{
    if (g_key_context->current_value != value) {
        return 1;
    } else {
        g_key_context->change_cnt = 0;
        g_key_context->income_value = 0;
        return 0;
    }
}

static inline int key_value_jitter(int value)
{
    if (g_key_context->income_value == value) {
        g_key_context->change_cnt++;
        if (g_key_context->change_cnt >= NORKEY_PRESS_JITTER) {
            g_key_context->change_cnt = 0;
            g_key_context->current_value = value;
            return 1;
        } else {
            return 0;
        }
    } else {
        g_key_context->income_value = value;
        g_key_context->change_cnt = 0;
        return 0;
    }
}

static inline int key_value_is_release(int value)
{
    if (g_key_context->idle_value == value) {
        return 1;
    } else {
        return 0;
    }
}

static inline int key_release_type(void)
{
    if (g_key_context->long_cnt >= (NORKEY_LONG_PRESS_TIME - NORKEY_LONG_PRESS_STILL)) {
        g_key_context->long_cnt = 0;
        return NORKEY_LONG_PRESS_RE;
    } else {
        g_key_context->long_cnt = 0;
        return NORKEY_SHORT_PRESS_RE;
    }
}

static inline int key_value_long_press(void)
{
    g_key_context->long_cnt++;
    if (g_key_context->long_cnt >= NORKEY_LONG_PRESS_TIME) {
        g_key_context->long_cnt -= NORKEY_LONG_PRESS_STILL;
        return 1;
    } else {
        return 0;
    }
}

static inline int key_value_report(int type)
{
    input_report_nkey(dev_keynol, type, g_key_context->current_value);
    input_sync(dev_keynol);
}

static int key_value_get(void)
{
    gpio_data_t gpio_data = GPIO_DATA_LOW;
    int value = 0;

    for (int i = 0; i < g_key_context->key_num; i++) {
        hal_gpio_get_data(g_key_context->key[i], &gpio_data);
        value |= (gpio_data << i);
    }
    return value;
}

static inline int key_value_deal(int value)
{
    if (g_key_context->keystate != NULL) {
        g_key_context->keystate = g_key_context->keystate(value);
    }
}

/**********************
 *   fsm FUNCTIONS
 **********************/
static void *sunxi_normal_key_release(int value);
static void *sunxi_normal_key_press(int value);

static void *sunxi_normal_key_release(int value)
{
    if (key_value_change(value)) {
        if (key_value_jitter(value)) {
            sunxiknl_info("press\n");
            key_value_report(KEY_EVENT_PRESS);
            return sunxi_normal_key_press;
        }
    }

    return sunxi_normal_key_release;
}

static void *sunxi_normal_key_press(int value)
{
    if (key_value_change(value)) {
        if (key_value_jitter(value)) {
            if (key_value_is_release(value)) {
                if (key_release_type() == NORKEY_LONG_PRESS_RE) {
                    key_value_report(KEY_EVENT_LONG_RELEASE);
                    sunxiknl_info("long release\n");
                } else {
                    key_value_report(KEY_EVENT_SHORT_RELEASE);
                    sunxiknl_info("short release\n");
                }
                return sunxi_normal_key_release;
            } else {
                key_value_report(KEY_EVENT_PRESS);
                sunxiknl_info("other key\n");
            }
        }
    } else {
        if (key_value_long_press()) {
            sunxiknl_info("long press\n");
            key_value_report(KEY_EVENT_LONG_PRESS);
        }
    }

    return sunxi_normal_key_press;
}

static void sunxi_normal_key_scan(void *parm)
{
    int i = 0;

    int value;

    while (1) {
        value = key_value_get();

        sunxiknl_info("nk_value : %#x\n", value);

        key_value_deal(value);

        vTaskDelay(NORKEY_SCAN_CYCLE);
    }
}

static int sunxi_normal_key_hw_init(void)
{
#ifdef CONFIG_DRIVER_SYSCONFIG
#define NORMAL_KRY_NAME "normal-key"

    int ret = -1;
    unsigned value = 0;
    char kpname[16] = { 0 };
    user_gpio_set_t gpio_set = { 0 };

    ret = hal_cfg_get_keyvalue(NORMAL_KRY_NAME, "nk_used", &value, 1);
    if ((ret < 0) || (value == 0)) {
        sunxiknl_err("sysconfig get error!\n");
        return -1;
    }
    if (hal_cfg_get_keyvalue(NORMAL_KRY_NAME, "nk_num", &value, 1) < 0) {
        sunxiknl_err("sysconfig get num error!\n");
    }

    g_key_context = malloc(sizeof(gpio_key_normal) + value * sizeof(gpio_pin_t));
    if (g_key_context == NULL) {
        sunxiknl_err("g_key_context malloc error!\n");
        return -1;
    }
    memset(g_key_context, 0, (sizeof(gpio_key_normal) + value * sizeof(gpio_pin_t)));
    g_key_context->key_num = value;
    g_key_context->keystate = sunxi_normal_key_release;

    for (int i = 0; i < g_key_context->key_num; i++) {
        sprintf(kpname, "nk_gpio_%d", i);
        memset(&gpio_set, 0x00, sizeof(gpio_set));
        ret = hal_cfg_get_keyvalue(NORMAL_KRY_NAME, kpname, (unsigned *)&gpio_set,
                                   sizeof(user_gpio_set_t) >> 2);
        if (ret == 0) {
            g_key_context->key[i] = (gpio_set.port - 1) * 32 + gpio_set.port_num;
            hal_gpio_set_pull(g_key_context->key[i], gpio_set.pull);
            hal_gpio_set_direction(g_key_context->key[i], GPIO_DIRECTION_INPUT);
            hal_gpio_pinmux_set_function(g_key_context->key[i], GPIO_MUXSEL_IN);
            g_key_context->idle_value |= (gpio_set.data << i);
        }
    }
    g_key_context->current_value = g_key_context->idle_value;
    return 0;
#else
    sunxiknl_err("to do!\n");
    return -1;
#endif
}

/**********************
 *   global FUNCTIONS
 **********************/
int sunxi_normal_key_init(void)
{
    int i, j = 0;
    int ret = -1;
    int key_code = 0;

    if (dev_keynol != NULL) {
        sunxiknl_info("dev_keynol is ready \n");
        return 0;
    }

    //input dev init
    dev_keynol = sunxi_input_allocate_device();
    if (NULL == dev_keynol) {
        sunxiknl_err("allocate dev_keynol err\n");
        return -1;
    } else
        sunxiknl_info("allocate dev_keynol ok! \n");

    dev_keynol->name = NK_NAME;

    ret = sunxi_normal_key_hw_init();
    if (ret < 0) {
        sunxiknl_err("key_hw_init fail\n");
        return -1;
    }

    input_set_capability(dev_keynol, EV_KEY, KEY_EVENT_PRESS);
    input_set_capability(dev_keynol, EV_KEY, KEY_EVENT_SHORT_RELEASE);
    input_set_capability(dev_keynol, EV_KEY, KEY_EVENT_LONG_RELEASE);
    input_set_capability(dev_keynol, EV_KEY, KEY_EVENT_LONG_PRESS);

    ret = sunxi_input_register_device(dev_keynol);
    if (ret < 0)
        sunxiknl_err("sunxi_input_register dev_keynol err!.\n");
    else
        sunxiknl_info("sunxi_input_register dev_keynol ok! \n");

    portBASE_TYPE task_ret;
    task_ret = xTaskCreate(sunxi_normal_key_scan, (signed portCHAR *)"normal_key_scan_task", 1024,
                           NULL, 16, NULL);
    if (task_ret != pdPASS) {
        sunxiknl_err("dev_keynol task create err\n");
    }

    return 0;
}

/**********************
 *   demo
 **********************/
#ifdef CONFIG_NORMAL_KEY_TEST
#include <console.h>
#include <unistd.h>

char *nk_event_name[KEY_EVENT_MAX] = { "key_press", "short_key_relase", "long_key_release",
                                       "long_key_press" };

static void nk_demo_task(void *parm)
{
    // sunxi_input_init(); // should apply if app not init

    struct sunxi_input_event event;
    memset(&event, 0, sizeof(struct sunxi_input_event));

    int k_fd = sunxi_input_open(NK_NAME);
    if (k_fd < 0) {
        sunxiknl_alway("%s open fail?\n", NK_NAME);
    }
    while (1) {
        sunxi_input_readb(k_fd, &event, sizeof(struct sunxi_input_event));
        if (event.type == EV_KEY) {
            sunxiknl_alway(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
            sunxiknl_alway("get event code : %s\n", nk_event_name[event.code]);
            sunxiknl_alway("get event value : %#x\n", event.value);
        }
    }
}

static int nk_demo(int argc, char *argv[])
{
    int c = 0;
    optind = 0;
    TaskHandle_t nk_task_handle;

    while ((c = getopt(argc, argv, "pq")) != -1) {
        switch (c) {
        case 'p':
            xTaskCreate(nk_demo_task, (signed portCHAR *)"nk_demo_task", 1024, NULL, 16,
                        &nk_task_handle);
            break;
        case 'q':
            vTaskDelete(nk_task_handle);
            break;
        default:
            break;
        }
    }
}

FINSH_FUNCTION_EXPORT_CMD(nk_demo, get_nk, norkel demo);
#endif