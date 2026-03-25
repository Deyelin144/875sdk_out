#ifndef __WEBSOCKET_WRAPPER_H__
#define __WEBSOCKET_WRAPPER_H__

#include "../../gulitesf_config.h"

#if defined(CONFIG_WEBSOCKET_SUPPORT) && !defined(CONFIG_WEBSOCKET_USER_INTERNAL)
#include "../../../mod_realize/realize_unit_websocket/realize_unit_websocket.h"

/**
 * @brief ws客户端连接
 * @param conn_info：连接信息
 * @param ws_protocol：子协议
 * @param extra_headers：额外的头部信息
 * @param timeout_ms：连接超时时间
 * @return 连接成功则返回一个ws句柄，失败返回空
 */
typedef void *(*websocket_client_connect_t)(unit_ws_conn_info_t *conn_info, unit_ws_protocol_t * ws_protocol, char *extra_headers, int timeout_ms);

/**
 * @brief 断开连接
 * @param context：ws句柄
 * @param close：断开的原因，可为NULL
 * @return 成功返回0，否则返回-1
 */
typedef int (*websocket_client_disconnect_t)(void *context, unit_ws_close_t *close);

/**
 * @brief 发送消息
 * @param context：ws句柄
 * @param send_buf：数据包
 * @param timeout_ms：超时时间
 * @return 成功返回0，否则返回-1
 */
typedef int (*websocket_client_send_t)(void *context, unit_ws_send_buf_t *send_buf, int timeout_ms);

/**
 * @brief 接收消息
 * @param context：ws句柄
 * @param msg：接收的数据包
 * @return 成功返回0，否则返回-1
 */
typedef int (*websocket_client_recv_t)(void *context, unit_ws_msg_t *msg);

typedef struct {
	websocket_client_connect_t client_connect;
	websocket_client_send_t client_send;
	websocket_client_disconnect_t client_disconnect;
	websocket_client_recv_t client_recv;
} websocket_wrapper_t;

#endif

#endif // !__WEBSOCKET_WRAPPER_H__

