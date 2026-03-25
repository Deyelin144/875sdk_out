#ifndef __REALIZE_UNIT_OPUS_H__
#define __REALIZE_UNIT_OPUS_H__

#include "../../platform/gulitesf_config.h"

#ifdef CONFIG_OPUS_SUPPORT

typedef struct {
    size_t total_allocated;    // 总分配内存
    size_t total_freed;        // 总释放内存
    size_t peak_usage;         // 峰值内存使用量
} opus_mem_t;

typedef enum {
	UNIT_OPUS_ENCODE,
	UNIT_OPUS_DECODE,
} unit_opus_codec_t;

typedef struct {
	unit_opus_codec_t codec;
    int sample_rate;
	short frame_size;
    char quality;
    char channels : 6;
    char bit_width : 8;
} opus_param_t;

typedef int (*opus_create)(void *ctx, opus_param_t *param, void *userdata);
typedef int (*opus_codec)(void *ctx, void *in, int in_size, void *out, int *out_len);
typedef void (*opus_destory)(void *ctx);
typedef int (*opus_set_param_using_json)(void *ctx, char *json_str);
typedef int (*opus_get_param)(void *ctx, const char **param);
typedef int (*opus_get_header)(void *ctx, const char **header, int *len);
// typedef int (*opus_prase_header)(void *ctx, const char *header, opus_param_t *opus_param, int *header_len);

typedef struct {
	void *ctx;
	opus_create create;
	opus_codec codec;
	opus_destory destory;
	opus_set_param_using_json set_param_using_json;
    opus_get_header get_header;
	opus_get_param get_param;
} opus_obj_t;

opus_obj_t *realize_unit_opus_new(void);
void realize_unit_opus_delete(opus_obj_t **opus_obj);
#endif
#endif