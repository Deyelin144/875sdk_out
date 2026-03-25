#include <stdio.h>
#include "FreeRTOS.h"
#include <task.h>
#include "native_app_ota.h"
#include "../lv_port/lv_init.h"
#include "../src/os_adapter/os_adapter.h"
#include "../src/wifi_adapter/wifi_adapter.h"
#include "../src/upgrade_logic/upgrade_logic.h"
#include "../src/upgrade_logic/upgrade_fw.h"
#include "../src/upgrade_logic/upgrade_app.h"
#include "recovery_main.h"
#include "lvgl.h"
#include "kernel/os/os.h"
#include <string.h>
#include "ota_msg.h"

#define LV_BASE_WIDTH    320
#define LV_BASE_HEIGHT   240

#define SCAN_RESULT_MAX_NUM  30
#define FUNC_KEY_NUM  4
#define EVENT_LIST_NUM  5

typedef struct {
    lv_obj_t *main_cont_obj;

    // 初始页面
    lv_obj_t *start_obj;

    //加载页面
    lv_obj_t *loading_obj;

    // wifi列表页面
    lv_obj_t *wifi_obj;
    lv_obj_t *refresh_win;
    lv_obj_t *suc_win;
    lv_obj_t *connect_info;
    lv_obj_t *wifi_list;
    lv_obj_t *wifi_img;

    //wifi删除
    lv_obj_t *wifi_del_obj;

    //wifi详情
    lv_obj_t *wifi_info_obj;

    //连接失败
    lv_obj_t *con_fail_obj;

    // 键盘页面
    lv_obj_t *key_obj;
    lv_obj_t *kb_cont;
    lv_obj_t *textarea;

    //升级页面
    lv_obj_t *upgrade_obj;
    lv_obj_t *bar;
    lv_obj_t *progress_label;

    // 结束页面
    lv_obj_t *end_obj;

    lv_obj_t *neterr_obj;

    lv_obj_t *uperr_obj;

} ota_lv_obj_t;

typedef enum {
    OTA_PAGE_UNKNOW = 0,
    OTA_PAGE_START,      // 初始页面
    OTA_PAGE_LOADING,    // 加载页面
    OTA_PAGE_WIFI,       // wifi列表页面
    OTA_PAGE_WIFI_INFO,  // wifi详情页面
    OTA_PAGE_WIFI_DEL,   // wifi删除页面
    OTA_PAGE_CON_FAIL,   // 连接失败页面
    OTA_PAGE_KEYBOARD,   // 键盘页面
    OTA_PAGE_UPGRADE,    // 升级页面
    OTA_PAGE_END,        // 结束页面
    OTA_PAGE_NET_ERR,    // 网络异常页面
    OTA_PAGE_UP_ERR,     // 升级异常页面
    OTA_PAGE_MAX,
} ota_page_idx_t;

// 事件，页面跳转事件与ota_page_idx_t对应，在后面添加其余处理事件
typedef enum {
    OTA_EVENT_UNKNOW   = 0,
    OTA_EVENT_START,          // 开始页
    OTA_EVENT_LOADING,        // 加载页
    OTA_EVENT_WIFI,           // 显示wifi页面
    OTA_EVENT_WIFI_INFO,      // 显示wifi详情页面
    OTA_EVENT_WIFI_DEL,       // 显示wifi删除页面
    OTA_EVENT_CON_FAIL,       // 显示连接失败页面
    OTA_EVENT_KEY,            // 显示键盘
    OTA_EVENT_UPGRADE,        // 升级页面
    OTA_EVENT_END,            // 结束页面
    OTA_EVENT_NET_ERR,        // 网络异常页面
    OTA_EVENT_UP_ERR,         // 升级异常页面
// 以下是区别于页面跳转的其他事件
    OTA_EVENT_WIFI_FRESH,     // wifi列表信息刷新（用于更新wifi状态等信息，例如输入密码后更新为连接中状态）
    OTA_EVENT_WIFI_CONNECT,   // wifi连接
    OTA_EVENT_WIFI_TOP_WIN,   // wifi刷新弹窗，用户手动刷新，创建弹窗并释放扫描信号量，扫描成功后会自动触发列表刷新
    OTA_EVENT_KEY_UPDATE,     // 更新键盘
    OTA_EVENT_REBOOT,         // 重启 优先重启到正常模式，不行则重启到升级模式
} ota_event_t;

typedef struct {
    char *name;                     // 子页面名字
    void (*ota_page_enter_cb)(void); // 子页面的入口函数
} ota_page_t;

typedef enum {
  OTA_WIFI_CONNECT,   //已连接的wifi
  OTA_WIFI_SCAN,   //扫描到的wifi
  OTA_WIFI_SAVED,  //扫描到且本地已经保存的wifi
} ota_wifi_state_t;


typedef struct {
    wifi_info_t ap_info;
    ota_wifi_state_t state;
    wifi_sec_type_t sec_type;
    int rssi;
} ota_wifi_info_t;

typedef struct {
    ota_wifi_info_t *wifi_info;
    int num;
    int size;
} ota_wifi_list_t;

typedef struct {
    wifi_info_t cur_ap_info;         //当前连接
    wifi_scan_results_t scan_result; //扫描结果
    wifi_scan_results_t wifi_list;   //本地已保存且被扫描到的wifi
    wifi_list_t local_wifi_list;     //本地保存的wifi信息
    ota_wifi_list_t show_list;       //用于渲染的wifi列表
    int connected;
    int abort;
    int wifi_cb_event;
    int wifi_cb_prcossing;
    void *scan_tid;
    void *cb_tid;
    void *scan_sem;
    void *cb_sem;
    void *auto_tid;
    void *mutex;
} ota_wifi_handle_t;


static wifi_info_t s_wifi_con_info;

static void native_app_ota_unknow_page(void);
static void native_app_ota_start_page_init(void);
static void native_app_ota_loading_page_init(void);
static void native_app_ota_wifi_page_init(void);
static void native_app_ota_wifi_info_page_init(void);
static void native_app_ota_wifi_del_page_init(void);
static void native_app_ota_connect_fail_page_init(void);
static void native_app_ota_keybord_page_init(void);
static void native_app_ota_upgrade_page_init(void);
static void native_app_ota_end_page_init(void);
static void native_app_ota_neterr_page_init(void);
static void native_app_ota_uperr_page_init(void);

static void native_app_ota_wifi_list_create();
static void wifi_connect(char *ssid, char* pwd, wifi_sec_type_t sec_type);
static void native_app_ota_wifi_list_update(bool isScan);
static void native_app_ota_keybord_create(const char *key_list);
static void update_focus(int new_idx);
static void create_top_refresh_win();
static void native_app_ota_auto_connect();
static void create_top_suc_win(char *text);


static ota_page_t s_ota_page_handle[OTA_PAGE_MAX] = {
    [OTA_PAGE_UNKNOW]     = {"unknown",   native_app_ota_unknow_page},
    [OTA_PAGE_START]      = {"start",     native_app_ota_start_page_init},
    [OTA_PAGE_LOADING]    = {"loading",   native_app_ota_loading_page_init},
    [OTA_PAGE_WIFI]       = {"wifi",      native_app_ota_wifi_page_init},
    [OTA_PAGE_WIFI_INFO]  = {"wifi_info", native_app_ota_wifi_info_page_init},
    [OTA_PAGE_WIFI_DEL]   = {"wifi_del",  native_app_ota_wifi_del_page_init},
    [OTA_PAGE_CON_FAIL]   = {"con_fail",  native_app_ota_connect_fail_page_init},
    [OTA_PAGE_KEYBOARD]   = {"keyboard",  native_app_ota_keybord_page_init},
    [OTA_PAGE_UPGRADE]    = {"upgrade",   native_app_ota_upgrade_page_init},
    [OTA_PAGE_END]        = {"end",       native_app_ota_end_page_init},
    [OTA_PAGE_NET_ERR]    = {"net_err",   native_app_ota_neterr_page_init},
    [OTA_PAGE_UP_ERR]     = {"up_err",    native_app_ota_uperr_page_init},
};

static void *s_ota_even_mutex = NULL;
static void *s_ota_upgrade_tid = NULL;
// static lv_font_t *s_bin_font = NULL;
static lv_font_t *s_ft_font = NULL;
static ota_lv_obj_t *s_ota_lv_handle = NULL;
static ota_wifi_handle_t *s_wifi_handle = NULL;
static ota_page_idx_t s_cur_page_idx = OTA_PAGE_START;

static int up_num = 0;  // 升级次数（0、1、2）
static bool is_uppercase = false;  // 大小写切换标志
static bool is_symbol = false;     // 数字符号切换标志
const char *lower_keys = "abcdefghijklmnopqrstuvwxyz";
const char *upper_keys = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char *symbols = "1234567890,.?!-`\"':;\\/*#$@%^&+-= ()<>[]{}|~";
const char *func_keys[FUNC_KEY_NUM] = {"OK", "del", "A/a", "123"};
static ota_event_t s_event_list[EVENT_LIST_NUM] = {0};
static int s_event_num = 0;
static int s_encoder_enable = 0;
static int s_touchpad_enable = 0;
static bool s_fresh_upd_list = false; // 防止wifi列表重复刷新
static bool s_forbid_scan = false; // 部分情况下不刷新列表，直接显示之前的结果

static int lv_suit_screen_width(int x)
{
    // printf("(w)x: %d hor: %d base: %d\n", x, lv_disp_get_hor_res(NULL), LV_BASE_WIDTH);
    return x * lv_disp_get_hor_res(NULL) / LV_BASE_WIDTH;
}

static int lv_suit_screen_height(int x)
{
    // printf("(h)x: %d ver: %d base: %d\n", x, lv_disp_get_ver_res(NULL), LV_BASE_HEIGHT);
    return x * lv_disp_get_ver_res(NULL) / LV_BASE_HEIGHT;
}

static int lv_suit_screen_size(int x)
{
    int width = lv_suit_screen_width(x);
    int height = lv_suit_screen_height(x);
    return width < height? width : height;
}

static void event_lock()
{
    if (s_ota_even_mutex != NULL) {
        if (0 != os_adapter()->mutex_lock(s_ota_even_mutex, os_adapter()->get_forever_time())) {
            LOG_ERR("event mutex lock err\n");
        }
    }
}

static void event_unlock()
{
    if (s_ota_even_mutex != NULL) {
        if (0 != os_adapter()->mutex_unlock(s_ota_even_mutex)) {
            LOG_ERR("event mutex lock err\n");
        }
    }
}

static ota_event_t get_event()
{
    static int ptr = 0;
    ota_event_t event = OTA_EVENT_UNKNOW;

    if (s_event_num == 0) {
        goto exit;
    }

    event_lock();
    event = s_event_list[ptr];
    ptr = (ptr + 1) % EVENT_LIST_NUM;
    s_event_num--;
    event_unlock();
    printf("[%s %d] get event %d\n", __func__, __LINE__, event);

exit:
    return event;
}

static void set_event(ota_event_t event)
{
    static int ptr = 0;

    if (s_event_num >= EVENT_LIST_NUM) {
        LOG_ERR("max num!\n");
        return;
    }

    printf("[%s %d] add event %d\n", __func__, __LINE__, event);
    event_lock();
    s_event_list[ptr] = event;
    ptr = (ptr + 1) % EVENT_LIST_NUM;
    s_event_num++;
    event_unlock();
}

static void event_list_init()
{
    for (int i = 0; i < EVENT_LIST_NUM; i++) {
        s_event_list[i] = OTA_EVENT_UNKNOW;
    }
}

static void native_app_ota_clear_top_win()
{
    if (s_ota_lv_handle->refresh_win) {
        printf("change page,del refresh win\n");
        lv_obj_del(s_ota_lv_handle->refresh_win);
        s_ota_lv_handle->refresh_win = NULL;
    }

     if (s_ota_lv_handle->suc_win != NULL) {  
        printf("change page,del suc_win win\n");
        lv_obj_del(s_ota_lv_handle->suc_win);
        s_ota_lv_handle->suc_win = NULL;
    }
}

static void native_app_ota_refresh_progress(int progress)
{
    if(NULL == s_ota_lv_handle->upgrade_obj) {
        return;
    }

    lv_lock();
    lv_obj_clear_flag(lv_obj_get_parent(s_ota_lv_handle->bar), LV_OBJ_FLAG_HIDDEN);
    lv_bar_set_value(s_ota_lv_handle->bar, progress, LV_ANIM_OFF);
    lv_label_set_text_fmt(s_ota_lv_handle->progress_label, "%d%%", progress);
    lv_unlock();
}


static int native_app_ota_start_cb(upgrade_type_t type, upgrade_method_t method, upgrade_handle_t *hdl)
{
    printf("start!!\n");
    return 0;
}

static int native_app_ota_process_cb(upgrade_type_t type, upgrade_method_t method, upgrade_handle_t *hdl, int cur_size)
{
    int ret = -1;
    static int last_size = 0;

    if (hdl->fw_handle.need_upgrade == 1 && hdl->app_handle.need_upgrade == 1) {
        cur_size = cur_size / 2;
        if (up_num == 1) {
            cur_size += 50;
        }
    }

    printf("process_cb type[%d]  %d!!\n", type,cur_size);
    if (cur_size >= 0 && cur_size <= 100 && cur_size != last_size) {
        native_app_ota_refresh_progress(cur_size);
        last_size = cur_size;
        ret = 0;
    }

    return ret;
}

static int native_app_ota_finish_cb(upgrade_type_t type, upgrade_method_t method, upgrade_handle_t *hdl)
{
    up_num++;
    if (hdl->fw_handle.need_upgrade + hdl->app_handle.need_upgrade == up_num) {
        printf("end!!\n");
        set_event(OTA_EVENT_END);
        upgrade_logic_set_replace(hdl, 0);
        os_adapter()->msleep(2000);
        set_event(OTA_EVENT_REBOOT);
        hdl->bat_chk_succ = 0;
    }
    return 0;
}

static int native_app_ota_error_cb(upgrade_type_t type, upgrade_method_t method, upgrade_handle_t *hdl)
{
    LOG_ERR("err %d  %d!!\n", upgrade_logic_get_handle()->fw_handle.err, upgrade_logic_get_handle()->app_handle.err);
    if ((upgrade_logic_get_handle()->fw_handle.err == UPGRADE_ERR_HTTP_GET 
        || upgrade_logic_get_handle()->app_handle.err == UPGRADE_ERR_HTTP_GET) 
        && s_wifi_handle->connected == 2) {
        s_wifi_handle->connected = 0;
        wifi_adapter_disconnect();
    }
    set_event(OTA_EVENT_UP_ERR);
    hdl->bat_chk_succ = 0;
    return 0;
}

static void _ota_task(void *arg)
{
    int ret = -1;
    upgrade_handle_t *hdl = upgrade_logic_get_handle();

    up_num = 0;
    if (hdl->fw_handle.need_upgrade == 1 && hdl->fw_handle.upgrade_status == UPGRADE_SUCC) {
        up_num++;
    }

    if (hdl->app_handle.need_upgrade == 1 && hdl->app_handle.upgrade_status == UPGRADE_SUCC) {
        up_num++;
    }
    if (0 != upgrade_logic_fw_start()) {
        LOG_ERR("fw up err.ret = %d\n", ret);
        goto exit;
    }

    if (0 != upgrade_logic_app_start()) {
        LOG_ERR("app up err.ret = %d\n", ret);
    }

exit:
    os_adapter()->thread_delete(&s_ota_upgrade_tid);
}

static void native_app_ota_task()
{
    int ret = -1;
    if (s_ota_upgrade_tid != NULL) {
        printf("task running!\n");
        return;
    }

    ret = os_adapter()->thread_create(&s_ota_upgrade_tid,
                                            _ota_task,
                                            NULL,
                                            "_ota_task",
                                            OS_ADAPTER_PRIORITY_NORMAL + 1,
                                            1024 * 5);
    if (ret != 0) {
        LOG_ERR("create ota thread failed ret = %d.\n", ret);
        s_ota_upgrade_tid = NULL;
    }

}



