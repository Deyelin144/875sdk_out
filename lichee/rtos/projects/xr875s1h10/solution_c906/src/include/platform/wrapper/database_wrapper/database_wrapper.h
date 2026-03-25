#ifndef _DRV_DATABASE_WRAPPER_H_
#define _DRV_DATABASE_WRAPPER_H_

#define DATABASE_MAX_SIZE 20
// #define DATABASE_MAX_NUMBER 10

// typedef enum {
//    	FIND_ALL = 0,
//     FIND_GET,
//     FIND_EACH,
//     NO_FIND
// }database_find_type_t;

// typedef struct{
//     database_find_type_t op_type;
//     void *db_handle;
//     char *root;
//     int callbackid;
//     int row;
//     void (*callback)(char *buf, int callback_id);
// }ret_info_db_t;

/**
 * @brief 打开数据库
 * @param option 打开数据库需要的信息
 * @param exec_result_cb 执行结果回调
 * @return 数据库句柄
 */
typedef void* (*db_open_t)(char * option, void *exec_result_cb);

/**
 * @brief 执行数据库的操作
 * @param db_handle 数据库句柄
 * @param sql sql语句
 * @return 成功返回0， 失败返回-1
 */
typedef int (*db_exec_t)(void * db_handle, char *sql);

/**
 * @brief 执行数据库的操作
 * @param db_handle 数据库句柄
 * @param sql sql语句
 * @return 数据库句柄
 */
typedef int (*db_close_t)(void * db_handle);

typedef struct {
    char name[DATABASE_MAX_SIZE];
    db_open_t open;
    db_exec_t exec;
    db_close_t close;
} db_wrapper_cb_t;

typedef struct {
    int db_num;
	db_wrapper_cb_t *db;
} db_wrapper_t;


#endif