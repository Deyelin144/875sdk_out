#ifndef _CAMERA_WRAPPER_H_
#define _CAMERA_WRAPPER_H_

#include "../../gulitesf_config.h"
#include "../../../mod_realize/realize_unit_lv_expand/camera/realize_unit_lv_camera.h"

#ifdef CONFIG_CAMERA_SUPPORT

typedef unit_camera_src_info_t wrapper_camera_src_info_t;
typedef unit_camera_ctrl_info_t wrapper_camera_ctrl_info_t;
typedef unit_camera_data_t wrapper_camera_data_t;

typedef void *(*camera_open_t)(const char *path, wrapper_camera_src_info_t *src_info);

typedef int (*camera_close_t)(void **ctx);

typedef int (*camera_update_next_frame_t)(void *ctx, wrapper_camera_data_t *data);

typedef int (*camera_decoder_ctrl_t)(void *ctx, wrapper_camera_ctrl_info_t *ctrl_info);

typedef struct {
	camera_open_t c_open;
    camera_close_t c_close;
	camera_update_next_frame_t c_update_next_frame;
	camera_decoder_ctrl_t c_ctrl;
} camera_wrapper_t;

#endif
#endif