static void native_app_ota_page_switch(ota_page_idx_t page)
{
    ota_page_idx_t cur_idx = s_cur_page_idx;
    ota_page_idx_t next_idx = page;

    if (cur_idx == next_idx) {
        printf("same page!\n");
        return;
    }

    printf("[%s %d] %d -> %d\n", __func__, __LINE__, cur_idx, next_idx);
    if (next_idx == OTA_PAGE_WIFI) {
        //更新列表信息，不要放在lv_lock里面
        if (!s_forbid_scan) {
            native_app_ota_wifi_list_update(false);
        } else {
            s_forbid_scan = false;
        }
    }
    lv_lock();
    switch(cur_idx) {
        case OTA_PAGE_START:
            lv_obj_clean(s_ota_lv_handle->start_obj);
            lv_obj_del(s_ota_lv_handle->start_obj);
            s_ota_lv_handle->start_obj = NULL;
            break;
        case OTA_PAGE_LOADING:
            lv_obj_clean(s_ota_lv_handle->loading_obj);
            lv_obj_del(s_ota_lv_handle->loading_obj);
            s_ota_lv_handle->loading_obj = NULL;
            break;
        case OTA_PAGE_WIFI:
            lv_obj_clean(s_ota_lv_handle->wifi_obj);
            lv_obj_del(s_ota_lv_handle->wifi_obj);
            s_ota_lv_handle->wifi_obj = NULL;
            break;
        case OTA_PAGE_WIFI_INFO:
            lv_obj_clean(s_ota_lv_handle->wifi_info_obj);
            lv_obj_del(s_ota_lv_handle->wifi_info_obj);
            s_ota_lv_handle->wifi_info_obj = NULL;
            break;
        case OTA_PAGE_WIFI_DEL:
            lv_obj_clean(s_ota_lv_handle->wifi_del_obj);
            lv_obj_del(s_ota_lv_handle->wifi_del_obj);
            s_ota_lv_handle->wifi_del_obj = NULL;
            break;
        case OTA_PAGE_CON_FAIL:
            lv_obj_clean(s_ota_lv_handle->con_fail_obj);
            lv_obj_del(s_ota_lv_handle->con_fail_obj);
            s_ota_lv_handle->con_fail_obj = NULL;
            break;
        case OTA_PAGE_KEYBOARD:
            lv_obj_clean(s_ota_lv_handle->key_obj);
            lv_obj_del(s_ota_lv_handle->key_obj);
            s_ota_lv_handle->key_obj = NULL;
            break;
        case OTA_PAGE_UPGRADE:
            lv_obj_clean(s_ota_lv_handle->upgrade_obj);
            lv_obj_del(s_ota_lv_handle->upgrade_obj);
            s_ota_lv_handle->upgrade_obj = NULL;
            break;
        case OTA_PAGE_END:
            lv_obj_clean(s_ota_lv_handle->end_obj);
            lv_obj_del(s_ota_lv_handle->end_obj);
            s_ota_lv_handle->end_obj = NULL;
            break;
        case OTA_PAGE_UP_ERR:
            lv_obj_clean(s_ota_lv_handle->uperr_obj);
            lv_obj_del(s_ota_lv_handle->uperr_obj);
            s_ota_lv_handle->uperr_obj = NULL;
            break;
        case OTA_PAGE_NET_ERR:
            lv_obj_clean(s_ota_lv_handle->neterr_obj);
            lv_obj_del(s_ota_lv_handle->neterr_obj);
            s_ota_lv_handle->neterr_obj = NULL;
            break;
        default:
            break;
        
    }

    s_cur_page_idx = next_idx;
    native_app_ota_clear_top_win();
    s_ota_page_handle[next_idx].ota_page_enter_cb();
    lv_unlock();
}

static void wifi_click_event_handle(ota_wifi_info_t *wifi)
{
    if (OTA_WIFI_CONNECT == wifi->state) {
        set_event(OTA_EVENT_WIFI_INFO);
    } else if (OTA_WIFI_SAVED == wifi->state) {
        memset(s_wifi_handle->cur_ap_info.ssid, 0, SSID_MAX_LEN + 1);
        memset(s_wifi_handle->cur_ap_info.pwd, 0, PWD_MAX_LEN + 1);
        strcpy(s_wifi_handle->cur_ap_info.ssid, wifi->ap_info.ssid);
        strcpy(s_wifi_handle->cur_ap_info.pwd, wifi->ap_info.pwd);
        s_wifi_handle->connected = 1;
        // printf("---> %s %s %s %s\n", s_wifi_handle->cur_ap_info.ssid, s_wifi_handle->cur_ap_info.pwd, wifi->ap_info.ssid, wifi->ap_info.pwd);
        s_fresh_upd_list = true;
        set_event(OTA_EVENT_WIFI_FRESH);
        set_event(OTA_EVENT_WIFI_CONNECT);
    } else if (OTA_WIFI_SCAN == wifi->state) {
        memset(s_wifi_con_info.ssid, 0, SSID_MAX_LEN + 1);
        memset(s_wifi_con_info.pwd, 0, PWD_MAX_LEN + 1);
        strcpy(s_wifi_con_info.ssid, wifi->ap_info.ssid);
        if (SECURITY_TYPE_OPEN == wifi->sec_type) {
            memset(s_wifi_handle->cur_ap_info.ssid, 0, SSID_MAX_LEN + 1);
            memset(s_wifi_handle->cur_ap_info.pwd, 0, PWD_MAX_LEN + 1);
            strcpy(s_wifi_handle->cur_ap_info.ssid, wifi->ap_info.ssid);
            s_wifi_handle->cur_ap_info.sec_type = wifi->sec_type;
            s_wifi_handle->connected = 1;
            s_fresh_upd_list = false;
            set_event(OTA_EVENT_WIFI_FRESH);
            set_event(OTA_EVENT_WIFI_CONNECT);
        } else {
            set_event(OTA_EVENT_KEY);
        }
    }
}

static void keybord_click_event_handle(char *key_txt)
{
    int is_func_key = 0;
    const char *pwd = NULL;

    for (int i = 0; i < FUNC_KEY_NUM; i++) {
        if (strcmp(key_txt, func_keys[i]) == 0) {
            is_func_key = 1;
            break;
        }
    }
    if (is_func_key == 0) {
        //长度判断
        if (strlen(lv_textarea_get_text(s_ota_lv_handle->textarea)) >= 64 ) {
            printf("pwd can not > 64 ");
            create_top_suc_win("密码不能超过64位");
            return;
        }

        lv_textarea_add_text(s_ota_lv_handle->textarea, key_txt);
        // printf("-----> %d  %s\n", strlen(lv_textarea_get_text(s_ota_lv_handle->textarea)), lv_textarea_get_text(s_ota_lv_handle->textarea));
        if (strlen(lv_textarea_get_text(s_ota_lv_handle->textarea)) >= 8) {
            lv_obj_set_style_bg_color(lv_obj_get_child(s_ota_lv_handle->kb_cont, 5), lv_color_hex(0x2094FA), 0);
            lv_obj_set_style_bg_opa(lv_obj_get_child(s_ota_lv_handle->kb_cont, 5), LV_OPA_100, 0);
        }
        if (strlen(lv_textarea_get_text(s_ota_lv_handle->textarea)) > 0) {
            lv_obj_set_style_text_opa(s_ota_lv_handle->textarea, LV_OPA_COVER, LV_PART_MAIN);
        }
    } else {
        if (0 == strcmp(key_txt, func_keys[0])) {
            if (strlen(lv_textarea_get_text(s_ota_lv_handle->textarea)) >= 8) {
                pwd = lv_textarea_get_text(s_ota_lv_handle->textarea);
                // printf("Text in textarea: %s\n", pwd);
                memset(s_wifi_handle->cur_ap_info.ssid, 0, SSID_MAX_LEN + 1);
                memset(s_wifi_handle->cur_ap_info.pwd, 0, PWD_MAX_LEN + 1);
                strcpy(s_wifi_handle->cur_ap_info.ssid, s_wifi_con_info.ssid);
                strcpy(s_wifi_handle->cur_ap_info.pwd, pwd);
                s_wifi_handle->cur_ap_info.sec_type = s_wifi_con_info.sec_type;
                s_wifi_handle->connected = 1;
                // s_forbid_scan = true;
                set_event(OTA_EVENT_WIFI);
                set_event(OTA_EVENT_WIFI_CONNECT);
            }
        } else if (0 == strcmp(key_txt, func_keys[1])) {
            lv_textarea_del_char(s_ota_lv_handle->textarea);
            if (strlen(lv_textarea_get_text(s_ota_lv_handle->textarea)) < 8) {
                lv_obj_set_style_bg_color(lv_obj_get_child(s_ota_lv_handle->kb_cont, 5), lv_color_hex(0xFFFFFF), 0);
                lv_obj_set_style_bg_opa(lv_obj_get_child(s_ota_lv_handle->kb_cont, 5), LV_OPA_40, 0);
            }
        } else if (0 == strcmp(key_txt, func_keys[2])) {
            if (is_symbol) {
                is_symbol = !is_symbol;
            } else {
                is_uppercase = !is_uppercase;
            }
            set_event(OTA_EVENT_KEY_UPDATE);
        } else if (0 == strcmp(key_txt, func_keys[3])) {
            if (!is_symbol) {
                is_symbol = true;
                set_event(OTA_EVENT_KEY_UPDATE);
            }
        }
    }
}

static void lv_event_cb(lv_event_t *event)
{
    lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t *obj = lv_event_get_target(event);
    lv_obj_t *user_data = lv_event_get_user_data(event);
    // uint32_t key = lv_event_get_key(event);
    uint32_t key = 0;
    printf("[%s %d] keybord event %d\n", __func__, __LINE__, code);

    char *key_txt = NULL;
    ota_wifi_info_t *wifi = NULL;
    int idx = lv_obj_get_index(obj);
    static int stop_click_event = 0;
    static int enter_pressed = 0;

    switch (code) {
        case LV_EVENT_PRESSED:
            enter_pressed = 1;
            break;
        case LV_EVENT_RELEASED:
            enter_pressed = 0;
            break;
        case LV_EVENT_FOCUSED:
            lv_keyboard_set_textarea(user_data, obj);
            lv_obj_clear_flag(user_data, LV_OBJ_FLAG_HIDDEN);
            break;
        case LV_EVENT_DEFOCUSED:
            lv_keyboard_set_textarea(user_data, NULL);
            lv_obj_add_flag(user_data, LV_OBJ_FLAG_HIDDEN);
            break;
        case LV_EVENT_CLICKED:
            if (stop_click_event == 1) {
                stop_click_event = 0;
                printf("[%s %d] stop click event!\n", __func__, __LINE__);
                break;
            }
            if (s_cur_page_idx == OTA_PAGE_WIFI) {
                if (s_ota_lv_handle->suc_win != NULL || s_ota_lv_handle->refresh_win != NULL) {
                    printf("the top win showing stop event!\n");
                    break;
                }

                wifi = (ota_wifi_info_t *)user_data;
                if ( NULL == wifi) {
                    set_event(OTA_EVENT_WIFI_TOP_WIN);
                } else {
                    wifi_click_event_handle(wifi);
                }
                break;
            }
            key_txt = (char *)user_data;
            printf("[%s %d] click %s\n", __func__, __LINE__, key_txt);
            if (0 == strcmp(key_txt, "del_page")) {
                set_event(OTA_EVENT_WIFI_DEL);
            } else if (0 == strcmp(key_txt, "del_wifi")) {
                s_wifi_handle->connected = 0;
                wifi_adapter_disconnect();
                wifi_adapter_del_save_wifi(s_wifi_handle->cur_ap_info.ssid);
                // set_event(OTA_EVENT_WIFI);
                set_event(OTA_EVENT_LOADING);
            } else if (0 == strcmp(key_txt, "cancel")) {
                // set_event(OTA_EVENT_WIFI);
                set_event(OTA_EVENT_LOADING);
            } else if (0 == strcmp(key_txt, "con_again")) {
                s_wifi_handle->connected = 1;
                s_forbid_scan = true;
                set_event(OTA_EVENT_WIFI);
                set_event(OTA_EVENT_WIFI_CONNECT);
            } else if (0 == strcmp(key_txt, "up_again")) {
                if (s_wifi_handle->connected == 2 || UPGRADE_METHOD_LOCAL == upgrade_logic_get_handle()->method) {
                    set_event(OTA_EVENT_UPGRADE);
                    os_adapter()->msleep(100);
                    printf("[%s %d] going to up\n", __func__, __LINE__);
                    native_app_ota_task();
                } else {
                    set_event(OTA_EVENT_NET_ERR);
                }
            } else if (0 == strcmp(key_txt, "go_for_wifi")) {
                set_event(OTA_EVENT_LOADING);
            } else if (0 == strcmp(key_txt, "reboot")) {
                if (enter_pressed == 1) {
                    printf("[%s %d] skip this click, press %d\n", __func__, __LINE__, enter_pressed);
                    enter_pressed = 0;
                } else {
                    set_event(OTA_EVENT_REBOOT);
                }
            } else if (0 == strcmp(key_txt, "back")) {
                // set_event(OTA_EVENT_WIFI);
                set_event(OTA_EVENT_LOADING);
            } else if (s_cur_page_idx == OTA_PAGE_KEYBOARD) {
                keybord_click_event_handle(key_txt);
            }
            break;
        case LV_EVENT_KEY:
            printf("[%s %d] key %d\n", __func__, __LINE__, key);
            if (s_cur_page_idx == OTA_PAGE_NET_ERR && key == LV_KEY_ENTER) {
                stop_click_event = 1;
                set_event(OTA_EVENT_LOADING);
            } else if(s_cur_page_idx == OTA_PAGE_WIFI){
                if (s_ota_lv_handle->suc_win != NULL || s_ota_lv_handle->refresh_win != NULL) {
                    printf("the top win showing stop event!\n");
                    break;
                }
                if (key == LV_KEY_ENTER) {
                    stop_click_event = 1;
                    wifi_click_event_handle((ota_wifi_info_t *)user_data);
                }  else if (0 != s_encoder_enable) {
                    if (key == LV_KEY_HOME) {
                        set_event(OTA_EVENT_WIFI_TOP_WIN);
                    } else {
                        if (key == LV_KEY_RIGHT) {
                            lv_group_focus_next(lv_group_get_default());
                        } else if (key == LV_KEY_LEFT) {
                            lv_group_focus_prev(lv_group_get_default());
                        }
                        lv_obj_scroll_to_view(lv_group_get_focused(lv_obj_get_group(obj)), LV_ANIM_ON);
                    }
                } else if (0 == s_encoder_enable) {
                    if (key == LV_KEY_RIGHT) {
                        set_event(OTA_EVENT_WIFI_TOP_WIN);
                    } else {
                        if (key == LV_KEY_DOWN) {
                            lv_group_focus_next(lv_group_get_default());
                        } else if (key == LV_KEY_UP) {
                            lv_group_focus_prev(lv_group_get_default());
                        }
                        lv_obj_scroll_to_view(lv_group_get_focused(lv_obj_get_group(obj)), LV_ANIM_ON);
                    }
                }
            } else if (s_cur_page_idx == OTA_PAGE_KEYBOARD) {
                key_txt = (char *)user_data;
                printf("[%s %d] click %s\n", __func__, __LINE__, key_txt);
                switch(key) {
                    case LV_KEY_DOWN:
                        update_focus(idx + 6);
                        break;
                    case LV_KEY_UP:
                        update_focus(idx - 6);
                        break;
                    case LV_KEY_LEFT:
                        update_focus(idx - 1);
                        break;
                    case LV_KEY_RIGHT:
                        update_focus(idx + 1);
                        break;
                    case LV_KEY_BACKSPACE:
                        // set_event(OTA_EVENT_WIFI);
                        set_event(OTA_EVENT_LOADING);
                        break;
                    case LV_KEY_ENTER:
                        stop_click_event = 1;
                        keybord_click_event_handle(key_txt);
                        break;
                    default:
                    break;
                }
            }
            break;
        default:
            break;
    }

    return;
}

/**
 * @brief  创建新的子页面, 其大小为屏幕大小
 * @param  parent 父级对象 
 * @return NULL:创建失败, 其他:创建成功
 */
