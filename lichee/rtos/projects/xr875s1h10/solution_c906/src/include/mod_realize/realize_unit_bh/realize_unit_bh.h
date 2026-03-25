#ifndef _REALIZE_UNIT_BH_H_
#define _REALIZE_UNIT_BH_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stdbool.h"
#include "../realize_unit_log/realize_unit_log.h"
#include "../realize_unit_mem/realize_unit_mem.h"

#define ON_DEBUG 0

#if ON_DEBUG
#define bh_log_debug(format, ...)
#define bh_log_info(format, ...)
#else
#define bh_log_debug realize_unit_log_debug
#define bh_log_info realize_unit_log_info
#endif
#define bh_log_wran realize_unit_log_warn
#define bh_log_error realize_unit_log_error
#define bh_log_printf realize_unit_log_print

#define bh_malloc realize_unit_mem_malloc
#define bh_free realize_unit_mem_free

typedef struct {
	int a;
} default_data_t;

typedef struct {
	bool (*data_cmp)(void *u_data, void *a, void *b);
	void (*print)(void *u_data, void *data);
	bool (*match)(void *u_data, void *a, void *b);
	bool (*modify)(void *u_data, void *original, void *destination);
} bh_vtbl_t;

typedef struct _bh_obj {
	bh_vtbl_t *vptr;
	void *ctx;
	void *user_data;
	int (*create)(struct _bh_obj *obj);
	void (*destory)(struct _bh_obj *obj);
	int (*insert_node)(struct _bh_obj *obj, void *value);
	int (*delete_node)(struct _bh_obj *obj, int idx, void **node);
	int (*modify_node)(struct _bh_obj *obj, int idx, void *value);
	int (*search_node)(struct _bh_obj *obj, int start_idx, void *value);
	void (*print)(struct _bh_obj *obj);
	int (*get_size)(struct _bh_obj *obj);
	int (*get_capacity)(struct _bh_obj *obj);
} bh_obj_t;

bh_obj_t *realize_unit_bh_new(int capacity, void *user_data);
void realize_unit_bh_delete(bh_obj_t **obj);

#endif


