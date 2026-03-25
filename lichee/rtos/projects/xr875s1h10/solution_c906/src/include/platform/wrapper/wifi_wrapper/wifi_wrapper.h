#ifndef __WIFI_WRAPPER_H__
#define __WIFI_WRAPPER_H__

#include <stdbool.h>
#include "../../../mod_realize/realize_unit_wifi/realize_unit_wifi.h"

/**
 * @brief 初始化，使能sta
 * @param state_t 网络状态回调函数
 * @return 成功则返回0，失败返回-1
 */
typedef int (*sta_init_t)(net_state_cb state_cb);	

/**
 * @brief 删除初始化
 * @param 
 * @return 成功则返回0，失败返回-1
 */
typedef int (*sta_deinit_t)(void);	

/**
 * @brief 连接热点
 * @param is_hidden：是否是隐藏wifi
 * @param ssid：wifi名称
 * @param pwd：密码
 * @param bssid：mac地址
 * @param sec_type: 加密类型
 * @return 成功则返回0，失败返回-1
 */
typedef int (*sta_connect_t)(bool is_hidden, char *ssid, char *pwd, char *bssid, unit_wifi_sec_type_e sec_type);

/**
 * @brief 断开连接
 * @param 
 * @return 成功则返回0，失败返回-1
 */
typedef int (*sta_disconnect_t)(void);

/**
 * @brief 网络连接错误时删除该连接配置，防止设备一直重试连接该网络
 * @param 
 * @return 成功则返回0，失败返回-1
 */
typedef int (*sta_disable_net_conf_t)(void);

/**
 * @brief 获取当前已连接WiFi的信息
 * @param ap_info：当前连接wifi信息
 * @return 成功则返回0，失败返回-1
 */
typedef int (*sta_get_info_t)(unit_wifi_ap_info_t *ap_info);

/**
 * @brief 启动一次扫描
 * @param 
 * @return 成功则返回0，失败返回-1
 */
typedef int (*sta_scan_t)(void);

/**
 * @brief 设置扫描间隔时间
 * @param sec：时间, 单位s
 * @return 成功则返回0，失败返回-1
 */
typedef int (*sta_set_scan_interval_t)(int sec);

/**
 * @brief 获取扫描到的WiFi列表
 * @param results：扫描结果
 * @return 成功则返回0，失败返回-1
 */
typedef int (*sta_get_scan_result_t)(unit_wifi_scan_stat_results_t *results);

/**
 * @brief 获取已保存的WiFi列表
 * @param save_wifi_list：保存的WiFi列表
 * @return 成功则返回0，失败返回-1
 */
typedef int (*sta_get_save_wifi_t)(unit_wifi_list_t *save_wifi_list);

/**
 * @brief 删除已保存的wifi
 * @param is_all_del：是否全部删除，全部删除时ssid和mac_addr可传NULL
 * @param ssid：wifi名称
 * @param mac_addr：mac地址
 * @return 成功则返回0，失败返回-1
 */
typedef int (*sta_del_save_wifi_t)(bool is_all_del, char *ssid, char *mac_addr);

/**
 * @brief 切换模式
 * @param mode：net模式
 * @return 成功则返回0，失败返回-1
 */
typedef int (*switch_mode_t)(unit_wifi_mode_e mode);

/**
 * @brief 获取自动连接的方式
 * @param 
 * @return WIFI_AUTO_CONN_FROM_INTERNAL：由引擎实现自动连接， WIFI_AUTO_CONN_FROM_EXTERNAL：由外部实现自动连接
 */
typedef unit_wifi_auto_conn_method_e (*sta_get_auto_connect_method_t)(void);

/**
 * @brief 获取wifi开关使能状态
 * @param 
 * @return 1：使能， 0：失能，首次开机默认为1
 */
typedef int (*sta_get_wifi_switch_state_t)(void);


typedef struct {
	sta_init_t sta_init;	
	sta_deinit_t sta_deinit;
	sta_connect_t sta_connect;				
	sta_disconnect_t sta_disconnect;
	sta_disable_net_conf_t sta_disable_net_conf;
	sta_get_info_t sta_get_info;
	sta_scan_t sta_scan;
	sta_set_scan_interval_t sta_set_scan_interval;
	sta_get_scan_result_t sta_get_scan_result;
	sta_get_save_wifi_t sta_get_save_wifi;
	sta_del_save_wifi_t sta_del_save_wifi;
	switch_mode_t switch_mode;
	sta_get_auto_connect_method_t  sta_get_auto_connect_method;
	sta_get_wifi_switch_state_t switch_state;
} wifi_wrapper_t;	

#endif
