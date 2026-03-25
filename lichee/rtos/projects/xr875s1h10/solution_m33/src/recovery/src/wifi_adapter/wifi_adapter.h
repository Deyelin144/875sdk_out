
#ifndef __WIFI_ADAPTER_H__
#define __WIFI_ADAPTER_H__

#define SSID_MAX_LEN 32
#define PWD_MAX_LEN 64
#define BSSID_MAX_LEN 17
#define MAX_WIFI_SAVE_CNT 10

typedef enum {
    SECURITY_TYPE_OPEN,       /* Open system. */
	SECURITY_TYPE_WPA_PSK,
	SECURITY_TYPE_WPA2_PSK,
	SECURITY_TYPE_WPA3_PSK,
} wifi_sec_type_t;

typedef enum {
	WIFI_STAT_CONNECT_TIMEOUT,
	WIFI_STAT_PWD_ERR,
	WIFI_STAT_DISCONN,
    WIFI_STAT_SACN_SUCC,
    WIFI_STAT_SCAN_FAIL,
	WIFI_STAT_CONNECTING,
	WIFI_STAT_CONNECTED,	//wifi连接成功，不确保该wifi有网络
	WIFI_STAT_WLAN_UP,		//wifi连接成功且该wifi有网络
	WIFI_STAT_CONNECT_FAILED, 
	WIFI_STAT_NETWORK_NOT_EXIST, 	//网络不存在
	WIFI_STAT_CONNECT_REJECT,		//连接被拒绝
	WIFI_STAT_CONNECT_ABORT,		//连接被终止
	WIFI_STAT_EXIT,
} wifi_conn_stat_t;

typedef enum {
	NET_EVENT_CONNECT				            = 0, 
	NET_EVENT_DISCONNECTED,			             
	NET_EVENT_SCAN_SUCCESS, 
	NET_EVENT_SCAN_FAILED, 
	NET_EVENT_4WAY_HANDSHAKE_FAILED, 
	NET_EVENT_CONNECT_FAILED, 
	NET_EVENT_CONNECTION_LOSS, 

	NET_EVENT_NETWORK_UP, 
	NET_EVENT_NETWORK_DOWN, 
	NET_EVENT_NETWORK_CONNECTING,

	NET_EVENT_NETWORK_NOT_EXIST,
	NET_EVENT_PASSWORD_ERR,
	NET_EVENT_CONNECT_REJECT,
	NET_EVENT_CONNECT_ABORT,
	NET_EVENT_CONNECT_TIMEOUT,

	NET_EVENT_ALL                               = 0xffff,
} wifi_net_event_t;

typedef struct {
	char	ssid[SSID_MAX_LEN + 1];
	char	bssid[BSSID_MAX_LEN + 1];
	char	channel;
	wifi_sec_type_t sec_type;     /*加密方式, 0:开放wifi、1：加密wifi*/
	int		rssi;	/* unit is 0.5db */
	int	    freq;
	int		level;	/* signal level, unit is dbm */
} wifi_ap_info_t;


typedef struct {
	wifi_ap_info_t *ap;
	int size;       /*最大扫描数*/
	int num;        /*实际扫描数量*/
} wifi_scan_results_t;

typedef struct {
	char ssid[SSID_MAX_LEN + 1];
	char pwd[PWD_MAX_LEN + 1];
    wifi_sec_type_t sec_type;
} wifi_info_t;

typedef struct {
    int wifi_cnt;
    wifi_info_t wifi_info[MAX_WIFI_SAVE_CNT];
} wifi_list_t;

wifi_conn_stat_t wifi_adapter_get_conn_stat(void);
int wifi_adapter_init(void (*cb)(unsigned int event, void *arg));
int wifi_adapter_deinit(void);
int wifi_adapter_connect(char *ssid, char *pwd, wifi_sec_type_t sec_type);
int wifi_adapter_disconnect(void);
int wifi_adapter_get_apinfo(wifi_ap_info_t *apinfo);
int wifi_adapter_scan(void);
int wifi_adapter_get_scan_result(wifi_scan_results_t *result);
int wifi_adapter_add_wifi_to_list(wifi_info_t *wifi_info);
int wifi_adapter_get_save_list(wifi_list_t *wifi_list);
int wifi_adapter_del_save_wifi(char *ssid);

#endif