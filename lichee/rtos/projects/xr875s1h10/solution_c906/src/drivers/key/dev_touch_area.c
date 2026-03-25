#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "kernel/os/os.h"
#include "mfbd/mfbd.h"
#include "../drv_log.h"

#include "dev_touch_area.h"

#define TOUCH_AREA_EVENT_ALL            "all"
#define TOUCH_AREA_EVENT_SLIDER_LEFT    "slider_left"
#define TOUCH_AREA_EVENT_SLIDER_RIGHT   "slider_right"
#define TOUCH_AREA_EVENT_SLIDER_UP      "slider_up"
#define TOUCH_AREA_EVENT_SLIDER_DOWN    "slider_down"

// 参数说明: 名称、下一个按钮、索引、过滤时间、重复时间、长按时间、多击间隔、最大点击次数、事件码...
MFBD_MBTN_DEFINE(area9_btn, NULL, TOUCH_AREA_9,
                0, 0, 100, 60, 2,
                TOUCH_AREA_EVENT_DOWN, TOUCH_AREA_EVENT_UP, TOUCH_AREA_EVENT_LONG,
                TOUCH_AREA_EVENT_DOUBLE);

MFBD_MBTN_DEFINE(area8_btn, &area9_btn, TOUCH_AREA_8,
                0, 0, 100, 60, 2,
                TOUCH_AREA_EVENT_DOWN, TOUCH_AREA_EVENT_UP, TOUCH_AREA_EVENT_LONG,
                TOUCH_AREA_EVENT_DOUBLE);

MFBD_MBTN_DEFINE(area7_btn, &area8_btn, TOUCH_AREA_7,
                0, 0, 100, 60, 2,
                TOUCH_AREA_EVENT_DOWN, TOUCH_AREA_EVENT_UP, TOUCH_AREA_EVENT_LONG,
                TOUCH_AREA_EVENT_DOUBLE);

MFBD_MBTN_DEFINE(area6_btn, &area7_btn, TOUCH_AREA_6,
                0, 0, 100, 60, 2,
                TOUCH_AREA_EVENT_DOWN, TOUCH_AREA_EVENT_UP, TOUCH_AREA_EVENT_LONG,
                TOUCH_AREA_EVENT_DOUBLE);

MFBD_MBTN_DEFINE(area5_btn, &area6_btn, TOUCH_AREA_5,
                0, 0, 100, 60, 2,
                TOUCH_AREA_EVENT_DOWN, TOUCH_AREA_EVENT_UP, TOUCH_AREA_EVENT_LONG,
                TOUCH_AREA_EVENT_DOUBLE);

MFBD_MBTN_DEFINE(area4_btn, &area5_btn, TOUCH_AREA_4,
                0, 0, 100, 60, 2,
                TOUCH_AREA_EVENT_DOWN, TOUCH_AREA_EVENT_UP, TOUCH_AREA_EVENT_LONG,
                TOUCH_AREA_EVENT_DOUBLE);

MFBD_MBTN_DEFINE(area3_btn, &area4_btn, TOUCH_AREA_3,
                0, 0, 100, 60, 2,
                TOUCH_AREA_EVENT_DOWN, TOUCH_AREA_EVENT_UP, TOUCH_AREA_EVENT_LONG,
                TOUCH_AREA_EVENT_DOUBLE);

MFBD_MBTN_DEFINE(area2_btn, &area3_btn, TOUCH_AREA_2,
                0, 0, 100, 60, 2,
                TOUCH_AREA_EVENT_DOWN, TOUCH_AREA_EVENT_UP, TOUCH_AREA_EVENT_LONG,
                TOUCH_AREA_EVENT_DOUBLE);

