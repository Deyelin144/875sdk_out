#ifndef _REALIZE_UNIT_DATABASE_H_
#define _REALIZE_UNIT_DATABASE_H_

#include "../../platform/gulitesf_config.h"

#ifdef CONFIG_DATABASE_SUPPORT

typedef enum {
	UNIT_SQL_TYPE_BUFFER = 0,
	UNIT_SQL_TYPE_STRING,
	UNIT_SQL_TYPE_NULL,
	UNIT_SQL_TYPE_INT,
	UNIT_SQL_TYPE_FLOAT
} unit_sqlparam_type_t;

typedef struct {
	union {
		unsigned char *p;
		double d;
		int i;
	} data;
	int data_len : 24;
	unit_sqlparam_type_t type : 8;
} unit_sqlparam_t;

typedef struct {
	char *sql;
	unit_sqlparam_t *param_array;
	int array_size;
} unit_sql_t;

typedef struct {
	unit_sqlparam_t **result;
    char **column_name;
	int column;
	int row;
} unit_db_result_t;

typedef int (*db_result_cb_t)(void *userdata, unit_db_result_t *result, int is_last_frame);
typedef void (*db_error_cb_t)(void *userdata, const char *err);
typedef int (*db_set_cb_t)(void *userdata, int is_err, char *err_msg);
typedef struct {
	db_result_cb_t result;
	db_error_cb_t error;
	db_set_cb_t set;
} db_event_cb_t;

typedef int (*unit_db_open_t)(void *obj, char *option);
typedef int (*unit_db_close_t)(void *obj);
typedef int (*unit_db_exec_t)(void *obj, unit_sql_t *sql);
typedef void (*unit_db_free_t)(unit_sql_t **sql);

typedef struct db_obj{
	void *ctx;
	void *user_data;
	db_event_cb_t db_event_cb;
	unit_db_open_t db_open;
	unit_db_close_t db_close;
	unit_db_exec_t db_exec;
	unit_db_free_t db_free;
} db_obj_t;

db_obj_t *realize_unit_database_new(char *dbname, void *userdata, db_event_cb_t db_cb);
void realize_unit_database_delete(db_obj_t **obj);

#endif // CONFIG_DATABASE_SUPPORT

#endif