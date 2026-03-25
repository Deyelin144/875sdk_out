#ifndef __REALIZE_UNIT_WIFI_H__
#define __REALIZE_UNIT_WIFI_H__

#include "../../platform/gulitesf_config.h"

//扫描间隔时间
#define WIFI_SCAN_INTERVAL_POSITIVE		2000	//扫描间隔两秒
#define WIFI_INFO_CNT CONFIG_MAX_SAVE_WIFI_CNT

#define DEFAULT_WIFI_SSID1		"G100test" //"G100test"
#define DEFAULT_WIFI_PWD1		"12344321"
#define DEFAULT_WIFI_SSID2		"8"
#define DEFAULT_WIFI_PWD2		"12345678"

#define SSID_MAX_LEN 32
#define PWD_MAX_LEN 64
#define BSSID_MAX_LEN 17
#define WIFI_AP_SCAN_SIZE 50

typedef enum {
    SECURITY_TYPE_OPEN,       /* Open system. */
#if 0
    SECURITY_TYPE_WEP,        /* Wired Equivalent Privacy. WEP security. */
    SECURITY_TYPE_WPA_TKIP,   /* WPA /w TKIP */
    SECURITY_TYPE_WPA_AES,    /* WPA /w AES */
    SECURITY_TYPE_WPA2_TKIP,  /* WPA2 /w TKIP */
    SECURITY_TYPE_WPA2_AES,   /* WPA2 /w AES */
    SECURITY_TYPE_WPA2_MIXED, /* WPA2 /w AES or TKIP */
    SECURITY_TYPE_AUTO,       /* It is used when calling @ref micoWlanStartAdv, MICO read security type from scan result. */
#else
	SECURITY_TYPE_WPA_PSK,
	SECURITY_TYPE_WPA2_PSK,
	SECURITY_TYPE_WPA3_PSK,
#endif
} unit_wifi_sec_type_e;

typedef struct {
	char ssid[SSID_MAX_LEN + 1];
	char pwd[PWD_MAX_LEN + 1];
	char bssid[BSSID_MAX_LEN + 1];
	unit_wifi_sec_type_e sec_type;
} unit_wifi_info_t;

typedef unit_wifi_info_t unit_wifi_del_info_t;

typedef struct {
	unsigned char wifi_cnt;		// 记录保存的wifi数，最大为WIFI_INFO_CNT
	unit_wifi_info_t wifi_info[WIFI_INFO_CNT];
} unit_wifi_list_t;

typedef enum {
	WIFI_AUTO_CONN_FROM_INTERNAL = 0,
	WIFI_AUTO_CONN_FROM_EXTERNAL
} unit_wifi_auto_conn_method_e;

// typedef enum {
// 	PASSWORD_ERR,		//密码错误
// 	SSID_5G_ERR,		//5G WIFI错误
// 	SSID_NOT_EXIST,		//WIFI信号不存在
// 	DEFAULT_ERR,
// } e_wifi_err_stat_t;

typedef enum {
	WIFI_SCANNING,
	WIFI_SCAN_SUC,
	WIFI_SCAN_FAIL,
	WIFI_SCAN_NUM,
} unit_wifi_scan_stat_e;

typedef enum {
	WIFI_STAT_CONNECT_TIMEOUT,
	WIFI_STAT_PWD_ERR,
	WIFI_STAT_DISCONN,
	WIFI_STAT_CONNECTING,
	WIFI_STAT_CONNECTED,	//wifi连接成功，不确保该wifi有网络
	WIFI_STAT_WLAN_UP,		//wifi连接成功且该wifi有网络
	WIFI_STAT_CONNECT_FAILED, 
	WIFI_STAT_NETWORK_NOT_EXIST, 	//网络不存在
	WIFI_STAT_CONNECT_REJECT,		//连接被拒绝
	WIFI_STAT_CONNECT_ABORT,		//连接被终止
	WIFI_STAT_EXIT,
} unit_wifi_conn_stat_t;