MFBD_MBTN_DEFINE(area1_btn, &area2_btn, TOUCH_AREA_1,
                0,                          // 过滤时间（消抖）
                0,                          // 重复触发时间（禁用）
                100,                        // 长按判定时间（50个扫描周期）
                60,                         // 多点击间隔时间（60个扫描周期内的点击视为连续）
                2,                          // 最大连续点击次数
                TOUCH_AREA_EVENT_DOWN,      // 按下事件
                TOUCH_AREA_EVENT_UP,        // 抬起事件
                TOUCH_AREA_EVENT_LONG,      // 长按事件
                TOUCH_AREA_EVENT_DOUBLE);   // 双击事件

// 定义透明触摸区域消息结构体
#define TOUCH_MSG_MAX_SIZE 64
#define TOUCH_QUEUE_MAX_NUM 10

typedef struct {
    char msg[TOUCH_MSG_MAX_SIZE];
} touch_area_msg_t;

static XR_OS_Queue_t g_touch_area_msg_queue;
static touch_area_pos_t g_touch_areas[TOUCH_AREA_MAX] = {};
static touch_area_pos_t s_touch_area_pos = {0};
static XR_OS_Thread_t g_touch_area_thread;
static XR_OS_Thread_t g_touch_area_upload_thread;
static int g_touch_area_enable = 0;
static touch_area_event_t s_touch_area_event = 0;
static touch_area_event_t s_touch_area_last_event = 0;
static uint32_t s_event_current_time = 0;
static uint32_t s_event_current_index = 0;

static int touch_area_send_message(char *msg);

// 检查指定区域是否被按下
static unsigned char touch_area_is_down(mfbd_btn_index_t btn_index)
{
    if (btn_index >= TOUCH_AREA_MAX || s_touch_area_event != TOUCH_AREA_EVENT_DOWN) {
        return 0;
    }

    // 判断当前触摸坐标是否在指定区域内
    const touch_area_pos_t *area = &g_touch_areas[btn_index];
    return (s_touch_area_pos.start.x > area->start.x && s_touch_area_pos.start.x <= area->end.x &&
            s_touch_area_pos.start.y > area->start.y && s_touch_area_pos.start.y <= area->end.y) ? 1 : 0;
}

// 事件报告回调函数
static void touch_area_report(mfbd_btn_index_t btn_index, mfbd_btn_code_t btn_value)
{
    char type[64] = {0};

    s_event_current_index = btn_index;

    if (0 != strcmp(g_touch_areas[btn_index].event, TOUCH_AREA_EVENT_ALL)) {
        return;
    }

    if (btn_value == TOUCH_AREA_EVENT_LONG) {
        s_event_current_time = 0;
        sprintf(type, "{\"type\":\"%s\",\"event\":\"%d\"}", g_touch_areas[btn_index].type, btn_value);
        touch_area_send_message(type);
        return;
    }

    if (btn_value == TOUCH_AREA_EVENT_DOWN) {
        s_event_current_time = XR_OS_GetTicks();
    } else {
        // 若当前时间仍在双击事件的过滤时长内，则直接返回不处理
        if (XR_OS_GetTicks() - s_event_current_time < 300) {
            if (btn_value == TOUCH_AREA_EVENT_DOUBLE) {
                s_event_current_time = 0;
                sprintf(type, "{\"type\":\"%s\",\"event\":\"%d\"}", g_touch_areas[btn_index].type, btn_value);
                touch_area_send_message(type);
            }
        }
    }
    s_touch_area_last_event = btn_value;
}

// 定义按钮组
MFBD_GROUP_DEFINE(touch_area_group,
    .is_btn_down_func = touch_area_is_down,
    .btn_value_report = touch_area_report,
    .btn_list_head = &area1_btn
);

static void touch_area_thread(void *arg)
{
    drv_logi("touch thread start");
    char type[64] = {0};
    g_touch_area_enable = 1;

    while (g_touch_area_enable) {
        mfbd_mbtn_scan(&touch_area_group);

        if (s_touch_area_last_event == TOUCH_AREA_EVENT_UP && s_event_current_time != 0
            && 0 == strcmp(g_touch_areas[s_event_current_index].event, TOUCH_AREA_EVENT_ALL)) {
            if (XR_OS_GetTicks() - s_event_current_time > 300) {
                s_event_current_time = 0;
                sprintf(type, "{\"type\":\"%s\",\"event\":\"%d\"}", g_touch_areas[s_event_current_index].type, TOUCH_AREA_EVENT_DOWN);
                touch_area_send_message(type);
            }
        }

        XR_OS_MSleep(5);
    }

    drv_logi("touch thread exit");
    XR_OS_ThreadDelete(&g_touch_area_thread);
}

