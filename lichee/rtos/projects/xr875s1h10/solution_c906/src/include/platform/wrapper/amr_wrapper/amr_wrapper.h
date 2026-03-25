#ifndef __AMR_WRAPPER_H__
#define __AMR_WRAPPER_H__

#include "../../gulitesf_config.h"

#if defined(CONFIG_AMR_SUPPORT) && !defined(CONFIG_AMR_USER_INTERNAL)

typedef enum {
    AMR_ENCODE_MR475 = 0,
    AMR_ENCODE_MR515,
    AMR_ENCODE_MR59,
    AMR_ENCODE_MR67,
    AMR_ENCODE_MR74,
    AMR_ENCODE_MR795,
    AMR_ENCODE_MR102,
    AMR_ENCODE_MR122,
    AMR_ENCODE_MRDTX
}amr_encode_rate_t;

typedef void *(*encoder_init_t)(int dtx);
typedef int (*encode_t)(int *enstate, void *raw, unsigned char *amr_buf, amr_encode_rate_t rate);
typedef void (*encoder_exit_t)(int *enstate);

typedef struct {
	encoder_init_t encoder_init;
	encode_t encode;
	encoder_exit_t encoder_exit;
} amr_wrapper_t;

#endif

#endif