
#ifndef __UPGRADE_LOGIC_H__
#define __UPGRADE_LOGIC_H__

#include "md5_adapter/md5_adapter.h"
#include "os_adapter/os_adapter.h"
#include "HTTPCUsr_api.h"

// #define IMAGE_BUFFER_SIZE (PRJCONF_IMAGE0_SIZE + PRJCONF_IMAGE0_ADDR)

typedef enum {
    UPGRADE_FAIL_NO_REPLACE,
    UPGRADE_FAIL_REPLACED,
    UPGRADE_SUCC,
} upgrade_status_t;

typedef enum {
    VERIFY_FAIL,
    VERIFY_SUCC,
} verify_status_t;

typedef enum {
    UPGRADE_TYPE_COMMON,
    UPGRADE_TYPE_FW,
    UPGRADE_TYPE_APP
} upgrade_type_t;

typedef enum {
    UPGRADE_METHOD_LOCAL,
    UPGRADE_METHOD_CLOUD,
} upgrade_method_t;

typedef enum {
    UPGRADE_MODE_NORMAL,
    UPGRADE_MODE_RECOVERY,
} upgrade_mode_t;

typedef enum {
    UPGRADE_ERR_OK,

    UPGRADE_ERR_MALLOC_FAIL,
    UPGRADE_ERR_MSG_FILE_NOT_FIND,
    UPGRADE_ERR_APP_FILE_OVERFLOW,

    UPGRADE_ERR_IMAGE_OVERFLOW,
    UPGRADE_ERR_IMAGE_CHECK,


    UPGRADE_ERR_JSON_NULL,
    UPGRADE_ERR_JSON_INVALID,

    UPGRADE_ERR_MD5_INIT,
    UPGRADE_ERR_MD5_UPDATE,
    UPGRADE_ERR_MD5_VERIFY,
    UPGRADE_ERR_MD5_GET_FAIL,

    UPGRADE_ERR_FS_OPEN,
    UPGRADE_ERR_FS_READ,
    UPGRADE_ERR_FS_WRITE,
    UPGRADE_ERR_FS_SEEK,
    UPGRADE_ERR_FS_STAT,
    UPGRADE_ERR_FS_CLOSE,

    UPGRADE_ERR_BAT_LOW,
    UPGRADE_ERR_HTTP_GET,
    UPGRADE_ERR_UNKNOW_FW,
} errno_t;

#define OTA_IMG_PATH_LEN 50
typedef struct {
    char path[OTA_IMG_PATH_LEN];
    int w;
    int h;
} ota_img_info_t;

typedef struct {
    ota_img_info_t ota_check;
    ota_img_info_t ota_delete;
    ota_img_info_t ota_end;
    ota_img_info_t ota_net_err;
    ota_img_info_t ota_question;
    ota_img_info_t ota_right_btn;
    ota_img_info_t ota_warn;
    ota_img_info_t ota_wifi_1;
    ota_img_info_t ota_wifi_2;
    ota_img_info_t ota_wifi_3;
    ota_img_info_t ota_wifi_connected;
    ota_img_info_t ota_wifi_info;
    ota_img_info_t ota_wifi_lock;
    ota_img_info_t ota_back_btn;
} ota_img_t;

typedef struct {
    char *name;
    char *url;
    char *data;
    char md5[MD5_LEN * 2 + 1];
    char file_change;
    int size;
} file_info_t;

typedef struct {
    upgrade_status_t upgrade_status;
    char is_force;
    errno_t err;
    char need_upgrade;
    int file_num;
    int file_size_all;
    int cache_size;
    void *fp;
    char *version;
    HTTPParameters *http_param;
    file_info_t *file_info;
} app_upgrade_handle_t;

typedef struct {
    char fex_name[50];
    char part_name[50];
    char is_force;
    upgrade_status_t upgrade_status;
    char *url;
    char md5[MD5_LEN * 2 + 1];
    // uint32_t size;      //目前只有网络升级时,mcu使用,其他核直接读分区大小
} fw_info_t;

typedef struct {
    char *image_buffer;
    errno_t err;
    upgrade_status_t upgrade_status;
    char need_upgrade;
    char *version; // 只使用RV版本号判断
    fw_info_t *fw_info;
    int fw_num;
    int file_size_all;
    void *fp;
    HTTPParameters *http_param;
} fw_upgrade_handle_t;