static int touch_area_send_message(char *msg)
{
    touch_area_msg_t touch_msg;

    if (!XR_OS_QueueIsValid(&g_touch_area_msg_queue)) {
        drv_loge("queue is Valid.");
        return -1;
    }

    // 复制消息内容
    strncpy(touch_msg.msg, msg, TOUCH_MSG_MAX_SIZE - 1);
    touch_msg.msg[TOUCH_MSG_MAX_SIZE - 1] = '\0';
    
    // 发送消息到队列
    if (XR_OS_OK != XR_OS_QueueSend(&g_touch_area_msg_queue, &touch_msg, 10)) {
        drv_loge("send touch message failed");
        return -1;
    }
    
    return 0;
}

// 处理透明触摸区域消息的线程函数
static void touch_area_upload_thread(void *arg)
{
    touch_area_msg_t touch_msg;

    drv_logi("touch area upload thread start");
    g_touch_area_enable = 1;
    while (g_touch_area_enable) {
        if (XR_OS_OK == XR_OS_QueueReceive(&g_touch_area_msg_queue, &touch_msg, XR_OS_WAIT_FOREVER)) {
            drv_logi("touch msg:%s", touch_msg.msg);
        }
    }

    drv_logi("touch area upload thread exit");
    XR_OS_ThreadDelete(&g_touch_area_upload_thread);
}

int dev_touch_area_init(void)
{
    if (XR_OS_ThreadIsValid(&g_touch_area_thread)) {
        drv_loge("touch area thread already exist");
        return -1;
    }

    if (XR_OS_OK != XR_OS_ThreadCreate(&g_touch_area_thread,
                                        "touch_area_thread",
                                        touch_area_thread,
                                        NULL,
                                        XR_OS_PRIORITY_NORMAL,
                                        (2 * 1024))) {
        drv_loge("touch area thread create fail");
        XR_OS_QueueDelete(&g_touch_area_msg_queue);
        return -1;
    }

    // 创建消息队列
    if (XR_OS_OK != XR_OS_QueueCreate(&g_touch_area_msg_queue, 
                                     TOUCH_QUEUE_MAX_NUM,
                                     sizeof(touch_area_msg_t))) {
        drv_loge("create touch message queue fail");
        return -1;
    }

    if (XR_OS_ThreadIsValid(&g_touch_area_upload_thread)) {
        drv_loge("touch area upload thread already exist");
        return -1;
    }

    if (XR_OS_OK != XR_OS_ThreadCreate(&g_touch_area_upload_thread,
                                        "touch_area_upload_thread",
                                        touch_area_upload_thread,
                                        NULL,
                                        XR_OS_PRIORITY_NORMAL,
                                        (2 * 1024))) {
        drv_loge("touch area upload thread create fail");
        XR_OS_QueueDelete(&g_touch_area_msg_queue);
        return -1;
    }
    return 0;
}

int dev_touch_area_deinit(void)
{
    g_touch_area_enable = 0;
    
    // 延迟一段时间，让线程有机会退出
    XR_OS_MSleep(50);

    if (XR_OS_QueueIsValid(&g_touch_area_msg_queue)) {
        XR_OS_QueueDelete(&g_touch_area_msg_queue);
    }
    
    return 0;
}

