#ifndef _MQTT_WRAPPER_H_
#define _MQTT_WRAPPER_H_

#include "../../gulitesf_config.h"
#include "../../../mod_realize/realize_unit_mqtt/realize_unit_mqtt.h"


#if defined(CONFIG_MQTT_SUPPORT) && !defined(CONFIG_MQTT_USER_INTERNAL)

/**
 * @brief 网络层初始化
 * @param network：
 * @param url：url
 * @return 成功则返回0，失败返回-1
 */
typedef int (*mqtt_network_init_t)(void **network, void *url);

/**
 * @brief 初始化mqtt
 * @param client:
 * @param network:
 * @param timeout_ms:
 * @return 成功则返回0，失败返回-1
 */
typedef int (*mqtt_client_init_t)(void **client, void *network, int timeout_ms,\
						unsigned char* send_buf, size_t send_buf_size,\
						unsigned char* recv_buf, size_t recv_buf_size);

/**
 * @brief mqtt网络层连接
 * @param 
 * @return 成功则返回0，失败返回-1
 */
typedef int (*mqtt_network_connect_t)(void *network, const char *host, const char *port, char *ca, int ca_len);

/**
 * @brief mqtt客户端连接
 * @param subscribe_msg_unify_process_cb: 订阅的消息的回调函数
 * @return 成功则返回0，失败返回-1
 */
typedef int (*mqtt_client_connect_t)(void *client, mqtt_packet_data_t *data, subscribe_msg_unify_process_cb cb);

/**
 * @brief 订阅主题
 * @param client：句柄
 * @param topic：主题
 * @param qos：等级
 * @return 成功则返回0，失败返回-1
 */
typedef int (*mqtt_client_subscribe_t)(void *client, char *topic, int qos, void *userdata);

/**
 * @brief 取消订阅
 * @param client：句柄
 * @param topic：主题
 * @return 成功则返回0，失败返回-1
 */
typedef int (*mqtt_client_unsubscribe_t)(void *client, char *topic);

/**
 * @brief 发布消息
 * @param 
 * @return 成功则返回0，失败返回-1
 */
typedef int (*mqtt_client_publish_t)(void *client, char *topic, char *buf, unsigned int len,\
							int qos, char retained, char dup, unsigned short id);

/**
 * @brief 断开客户端连接
 * @param 
 * @return 成功则返回0，失败返回-1
 */
typedef int (*mqtt_client_disconnect_t)(void *client);

/**
 * @brief 断开网络层连接
 * @param 
 * @return
 */
typedef void (*mqtt_network_disconnect_t)(void *network);

/**
 * @brief 
 * @param 
 * @return 成功则返回0，失败返回-1
 */
typedef int (*mqtt_client_yield_t)(void *client, unsigned int timeout_ms);

typedef struct {
	mqtt_network_init_t network_init;
	mqtt_client_init_t client_init;
	mqtt_network_connect_t network_connect;
	mqtt_client_connect_t client_connect;
	mqtt_client_subscribe_t client_subscribe;
	mqtt_client_unsubscribe_t client_unsubscribe;
	mqtt_client_publish_t client_publish;
	mqtt_client_disconnect_t client_disconnect;
	mqtt_network_disconnect_t network_disconnect;
	mqtt_client_yield_t client_yield;
} mqtt_wrapper_t;

#endif

#endif
