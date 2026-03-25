#ifndef _REALIZE_UNIT_PQUEUE_H_
#define _REALIZE_UNIT_PQUEUE_H_

#include "../realize_unit_bh/realize_unit_bh.h"
#include "../../platform/wrapper/wrapper.h"

#define pqueue_log_debug bh_log_debug
#define pqueue_log_info bh_log_info
#define pqueue_log_wran bh_log_wran
#define pqueue_log_error bh_log_error
#define pqueue_log_printf bh_log_printf

#define pqueue_malloc bh_malloc
#define pqueue_free bh_free

#define PQUEUE_NODE_DESCRIBE_SIZE 24

typedef enum {
	PQUEUE_URGENT = 0,
	PQUEUE_HIGH,
	PQUEUE_ABOVE_NORMAL,
	PQUEUE_NORMAL,
	PQUEUE_UNDER_NORMAL,
	PQUEUE_LOW,
} pqueue_priority_t;

typedef enum {
	PQUEUE_ADD,
	PQUEUE_DELETE,
	PQUEUE_MODIFY,
} pqueue_cmd_t;

typedef struct {
	int priority;
	char describe[PQUEUE_NODE_DESCRIBE_SIZE];
	pqueue_cmd_t cmd;
	void *msg;
} pqueue_push_param_t;

typedef struct {
	void (*free_cb)(void *msg);
} pqueue_cb_t;

typedef struct _pqueue_obj {
	bh_obj_t *super;
	void *ctx;
	void *user_data;
	pqueue_cb_t *pqueue_cb;
	int (*push)(struct _pqueue_obj* obj, pqueue_push_param_t push_param[], int num);
	int (*pop)(struct _pqueue_obj* obj, void **msg);
	int (*get_size)(struct _pqueue_obj* obj);
	int (*get_capacity)(struct _pqueue_obj* obj);
} pqueue_obj_t;

pqueue_obj_t *realize_unit_pqueue_new(int capacity, void *user_data, pqueue_cb_t *cb);
void realize_unit_pqueue_delete(pqueue_obj_t **obj);

#endif