// 从屏幕驱动更新触摸状态（需要在中断或定时任务中调用）
void dev_touch_area_update_state(int x, int y, int pressed)
{
    static uint8_t filterate_flag = 0;  //过滤标志，过滤第一个x值
    char type[64] = {0};

    drv_logd("x = %d---y = %d, event = %d\n", x, y, pressed);
    if (pressed) {
        if (s_touch_area_pos.start.x && s_touch_area_pos.start.y) {
            return;
        }

        if (filterate_flag == 0) {
            filterate_flag = 1;
            drv_logd("filterate_flag = %d\n", filterate_flag);
        } else {
            s_touch_area_pos.start.x = x;
            s_touch_area_pos.start.y = y;
            s_touch_area_event = TOUCH_AREA_EVENT_DOWN;
            drv_logd("x = %d---y = %d\n", s_touch_area_pos.start.x, s_touch_area_pos.start.y);
        }
    } else {
        if ((s_touch_area_pos.start.x != 0 && s_touch_area_pos.start.y != 0)
             && (s_touch_area_pos.end.x == 0 && s_touch_area_pos.end.y == 0)) {
            s_touch_area_pos.end.x = x;
            s_touch_area_pos.end.y = y;
            s_touch_area_event = TOUCH_AREA_EVENT_UP;
            drv_logd("X current_start = %d---current_end = %d\n", s_touch_area_pos.start.x, s_touch_area_pos.end.x);
            drv_logd("Y current_start = %d---current_end = %d\n", s_touch_area_pos.start.y, s_touch_area_pos.end.y);

            // 判断滑动事件（X/Y方向差值超过20）
            int y_diff = s_touch_area_pos.end.y - s_touch_area_pos.start.y;
            if (abs(y_diff) > 20) {
                for (int i = 0; i < TOUCH_AREA_MAX; i++) {
                    if (0 == strcmp(g_touch_areas[i].event, TOUCH_AREA_EVENT_SLIDER_UP)) {
                        if (s_touch_area_pos.start.y >= g_touch_areas[i].start.y && s_touch_area_pos.end.y <= g_touch_areas[i].end.y) {
                            sprintf(type, "{\"type\":\"%s\",\"event\":\"%d\"}", g_touch_areas[i].type, TOUCH_AREA_EVENT_SLIDER);
                            break;
                        }
                    }

                    if (0 == strcmp(g_touch_areas[i].event, TOUCH_AREA_EVENT_SLIDER_DOWN)) {
                        if (s_touch_area_pos.start.y <= g_touch_areas[i].start.y && s_touch_area_pos.end.y >= g_touch_areas[i].end.y) {
                            sprintf(type, "{\"type\":\"%s\",\"event\":\"%d\"}", g_touch_areas[i].type, TOUCH_AREA_EVENT_SLIDER);
                            break;
                        }
                    }

                    if (s_touch_area_pos.start.x >= g_touch_areas[i].start.x && s_touch_area_pos.end.x <= g_touch_areas[i].end.x
                        && s_touch_area_pos.start.y >= g_touch_areas[i].start.y && s_touch_area_pos.end.y <= g_touch_areas[i].end.y
                        && 0 == strcmp(g_touch_areas[i].event, TOUCH_AREA_EVENT_ALL)) {
                        sprintf(type, "{\"type\":\"%s\",\"event\":\"%d\"}", g_touch_areas[i].type, TOUCH_AREA_EVENT_SLIDER);
                        break;
                    }
                }
            }
        }

        if (strlen(type) != 0) {
            s_event_current_time = 0;
            touch_area_send_message(type);
        }
        s_touch_area_pos.start.x = 0;
        s_touch_area_pos.start.y = 0;
        s_touch_area_pos.end.x = 0;
        s_touch_area_pos.end.y = 0;
        filterate_flag = 0;
    }
}

int dev_touch_area_config(touch_area_pos_t *pos, int num)
{
    for (int i = 0; i < num; i++) {
        g_touch_areas[i].start.x = pos[i].start.x;
        g_touch_areas[i].start.y = pos[i].start.y;
        g_touch_areas[i].end.x = pos[i].end.x;
        g_touch_areas[i].end.y = pos[i].end.y;
        strcpy(g_touch_areas[i].type, pos[i].type);
        strcpy(g_touch_areas[i].event, pos[i].event);
    }
    return 0;
}
