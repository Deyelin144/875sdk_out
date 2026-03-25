#ifndef __RECORD_WRAPPER_H__
#define __RECORD_WRAPPER_H__

#include "../../gulitesf_config.h"
#include "../../../mod_realize/realize_unit_record/realize_unit_record.h"

typedef int (*record_open_t)(record_pcm_config_t *config);
typedef int (*record_read_t)(void *data, unsigned int len);
typedef void (*record_close_t)(void);

typedef struct {
	record_open_t record_open;
	record_read_t record_read;
	record_close_t record_close;
} record_wrapper_t;

#endif