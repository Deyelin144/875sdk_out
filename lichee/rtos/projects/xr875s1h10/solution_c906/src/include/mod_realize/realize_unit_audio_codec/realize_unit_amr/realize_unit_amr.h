#ifndef __REALIZE_UNIT_AMR_H__
#define __REALIZE_UNIT_AMR_H__

#include "gulitesf_config.h"

#ifdef CONFIG_AMR_SUPPORT

// #include "../realize_unit_record/realize_unit_record.h"

#define AMR_NB_HEADER   "#!AMR\n"
#define AMR_WB_HEADER   "#!AMR-WB\n"

typedef enum {
	UNIT_AMR_ENCODE,
	UNIT_AMR_DECODE,
} amr_codec_t;

typedef enum {
    AMR_NB_MODE,
    AMR_WB_MODE,
} amr_mode_t;

typedef enum {
    UNIT_AMR_ENCODE_MR475 = 0,
    UNIT_AMR_ENCODE_MR515,
    UNIT_AMR_ENCODE_MR59,
    UNIT_AMR_ENCODE_MR67,
    UNIT_AMR_ENCODE_MR74,
    UNIT_AMR_ENCODE_MR795,
    UNIT_AMR_ENCODE_MR102,
    UNIT_AMR_ENCODE_MR122,
    UNIT_AMR_ENCODE_MRDTX
} amr_encode_rate_t;

typedef enum {
    UNIT_AMR_ENCODE_WB_MD66 = 0,
    UNIT_AMR_ENCODE_WB_MD885,
    UNIT_AMR_ENCODE_WB_MD1265,
    UNIT_AMR_ENCODE_WB_MD1425,
    UNIT_AMR_ENCODE_WB_MD1585,
    UNIT_AMR_ENCODE_WB_MD1825,
    UNIT_AMR_ENCODE_WB_MD1985,
    UNIT_AMR_ENCODE_WB_MD2305,
    UNIT_AMR_ENCODE_WB_MD2385,
    UNIT_AMR_ENCODE_WB_N_MODES,
} amr_wb_encode_rate_t;

typedef enum {
    UNIT_AMR_WB_FRAMETYPE_DEFAULT = 0,
    UNIT_AMR_WB_FRAMETYPE_ITU,
    UNIT_AMR_WB_FRAMETYPE_RFC3267,
    UNIT_AMR_WB_FRAMETYPE_TMAX
} amr_wb_frametype_t;

typedef struct {
	amr_codec_t codec;
    amr_mode_t mode;
    union {
        amr_encode_rate_t nb;
        amr_wb_encode_rate_t wb;
    } rate;
    union {
        amr_wb_frametype_t wb;
    } frametype;
} amr_param_t;

typedef int (*amr_create)(void *ctx, amr_param_t *param, void *userdata);
typedef int (*amr_codec)(void *ctx, void *in, int in_size, void *out, int *out_len);
typedef void (*amr_destory)(void *ctx);
typedef int (*amr_get_header)(void *ctx, const char **header, int *len);
typedef int (*amr_set_param_using_json)(void *ctx, char *json_str); 
typedef int (*amr_get_param)(void *ctx, const char **param);


typedef struct {
	void *ctx;
	amr_create create;
	amr_codec codec;
	amr_destory destory;
	amr_get_header get_header;
	amr_set_param_using_json set_param_using_json;
	amr_get_param get_param;
} amr_obj_t;

amr_obj_t *realize_unit_amr_new(void);
void realize_unit_amr_delete(amr_obj_t **);

#endif
#endif


