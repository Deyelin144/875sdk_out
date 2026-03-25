#ifndef __REALIZE_UNIT_WEBSOCKET_H__
#define __REALIZE_UNIT_WEBSOCKET_H__

#define WEBSOCKET_DEFAULT_PORT 80
#define WEBSOCKET_SSL_DEFAULT_PORT 443
#define REASON_MAX_LEN  123

#define CA_CERT_PATH "lfs:/ca"

typedef enum {
	WS_CONNECTING,
	WS_CONNECTED,
	WS_DISCONNECTING,
    WS_DISCONNECT,
	WS_CONN_ERR,
    WS_SEND_DATA_ERR,
    WS_RECV_DATA_ERR
} unit_ws_conn_state_t;

typedef enum {
	WS_BINARY,
	WS_STRING,
    WS_UNKNOW
} buf_type_t;

typedef struct {
    int code;
    char reason[REASON_MAX_LEN + 1];
} unit_ws_close_t;

typedef struct {
	char* content;		    //发送内容
	unsigned int size;		//发送内容大小
	buf_type_t type;	        //发送内容类型
} unit_ws_send_buf_t;

typedef struct {
    char *ca;
    int ca_len;
    char *key;
    int key_len;
    char *cert;
    int cert_len;
    char *chain_cert;
    int chain_cert_len;
} unit_ws_ssl_info_t;

typedef struct {
    char *host;
    char ip[16];
    char *port;
    char *path;
    int use_ssl;
    unit_ws_ssl_info_t ssl_info;
} unit_ws_conn_info_t;

typedef struct {
    char *payload;
    int payload_len;
    buf_type_t payload_type;    //数据类型
    unsigned char is_final;     //是否最后一帧数据
    unsigned char is_fragment;  //数据是否是片段
} unit_ws_msg_t;

typedef struct {
    char **protocol;
    int num;
} unit_ws_protocol_t;

typedef struct _unit_ws{
    void *context;
    void *mutex;
    char *url;
    void *user_data;
    int timeout_ms;
    void *listener_thread_id;
    unsigned char stop_listener_task;
    unit_ws_conn_state_t conn_state;
    char record_truncation[4];
    char record_truncation_len; // 记录截断字节数

    void (*websocket_open_cb)(struct _unit_ws *, char *);	            //当WebSocket创建成功时，触发open事件
	void (*websocket_message_cb)(struct _unit_ws *, unit_ws_msg_t *);	    //当客户端收到服务端发来的消息时，触发message事件
	void (*websocket_close_cb)(struct _unit_ws *);	            //当客户端收到服务端发送的关闭连接请求时，触发close事件
	void (*websocket_error_cb)(struct _unit_ws *, int );	            //如果出现连接、处理、接收、发送数据失败的时候触发error事件
} unit_ws_t;


int realize_unit_websocket_connect(unit_ws_t *ws, unit_ws_protocol_t *ws_protocol, char *extra_headers);
int realize_unit_websocket_send(unit_ws_t *ws, unit_ws_send_buf_t *send_buf);
int realize_unit_websocket_disconnect(unit_ws_t *ws, unit_ws_close_t *close);

#endif // __REALIZE_UNIT_WEBSOCKET_H__