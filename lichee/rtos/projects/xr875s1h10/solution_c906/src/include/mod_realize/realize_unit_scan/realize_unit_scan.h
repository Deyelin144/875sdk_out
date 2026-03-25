#ifndef _REALIZE_UNIT_CAMERA_H_
#define _REALIZE_UNIT_CAMERA_H_
#include "../../platform/gulitesf_config.h"
#ifdef CONFIG_FUNC_SCAN_READ

#define UNIT_SCAN_RECV_QUEUE_MAX CONFIG_SCAN_READ_BUFF_NUM_SIZE

typedef struct {
    char *content;
	int opt;
} unit_nlp_msg_t;

typedef struct {
	void *data;
	unsigned int data_len;
	unsigned int last_data_state;
} scan_data_t; //这里的命名需要修改

typedef enum {
    SCAN_MODE_TEXT,
    SCAN_MODE_IMG,
} scan_mode_t;

typedef int scan_open_cb(void *);

typedef struct {
	int (*scan_open)(scan_open_cb cb, void *arg);
	int (*scan_write)(scan_data_t* scan_data);
	int (*scan_close)(void);
} scan_cb_t; //这里为输出的回调接口

typedef struct scan_ctx {
	void *user_data;
	scan_data_t pic_data;
	int (*scan_open)(struct scan_ctx**);
	int (*scan_write)(struct scan_ctx**);
	int (*scan_close)(struct scan_ctx**);
} unit_scan_ctx_t;	

int realize_unit_scan_init(unit_scan_ctx_t *ctx);
void realize_unit_scan_deinit(void);
unit_scan_ctx_t *realize_unit_scan_get_ctx(void);
int realize_unit_scan_nlp_send(const char *content, int opt);
int realize_unit_scan_nlp_stop(void);
int realize_unit_scan_set_tts_speed(float speed);
int realize_unit_scan_control(char *json_str);

#endif
#endif

