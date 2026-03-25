#ifndef __REALIZE_UNIT_SPEEX_H__
#define __REALIZE_UNIT_SPEEX_H__

#include "../../../platform/gulitesf_config.h"

#ifdef CONFIG_SPEEX_SUPPORT

// #include "../../realize_unit_record/realize_unit_record.h"

#define UNIT_SPEEX_GET_FRAME_SIZE    3
#define UNIT_SPEEX_SET_QUALITY       4

#define UNIT_SPEEX_HEADER_MARK "#!speex\n"

typedef enum {
	UNIT_SPEEX_ENCODE,
	UNIT_SPEEX_DECODE,
} unit_speex_codec_t;

typedef enum {
	UNIT_SPEEX_NB_MODE,
	UNIT_SPEEX_WB_MODE
} unit_speex_mode_t;

typedef struct {
	unit_speex_codec_t codec;
	unit_speex_mode_t mode;
	int quality;
	int frame_size;
	int write_header;
} speex_param_t;

typedef int (*speex_create)(void *ctx, speex_param_t *param, void *userdata);
typedef int (*speex_codec)(void *ctx, void *in, int in_size, void *out, int *out_len);
typedef void (*speex_destory)(void *ctx);
typedef int (*speex_set_param_using_json)(void *ctx, char *json_str);
typedef int (*speex_get_param)(void *ctx, const char **param);
typedef int (*speex_get_header)(void *ctx, const char **header, int *len);
typedef int (*speex_prase_header)(void *ctx, const char *header, speex_param_t *speex_param, int *header_len);

typedef struct {
	void *ctx;
	speex_create create;
	speex_codec codec;
	speex_destory destory;
	speex_set_param_using_json set_param_using_json;
	speex_get_param get_param;
	speex_get_header get_header;
	speex_prase_header prase_header;
} speex_obj_t;

speex_obj_t *realize_unit_speex_new(void);
void realize_unit_speex_delete(speex_obj_t **speex_obj);
#endif
#endif