typedef enum {
    WIFI_MODE_STA           = 0, 
    WIFI_MODE_HOSTAP        = 1, 
    WIFI_MODE_MONITOR       = 2, 
    WIFI_MODE_NUM           = 3, 
    WIFI_MODE_INVALID       = WIFI_MODE_NUM, 
} unit_wifi_mode_e;

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
} unit_wifi_net_event_e;


typedef struct {
	char	ssid[SSID_MAX_LEN + 1];
	char	bssid[BSSID_MAX_LEN + 1];
	char	channel;
	unsigned char sec_type;     /*加密方式, 0:开放wifi、1：加密wifi*/
	int		rssi;	/* unit is 0.5db */
	int	    freq;
	int		level;	/* signal level, unit is dbm */
} unit_wifi_ap_info_t;


typedef struct {
	unit_wifi_ap_info_t *ap;
	int size;       /*最大扫描数*/
	int num;        /*实际扫描数量*/
} unit_wifi_scan_stat_results_t;


typedef struct _wifi_ctrl {
	unit_wifi_scan_stat_results_t	scan_results;	//wifi扫描结果，需要确保wifi_scan_stat标志位为WIFI_SCAN_SUC
	long interval_ms;					//wifi扫描间隔
	unit_wifi_scan_stat_e wifi_scan_stat;		//wifi扫描状态，获取扫描结果前需要确保该状态为WIFI_SCAN_SUC
	unit_wifi_conn_stat_t wifi_conn_stat;		//wifi连接状态，
	unsigned char is_auto_conn;					//该标志位置一后扫描到连接过的wifi会自动连接并清除该标志位

	char wifi_ssid[SSID_MAX_LEN + 1];		//用户配置的wifi账号密码，账号密码为用户输入，不确保正确
	char wifi_pwd[PWD_MAX_LEN + 1];
	char cur_mac[BSSID_MAX_LEN + 1];
	unit_wifi_sec_type_e cur_sec_type;		//
	unsigned char wifi_channel;				//用户当前的信道
	int wifi_enable;
} wifi_ctrl_t;

typedef struct {
	int (*wifi_state_cb)(int state);
} wifi_state_cb_t;

typedef void (*net_state_cb)(unsigned int event, void *arg);
// int realize_unit_wifi_get_cur_wifi_signal();


wifi_ctrl_t *realize_unit_wifi_get_opt();

//获取当前连接热点信息
int realize_unit_wifi_get_cur_ap_info(unit_wifi_ap_info_t *ap_info);

//获取网络连接状态， 1：网络已连接，0：当前无网络连接
int realize_unit_wifi_get_wlan_up();

//设置扫描状态
void realize_unit_wifi_set_scan_stat(unit_wifi_scan_stat_e stat);

//设置扫描间隔
void _realize_unit_wifi_set_scan_interval(const char *name, long interval_ms, unsigned char cmd);
#define realize_unit_wifi_set_scan_interval(interval_ms, cmd) _realize_unit_wifi_set_scan_interval(__func__, interval_ms, cmd)

//获取保存的WiFi列表
int realize_unit_wifi_get_save_wifi(unit_wifi_list_t *wifi_list);

//移除网络
int realize_unit_wifi_remove_network();

//断开连接
int realize_unit_wifi_disconnect();

//连接网络
int realize_unit_wifi_connect(int is_hidden, const char* ssid, const char* pwd, const char *mac_addr, unit_wifi_sec_type_e sec_type);

//获取扫描到的wifi列表
int realize_unit_wifi_get_scan_results(char **scan_result);

//初始化wifi
int realize_unit_wifi_init(wifi_state_cb_t* wifi_state_cb);


//设置是否自动连接
void realize_unit_wifi_set_auto_conn(unsigned char flag);

int realize_unit_wifi_delete(unit_wifi_del_info_t *del_info);

int realize_unit_wifi_deinit();


#endif 





