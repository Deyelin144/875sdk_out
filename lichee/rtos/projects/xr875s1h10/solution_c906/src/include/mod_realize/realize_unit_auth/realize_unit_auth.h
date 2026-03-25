
#ifndef _GU_AUTH_H_
#define _GU_AUTH_H_
#include "../../platform/gulitesf_config.h"

#ifdef CONFIG_USE_AUTH

// #define DEFAULT_AUTH_FILE_NAME "guLicense.txt"
#define DEFAULT_AUTH_TIMEOUT_CNT CONFIG_AUTH_REPORT_CNT
#define DEFAULT_AUTH_INTERVAL_MS (7 * 24 * 60 * 60 * 1000)

typedef enum {
    AUTH_STATUS_INIT,
    AUTH_STATUS_AUTH_FAIL = AUTH_STATUS_INIT,
    AUTH_STATUS_NO_LICENSE,
    AUTH_STATUS_DOWNLOADING,
    AUTH_STATUS_DOWNLOAD_FAIL,
    AUTH_STATUS_DOWNLOAD_FINISHED,
    AUTH_STATUS_AUTHING,
    AUTH_STATUS_AUTH_FINISHED
} auth_status_t;

typedef void (*auth_cb_t)(int timeout_cnt);

typedef struct gu_auth_info {
    char *auth_path;
    auth_status_t auth_status;
    unsigned int timer_id;
    int timeout_cnt;
    auth_cb_t auth_cb;
    void *mutex;
    void *power_off_tid;
    long last_auth_time;
    unsigned char match_old;
    unsigned char need_request;
    unsigned char is_replaced;
    unsigned char passed;
} gu_auth_info_t;

typedef struct realize_unit_auth_ctx {
    void *user_data;
    int (*gu_auth_cb)(struct realize_unit_auth_ctx *ctx, int type, int cnt);
} realize_unit_auth_ctx_t;

realize_unit_auth_ctx_t *realize_unit_auth_get_ctx(void);
void realize_unit_auth_set_request(unsigned char is_need_request);
unsigned char realize_unit_auth_is_need_request(void);
long realize_unit_auth_get_last_auth_time(void);
unsigned char realize_unit_auth_is_start(void);
auth_status_t realize_unit_auth_get_status(void);
int realize_unit_auth_create_timer(void);
int realize_unit_auth_start(const char *auth_path, auth_cb_t auth_cb);
int realize_unit_auth_trigger(const char *func);
void realize_unit_auth_init(realize_unit_auth_ctx_t *gu_auth_ctx);
void realize_unit_auth_deinit(void);
#endif

#endif