static lv_obj_t *ota_page_create(lv_obj_t *parent)
{
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_set_size(obj, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
    lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_style_bg_color(obj, lv_color_black(), 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    return obj;
}

/**
 * @brief 创建父容器
 */
static void native_app_ota_main_cont_init(void)
{
    static lv_style_t main_style;

    s_ota_lv_handle->main_cont_obj = ota_page_create(lv_scr_act());
    lv_style_init(&main_style);
    lv_style_set_bg_opa(&main_style, LV_OPA_COVER); //透明度设置：不透明
    lv_style_set_bg_color(&main_style, lv_color_black());
    lv_style_set_text_color(&main_style, lv_color_white());
    lv_obj_add_style(s_ota_lv_handle->main_cont_obj, &main_style, LV_PART_MAIN);
}

/**
 * @brief 字体初始化
 */
static void native_app_ota_label_init(lv_obj_t *label)
{
    lv_obj_set_size(label, lv_suit_screen_width(300), LV_SIZE_CONTENT);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(label, s_ft_font, LV_PART_MAIN);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_opa(label, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_pad_all(label, 0, 0);
}

static void native_app_ota_unknow_page(void)
{
    LOG_ERR("unknow page.\n");
}

/**
 * @brief 创建初始页面
 */
static void native_app_ota_start_page_init(void)
{
    s_cur_page_idx = OTA_PAGE_START;

    s_ota_lv_handle->start_obj = ota_page_create(s_ota_lv_handle->main_cont_obj);
    lv_obj_t *spinner = lv_spinner_create(s_ota_lv_handle->start_obj, 3000, 200);
    lv_obj_set_size(spinner, lv_suit_screen_size(22), lv_suit_screen_size(22));
	lv_obj_set_style_arc_width(spinner, 3, LV_PART_MAIN);
    lv_obj_set_style_arc_color(spinner, lv_color_hex(0xD8D8D8), LV_PART_MAIN);

    lv_obj_align(spinner, LV_ALIGN_TOP_MID, 0, lv_suit_screen_height(92));
    lv_obj_set_style_arc_width(spinner, 3, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(spinner, lv_palette_main(LV_PALETTE_YELLOW), LV_PART_INDICATOR);
    lv_obj_t *label = lv_label_create(s_ota_lv_handle->start_obj);
    native_app_ota_label_init(label);
    lv_label_set_text(label, get_real_ptr(upgrade_logic_get_handle()->note.upgrade_preparing)); // 升级准备中，请稍侯~
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, lv_suit_screen_height(131));
    lv_obj_invalidate(lv_scr_act());
}

/**
 * @brief 显示加载页面
 */
static void native_app_ota_loading_page_init(void)
{
    s_ota_lv_handle->loading_obj = ota_page_create(s_ota_lv_handle->main_cont_obj);

    lv_obj_t *spinner = lv_spinner_create(s_ota_lv_handle->loading_obj, 3000, 30);
    lv_obj_set_size(spinner, lv_suit_screen_size(22), lv_suit_screen_size(22));
	lv_obj_set_style_arc_width(spinner, 3, LV_PART_MAIN);
    lv_obj_set_style_arc_color(spinner, lv_color_hex(0xD8D8D8), LV_PART_MAIN);
    lv_obj_align(spinner, LV_ALIGN_TOP_MID, 0, lv_suit_screen_height(92));

    lv_obj_set_style_arc_width(spinner, 3, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(spinner, lv_palette_main(LV_PALETTE_YELLOW), LV_PART_INDICATOR);

    lv_obj_t *label = lv_label_create(s_ota_lv_handle->loading_obj);
    native_app_ota_label_init(label);
    lv_label_set_text(label, get_real_ptr(upgrade_logic_get_handle()->note.loading)); // 加载中,请稍后...
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, lv_suit_screen_height(131));

    os_adapter()->signal_post(s_wifi_handle->scan_sem);
}


static void create_wifi_list_element(lv_obj_t *list_cont, ota_wifi_info_t *wifi, int idx)
{
    upgrade_handle_t *hdl = upgrade_logic_get_handle();

    lv_obj_t *btn = lv_btn_create(list_cont);
    lv_obj_remove_style_all(btn);
    lv_obj_set_size(btn, lv_suit_screen_width(316), 58); // 列表项固定高度
    lv_obj_set_style_radius(btn, 14, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_30, 0);

    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_width(btn, 2, LV_PART_MAIN | LV_STATE_FOCUSED); 
    lv_obj_set_style_border_opa(btn, LV_OPA_100, LV_PART_MAIN | LV_STATE_FOCUSED);

    lv_obj_t * lab = lv_label_create(btn);
    lv_obj_set_size(lab, lv_suit_screen_width(178), lv_suit_screen_height(28));
    lv_label_set_long_mode(lab, LV_LABEL_LONG_DOT);

    lv_obj_set_style_text_font(lab, s_ft_font, LV_PART_MAIN);
    lv_label_set_text_fmt(lab, "%s", wifi->ap_info.ssid);
    lv_obj_set_style_text_color(lab, lv_color_white(), LV_PART_MAIN);
    // lv_obj_align(lab, LV_ALIGN_LEFT_MID, 14, 0);
    lv_obj_set_pos(lab, lv_suit_screen_width(14), lv_suit_screen_height(13));
    lv_group_add_obj(lv_group_get_default(), btn);

    lv_obj_t *img = NULL;
    if (idx == 0 && s_wifi_handle->connected != 0) {
        s_ota_lv_handle->wifi_img = lv_img_create(btn);
        lv_obj_set_size(s_ota_lv_handle->wifi_img, 28, 28);
        lv_obj_align(s_ota_lv_handle->wifi_img, LV_ALIGN_LEFT_MID, lv_suit_screen_width(241), 0);
    } else {
        img = lv_img_create(btn);
        lv_obj_set_size(img, 28, 28);
        lv_obj_align(img, LV_ALIGN_LEFT_MID, lv_suit_screen_width(241), 0);
    }

    if (idx == 0) {
        if (s_wifi_handle->connected == 2) {
            if (strlen(hdl->img.ota_wifi_connected.path) > 0 && hdl->img.ota_wifi_connected.w > 0 && hdl->img.ota_wifi_connected.h > 0) {
                lv_img_set_src(s_ota_lv_handle->wifi_img, hdl->img.ota_wifi_connected.path);
                lv_obj_set_size(s_ota_lv_handle->wifi_img, hdl->img.ota_wifi_connected.w, hdl->img.ota_wifi_connected.h);
            } else {
                lv_img_set_src(s_ota_lv_handle->wifi_img, "P:"APP_DIR"/ota_wifi_connected.bin");
            }
        } else if (s_wifi_handle->connected == 1) {
            lv_obj_t *spinner = lv_spinner_create(s_ota_lv_handle->wifi_img, 3000, 30);
            lv_obj_set_size(spinner, 28, 28);
            lv_obj_set_style_arc_width(spinner, 3, LV_PART_MAIN);
            lv_obj_set_style_arc_color(spinner, lv_color_hex(0xD8D8D8), LV_PART_MAIN);
            lv_obj_align(spinner, LV_ALIGN_CENTER, 0, 0);

            lv_obj_set_style_arc_width(spinner, 3, LV_PART_INDICATOR);
            lv_obj_set_style_arc_color(spinner, lv_palette_main(LV_PALETTE_YELLOW), LV_PART_INDICATOR);
        } else {
            if (wifi->sec_type != SECURITY_TYPE_OPEN) {
                if (strlen(hdl->img.ota_wifi_lock.path) > 0 && hdl->img.ota_wifi_lock.w > 0 && hdl->img.ota_wifi_lock.h > 0) {
                    lv_img_set_src(img, hdl->img.ota_wifi_lock.path);
                    lv_obj_set_size(img, hdl->img.ota_wifi_lock.w, hdl->img.ota_wifi_lock.h);
                } else {
                    lv_img_set_src(img, "P:"APP_DIR"/ota_wifi_lock.bin");
                }
            }
        }
    } else {
        if (wifi->sec_type != SECURITY_TYPE_OPEN) {
            if (strlen(hdl->img.ota_wifi_lock.path) > 0 && hdl->img.ota_wifi_lock.w > 0 && hdl->img.ota_wifi_lock.h > 0) {
                lv_img_set_src(img, hdl->img.ota_wifi_lock.path);
                lv_obj_set_size(img, hdl->img.ota_wifi_lock.w, hdl->img.ota_wifi_lock.h);
            } else {
                lv_img_set_src(img, "P:"APP_DIR"/ota_wifi_lock.bin");
            }
        }
    }

    img = lv_img_create(btn);
    lv_obj_set_size(img, 28, 28);
    lv_obj_align(img, LV_ALIGN_LEFT_MID, lv_suit_screen_width(274), 0);

#define WIFI_RSSI_HIGH (-55)
#define WIFI_RSSI_LOW (-70)
    if (wifi->rssi >= WIFI_RSSI_HIGH) {
        if (strlen(hdl->img.ota_wifi_3.path) > 0 && hdl->img.ota_wifi_3.w > 0 && hdl->img.ota_wifi_3.h > 0) {
            lv_img_set_src(img, hdl->img.ota_wifi_3.path);
            lv_obj_set_size(img, hdl->img.ota_wifi_3.w, hdl->img.ota_wifi_3.h);
        } else {
            lv_img_set_src(img, "P:"APP_DIR"/ota_wifi_3.bin");
        }
    } else if (wifi->rssi < WIFI_RSSI_LOW) {
        if (strlen(hdl->img.ota_wifi_1.path) > 0 && hdl->img.ota_wifi_1.w > 0 && hdl->img.ota_wifi_1.h > 0) {
            lv_img_set_src(img, hdl->img.ota_wifi_1.path);
            lv_obj_set_size(img, hdl->img.ota_wifi_1.w, hdl->img.ota_wifi_1.h);
        } else {
            lv_img_set_src(img, "P:"APP_DIR"/ota_wifi_1.bin");
        }
    } else {
        if (strlen(hdl->img.ota_wifi_2.path) > 0 && hdl->img.ota_wifi_2.w > 0 && hdl->img.ota_wifi_2.h > 0) {
            lv_img_set_src(img, hdl->img.ota_wifi_2.path);
            lv_obj_set_size(img, hdl->img.ota_wifi_2.w, hdl->img.ota_wifi_2.h);
        } else {
            lv_img_set_src(img, "P:"APP_DIR"/ota_wifi_2.bin");
        }
    }

    lv_obj_add_event_cb(btn, lv_event_cb, LV_EVENT_KEY, (void *)(wifi));
    lv_obj_add_event_cb(btn, lv_event_cb, LV_EVENT_CLICKED, (void *)(wifi));
}


//创建wifi信息列表
static void native_app_ota_wifi_list_create()
{
    if (s_ota_lv_handle->wifi_obj == NULL) {
        LOG_ERR("s_ota_lv_handle->wifi_obj is NULL\n");
        return;
    }
    
    int i = 0;
    lv_obj_t *label = NULL;
    // lv_obj_t *list_cont = NULL;

    lv_img_cache_invalidate_src(NULL);
    lv_img_cache_set_size(20);

    if (s_ota_lv_handle->wifi_list != NULL) {
        lv_obj_clean(s_ota_lv_handle->wifi_list);
        lv_obj_del(s_ota_lv_handle->wifi_list);
        s_ota_lv_handle->wifi_list = NULL;
    }

    s_ota_lv_handle->wifi_list = lv_list_create(s_ota_lv_handle->wifi_obj);
    lv_obj_remove_style_all(s_ota_lv_handle->wifi_list);
    lv_obj_set_size(s_ota_lv_handle->wifi_list, lv_suit_screen_width(320), lv_suit_screen_height(200));
    lv_obj_set_pos(s_ota_lv_handle->wifi_list, 0, lv_suit_screen_height(46));
    lv_obj_set_style_pad_all(s_ota_lv_handle->wifi_list, 0, 0);
    lv_obj_set_style_bg_color(s_ota_lv_handle->wifi_list, lv_color_black(), 0);
    lv_obj_set_style_border_width(s_ota_lv_handle->wifi_list, 0, 0);
    lv_obj_align(s_ota_lv_handle->wifi_list, LV_ALIGN_TOP_MID, 0, lv_suit_screen_height(46));
    lv_obj_set_style_width(s_ota_lv_handle->wifi_list, 0, LV_PART_SCROLLBAR);
    lv_obj_set_flex_flow(s_ota_lv_handle->wifi_list, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_style_flex_main_place(s_ota_lv_handle->wifi_list, LV_FLEX_ALIGN_START, 0);
    lv_obj_set_scrollbar_mode(s_ota_lv_handle->wifi_list, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_row(s_ota_lv_handle->wifi_list, 2, 0);
    lv_obj_set_style_pad_column(s_ota_lv_handle->wifi_list, 2, 0);

    if (s_wifi_handle->connected != 0) {
        lv_obj_t *spacer = lv_obj_create(s_ota_lv_handle->wifi_list);
        lv_obj_set_size(spacer, lv_suit_screen_width(320), lv_suit_screen_height(32));
        lv_obj_clear_flag(spacer, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(spacer, 0, 0);
        lv_obj_set_style_width(spacer, 0, LV_PART_SCROLLBAR);
        lv_obj_set_style_pad_all(spacer, 0, 0);
        // lv_obj_set_style_pad_bottom(spacer, 11, 0);
        lv_obj_align(spacer, LV_ALIGN_TOP_LEFT, 0, 0);

        s_ota_lv_handle->connect_info = lv_label_create(spacer);
        lv_obj_set_style_text_font(s_ota_lv_handle->connect_info, s_ft_font, LV_PART_MAIN);
        if (s_wifi_handle->connected == 1) {
            lv_label_set_text(s_ota_lv_handle->connect_info, get_real_ptr(upgrade_logic_get_handle()->note.wifi_connecting)); // 正在连接...
        } else {
            lv_label_set_text(s_ota_lv_handle->connect_info, get_real_ptr(upgrade_logic_get_handle()->note.wifi_list_connected)); // 已连接
        }
        lv_obj_set_style_text_opa(s_ota_lv_handle->connect_info, LV_OPA_60, LV_PART_MAIN);
        lv_obj_set_style_text_color(s_ota_lv_handle->connect_info, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_width(s_ota_lv_handle->connect_info, LV_SIZE_CONTENT);
        lv_obj_align(s_ota_lv_handle->connect_info, LV_ALIGN_TOP_LEFT, lv_suit_screen_width(16), 0);

        create_wifi_list_element(s_ota_lv_handle->wifi_list, &s_wifi_handle->show_list.wifi_info[i], i);
        i++;
    }

    if (s_wifi_handle->wifi_list.num != 0) {
        lv_obj_t *spacer = lv_obj_create(s_ota_lv_handle->wifi_list);
        lv_obj_set_size(spacer, lv_suit_screen_width(320), lv_suit_screen_height(32));
        lv_obj_clear_flag(spacer, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(spacer, 0, 0);
        lv_obj_set_style_width(spacer, 0, LV_PART_SCROLLBAR);
        lv_obj_set_style_pad_all(spacer, 0, 0);

        label = lv_label_create(spacer);
        lv_obj_set_style_text_font(label, s_ft_font, LV_PART_MAIN);
        // lv_label_set_text(label, "已保存");
        lv_label_set_text(label, get_real_ptr(upgrade_logic_get_handle()->note.wifi_saved));
        lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);
        lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_width(label, LV_SIZE_CONTENT);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, lv_suit_screen_width(16), 0);

        for(int j = 0; j < s_wifi_handle->wifi_list.num; j++) {
            i+=j;
            if (s_wifi_handle->show_list.wifi_info[i].state != OTA_WIFI_SAVED) {
                continue;
            }
            // printf("create list %s %s\n", s_wifi_handle->show_list.wifi_info[i].ap_info.ssid, s_wifi_handle->show_list.wifi_info[i].ap_info.pwd);
            create_wifi_list_element(s_ota_lv_handle->wifi_list, &s_wifi_handle->show_list.wifi_info[i], i);
        }
    }

    printf("wifi scan num %d\n", s_wifi_handle->scan_result.num);
    lv_obj_t *spacer = lv_obj_create(s_ota_lv_handle->wifi_list);
    lv_obj_set_size(spacer, lv_suit_screen_width(320), lv_suit_screen_height(32));
    lv_obj_clear_flag(spacer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(spacer, 0, 0);
    lv_obj_set_style_width(spacer, 0, LV_PART_SCROLLBAR);
    lv_obj_set_style_pad_all(spacer, 0, 0);

    //可用2.4G网络
    label = lv_label_create(spacer);
    lv_obj_set_size(label, lv_suit_screen_width(300), LV_SIZE_CONTENT);
    lv_obj_set_style_text_font(label, s_ft_font, LV_PART_MAIN);
    lv_label_set_text(label, get_real_ptr(upgrade_logic_get_handle()->note.wifi_only_2g));//可用2.4G网络
    lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_width(label, LV_SIZE_CONTENT);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, lv_suit_screen_width(16), 0);

    //刷新
    lv_obj_t *btn_refresh = lv_btn_create(spacer);
    lv_obj_remove_style_all(btn_refresh);
    lv_obj_set_size(btn_refresh, lv_suit_screen_width(68), lv_suit_screen_height(32));
    lv_obj_align(btn_refresh, LV_ALIGN_RIGHT_MID, lv_suit_screen_width(-10), 0);
    lv_obj_set_style_radius(btn_refresh, 23, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn_refresh, lv_color_hex(0x366BF9), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn_refresh, LV_OPA_20, LV_PART_MAIN);
    lv_obj_set_style_pad_all(btn_refresh, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(btn_refresh, lv_event_cb, LV_EVENT_CLICKED, NULL);

    label = lv_label_create(btn_refresh);
    lv_obj_set_size(label, lv_suit_screen_width(68), LV_SIZE_CONTENT);
    lv_obj_set_style_text_font(label, s_ft_font, LV_PART_MAIN);
    lv_label_set_text(label, get_real_ptr(upgrade_logic_get_handle()->note.wifi_fresh));
    lv_obj_set_style_text_color(label, lv_color_hex(0x366BF9),0);	//字体颜色
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_ALIGN_DEFAULT);	
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    for(; i < s_wifi_handle->show_list.num; i++) {
        if (s_wifi_handle->show_list.wifi_info[i].state != OTA_WIFI_SCAN) {
            continue;
        }
        create_wifi_list_element(s_ota_lv_handle->wifi_list, &s_wifi_handle->show_list.wifi_info[i], i);
    }

    lv_group_focus_obj(lv_obj_get_child(s_ota_lv_handle->wifi_list, 0));
}

static void win_close_timer_cb(lv_timer_t * timer)
{
    printf("timer callback is called!\n");
    if (s_ota_lv_handle->suc_win != NULL) {
        printf("del resuc win!\n");
        lv_obj_del(s_ota_lv_handle->suc_win);
        s_ota_lv_handle->suc_win = NULL;
    }

    lv_timer_del(timer);
}

static void create_top_suc_win(char *text)
{
    printf("create win %s\n", text);
    if (s_ota_lv_handle->suc_win != NULL) {
        printf("win exist\n");
        return;
    }
    // s_ota_lv_handle->suc_win = lv_obj_create(s_ota_lv_handle->wifi_obj);
    s_ota_lv_handle->suc_win = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(s_ota_lv_handle->suc_win);
    
    lv_obj_set_size(s_ota_lv_handle->suc_win, LV_SIZE_CONTENT, lv_suit_screen_height(48));
    lv_obj_align(s_ota_lv_handle->suc_win, LV_ALIGN_TOP_MID, 0, lv_suit_screen_height(8));
    lv_obj_set_style_bg_color(s_ota_lv_handle->suc_win, lv_color_hex(0x2094FA), 0);
    lv_obj_set_style_bg_opa(s_ota_lv_handle->suc_win, LV_OPA_90, 0);
    
    lv_obj_set_style_border_width(s_ota_lv_handle->suc_win, 0, 0);
    lv_obj_set_style_radius(s_ota_lv_handle->suc_win, lv_suit_screen_width(24), 0);

    lv_obj_t * label = lv_label_create(s_ota_lv_handle->suc_win);
    lv_obj_set_style_text_font(label, s_ft_font, LV_PART_MAIN);
    lv_obj_set_size(label, LV_SIZE_CONTENT, lv_suit_screen_height(48));
    lv_obj_set_style_pad_right(label, lv_suit_screen_width(16), 0);
    lv_obj_set_style_pad_left(label, lv_suit_screen_width(16), 0);
    lv_obj_set_style_pad_top(label, lv_suit_screen_height(12), 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_label_set_text(label, text);

    lv_timer_create(win_close_timer_cb, 1000, NULL);
}


static void create_top_refresh_win()
{
    if (s_cur_page_idx != OTA_PAGE_WIFI) {
        LOG_ERR("cur win no wifi page,no refresh\n");
        return;
    }
    if (s_ota_lv_handle->refresh_win) {
        printf("refresh win exist\n");
        return;
    }
    // s_ota_lv_handle->refresh_win = lv_obj_create(s_ota_lv_handle->wifi_obj);
    s_ota_lv_handle->refresh_win = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(s_ota_lv_handle->refresh_win);
    
    lv_obj_set_size(s_ota_lv_handle->refresh_win, LV_SIZE_CONTENT, lv_suit_screen_height(48));
    lv_obj_align(s_ota_lv_handle->refresh_win, LV_ALIGN_TOP_MID, 0, lv_suit_screen_height(8));
    lv_obj_set_style_bg_color(s_ota_lv_handle->refresh_win, lv_color_hex(0x323232), 0);
    lv_obj_set_style_bg_opa(s_ota_lv_handle->refresh_win, LV_OPA_100, 0);
    
    lv_obj_set_style_border_width(s_ota_lv_handle->refresh_win, 0, 0);
    lv_obj_set_style_radius(s_ota_lv_handle->refresh_win, 24, 0);

    lv_obj_t *spinner = lv_spinner_create(s_ota_lv_handle->refresh_win, 2000, 30);
    lv_obj_set_size(spinner, lv_suit_screen_size(28), lv_suit_screen_size(28));
	lv_obj_set_style_arc_width(spinner, lv_suit_screen_height(5), LV_PART_MAIN);
    lv_obj_align(spinner, LV_ALIGN_LEFT_MID, lv_suit_screen_width(16), 0);
    lv_obj_set_style_arc_width(spinner, lv_suit_screen_height(3), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(spinner, lv_palette_main(LV_PALETTE_YELLOW), LV_PART_INDICATOR);

    lv_obj_t * label = lv_label_create(s_ota_lv_handle->refresh_win);
    lv_obj_set_style_text_font(label, s_ft_font, LV_PART_MAIN);
    lv_obj_set_size(label, LV_SIZE_CONTENT, lv_suit_screen_height(48));
    lv_obj_set_style_pad_right(label, lv_suit_screen_width(16), 0);
    lv_obj_set_style_pad_left(label, lv_suit_screen_width(51), 0);
    lv_obj_set_style_pad_top(label, lv_suit_screen_height(14), 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    // lv_label_set_text(label, "刷新中,请稍侯...");
    lv_label_set_text(label, get_real_ptr(upgrade_logic_get_handle()->note.wifi_freshing));
}

/**
 * @brief 创建wifi详情页面
 */
static void native_app_ota_wifi_info_page_init(void)
{
    upgrade_handle_t *hdl = upgrade_logic_get_handle();

    s_cur_page_idx = OTA_PAGE_WIFI_INFO;
    printf("wifi info page.\n");

    s_ota_lv_handle->wifi_info_obj = ota_page_create(s_ota_lv_handle->main_cont_obj);

    lv_obj_t *circle = lv_obj_create(s_ota_lv_handle->wifi_info_obj);
    lv_obj_set_scrollbar_mode(circle, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_size(circle, lv_suit_screen_size(80), lv_suit_screen_size(80));
    lv_obj_align(circle, LV_ALIGN_TOP_MID, 0, lv_suit_screen_height(21));
    lv_obj_set_style_bg_opa(circle, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(circle, lv_color_hex(0x04DE71), 0);

    lv_obj_t *img = lv_img_create(circle);
    lv_obj_set_scrollbar_mode(img, LV_SCROLLBAR_MODE_OFF);
    if (strlen(hdl->img.ota_wifi_info.path) > 0 && hdl->img.ota_wifi_info.w > 0 && hdl->img.ota_wifi_info.h > 0) {
        lv_img_set_src(img, hdl->img.ota_wifi_info.path);
        lv_obj_set_size(img, hdl->img.ota_wifi_info.w, hdl->img.ota_wifi_info.h);
    } else {
        lv_img_set_src(img, "P:"APP_DIR"/ota_wifi_info.bin");
        lv_obj_set_size(img, 48, 48);
    }
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *label = lv_label_create(s_ota_lv_handle->wifi_info_obj);
    lv_obj_set_height(label, lv_suit_screen_height(28));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, lv_suit_screen_height(107));
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_opa(label, LV_OPA_COVER, 0);
    // lv_label_set_text(label, "网络名称");
    lv_label_set_text_fmt(label, "%s", s_wifi_handle->cur_ap_info.ssid);
    lv_obj_set_style_text_font(label, s_ft_font, 0);

    label = lv_label_create(s_ota_lv_handle->wifi_info_obj);
    lv_obj_set_height(label, lv_suit_screen_height(26));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, lv_suit_screen_height(139));
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_opa(label, LV_OPA_COVER, 0);
    // lv_label_set_recolor(label, true);
    // lv_label_set_text(label, "已连接");
    lv_obj_set_style_text_color(label, lv_color_hex(0x04DE71), 0);
    lv_label_set_text(label, get_real_ptr(upgrade_logic_get_handle()->note.wifi_connected));
    lv_obj_set_style_text_font(label, s_ft_font, 0);

    lv_obj_t *btn_cont = lv_obj_create(s_ota_lv_handle->wifi_info_obj);
    lv_obj_set_style_bg_opa(btn_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_cont, 0, 0);
    lv_obj_align(btn_cont, LV_ALIGN_TOP_MID, 0, lv_suit_screen_height(178));
    lv_obj_set_size(btn_cont, lv_suit_screen_width(320), lv_suit_screen_height(60));
    lv_obj_set_style_pad_all(btn_cont, 0, LV_PART_MAIN);

    lv_obj_t *btn_del = lv_btn_create(btn_cont);
    lv_obj_remove_style_all(btn_del);
    lv_obj_set_size(btn_del, lv_suit_screen_width(304), lv_suit_screen_height(54));
    lv_obj_align(btn_del, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_radius(btn_del, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn_del, lv_color_hex(0xFF3B30), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn_del, LV_OPA_30, LV_PART_MAIN);
    lv_obj_set_style_pad_all(btn_del, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(btn_del, lv_event_cb, LV_EVENT_CLICKED, "del_page");

    lv_obj_set_style_border_width(btn_del, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(btn_del, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_width(btn_del, 2, LV_PART_MAIN | LV_STATE_FOCUSED); 
    lv_obj_set_style_border_opa(btn_del, LV_OPA_100, LV_PART_MAIN | LV_STATE_FOCUSED);

    label = lv_label_create(btn_del);
    lv_obj_set_style_text_font(label, s_ft_font, LV_PART_MAIN);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_ALIGN_DEFAULT);
    lv_obj_center(label);
    // lv_label_set_recolor(label, true);
    // lv_label_set_text_fmt(label, "移除此网络");
    lv_obj_set_style_text_color(label, lv_color_hex(0xFF3B30), 0);
    lv_label_set_text_fmt(label, get_real_ptr(upgrade_logic_get_handle()->note.wifi_rm_this_net));

    lv_gridnav_add(btn_cont, LV_GRIDNAV_CTRL_ROLLOVER);
    lv_group_add_obj(lv_group_get_default(), btn_cont);
    lv_group_focus_obj(btn_cont);
}


/**
 * @brief 创建连接失败页面
 */
static void native_app_ota_connect_fail_page_init(void)
{
    upgrade_handle_t *hdl = upgrade_logic_get_handle();

    s_cur_page_idx = OTA_PAGE_CON_FAIL;
    printf("con fail page.\n");

    s_ota_lv_handle->con_fail_obj = ota_page_create(s_ota_lv_handle->main_cont_obj);
   
    //眼睛
    lv_obj_t *obj =  lv_obj_create(s_ota_lv_handle->con_fail_obj);
    lv_obj_set_size(obj, 95, 75);
    lv_obj_align(obj, LV_ALIGN_TOP_LEFT, lv_suit_screen_height(40), lv_suit_screen_height(40));
    lv_obj_set_style_bg_opa(obj, 0, 0);
    lv_obj_set_style_border_width(obj,0,0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *img = lv_img_create(obj);
    lv_img_set_src(img, "P:"APP_DIR"/ota_net_eys.bin"); 
    lv_obj_set_size(img, 95, 75);  
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_img_recolor(img, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_img_recolor_opa(img, LV_OPA_100, 0); 
    lv_obj_clear_flag(img, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_img_tiled(img, false, 0);  // 禁用平铺

    obj =  lv_obj_create(s_ota_lv_handle->con_fail_obj);
    lv_obj_set_size(obj, 95, 75);
    lv_obj_align(obj, LV_ALIGN_TOP_RIGHT, lv_suit_screen_height(-40), lv_suit_screen_height(40));
    lv_obj_set_style_bg_opa(obj, 0, 0);
    lv_obj_set_style_border_width(obj,0,0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    img = lv_img_create(obj);
    lv_img_set_src(img, "P:"APP_DIR"/ota_net_eys2.bin"); 
    lv_obj_set_size(img, 95, 75);  
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_img_recolor(img, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_img_recolor_opa(img, LV_OPA_100, 0); 
    lv_obj_clear_flag(img, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_img_tiled(img, false, 0);  // 禁用平铺

    lv_obj_t *label = lv_label_create(s_ota_lv_handle->con_fail_obj);
    native_app_ota_label_init(label);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, lv_suit_screen_height(104));
    lv_obj_set_style_text_color(label, lv_color_hex(0xFF3B30), 0);
    lv_label_set_text(label, get_real_ptr(upgrade_logic_get_handle()->note.wifi_connect_fail)); // 网络连接失败

    label = lv_label_create(s_ota_lv_handle->con_fail_obj);
    native_app_ota_label_init(label);
    lv_label_set_text(label, get_real_ptr(upgrade_logic_get_handle()->note.wifi_conn_fail_info)); // Wi-Fi密码已修改或Wi-Fi距离过远
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, lv_suit_screen_height(132));

    lv_obj_t *btn_cont = lv_obj_create(s_ota_lv_handle->con_fail_obj);
    lv_obj_set_style_bg_opa(btn_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_cont, 0, 0);
    lv_obj_align(btn_cont, LV_ALIGN_TOP_MID, 0, lv_suit_screen_height(180));
    lv_obj_set_size(btn_cont, lv_suit_screen_width(320), lv_suit_screen_height(48));
    lv_obj_set_style_pad_all(btn_cont, 0, LV_PART_MAIN);

    lv_obj_t *btn_cancel = lv_btn_create(btn_cont);
    lv_obj_remove_style_all(btn_cancel);
    lv_obj_set_size(btn_cancel, lv_suit_screen_width(145), lv_suit_screen_height(48));
    lv_obj_align(btn_cancel, LV_ALIGN_LEFT_MID, 8, 0);
    lv_obj_set_style_radius(btn_cancel, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn_cancel, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn_cancel, LV_OPA_30, LV_PART_MAIN);
    lv_obj_add_event_cb(btn_cancel, lv_event_cb, LV_EVENT_CLICKED, "cancel");

    lv_obj_set_style_border_width(btn_cancel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(btn_cancel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_width(btn_cancel, 2, LV_PART_MAIN | LV_STATE_FOCUSED); 
    lv_obj_set_style_border_opa(btn_cancel, LV_OPA_100, LV_PART_MAIN | LV_STATE_FOCUSED);

    label = lv_label_create(btn_cancel);
    lv_obj_set_style_text_font(label, s_ft_font, LV_PART_MAIN);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_ALIGN_DEFAULT);
    lv_obj_center(label);
    // lv_label_set_text(label, "取消");
    lv_label_set_text(label, get_real_ptr(upgrade_logic_get_handle()->note.upgrade_cancel));

    lv_obj_t *btn_con = lv_btn_create(btn_cont);
    lv_obj_remove_style_all(btn_con);
    lv_obj_set_size(btn_con, lv_suit_screen_width(145), lv_suit_screen_height(48));
    lv_obj_align(btn_con, LV_ALIGN_LEFT_MID, lv_suit_screen_width(162), 0);
    lv_obj_set_style_radius(btn_con, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn_con, lv_color_hex(0XF45138), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn_con, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_pad_all(btn_con, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(btn_con, lv_event_cb, LV_EVENT_CLICKED, "con_again");

    lv_obj_set_style_border_width(btn_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(btn_con, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_width(btn_con, 2, LV_PART_MAIN | LV_STATE_FOCUSED); 
    lv_obj_set_style_border_opa(btn_con, LV_OPA_100, LV_PART_MAIN | LV_STATE_FOCUSED);

    label = lv_label_create(btn_con);
    lv_obj_set_style_text_font(label, s_ft_font, LV_PART_MAIN);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_ALIGN_DEFAULT);
    lv_obj_center(label);
    lv_label_set_text_fmt(label, get_real_ptr(upgrade_logic_get_handle()->note.wifi_retry_once));   //重试

    lv_gridnav_add(btn_cont, LV_GRIDNAV_CTRL_ROLLOVER);
    lv_group_add_obj(lv_group_get_default(), btn_cont);
    lv_group_focus_obj(btn_cont);
}

/**
 * @brief 创建wifi删除页面
 */
static void native_app_ota_wifi_del_page_init(void)
{
    upgrade_handle_t *hdl = upgrade_logic_get_handle();

    s_cur_page_idx = OTA_PAGE_WIFI_DEL;
    printf("wifi del page.\n");

    s_ota_lv_handle->wifi_del_obj = ota_page_create(s_ota_lv_handle->main_cont_obj);

    lv_obj_t *circle = lv_obj_create(s_ota_lv_handle->wifi_del_obj);
    lv_obj_set_scrollbar_mode(circle, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_size(circle, lv_suit_screen_size(80), lv_suit_screen_size(80));
    lv_obj_align(circle, LV_ALIGN_TOP_MID, 0, lv_suit_screen_height(21));
    lv_obj_set_style_bg_opa(circle, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(circle, lv_color_hex(0xFF8C40), 0);

    lv_obj_t *img = lv_img_create(circle);
    lv_obj_set_scrollbar_mode(img, LV_SCROLLBAR_MODE_OFF);
    if (strlen(hdl->img.ota_question.path) > 0 && hdl->img.ota_question.w > 0 && hdl->img.ota_question.h > 0) {
        lv_img_set_src(img, hdl->img.ota_question.path);
        lv_obj_set_size(img, hdl->img.ota_question.w, hdl->img.ota_question.h);
    } else {
        lv_img_set_src(img, "P:"APP_DIR"/ota_question.bin");
        lv_obj_set_size(img, 25, 38);
    }
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *label = lv_label_create(s_ota_lv_handle->wifi_del_obj);
    lv_obj_set_height(label, lv_suit_screen_height(21));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, lv_suit_screen_height(113));
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFF8C40), 0);
    lv_obj_set_style_text_opa(label, LV_OPA_COVER, 0);
    // lv_label_set_text(label, "名称");
    lv_label_set_text_fmt(label, "%s", s_wifi_handle->cur_ap_info.ssid);
    lv_obj_set_style_text_font(label, s_ft_font, 0);

    label = lv_label_create(s_ota_lv_handle->wifi_del_obj);
    native_app_ota_label_init(label);
    lv_label_set_text(label, get_real_ptr(upgrade_logic_get_handle()->note.wifi_rm_sure)); // 确认移除此网络？
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, lv_suit_screen_height(139));

    lv_obj_t *btn_cont = lv_obj_create(s_ota_lv_handle->wifi_del_obj);
    lv_obj_set_style_bg_opa(btn_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_cont, 0, 0);
    lv_obj_align(btn_cont, LV_ALIGN_TOP_MID, 0, lv_suit_screen_height(178));
    lv_obj_set_size(btn_cont, lv_suit_screen_width(320), lv_suit_screen_height(60));
    lv_obj_set_style_pad_all(btn_cont, 0, LV_PART_MAIN);

    lv_obj_t *btn_cancel = lv_btn_create(btn_cont);
    lv_obj_remove_style_all(btn_cancel);
    lv_obj_set_size(btn_cancel, lv_suit_screen_width(150), lv_suit_screen_height(54));
    lv_obj_align(btn_cancel, LV_ALIGN_LEFT_MID, lv_suit_screen_width(8), 0);
    lv_obj_set_style_radius(btn_cancel, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn_cancel, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn_cancel, LV_OPA_30, LV_PART_MAIN);
    lv_obj_add_event_cb(btn_cancel, lv_event_cb, LV_EVENT_CLICKED, "cancel");

    lv_obj_set_style_border_width(btn_cancel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(btn_cancel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_width(btn_cancel, 2, LV_PART_MAIN | LV_STATE_FOCUSED); 
    lv_obj_set_style_border_opa(btn_cancel, LV_OPA_100, LV_PART_MAIN | LV_STATE_FOCUSED);

    label = lv_label_create(btn_cancel);
    lv_obj_set_style_text_font(label, s_ft_font, LV_PART_MAIN);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_ALIGN_DEFAULT);
    lv_obj_center(label);
    // lv_label_set_text(label, "取消");
    lv_label_set_text(label, get_real_ptr(upgrade_logic_get_handle()->note.upgrade_cancel));

    lv_obj_t *btn_del = lv_btn_create(btn_cont);
    lv_obj_remove_style_all(btn_del);
    lv_obj_set_size(btn_del, lv_suit_screen_width(150), lv_suit_screen_height(54));
    lv_obj_align(btn_del, LV_ALIGN_LEFT_MID, lv_suit_screen_width(162), 0);
    lv_obj_set_style_radius(btn_del, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn_del, lv_color_hex(0xFF3B30), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn_del, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_pad_all(btn_del, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(btn_del, lv_event_cb, LV_EVENT_CLICKED, "del_wifi");

    lv_obj_set_style_border_width(btn_del, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(btn_del, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_width(btn_del, 2, LV_PART_MAIN | LV_STATE_FOCUSED); 
    lv_obj_set_style_border_opa(btn_del, LV_OPA_100, LV_PART_MAIN | LV_STATE_FOCUSED);

    label = lv_label_create(btn_del);
    lv_obj_set_style_text_font(label, s_ft_font, LV_PART_MAIN);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_ALIGN_DEFAULT);
    lv_obj_center(label);
    // lv_label_set_text_fmt(label, "移除");
    lv_label_set_text_fmt(label, get_real_ptr(upgrade_logic_get_handle()->note.wifi_rm));

    lv_gridnav_add(btn_cont, LV_GRIDNAV_CTRL_ROLLOVER);
    lv_group_add_obj(lv_group_get_default(), btn_cont);
    lv_group_focus_obj(btn_cont);
}

/**
 * @brief 创建wifi列表页面
 */
static void native_app_ota_wifi_page_init(void)
{
    int len = 0;
    upgrade_handle_t *hdl = upgrade_logic_get_handle();

    s_cur_page_idx = OTA_PAGE_WIFI;
    printf("wifi page\n");

    s_ota_lv_handle->wifi_obj = ota_page_create(s_ota_lv_handle->main_cont_obj);
    if (s_ota_lv_handle->suc_win != NULL) {
        printf("[ %s ][ %d ]  del suc win \n",__FUNCTION__,__LINE__);
        lv_obj_del(s_ota_lv_handle->suc_win);
        s_ota_lv_handle->suc_win = NULL;
    }
    if (s_ota_lv_handle->refresh_win) {
        printf("[ %s ][ %d ]  del refresh win \n",__FUNCTION__,__LINE__);
        lv_obj_del(s_ota_lv_handle->refresh_win);
        s_ota_lv_handle->refresh_win = NULL;
    }
    s_ota_lv_handle->wifi_img = NULL;
    s_ota_lv_handle->connect_info = NULL;
    
    //WLAN
    lv_obj_t *label = lv_label_create(s_ota_lv_handle->wifi_obj);
    lv_obj_set_style_text_font(label, s_ft_font, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text(label, get_real_ptr(upgrade_logic_get_handle()->note.wifi_WIFI));
    lv_obj_set_height(label, lv_suit_screen_height(28));
    lv_obj_align(label, LV_ALIGN_TOP_MID, lv_suit_screen_width(0), lv_suit_screen_height(16));

    s_ota_lv_handle->wifi_list = NULL;
    native_app_ota_wifi_list_create();
    // create_top_refresh_win();
}

//更新焦点
static void update_focus(int new_idx)
{
    int num = lv_obj_get_child_cnt(s_ota_lv_handle->kb_cont);

    printf("newid %d num %d\n", new_idx, num);
    if (new_idx >= num || new_idx < 0) {
        return;
    }

    for (int i = 0; i < num; i++) {
        lv_obj_clear_state(lv_obj_get_child(s_ota_lv_handle->kb_cont, i), LV_STATE_FOCUSED);
    }

    lv_group_focus_obj(lv_obj_get_child(s_ota_lv_handle->kb_cont, new_idx));
}

static void native_app_ota_keybord_create(const char *key_list)
{
    upgrade_handle_t *hdl = upgrade_logic_get_handle();

    printf("---> list %s\n", key_list);
    if (s_ota_lv_handle->kb_cont != NULL) {
        lv_obj_del(s_ota_lv_handle->kb_cont);
        s_ota_lv_handle->kb_cont = NULL;
    }

    int btn_size = (lv_disp_get_hor_res(NULL) - 10) / 6;
    int cont_width = 10 + btn_size * 6;

    s_ota_lv_handle->kb_cont = lv_obj_create(s_ota_lv_handle->key_obj);
    lv_obj_set_size(s_ota_lv_handle->kb_cont, cont_width, lv_suit_screen_height(160));
    lv_obj_align(s_ota_lv_handle->kb_cont, LV_ALIGN_TOP_MID, (lv_disp_get_hor_res(NULL) - cont_width) / 2, lv_suit_screen_height(s_touchpad_enable? 95: 82));
    lv_obj_set_flex_flow(s_ota_lv_handle->kb_cont, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_style_pad_all(s_ota_lv_handle->kb_cont, 0, 0);
    lv_obj_set_style_flex_main_place(s_ota_lv_handle->kb_cont, LV_FLEX_ALIGN_START, 0);
    lv_obj_set_style_pad_row(s_ota_lv_handle->kb_cont, 2, 0);
    lv_obj_set_style_pad_column(s_ota_lv_handle->kb_cont, 2, 0);
    lv_obj_set_scrollbar_mode(s_ota_lv_handle->kb_cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(s_ota_lv_handle->kb_cont, lv_color_black(), 0);
    lv_obj_set_style_border_width(s_ota_lv_handle->kb_cont, 0, 0);

    int key_num = strlen(key_list) + FUNC_KEY_NUM;
    int line = strlen(key_list) / 6 + 1;
    printf("key num %d  line %d\n", key_num, line);

    int j = 0;
    for (int i = 0; i < key_num; i++) {
        lv_obj_t *btn = lv_btn_create(s_ota_lv_handle->kb_cont);
        lv_obj_remove_style_all(btn);
        lv_obj_set_size(btn, btn_size, btn_size);
        lv_obj_set_style_radius(btn, 8, 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_set_style_border_width(btn, 2, LV_PART_MAIN | LV_STATE_FOCUSED); 
        lv_obj_set_style_border_opa(btn, LV_OPA_100, LV_PART_MAIN | LV_STATE_FOCUSED);

        lv_obj_t *label = lv_label_create(btn);

        // 修改 label 的字体大小和对齐方式
        lv_obj_set_style_text_font(label, s_ft_font, LV_PART_MAIN); // 使用自定义字体
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN); // 居中对齐

        if ((i + 1) % 6 != 0 || (i + 1) / 6 > FUNC_KEY_NUM) {
            lv_label_set_text_fmt(label, "%c", key_list[i - j]);
            lv_obj_add_event_cb(btn, lv_event_cb, LV_EVENT_KEY, (void *)lv_label_get_text(label));
            lv_obj_add_event_cb(btn, lv_event_cb, LV_EVENT_CLICKED, (void *)lv_label_get_text(label));
        } else if ((i + 1) % 6 == 0 && (i + 1) / 6 <= FUNC_KEY_NUM) {
            if (j == 0) {
                lv_label_set_text(label, "");
                lv_obj_t *img = lv_img_create(label);
                if (strlen(hdl->img.ota_check.path) > 0 && hdl->img.ota_check.w > 0 && hdl->img.ota_check.h > 0) {
                    lv_img_set_src(img, hdl->img.ota_check.path);
                    lv_obj_set_size(img, hdl->img.ota_check.w, hdl->img.ota_check.h);
                } else {
                    lv_img_set_src(img, "P:"APP_DIR"/ota_check.bin");
                    lv_obj_set_size(img, 28, 28);
                }
                lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
                lv_obj_add_event_cb(btn, lv_event_cb, LV_EVENT_KEY, (void *)func_keys[j]);
                lv_obj_add_event_cb(btn, lv_event_cb, LV_EVENT_CLICKED, (void *)func_keys[j]);
            } else if (j == 1) {
                lv_label_set_text(label, "");
                lv_obj_t *img = lv_img_create(label);
                if (strlen(hdl->img.ota_delete.path) > 0 && hdl->img.ota_delete.w > 0 && hdl->img.ota_delete.h > 0) {
                    lv_img_set_src(img, hdl->img.ota_delete.path);
                    lv_obj_set_size(img, hdl->img.ota_delete.w, hdl->img.ota_delete.h);
                } else {
                    lv_img_set_src(img, "P:"APP_DIR"/ota_delete.bin");
                    lv_obj_set_size(img, 28, 28);
                }
                lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
                lv_obj_add_event_cb(btn, lv_event_cb, LV_EVENT_KEY, (void *)func_keys[j]);
                lv_obj_add_event_cb(btn, lv_event_cb, LV_EVENT_CLICKED, (void *)func_keys[j]);
            } else {
                lv_label_set_text(label, func_keys[j]);
                lv_obj_add_event_cb(btn, lv_event_cb, LV_EVENT_KEY, (void *)lv_label_get_text(label));
                lv_obj_add_event_cb(btn, lv_event_cb, LV_EVENT_CLICKED, (void *)lv_label_get_text(label));
            }
            j++;
            
            if (j == FUNC_KEY_NUM) {
                lv_obj_set_style_text_color(label, lv_color_hex(0XD4FA67), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_opa(label, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
            }
        }
        lv_obj_set_style_bg_opa(btn, (j == 0 && i == 5)? LV_OPA_40:LV_OPA_30, 0);
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        lv_group_add_obj(lv_group_get_default(), btn);
    }

    lv_group_focus_obj(lv_obj_get_child(s_ota_lv_handle->kb_cont, 0));
}

/**
 * @brief 创建键盘页面
 */
static void native_app_ota_keybord_page_init(void)
{
    printf("keybord page.\n");
    s_cur_page_idx = OTA_PAGE_KEYBOARD;
    is_uppercase = false;
    is_symbol = false;
    upgrade_handle_t *hdl = upgrade_logic_get_handle();

    s_ota_lv_handle->key_obj = ota_page_create(s_ota_lv_handle->main_cont_obj);

    if (s_touchpad_enable != 0) {
        lv_obj_t *btn = lv_btn_create(s_ota_lv_handle->key_obj);
        lv_obj_remove_style_all(btn);
        lv_obj_set_size(btn, lv_suit_screen_size(32), lv_suit_screen_size(32));
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_bg_opa(btn, LV_OPA_40, 0);
        lv_obj_set_style_radius(btn, lv_suit_screen_size(8), 0);
        lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 16, lv_suit_screen_height(8));

        lv_obj_t *img = lv_img_create(btn);
        if (strlen(hdl->img.ota_back_btn.path) > 0 && hdl->img.ota_back_btn.w > 0 && hdl->img.ota_back_btn.h > 0) {
            lv_img_set_src(img, hdl->img.ota_back_btn.path);
            lv_obj_set_size(img, hdl->img.ota_back_btn.w, hdl->img.ota_back_btn.h);
        } else {
            lv_img_set_src(img, "P:"APP_DIR"/ota_back_btn.bin");
            lv_obj_set_size(img, 6, 12);
        }
        lv_obj_set_align(img, LV_ALIGN_CENTER);

        lv_obj_add_event_cb(btn, lv_event_cb, LV_EVENT_CLICKED, "back");
    }

    lv_obj_t *label = lv_label_create(s_ota_lv_handle->key_obj);
    lv_obj_set_style_text_font(label, s_ft_font, LV_PART_MAIN);
    if (s_touchpad_enable != 0) {
        lv_obj_set_style_pad_top(label, lv_suit_screen_height((28 - lv_font_get_line_height(s_ft_font))/2), LV_PART_MAIN);
    }
    // lv_label_set_text(label, "请输入密码");
    lv_label_set_text_fmt(label, "%s", s_wifi_con_info.ssid);
    lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_height(label, lv_suit_screen_height(30));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, lv_suit_screen_width(s_touchpad_enable? 52 : 16), lv_suit_screen_width(s_touchpad_enable? 10 : 8));

    s_ota_lv_handle->textarea = lv_textarea_create(s_ota_lv_handle->key_obj);
    lv_group_remove_obj(s_ota_lv_handle->textarea);
    lv_obj_remove_style_all(s_ota_lv_handle->textarea);
    
    lv_obj_set_style_text_font(s_ota_lv_handle->textarea, s_ft_font, LV_PART_MAIN);
    lv_obj_set_style_pad_left(s_ota_lv_handle->textarea, lv_suit_screen_width(14), 0);
    lv_obj_set_style_pad_top(s_ota_lv_handle->textarea, lv_suit_screen_height((50 - lv_font_get_line_height(s_ft_font))/2), LV_PART_MAIN);
    lv_obj_set_style_text_align(s_ota_lv_handle->textarea, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    // lv_textarea_set_placeholder_text(s_ota_lv_handle->textarea, "请输入密码");
    lv_textarea_set_placeholder_text(s_ota_lv_handle->textarea, get_real_ptr(upgrade_logic_get_handle()->note.wifi_enter_pwd));
    lv_obj_set_style_text_opa(s_ota_lv_handle->textarea, LV_OPA_60, LV_PART_MAIN);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    // lv_obj_set_style_text_opa(s_ota_lv_handle->textarea, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_size(s_ota_lv_handle->textarea, lv_suit_screen_width(316), lv_suit_screen_height(s_touchpad_enable? 51 : 50));
    lv_obj_set_style_bg_color(s_ota_lv_handle->textarea, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_width(s_ota_lv_handle->textarea, 0, 0);
    lv_obj_set_style_bg_opa(s_ota_lv_handle->textarea, LV_OPA_30, 0);
    lv_obj_set_style_radius(s_ota_lv_handle->textarea, 8, 0);
    lv_obj_align(s_ota_lv_handle->textarea, LV_ALIGN_TOP_MID, 0, lv_suit_screen_height(s_touchpad_enable? 42 : 30));
    s_ota_lv_handle->kb_cont = NULL;

    native_app_ota_keybord_create(lower_keys);
}

/**
 * @brief 创建升级页面
 */
static void native_app_ota_upgrade_page_init(void)
{
    printf("up page.\n");

    s_ota_lv_handle->upgrade_obj = ota_page_create(s_ota_lv_handle->main_cont_obj);

    //圆球进度条
    s_ota_lv_handle->bar = lv_bar_create(s_ota_lv_handle->upgrade_obj);
    lv_obj_set_size(s_ota_lv_handle->bar, 164, 165);
    lv_obj_align(s_ota_lv_handle->bar, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_radius(s_ota_lv_handle->bar, LV_RADIUS_CIRCLE, 0);           // 将半径设置为圆形
    lv_obj_set_style_bg_color(s_ota_lv_handle->bar, lv_color_hex(0x969899), 0);   // 设置背景色为灰色
    lv_bar_set_range(s_ota_lv_handle->bar,0,100);
    lv_obj_set_style_radius(s_ota_lv_handle->bar, 0, LV_PART_INDICATOR);  // 填充部分设为无圆角（平边）
    lv_obj_set_style_bg_color(s_ota_lv_handle->bar, lv_color_hex(0x366BF9), LV_PART_INDICATOR);  // 填充颜色（深蓝）
    lv_bar_set_value(s_ota_lv_handle->bar,0, LV_ANIM_OFF);

    //火箭
    lv_obj_t *obj =  lv_obj_create(s_ota_lv_handle->bar);
    lv_obj_set_size(obj, 150, 150);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_opa(obj, 0, 0);
    lv_obj_set_style_border_width(obj,0,0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *img = lv_img_create(obj);
    lv_obj_center(img);
    lv_obj_set_size(img, 110, 120);
    lv_img_set_src(img, "P:"APP_DIR"/upgrade_rocket.bin"); 
    lv_obj_set_style_img_recolor(img, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_img_recolor_opa(img, LV_OPA_100, 0);  
    lv_obj_clear_flag(img, LV_OBJ_FLAG_SCROLLABLE);

    //进度值
    s_ota_lv_handle->progress_label = lv_label_create(s_ota_lv_handle->upgrade_obj);
    lv_obj_align(s_ota_lv_handle->progress_label, LV_ALIGN_BOTTOM_MID, 0, lv_suit_screen_height(-32));
    lv_obj_set_size(s_ota_lv_handle->progress_label, lv_disp_get_hor_res(NULL) - 34, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(s_ota_lv_handle->progress_label, 0, 0);
    lv_label_set_text(s_ota_lv_handle->progress_label, "0%");
    lv_obj_set_style_text_align(s_ota_lv_handle->progress_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(s_ota_lv_handle->progress_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_opa(s_ota_lv_handle->progress_label, LV_OPA_COVER, 0);
    lv_obj_set_style_text_font(s_ota_lv_handle->progress_label, s_ft_font, 0);

    //提示词
    lv_obj_t *label = lv_label_create(s_ota_lv_handle->upgrade_obj);
    native_app_ota_label_init(label);
    lv_label_set_text(label, get_real_ptr(upgrade_logic_get_handle()->note.upgrading_notop)); // 下载中
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, lv_suit_screen_height(-6));
    lv_obj_set_style_text_color(label, lv_color_hex(0x969899), 0);

    // 添加一个隐藏按钮，用以记录当前页面确定键状态，防止按下时刚好升级异常跳转到下一页面后弹起按键，直接响应click事件
    lv_obj_t * button = lv_btn_create(s_ota_lv_handle->upgrade_obj);
    lv_obj_add_event_cb(button, lv_event_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(button, lv_event_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_flag(button, LV_OBJ_FLAG_HIDDEN);
}

/**
 * @brief 创建结束页面
 */
static void native_app_ota_end_page_init(void)
{
    upgrade_handle_t *hdl = upgrade_logic_get_handle();
    printf("end page.\n");
    s_ota_lv_handle->end_obj = ota_page_create(s_ota_lv_handle->main_cont_obj);

    //圆球进度条
    lv_obj_t *bar = lv_bar_create(s_ota_lv_handle->end_obj);
    lv_obj_set_size(bar, 164, 165);
    lv_obj_align(bar, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_radius(bar, LV_RADIUS_CIRCLE, 0);           // 将半径设置为圆形
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x969899), 0);   // 设置背景色为灰色
    lv_bar_set_range(bar,0,100);
    lv_obj_set_style_radius(bar, 0, LV_PART_INDICATOR);  // 填充部分设为无圆角（平边）
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x366BF9), LV_PART_INDICATOR);  // 填充颜色（深蓝）
    lv_bar_set_value(bar,100, LV_ANIM_OFF);

    //勾勾
    lv_obj_t *obj =  lv_obj_create(bar);
    lv_obj_set_size(obj, 100, 100);
    lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(obj, 0, 0);
    lv_obj_set_style_border_width(obj,0,0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *img = lv_img_create(obj);
    lv_obj_center(img);
    lv_obj_set_size(obj, 76, 76);
    lv_img_set_src(img, "P:"APP_DIR"/upgrade_ok.bin");  // 设置图片源
    lv_obj_set_style_img_recolor(img, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_img_recolor_opa(img, LV_OPA_100, 0); 
    lv_obj_clear_flag(img, LV_OBJ_FLAG_SCROLLABLE);

    //进度值
    obj = lv_label_create(s_ota_lv_handle->end_obj);
    lv_obj_align(obj, LV_ALIGN_BOTTOM_MID, 0, lv_suit_screen_height(-26));
    lv_obj_set_size(obj, lv_disp_get_hor_res(NULL) - 34, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_label_set_text(obj, "100%");
    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(obj, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_text_font(obj, s_ft_font, 0);

    lv_obj_t *label = lv_label_create(s_ota_lv_handle->end_obj);
    native_app_ota_label_init(label);
    lv_label_set_text(label, get_real_ptr(upgrade_logic_get_handle()->note.upgrade_succ_rebooting)); //下载完成，请等待设备重启
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, lv_suit_screen_height(0));
    lv_obj_set_style_text_color(label, lv_color_hex(0x969899), 0);
}

/**
 * @brief 创建网络异常页面
 */
static void native_app_ota_neterr_page_init(void)
{
    upgrade_handle_t *hdl = upgrade_logic_get_handle();

    printf("net err page.\n");
    s_ota_lv_handle->neterr_obj = ota_page_create(s_ota_lv_handle->main_cont_obj);

    //眼睛
    lv_obj_t *obj =  lv_obj_create(s_ota_lv_handle->neterr_obj);
    lv_obj_set_size(obj, 95, 75);
    lv_obj_align(obj, LV_ALIGN_TOP_LEFT, lv_suit_screen_height(40), lv_suit_screen_height(40));
    lv_obj_set_style_bg_opa(obj, 0, 0);
    lv_obj_set_style_border_width(obj,0,0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *img = lv_img_create(obj);
    lv_img_set_src(img, "P:"APP_DIR"/ota_net_eys.bin"); 
    lv_obj_set_size(img, 95, 75);  
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_img_recolor(img, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_img_recolor_opa(img, LV_OPA_100, 0); 
    lv_obj_clear_flag(img, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_img_tiled(img, false, 0);  // 禁用平铺

    obj =  lv_obj_create(s_ota_lv_handle->neterr_obj);
    lv_obj_set_size(obj, 95, 75);
    lv_obj_align(obj, LV_ALIGN_TOP_RIGHT, lv_suit_screen_height(-40), lv_suit_screen_height(40));
    lv_obj_set_style_bg_opa(obj, 0, 0);
    lv_obj_set_style_border_width(obj,0,0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    img = lv_img_create(obj);
    lv_img_set_src(img, "P:"APP_DIR"/ota_net_eys2.bin"); 
    lv_obj_set_size(img, 95, 75);  
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_img_recolor(img, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_img_recolor_opa(img, LV_OPA_100, 0); 
    lv_obj_clear_flag(img, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_img_tiled(img, false, 0);  // 禁用平铺

    //wifi错误
    lv_obj_t *label = lv_label_create(s_ota_lv_handle->neterr_obj);
    lv_obj_set_size(label, lv_suit_screen_width(124), lv_suit_screen_width(84));
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, lv_suit_screen_height(-53));
    lv_obj_set_style_bg_opa(label, LV_OPA_100, 0);
    lv_obj_set_style_border_width(label,0,0);
    lv_obj_clear_flag(label, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(label, LV_RADIUS_CIRCLE, 0);		
    lv_obj_set_style_bg_color(label, lv_color_hex(0x262628), 0);   // 设置背景色为灰色
    lv_label_set_text(label,"");

    img = lv_img_create(label);
    lv_obj_set_style_img_recolor(img, lv_color_hex(0xFE2D2F), 0);
    lv_obj_set_style_img_recolor_opa(img, LV_OPA_100, 0); 
    lv_obj_clear_flag(img, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_img_tiled(img, false, 0);  // 禁用平铺
    if (strlen(hdl->img.ota_net_err.path) > 0 && hdl->img.ota_net_err.w > 0 && hdl->img.ota_net_err.h > 0) {
        printf("net err path:%s\n", hdl->img.ota_net_err.path);
        lv_img_set_src(img, hdl->img.ota_net_err.path);
        lv_obj_set_size(img, hdl->img.ota_net_err.w, hdl->img.ota_net_err.h);
    } else {
        lv_img_set_src(img, "P:"APP_DIR"/ota_net_err.bin"); 
        lv_obj_set_size(img, 65, 53);     //替换
    }
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);

    //点击 检查网络
    label = lv_label_create(s_ota_lv_handle->neterr_obj);
    native_app_ota_label_init(label);
    lv_label_set_recolor(label, true);
    lv_label_set_text(label, get_real_ptr(upgrade_logic_get_handle()->note.ok_to_connect)); // 点击 检查网络
    lv_obj_set_style_text_color(label, lv_color_hex(0x366BF9),0);	//字体颜色
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, lv_suit_screen_height(-14));
    lv_obj_add_flag(label, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_add_event_cb(label, lv_event_cb, LV_EVENT_CLICKED, "go_for_wifi");
}

/**
 * @brief 创建升级异常页面
 */
static void native_app_ota_uperr_page_init(void)
{
    upgrade_handle_t *hdl = upgrade_logic_get_handle();

    s_cur_page_idx = OTA_PAGE_UP_ERR;
    printf("up err page.\n");
    s_ota_lv_handle->uperr_obj = ota_page_create(s_ota_lv_handle->main_cont_obj);

    //圆球进度条
    lv_obj_t *bar = lv_bar_create(s_ota_lv_handle->uperr_obj);
    lv_obj_set_size(bar, 164, 165);
    lv_obj_align(bar, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_radius(bar, LV_RADIUS_CIRCLE, 0);           // 将半径设置为圆形
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x969899), 0);   // 设置背景色为灰色
    lv_bar_set_range(bar,0,100);
    lv_obj_set_style_radius(bar, 0, LV_PART_INDICATOR);  // 填充部分设为无圆角（平边）
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x366BF9), LV_PART_INDICATOR);  // 填充颜色（深蓝）
    lv_bar_set_value(bar,0, LV_ANIM_OFF);

    //火箭
    lv_obj_t *obj =  lv_obj_create(bar);
    lv_obj_set_size(obj, 150, 150);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_opa(obj, 0, 0);
    lv_obj_set_style_border_width(obj,0,0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *img = lv_img_create(obj);
    lv_obj_center(img);
    lv_obj_set_size(img, 110, 120);
    lv_img_set_src(img, "P:"APP_DIR"/upgrade_rocket.bin");  // 设置图片源
    lv_obj_set_style_img_recolor(img, lv_color_hex(0xFE2D2F), 0);
    lv_obj_set_style_img_recolor_opa(img, LV_OPA_100, 0); 
    lv_obj_clear_flag(img, LV_OBJ_FLAG_SCROLLABLE);

    //提示词
    lv_obj_t *label = lv_label_create(s_ota_lv_handle->uperr_obj);
    native_app_ota_label_init(label);
    if (UPGRADE_ERR_BAT_LOW == hdl->fw_handle.err || UPGRADE_ERR_BAT_LOW == hdl->app_handle.err) {
        lv_label_set_text(label, get_real_ptr(hdl->note.bat_low));
    } else {
        char msg[100] = {0};
        sprintf(msg, "%s(%02d/%02d)", get_real_ptr(hdl->note.upgrade_fail), hdl->fw_handle.err, hdl->app_handle.err);
        lv_label_set_text(label, msg); // 新版本下载失败
    }
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 180);

    //重试按键
    lv_obj_t *btn_cont = lv_obj_create(s_ota_lv_handle->uperr_obj);
    lv_obj_set_style_bg_opa(btn_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_cont, 0, 0);
    lv_obj_align(btn_cont, LV_ALIGN_BOTTOM_MID, 0, lv_suit_screen_height(-8));
    lv_obj_set_size(btn_cont, lv_suit_screen_width(143), lv_suit_screen_height(26));
    lv_obj_set_style_pad_all(btn_cont, 0, LV_PART_MAIN);

    lv_obj_t *btn_reset = lv_btn_create(btn_cont);
    lv_obj_remove_style_all(btn_reset);
    lv_obj_set_size(btn_reset, lv_suit_screen_width(143), lv_suit_screen_height(26));
    lv_obj_align(btn_reset, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(btn_reset, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(btn_reset, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(btn_reset, lv_event_cb, LV_EVENT_CLICKED, "up_again");

    label = lv_label_create(btn_reset);
    lv_obj_set_style_text_font(label, s_ft_font, LV_PART_MAIN);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_ALIGN_DEFAULT);
    lv_obj_center(label);
    lv_label_set_text_fmt(label, get_real_ptr(hdl->note.upgrade_fail_retry));   //点击重试
    lv_obj_set_style_text_color(label, lv_color_hex(0xFE2D2F),0);

    // lv_gridnav_add(btn_cont, LV_GRIDNAV_CTRL_ROLLOVER);
    // lv_group_add_obj(lv_group_get_default(), btn_cont);
    // lv_group_focus_obj(btn_cont);

    //左上角返回按钮
    lv_obj_t *btn_return = lv_btn_create(s_ota_lv_handle->uperr_obj);
    lv_obj_set_style_radius(btn_return, LV_RADIUS_CIRCLE, 0);           // 将半径设置为圆形
    lv_obj_align(btn_return, LV_ALIGN_TOP_LEFT, lv_suit_screen_height(10), lv_suit_screen_height(10));
    lv_obj_set_size(btn_return, lv_suit_screen_width(44), lv_suit_screen_height(44));
    lv_obj_add_event_cb(btn_return, lv_event_cb, LV_EVENT_CLICKED, "reboot");
    lv_obj_set_style_bg_color(btn_return, lv_color_hex(0x262628), 0);  

    img = lv_img_create(btn_return);
    lv_obj_center(img);
    lv_img_set_src(img, "P:"APP_DIR"/upgrade_return.bin");  // 设置图片源
    lv_obj_set_style_img_recolor(img, lv_color_hex(0x969899), 0);
    lv_obj_set_style_img_recolor_opa(img, LV_OPA_100, 0);  // 100%不透明度
    lv_obj_clear_flag(img, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_img_tiled(img, false, 0);  // 禁用平铺
}

static void wifi_cb_process_task(void *arg)
{
    upgrade_handle_t *hdl = upgrade_logic_get_handle();
    while (1) {
        if (os_adapter()->signal_wait(s_wifi_handle->cb_sem, os_adapter()->get_forever_time()) != 0) {
            continue;
        }
        int event = s_wifi_handle->wifi_cb_event;
        printf("[%s %d] event = %d %d %d\n", __func__, __LINE__, event, s_cur_page_idx, s_wifi_handle->connected);
        s_wifi_handle->wifi_cb_prcossing = false;
        switch (event) {
            case NET_EVENT_CONNECT:
                s_wifi_handle->connected = 2;
                if (s_cur_page_idx == OTA_PAGE_WIFI) {
                    lv_lock();
                    if (NULL != s_ota_lv_handle->wifi_img) {
                        lv_obj_clean(s_ota_lv_handle->wifi_img);
                        if (strlen(hdl->img.ota_wifi_connected.path) > 0 && hdl->img.ota_wifi_connected.w > 0 && hdl->img.ota_wifi_connected.h > 0) {
                            lv_img_set_src(s_ota_lv_handle->wifi_img, hdl->img.ota_wifi_connected.path);
                            lv_obj_set_size(s_ota_lv_handle->wifi_img, hdl->img.ota_wifi_connected.w, hdl->img.ota_wifi_connected.h);
                        } else {
                            lv_img_set_src(s_ota_lv_handle->wifi_img, "P:"APP_DIR"/ota_wifi_connected.bin");
                        }
                    }
                    if (NULL != s_ota_lv_handle->connect_info) {
                        // lv_label_set_text(s_ota_lv_handle->connect_info, "已连接");
                        lv_label_set_text(s_ota_lv_handle->connect_info, get_real_ptr(upgrade_logic_get_handle()->note.wifi_list_connected));
                    }
                    // create_top_suc_win("网络连接成功");
                    create_top_suc_win(get_real_ptr(upgrade_logic_get_handle()->note.wifi_connect_succ));
                    lv_unlock();
                    wifi_adapter_add_wifi_to_list(&s_wifi_handle->cur_ap_info);
                }
                printf("connect!!\n");
                break;
            // 875扫描后马上返回扫描成功，不在这里删刷新窗口，只发送更新列表事件
            case NET_EVENT_SCAN_SUCCESS:
                if (s_cur_page_idx == OTA_PAGE_WIFI) {
                    if (NULL != s_ota_lv_handle->refresh_win) {
                        set_event(OTA_EVENT_WIFI_FRESH);
                        // lv_lock();
                        // lv_obj_del(s_ota_lv_handle->refresh_win);
                        // s_ota_lv_handle->refresh_win = NULL;
                        // native_app_ota_wifi_list_create();
                        // create_top_suc_win("刷新成功");
                        // create_top_suc_win(get_real_ptr(upgrade_logic_get_handle()->note.wifi_fresh_succ));
                        // lv_unlock();
                    }
                } else if (s_cur_page_idx == OTA_PAGE_START) {
                    native_app_ota_auto_connect();
                } else if (s_cur_page_idx == OTA_PAGE_LOADING) {
                    printf("scan suc!\n");
                    set_event(OTA_EVENT_WIFI);
                }
                break;
            case NET_EVENT_NETWORK_UP:
                set_event(OTA_EVENT_UPGRADE);
                os_adapter()->msleep(100);
                printf("[%s %d] going to up\n", __func__, __LINE__);
                native_app_ota_task();
                break;
            case NET_EVENT_PASSWORD_ERR:
            case NET_EVENT_4WAY_HANDSHAKE_FAILED:
                s_wifi_handle->connected = 0;
                wifi_adapter_del_save_wifi(s_wifi_handle->cur_ap_info.ssid);
                if (s_cur_page_idx == OTA_PAGE_WIFI) {
                    wifi_adapter_disconnect();
                    set_event(OTA_EVENT_CON_FAIL);
                }
                break;
            case NET_EVENT_DISCONNECTED:
            case NET_EVENT_NETWORK_NOT_EXIST:
            case NET_EVENT_CONNECT_REJECT:
            case NET_EVENT_CONNECT_ABORT:
            case NET_EVENT_CONNECT_TIMEOUT:
            case NET_EVENT_CONNECT_FAILED:
            case NET_EVENT_CONNECTION_LOSS:
                s_wifi_handle->connected = 0;
                if (s_cur_page_idx == OTA_PAGE_WIFI) {
                    wifi_adapter_disconnect();
                    set_event(OTA_EVENT_CON_FAIL);
                }/*  else if (s_cur_page_idx == OTA_PAGE_UPGRADE) {
                    set_event(OTA_EVENT_NET_ERR);
                }  */
                break;
            default:
                break;
        }
    }
}

static void native_app_wifi_cb(unsigned int event, void *arg)
{
    printf("[%s %d] event = %d %d %d\n", __func__, __LINE__, event, s_cur_page_idx, s_wifi_handle->connected);

    int cnt = 0;
    while (s_wifi_handle->wifi_cb_prcossing == true) {
        os_adapter()->msleep(50);
        if (cnt++ >= 100) {
            printf("wait for process event timeout\n");
            break;
        }
    }
    s_wifi_handle->wifi_cb_event = event;
    s_wifi_handle->wifi_cb_prcossing = true;
    os_adapter()->signal_post(s_wifi_handle->cb_sem);
}

static void *s_wifi_connect_tid = NULL;
static wifi_info_t s_connect_info = {0};
static void wifi_connect_task(void *arg)
{
    wifi_adapter_connect(s_connect_info.ssid, s_connect_info.pwd, s_connect_info.sec_type);
    os_adapter()->thread_delete(&s_wifi_connect_tid);
}

static void wifi_connect(char *ssid, char* pwd, wifi_sec_type_t sec_type)
{
    printf("[%s %d] connect %s %s\n", __func__, __LINE__, ssid, pwd);

    if (s_wifi_connect_tid) {
        LOG_ERR("connect is running");
        return;
    }
    memcpy(s_connect_info.ssid, ssid, sizeof(s_connect_info.ssid));
    memcpy(s_connect_info.pwd, pwd, sizeof(s_connect_info.pwd));
    s_connect_info.sec_type = sec_type;
    os_adapter()->thread_create(&s_wifi_connect_tid,
                                wifi_connect_task,
                                NULL,
                                "wifi_connect_task",
                                OS_ADAPTER_PRIORITY_NORMAL,
                                1024);
    return;
}


static void _auto_connect(void *arg)
{
    // int ret = -1;

    printf("auto_connect start\n");
    static long last_connect_time = 0;

    if (last_connect_time != 0 && os_adapter()->get_time_ms() - last_connect_time < 10000) {
        printf("skip auto connect\n");
        goto exit;
    }

    wifi_adapter_get_save_list(&s_wifi_handle->local_wifi_list);
    wifi_adapter_get_scan_result(&s_wifi_handle->scan_result);

    for (int i = 0; i < s_wifi_handle->local_wifi_list.wifi_cnt; i++) {
        for (int j = 0; j < s_wifi_handle->scan_result.num; j++) {
            if (0 == strcmp(s_wifi_handle->local_wifi_list.wifi_info[i].ssid, s_wifi_handle->scan_result.ap[j].ssid)) {
                memset(s_wifi_handle->cur_ap_info.ssid, 0, SSID_MAX_LEN + 1);
                memset(s_wifi_handle->cur_ap_info.pwd, 0, PWD_MAX_LEN + 1);
                strcpy(s_wifi_handle->cur_ap_info.ssid, s_wifi_handle->local_wifi_list.wifi_info[i].ssid);
                strcpy(s_wifi_handle->cur_ap_info.pwd, s_wifi_handle->local_wifi_list.wifi_info[i].pwd);
                s_wifi_handle->cur_ap_info.sec_type = s_wifi_handle->local_wifi_list.wifi_info[i].sec_type;
                s_wifi_handle->connected = 1;
                // 这里需要同步连接，直接使用wifi_adapter层的接口
                wifi_adapter_connect(s_wifi_handle->cur_ap_info.ssid, s_wifi_handle->cur_ap_info.pwd, s_wifi_handle->cur_ap_info.sec_type);
                last_connect_time = os_adapter()->get_time_ms();
            }

            //升级初始页面 wifi连接过程中阻塞等待
            while(s_wifi_handle->connected == 1 && s_cur_page_idx == OTA_PAGE_START) {
                os_adapter()->msleep(50);
            }

            //连接失败或已不是升级初始页面，则退出
            if (s_cur_page_idx != OTA_PAGE_START || s_wifi_handle->connected == 2) {
                goto exit;
            }
        }
    }

exit:
    if (s_wifi_handle->connected == 0 && s_cur_page_idx == OTA_PAGE_START) {
        set_event(OTA_EVENT_WIFI);
    }

    os_adapter()->thread_delete(&s_wifi_handle->auto_tid);
}

static void native_app_ota_auto_connect()
{
    int ret = -1;
    if (s_wifi_handle == NULL || s_wifi_handle->auto_tid != NULL) {
        LOG_ERR("param error or auto connet thread created %p\n", s_wifi_handle);
        return;
    }

    ret = os_adapter()->thread_create(&s_wifi_handle->auto_tid,
                                            _auto_connect,
                                            NULL,
                                            "_auto_connect",
                                            OS_ADAPTER_PRIORITY_NORMAL + 1,
                                            1024 * 3);
    if (ret != 0) {
        LOG_ERR("create auto con thread failed ret = %d.\n", ret);
        s_wifi_handle->auto_tid = NULL;
    }
}

// wifi列表排序
static void wifi_list_sort(wifi_scan_results_t *list)
{
    int i = 0;
    int j = 0;
    wifi_ap_info_t param;

    for(i = 0; i < list->num; i++) {
        for(j = 0; j < list->num - i - 1; j++) {
            if (list->ap[j].rssi < list->ap[j+1].rssi) {
                memset(&param, 0, sizeof(wifi_ap_info_t));
                memcpy(&param, &list->ap[j], sizeof(wifi_ap_info_t));
                memcpy(&list->ap[j], &list->ap[j+1], sizeof(wifi_ap_info_t));
                memcpy(&list->ap[j+1], &param, sizeof(wifi_ap_info_t));
            }
        }
    }
   
}

// 过滤扫描的wifi列表
static void wifi_list_check(wifi_scan_results_t *wifi)
{
    int i = 0;
    int j = 0;
    wifi_ap_info_t ap;

    if (NULL == wifi) {
        goto exit;
    }

    for (i = 0; i < wifi->num;) {
        if (strlen(wifi->ap[i].ssid) == 0) {
            for (int a = i; a < wifi->num - 1; a++) {
                memcpy(&wifi->ap[a], &wifi->ap[a+1], sizeof(wifi_ap_info_t));
            }
            wifi->num--;
            continue;
        }
        memcpy(&ap, &wifi->ap[i], sizeof(wifi_ap_info_t));
        for (j = i + 1; j < wifi->num; j++) {
            if (0 == strcmp(ap.ssid, wifi->ap[j].ssid)) {
                for (int a = j; a < wifi->num - 1; a++) {
                    memcpy(&wifi->ap[a], &wifi->ap[a+1], sizeof(wifi_ap_info_t));
                }
                wifi->num--;
            }
        }
        i++;
    }

exit:
    return;
}

//更新需要显示的wifi列表
static void native_app_ota_wifi_list_update(bool isScan)
{
    int i = 0;
    int j = 0;

    s_wifi_handle->wifi_list.num = 0;
    s_wifi_handle->show_list.num = 0;
    if (isScan) {
        wifi_adapter_get_save_list(&s_wifi_handle->local_wifi_list);
        wifi_adapter_get_scan_result(&s_wifi_handle->scan_result);
    }

    if (s_wifi_handle->scan_result.num > 1) {
        wifi_list_sort(&s_wifi_handle->scan_result);
    }

    wifi_list_check(&s_wifi_handle->scan_result);

    //更新本地wifi信息列表
    for (j = 0; j < s_wifi_handle->scan_result.num; j++) {
        for (i = 0; i < s_wifi_handle->local_wifi_list.wifi_cnt; i++) {
            //连接状态 且 是当前连接的wifi，跳过
            if (s_wifi_handle->connected != 0 && 0 == strcmp(s_wifi_handle->cur_ap_info.ssid, s_wifi_handle->local_wifi_list.wifi_info[i].ssid)) {
                // printf(">>> %s\n", s_wifi_handle->local_wifi_list.wifi_info[i].ssid);
                continue;
            }

            //更新本地已有的wifi信息
            // printf("==>>>>>  %s %d\n", s_wifi_handle->local_wifi_list.wifi_info[i].ssid, strcmp(s_wifi_handle->cur_ap_info.ssid, s_wifi_handle->local_wifi_list.wifi_info[i].ssid));
            if (0 == strcmp(s_wifi_handle->local_wifi_list.wifi_info[i].ssid, s_wifi_handle->scan_result.ap[j].ssid)) {
                memcpy(&s_wifi_handle->wifi_list.ap[s_wifi_handle->wifi_list.num], &s_wifi_handle->scan_result.ap[j], sizeof(wifi_ap_info_t));
                // printf("wifi list add ssid:%s \n", s_wifi_handle->scan_result.ap[j].ssid);
                s_wifi_handle->wifi_list.num++;
            }
        }
    }

    if (s_wifi_handle->wifi_list.num > 1) {
        wifi_list_sort(&s_wifi_handle->wifi_list);
    }

    //把当前连接的wifi信息放到 显示列表 第一个
    if (s_wifi_handle->connected != 0) {
        // printf("---> %s %d\n", __func__, __LINE__);
        memcpy(&s_wifi_handle->show_list.wifi_info[s_wifi_handle->show_list.num].ap_info, &s_wifi_handle->cur_ap_info, sizeof(wifi_info_t));
        s_wifi_handle->show_list.wifi_info[s_wifi_handle->show_list.num].state = OTA_WIFI_CONNECT;
        // printf("show list add ssid:%s %d\n", s_wifi_handle->show_list.wifi_info[s_wifi_handle->show_list.num].ap_info.ssid, s_wifi_handle->show_list.wifi_info[s_wifi_handle->show_list.num].state);
        s_wifi_handle->show_list.num++;
    }

    //本地wifi信息列表
    for (i = 0; i < s_wifi_handle->wifi_list.num; i++) {
        for (j = 0; j < s_wifi_handle->local_wifi_list.wifi_cnt; j++) {
            if (0 == strcmp(s_wifi_handle->local_wifi_list.wifi_info[j].ssid, s_wifi_handle->wifi_list.ap[i].ssid)) {
                memset(&s_wifi_handle->show_list.wifi_info[s_wifi_handle->show_list.num].ap_info.ssid, 0, SSID_MAX_LEN + 1);
                memset(&s_wifi_handle->show_list.wifi_info[s_wifi_handle->show_list.num].ap_info.pwd, 0, PWD_MAX_LEN + 1);
                strcpy(s_wifi_handle->show_list.wifi_info[s_wifi_handle->show_list.num].ap_info.ssid, s_wifi_handle->local_wifi_list.wifi_info[j].ssid);
                strcpy(s_wifi_handle->show_list.wifi_info[s_wifi_handle->show_list.num].ap_info.pwd, s_wifi_handle->local_wifi_list.wifi_info[j].pwd);
                s_wifi_handle->show_list.wifi_info[s_wifi_handle->show_list.num].sec_type = s_wifi_handle->wifi_list.ap[i].sec_type;
                s_wifi_handle->show_list.wifi_info[s_wifi_handle->show_list.num].rssi = s_wifi_handle->wifi_list.ap[i].rssi;
                s_wifi_handle->show_list.wifi_info[s_wifi_handle->show_list.num].state = OTA_WIFI_SAVED;
                s_wifi_handle->show_list.num++;
            }
        }
    }

    for (i = 0; i < s_wifi_handle->scan_result.num; i++) {
        for (j = 0; j < s_wifi_handle->wifi_list.num; j++) {
            if (0 == strcmp(s_wifi_handle->wifi_list.ap[j].ssid, s_wifi_handle->scan_result.ap[i].ssid)) {
                break;
            }
        }

        if (s_wifi_handle->connected != 0 && 0 == strcmp(s_wifi_handle->cur_ap_info.ssid, s_wifi_handle->scan_result.ap[i].ssid)) {
            continue;
        }

        if (j == s_wifi_handle->wifi_list.num) {
            memcpy(&s_wifi_handle->show_list.wifi_info[s_wifi_handle->show_list.num].ap_info, &s_wifi_handle->scan_result.ap[i], sizeof(wifi_ap_info_t));
            s_wifi_handle->show_list.wifi_info[s_wifi_handle->show_list.num].sec_type = s_wifi_handle->scan_result.ap[i].sec_type;
            s_wifi_handle->show_list.wifi_info[s_wifi_handle->show_list.num].rssi = s_wifi_handle->scan_result.ap[i].rssi;
            s_wifi_handle->show_list.wifi_info[s_wifi_handle->show_list.num].state = OTA_WIFI_SCAN;
            // printf("show list add ssid:%s %d\n", s_wifi_handle->show_list.wifi_info[s_wifi_handle->show_list.num].ap_info.ssid, s_wifi_handle->show_list.wifi_info[s_wifi_handle->show_list.num].state);
            s_wifi_handle->show_list.num++;
        }
    }

    for (int k = 0; k < s_wifi_handle->show_list.num; k++) {
        printf("show list %dth ssid:%s  %d\n", k, s_wifi_handle->show_list.wifi_info[k].ap_info.ssid, s_wifi_handle->show_list.wifi_info[k].state);
    }
}

static void _wifi_task(void *arg)
{
    int ret = -1;
    printf("wifi task start\n");
    ret = wifi_adapter_init(native_app_wifi_cb);
    if (0 != ret) {
        LOG_ERR("wifi init faild!\n");
        goto exit;
    }

    os_adapter()->signal_post(s_wifi_handle->scan_sem);

    while (0 == s_wifi_handle->abort) {
        os_adapter()->signal_wait(s_wifi_handle->scan_sem, os_adapter()->get_forever_time());
        printf("[%s %d] going to scan wifi\n", __func__, __LINE__);
        s_fresh_upd_list = true;
        wifi_adapter_scan();
    }

exit:
    os_adapter()->thread_delete(&s_wifi_handle->scan_tid);
}

static void native_app_ota_wifi_task()
{
    int ret = -1;

    if (s_wifi_handle == NULL || s_wifi_handle->scan_tid != NULL || s_wifi_handle->cb_tid != NULL) {
        LOG_ERR("param error!\n");
        return;
    }

    ret = os_adapter()->thread_create(&s_wifi_handle->cb_tid,
                                      wifi_cb_process_task,
                                      NULL,
                                      "wifi_scan_cb",
                                      1024 * 20,
                                      OS_ADAPTER_PRIORITY_NORMAL + 1);
    if (ret != 0) {
        LOG_ERR("create wifi thread failed ret = %d.\n", ret);
        s_wifi_handle->cb_tid = NULL;
    }

    ret = os_adapter()->thread_create(&s_wifi_handle->scan_tid,
                                            _wifi_task,
                                            NULL,
                                            "_wifi_task",
                                            OS_ADAPTER_PRIORITY_NORMAL + 1,
                                            1024 * 5);
    if (ret != 0) {
        LOG_ERR("create wifi thread failed ret = %d.\n", ret);
        s_wifi_handle->scan_tid = NULL;
    }
}


static void native_app_ota_free()
{
    if (s_wifi_handle != NULL) {
        if (s_wifi_handle->mutex != NULL) {
            os_adapter()->mutex_delete(&s_wifi_handle->mutex);
            s_wifi_handle->mutex = NULL;
        }

        if (s_wifi_handle->scan_sem != NULL) {
            os_adapter()->signal_delete(&s_wifi_handle->scan_sem);
            s_wifi_handle->scan_sem = NULL;
        }

        if (s_wifi_handle->cb_sem != NULL) {
            os_adapter()->signal_delete(&s_wifi_handle->cb_sem);
            s_wifi_handle->cb_sem = NULL;
        }

        os_adapter()->free(s_wifi_handle->scan_result.ap);
        s_wifi_handle->scan_result.ap = NULL;
        os_adapter()->free(s_wifi_handle->wifi_list.ap);
        s_wifi_handle->wifi_list.ap = NULL;
        os_adapter()->free(s_wifi_handle->show_list.wifi_info);
        s_wifi_handle->show_list.wifi_info = NULL;
        os_adapter()->free(s_wifi_handle);
        s_wifi_handle = NULL;
    }
    if (s_ota_even_mutex != NULL) {
        os_adapter()->mutex_delete(&s_ota_even_mutex);
        s_ota_even_mutex = NULL;
    }
    if (s_ota_lv_handle != NULL) {
        os_adapter()->free(s_ota_lv_handle);
        s_ota_lv_handle = NULL;
    }
}

int native_app_ota_create_init_page(void)
{
    int ret = -1;

    lv_task(upgrade_logic_get_handle());
    os_adapter()->msleep(20);

    if (NULL == s_ota_lv_handle) {
        s_ota_lv_handle = (ota_lv_obj_t *)os_adapter()->calloc(1, sizeof(ota_lv_obj_t));
        if (NULL == s_ota_lv_handle) {
            LOG_ERR("ota malloc failed!\n");
            goto exit;
        }
    }

    s_ft_font = lv_get_ft_font();
    lv_lock();
    native_app_ota_main_cont_init();
    native_app_ota_start_page_init();
    lv_unlock();
    ret = 0;
exit:
    return ret;
}

int native_app_ota_enter(void *arg)
{
    int ret = -1;
    upgrade_cb_t cb = {0};
    ota_event_t event = OTA_EVENT_UNKNOW;
    upgrade_handle_t *hdl = upgrade_logic_get_handle();

    cb.start = native_app_ota_start_cb;
    cb.process = native_app_ota_process_cb;
    cb.finish = native_app_ota_finish_cb;
    cb.error = native_app_ota_error_cb;
    ret = upgrade_logic_init(&cb);
    if (0 != ret) {
        LOG_ERR("ota init err!\n");
        goto exit;
    }

    if (hdl->fw_handle.need_upgrade == 0 && hdl->app_handle.need_upgrade == 0) {
        printf("no need upgrade!\n");
        goto exit;
    }

    // if (NULL == s_ota_lv_handle) {
    //     s_ota_lv_handle = (ota_lv_obj_t *)os_adapter()->calloc(1, sizeof(ota_lv_obj_t));
    //     if (NULL == s_ota_lv_handle) {
    //         LOG_ERR("ota malloc failed!\n");
    //         goto exit;
    //     }
    // }

    if (NULL == s_wifi_handle) {
        s_wifi_handle = (ota_wifi_handle_t *)os_adapter()->calloc(1, sizeof(ota_wifi_handle_t));
        if (NULL == s_wifi_handle) {
            LOG_ERR("ota malloc failed!\n");
            goto exit;
        }
        s_wifi_handle->scan_tid = NULL;
        s_wifi_handle->auto_tid = NULL;
        s_wifi_handle->abort = 0;
        s_wifi_handle->connected = 0;
        s_wifi_handle->scan_result.size = SCAN_RESULT_MAX_NUM;
        s_wifi_handle->scan_result.ap = (wifi_ap_info_t *)os_adapter()->calloc(1, s_wifi_handle->scan_result.size * sizeof(wifi_ap_info_t));
        if (s_wifi_handle->scan_result.ap == NULL) {
            LOG_ERR("scan ap malloc failed!\n");
            goto exit;
        }

        //给本地保存且被扫描到的wifi预分配空间
        s_wifi_handle->wifi_list.size = SCAN_RESULT_MAX_NUM;
        s_wifi_handle->wifi_list.ap = (wifi_ap_info_t *)os_adapter()->calloc(1, s_wifi_handle->wifi_list.size * sizeof(wifi_ap_info_t));
        if (s_wifi_handle->wifi_list.ap == NULL) {
            LOG_ERR("wifi list malloc failed!\n");
            goto exit;
        }

        //给渲染的wifi信息预分配空间
        s_wifi_handle->show_list.size = SCAN_RESULT_MAX_NUM;
        s_wifi_handle->show_list.wifi_info = (ota_wifi_info_t *)os_adapter()->calloc(1, s_wifi_handle->show_list.size * sizeof(ota_wifi_info_t));
        if (s_wifi_handle->show_list.wifi_info == NULL) {
            LOG_ERR("show_list malloc failed!\n");
            goto exit;
        }

        void *mutex = NULL;
        if ((mutex = os_adapter()->mutex_create()) == NULL) {
            s_wifi_handle->mutex = NULL;
            LOG_ERR("mutex create failed\n");
            goto exit;
        }
        s_wifi_handle->mutex = mutex;

        void *sem = NULL;
        if ((sem = os_adapter()->signal_create(0, 1)) == NULL) {
            s_wifi_handle->scan_sem = NULL;
            LOG_ERR("sem create failed\n");
        }
        s_wifi_handle->scan_sem = sem;

        void *cb_sem = NULL;
        if ((cb_sem = os_adapter()->signal_create(0, 1)) == NULL) {
            s_wifi_handle->cb_sem = NULL;
            LOG_ERR("cb_sem create failed\n");
        }
        s_wifi_handle->cb_sem = cb_sem;
    }

    if (NULL == s_ota_even_mutex) {
        void *mutex = NULL;
        if ((mutex = os_adapter()->mutex_create()) == NULL) {
            LOG_ERR("mutex create failed\n");
            goto exit;
        }
        s_ota_even_mutex = mutex;
    }

    // s_ft_font = lv_get_bin_font();
    // s_ft_font = lv_get_ft_font();

    event_list_init();
    // lv_lock();
    // native_app_ota_main_cont_init();
    // lv_unlock();
    s_encoder_enable = hdl->encoder.enable;
    s_touchpad_enable = hdl->tp.enable;

    if (UPGRADE_METHOD_LOCAL == upgrade_logic_get_handle()->method) {
        s_cur_page_idx = OTA_PAGE_UPGRADE;
        lv_lock();
        native_app_ota_upgrade_page_init();
        lv_unlock();
        printf("[%s %d] going to up\n", __func__, __LINE__);
        native_app_ota_task();
    } else {
        s_cur_page_idx = OTA_PAGE_START;
        native_app_ota_wifi_task();
        // lv_lock();
        // native_app_ota_start_page_init();
        // lv_unlock();
    }

    while(1) {
        event = get_event();
        // printf("event: %d\n", event);
        if (event != OTA_EVENT_UNKNOW) {
            switch (event) {
                case OTA_EVENT_WIFI_FRESH:
                    if (s_fresh_upd_list) {
                        //不是wifi页面，不刷新列表,只去除刷新窗口
                        if (s_cur_page_idx != OTA_PAGE_WIFI || s_ota_lv_handle->wifi_obj == NULL) {
                            LOG_ERR("cur win no wifi page,no refresh wifi\n");
                            s_fresh_upd_list = false;
                            lv_lock();
                            if (s_ota_lv_handle->refresh_win) {
                                lv_obj_del(s_ota_lv_handle->refresh_win);
                                s_ota_lv_handle->refresh_win = NULL;
                            }
                            lv_unlock();
                            break;
                        }

                        //更新列表信息, 不要放在lv_lock里面
                        native_app_ota_wifi_list_update(true);
                        s_fresh_upd_list = false;
                        lv_lock();
                        native_app_ota_wifi_list_create();
                        if (s_ota_lv_handle->refresh_win) {
                            lv_obj_del(s_ota_lv_handle->refresh_win);
                            s_ota_lv_handle->refresh_win = NULL;
                            create_top_suc_win(get_real_ptr(hdl->note.wifi_fresh_succ));
                        }
                        lv_unlock();
                    }
                    break;
                case OTA_EVENT_WIFI_CONNECT:
                    wifi_connect(s_wifi_handle->cur_ap_info.ssid, s_wifi_handle->cur_ap_info.pwd, s_wifi_handle->cur_ap_info.sec_type);
                    break;
                case OTA_EVENT_WIFI_TOP_WIN:
                    if (s_cur_page_idx != OTA_PAGE_WIFI) {
                        LOG_ERR("cur win no wifi page,no refresh\n");
                        break;
                    }
                    lv_lock();
                    create_top_refresh_win();
                    lv_unlock();
                    os_adapter()->signal_post(s_wifi_handle->scan_sem);
                    break;
                case OTA_EVENT_KEY_UPDATE:
                    lv_lock();
                    native_app_ota_keybord_create(is_symbol? symbols : (is_uppercase? upper_keys : lower_keys));
                    if (strlen(lv_textarea_get_text(s_ota_lv_handle->textarea)) >= 8) {
                        lv_obj_set_style_bg_color(lv_obj_get_child(s_ota_lv_handle->kb_cont, 5), lv_color_hex(0x2094FA), 0);
                        lv_obj_set_style_bg_opa(lv_obj_get_child(s_ota_lv_handle->kb_cont, 5), LV_OPA_100, 0);
                    }
                    lv_unlock();
                    break;
                case OTA_EVENT_REBOOT:
                    s_wifi_handle->abort = 1;

                    //先让mcu跳到app
                    extern bool mcu_jump_to_app(uint8_t arg);
                    mcu_jump_to_app(1);

                    if (upgrade_logic_reboot(upgrade_logic_get_handle(), UPGRADE_MODE_NORMAL) != 0) {
                        upgrade_logic_reboot(upgrade_logic_get_handle(), UPGRADE_MODE_RECOVERY);
                    }
                    break;
                default:
                    native_app_ota_page_switch(event);
                    break;
            }
            
        }
        os_adapter()->msleep(10);
    }

exit:
    native_app_ota_free();
    if (upgrade_logic_reboot(upgrade_logic_get_handle(), UPGRADE_MODE_NORMAL) != 0) {
        upgrade_logic_reboot(upgrade_logic_get_handle(), UPGRADE_MODE_RECOVERY);
    }
    return ret;
}