typedef struct ota_note {
    char *upgrade_preparing;
    char *upgrading;
    char *upgrading_notop;
    char *upgrade_succ_rebooting;
    char *upgrade_fail;
    char *upgrade_cancel;
    char *upgrade_fail_retry;
    char *help_connect;
    char *ok_to_connect;
    char *loading;
    char *wifi_press;
    char *wifi_fresh;
    char *wifi_freshing;
    char *wifi_fresh_succ;
    char *wifi_enter_pwd;
    char *wifi_connecting;
    char *wifi_connect_succ;
    char *wifi_connect_fail;
    char *wifi_conn_fail_info;
    char *wifi_retry_once;
    char *wifi_connected;
    char *wifi_list_connected;
    char *wifi_saved;
    char *wifi_rm_this_net;
    char *wifi_rm_sure;
    char *wifi_rm;
    char *wifi_only_2g;
    char *wifi_WIFI;
    char *bat_low;
} ota_note_t;

typedef struct {
    unsigned char enable;
    int type;
    int bl_mode;
    int te_support;
    int hor_res;
    int ver_res;
    double inch;
    int rotation;
} disp_t;

typedef struct {
    unsigned char enable;
} led_t;

typedef struct {
    unsigned char enable;
    int type;
} key_config_t;

typedef struct {
    unsigned char enable;
    int back;
    int ok;
    int left;
    int up;
    int right;
    int down;
} btn_t;

typedef struct {
    unsigned char enable;
    int type;
} tp_t;

typedef struct {
    unsigned char enable;
} encoder_t;

typedef struct {
    upgrade_method_t method;
    errno_t err;
    unsigned char bat_chk_succ;
    int adc_threshold;
    int adc_charge_threshold;
    disp_t disp;
    led_t led;
    tp_t tp;
    encoder_t encoder;
    key_config_t key;
    btn_t btn;
    ota_img_t img;
    ota_note_t note;
    char *msg_file;
    int max_file_size;
    char *max_file_buffer;
    app_upgrade_handle_t app_handle;
    fw_upgrade_handle_t fw_handle;
} upgrade_handle_t;

typedef struct {
    int (*start)(upgrade_type_t type, upgrade_method_t method, upgrade_handle_t *hdl);
    int (*process)(upgrade_type_t type, upgrade_method_t method, upgrade_handle_t *hdl, int percent);
    int (*finish)(upgrade_type_t type, upgrade_method_t method, upgrade_handle_t *hdl);
    int (*error)(upgrade_type_t type, upgrade_method_t method, upgrade_handle_t *hdl);
} upgrade_cb_t;

void upgrade_set_errno(upgrade_type_t type, errno_t err);
upgrade_handle_t *upgrade_logic_get_handle(void);
upgrade_cb_t *upgrade_logic_get_cb(void);
void upgrade_logic_set_replace(upgrade_handle_t *hdl, unsigned char replace);
unsigned char upgrade_logic_get_replace(void);
int upgrade_logic_reboot(upgrade_handle_t *hdl, upgrade_mode_t mode);
int upgrade_logic_get_otaimg_info(char *dst_name, char *cur_name, ota_img_info_t *img_info);
int upgrade_logic_pre_check(upgrade_handle_t *hdl, upgrade_type_t type);
int upgrade_logic_init(upgrade_cb_t *cb);
int upgrade_logic_deinit(void);
int upgrade_logic_get_config(void);

extern uint8_t __psram_end__[];

// 获取实际的地址
static inline void *get_real_ptr(void *ptr)
{
//    return ((void *)((uintptr_t)ptr & ~3));
    return ptr;
}

// // 获取地址第0位的值
// static inline int get_bit0(void *ptr) 
// {
//     return ((uintptr_t)ptr & 1);
// }

// // 获取地址第1位的值
// static inline int get_bit1(void *ptr) 
// {
//     return (((uintptr_t)ptr >> 1) & 1);
// }

// // 获取地址第0位和第1位的值
// static inline int get_bit0_and_bit1(void *ptr)
// {
//     return ((uintptr_t)ptr & 3);
// }

// // 设置地址第0位的值
// static inline void set_bit0(void **ptr, int bit_value)
// {
//     *ptr = (void *)(((uintptr_t)*ptr & ~1) | (bit_value & 1));
// }

// // 设置地址第1位的值
// static inline void set_bit1(void **ptr, int bit_value) 
// {
//     *ptr = (void *)(((uintptr_t)*ptr & ~2) | ((bit_value & 1) << 1));
// }

// // 同时设置地址第0位和第1位的值
// static inline void set_bit0_and_bit1(void **ptr, int bit_value)
// {
//     *ptr = (void *)(((uintptr_t)*ptr & ~3) | (bit_value & 3));
// }

static inline void try_sram_calloc(void **ptr, size_t size)
{
    *ptr = os_adapter()->calloc(1, size);
    // if (!*ptr) {
    //     *ptr = psram_calloc(1, size);
    // } else {
    //     set_bit0(ptr, 1);
    // }
}

static inline void try_sram_free(void *ptr)
{
    // if (get_bit0(ptr)) {
    //     free(get_real_ptr(ptr));
    // } else {
    //     psram_free(get_real_ptr(ptr));
    // }
    os_adapter()->free(ptr);
}

#endif