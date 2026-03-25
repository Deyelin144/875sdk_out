#ifndef __REALIZE_UNIT_HTTP_AGENT_H__
#define __REALIZE_UNIT_HTTP_AGENT_H__

#include "../realize_unit_hash/realize_unit_hash_v2.h"
#include "../realize_unit_thread_pool/realize_unit_thread_pool.h"
#include "../../platform/gulitesf_type.h"
#include "realize_unit_http.h"

//调度策略
typedef enum {
    AGENT_SCHED_FIFO = 0, // First In, First Out
    AGENT_SCHED_LIFO  // Last In, First Out
} scheduling_strategy_t;

typedef enum {
    REQ_FAIL_CLOSE_SOCKET   = -2,    // 请求失败且关闭socket
    REQ_FAIL_KEEP_SOCKET    = -1,    // 请求失败但保留socket
    REQ_SUCCESS             = 0,    // 请求成功
} agent_req_status_t;


typedef enum {
    AGENT_CONNECT_TIMEOUT               = 1,
    AGENT_MANY_OPEN_CONNECTIONS_ERROR   = 2,
    UNKNOWN_NETWORK_ERROR               = 3
} agent_err_t;

typedef struct {
    void *socket;
    unsigned long last_used;         //最后使用的时间戳  
} agent_socket_t;

typedef struct {
    int max_socket : 16;  //最大并发数
    int max_free_socket : 16;  //空闲状态下保持连接的最大数量
    char keep_live;     //是否启用keepalive
    int keep_alive_msecs;   //keep_live为ture时有效
    int max_total_sockets;       //最大连接数 
    scheduling_strategy_t scheduling;   //调度策略
    int timeout;
} agent_cfg_param_t;  

typedef void (*exec_cb_t)(void *);
typedef struct {
    void (*error_cb)(void *agent_key, void *usrdata, int err);
    void (*free_cb)(void *usrdata);
} report_cb_t;

typedef struct _agent_obj {
    void *ctx;
    report_cb_t report_cb;
    //创建一个新的实例, 返回一个key值，返回-1表示创建失败
    int (*create)(struct _agent_obj *, agent_cfg_param_t *, void *usr_data); 
    //设置host/port信息，成功返回 0，失败返回-1; 
    int (*set_host)(struct _agent_obj *, void *agent_key , char *url);
    //获取一个有效的socket
    void *(*get_valid_socket)(struct _agent_obj *, void *agent_key);  
    //销毁一个实例    
    void (*destroy)(struct _agent_obj *, void *agent_key); 
    //配置执行回调callback
    int (*set_exec_cb)(struct _agent_obj *, void *agent_key, exec_cb_t cb, void *arg);
    //获取一个agent
    void *(*get_agent_key)(struct _agent_obj *, char *url);

    /********************请求相关的函数************************/
    int (*request_header)(struct _agent_obj *, void *agent_key, unit_http_request_t *, char *url, char *header, char *method);  
    int (*request_body)(struct _agent_obj *, void *agent_key, unit_http_request_t *, char *body, int body_len);  
    int (*request_end)(struct _agent_obj *, void *agent_key, unit_http_request_t *);  
    int (*sync_request_end)(struct _agent_obj *, void *agent_key, unit_http_sync_request_t *); 
} http_agent_obj_t; 

//
http_agent_obj_t *realize_unit_http_agent_new(report_cb_t *cb);    
void realize_unit_http_agent_delete(http_agent_obj_t **agent);  

#endif