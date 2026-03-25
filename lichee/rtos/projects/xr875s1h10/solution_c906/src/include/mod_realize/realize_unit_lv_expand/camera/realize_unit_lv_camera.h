#ifndef _REALIZE_UNIT_LV_CAMERA_H_
#define _REALIZE_UNIT_LV_CAMERA_H_

#include "../../platform/gulitesf_config.h"

#ifdef CONFIG_CAMERA_SUPPORT

#include "stdbool.h"
#include "../../realize_unit_log/realize_unit_log.h"
#include "../../realize_unit_mem/realize_unit_mem.h"

#define camera_log_debug realize_unit_log_debug
#define camera_log_info realize_unit_log_info
#define camera_log_wran realize_unit_log_warn
#define camera_log_error realize_unit_log_error

#define camera_malloc realize_unit_mem_malloc
#define camera_free realize_unit_mem_free
#define camera_calloc realize_unit_mem_calloc

typedef void (*camera_copy_data_t)(void *arg, unsigned char *yuv_data);

typedef enum {
	ASPECT_RATIO_1_1 = 0,
	ASPECT_RATIO_3_4,
	ASPECT_RATIO_16_9,
	ASPECT_RATIO_FULL,
	ASPECT_RATIO_MAX
} unit_camera_aspect_ratio_t;

typedef enum {
	CAMERA_FORMAT_JPG,
	CAMERA_FORMAT_RAW,
	CAMERA_FORMAT_PNG,
	CAMERA_FORMAT_BMP,
	CAMERA_FORMAT_BIN,
} unit_camera_picture_format_t;

typedef enum {
	CAMERA_MODE_NULL = 0,
	CAMERA_MODE_TAKE_PHOTO,
	CAMERA_MODE_VIDEO_RECORD,
	//auto scan and auto return resault type, eg:qrdecode, Identify items etc.
	CAMERA_MODE_QRDECODE,
	CAMERA_MODE_NUM,
} unit_camera_funcmode_t;

typedef struct {
	unsigned int width : 16;
	unsigned int height : 16;
	char frame_rate_num;
	bool has_alpha;
} unit_camera_src_info_t;

typedef enum {
	CAMERA_CMD_TAKE_PHOTO,
	CAMERA_CMD_NUM,
} unit_lv_camera_cmd_t;

typedef struct {
	unsigned short w;
	unsigned short h;
	unit_camera_picture_format_t format;
	char *path;
} unit_camera_picture_t;

typedef struct {
	unit_camera_picture_t *picture;
	char num;
	const char *data;
	int data_len;
	unsigned short src_w;
	unsigned short src_h;
} cmd_photo_info_t;

typedef struct {
	unit_lv_camera_cmd_t cmd;
	union {
		cmd_photo_info_t photo_info;
		int d;
	} data;
} unit_camera_ctrl_info_t;

typedef struct {
	// int size;
	// unsigned char *dst_data;
	void *arg;
	camera_copy_data_t copy_data;
} unit_camera_data_t;

typedef enum {
	CAMERA_TYPE_FRONT,
	CAMERA_TYPE_REAR
} unit_camera_type_t;

typedef struct {
	unsigned short w;
	unsigned short h;
	unit_camera_aspect_ratio_t aspect_ratio;
	unsigned char type;
} unit_camera_param_t;

//录像-开始-停止后续增加
typedef struct _camera_obj {
    void *ctx;
    void *user_data;
    int (*lv_create)(struct _camera_obj *obj, void *parent);
    int (*start_preview)(struct _camera_obj *obj, char *path);
	int (*pasue_preview)(struct _camera_obj *obj);
	int (*resume_preview)(struct _camera_obj *obj);
    int (*capture)(struct _camera_obj *obj, unit_camera_picture_t *pic, int num);
	int (*scan_identification)(struct _camera_obj *obj, unsigned short x, unsigned short y, unsigned short w, unsigned short h, void *arg,
								int (*output_cb)(void *arg, int errcode, int total, int cur_num, unsigned char *payload, int len));
    int (*set_cmd)(struct _camera_obj *obj, unit_lv_camera_cmd_t cmd, void *val);
	int (*set_mode)(struct _camera_obj *obj, unit_camera_funcmode_t camera_funcmode);
	int (*get_mode)(struct _camera_obj *obj, int curr);
	int (*lv_destory)(struct _camera_obj *obj);
} camera_obj_t;

camera_obj_t *realize_unit_camera_new(void *user_data);
void realize_unit_camera_delete(camera_obj_t **obj);

#endif
#endif