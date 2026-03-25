#ifndef _REALIZE_UNIT_MQTT_H_
#define _REALIZE_UNIT_MQTT_H_

#include "../../platform/gulitesf_config.h"
#include "../realize_unit_hash/realize_unit_hash.h"

#ifdef CONFIG_MQTT_SUPPORT

#define MQTT_BUF_SIZE (1024)

typedef struct {
	char *data;
	unsigned int len;
} data_t;

typedef struct {
	data_t topic_name;
	data_t message;
	unsigned char retained;
	char qos;
} mqtt_will_t;

typedef struct {
	char *url;
	unsigned char mqtt_version;
	data_t client_id;
	unsigned short keep_alive_interval;
	unsigned char clean_session;
	mqtt_will_t will;
	data_t username;
	data_t password;
	unsigned short reconnect_period; //重连间隔时间，单位为毫秒，默认为 1000 毫秒，注意：当设置为 0 以后将取消自动重连
	unsigned short connect_timeout; //连接超时时长，收到 CONNACK 前的等待时间，单位为毫秒，默认 30000 毫秒
	char *ca; //只有在服务器使用自签名证书时才有必要，自签名证书中生成的 CA 文件
	unsigned short ca_len; //证书长度

	void (*mqtt_connect_cb)(void *userdata, int);	//mqtt 连接后调用
	void (*mqtt_reconnect_cb)(void *userdata);	//mqtt 重连
	void (*mqtt_close_cb)(void *userdata, void *);	//	mqtt 客户端主动断开
	void (*mqtt_offline_cb)(void *userdata);	// 不是客户端导致的异常断开，掉线
	void (*mqtt_error_cb)(void *userdata, char *);	// 出现各种错误的回调
} mqtt_packet_data_t;

//subscribe
typedef enum {
	MQTT_NONE = 0,
	MQTT_CONNECT,
	MQTT_RECONNECTING,
	MQTT_DISCONNECT,
} mqtt_status_t;

typedef struct {
	int qos;
    unsigned char retained;
    unsigned char dup;
    unsigned short id;
} array_msg_packet_t;

typedef struct {
	char *topic;
	char *payload;
	unsigned int payload_len;
	array_msg_packet_t packet;
} arrived_msg_t;

typedef void (*msg_arrived)(void *userdata, arrived_msg_t *);

typedef void (*subscribe_msg_unify_process_cb)(void *userdata, arrived_msg_t *msg);

typedef int (*mqtt_connect)(void *ctx, mqtt_packet_data_t *mqtt_packet_data, void *userdata);
typedef int (*mqtt_publish)(void *ctx, arrived_msg_t *msg);
typedef int (*mqtt_subscribe)(void *ctx, char *topic, int qos, void (*msg_arrived)(void *, arrived_msg_t *));
typedef int (*mqtt_unsubscribe)(void *ctx, char *topic);
typedef int (*mqtt_disconnect)(void *ctx);
typedef int (*mqtt_parse_url)(void *ctx, const char *url, char **host, char **port);
typedef mqtt_status_t (*mqtt_get_status)(void *ctx);

typedef struct {
	void *ctx;
	mqtt_connect connect;
	mqtt_publish publish;
	mqtt_subscribe subscribe;
	mqtt_unsubscribe unsubscribe;
	mqtt_disconnect disconnect;
	mqtt_parse_url parse_url;
	mqtt_get_status get_status;
} mqtt_obj_t;

mqtt_obj_t *realize_unit_mqtt_new(void);
void realize_unit_mqtt_delete(mqtt_obj_t **mqtt_obj);
char* realize_unit_mqtt_get_ip(void);

#endif
#endif

