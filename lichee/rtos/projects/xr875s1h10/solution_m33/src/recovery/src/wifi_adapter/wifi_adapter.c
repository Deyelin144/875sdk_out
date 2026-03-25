
#include "wifi_adapter.h"
#include <wifimg.h>
#include <stdio.h>
#include <string.h>
#include "recovery_main.h"
#include "os_adapter/os_adapter.h"

#define RSSI_MAX_VALUE -55
#define RSSI_MIN_VALUE -100
#define SIGNAL_LEVELS 5
#define WIFI_AP_SCAN_SIZE 50

static wifi_scan_result_t s_scan_info[WIFI_AP_SCAN_SIZE] = {0};
static char sta_msg_cb_char[32] = "sta msg cb arg";
static void (*s_cb)(unsigned int event, void *arg);
static wifi_list_t s_wifi_list = {0};
static char s_inited = 0;

void wifi_msg_cb(wifi_msg_data_t *msg)
{
	char msg_cb_arg[32] = "NULL";
	char *msg_cb_arg_p;
	//msg cb test, need to pass static parameters
	if(msg->private_data){
		msg_cb_arg_p = (char *)msg->private_data;
	} else {
		msg_cb_arg_p = msg_cb_arg;
	}
	printf("msg->id:%d\n", msg->id);
	switch(msg->id) {
		case WIFI_MSG_ID_DEV_STATUS:
			printf("dev (%d)\n", msg->data.d_status);
			switch(msg->data.d_status) {
				case WLAN_STATUS_DOWN:
					printf("down\n");
					if (NULL != s_cb) {
						s_cb(NET_EVENT_NETWORK_DOWN, NULL);
					}
                    // module_network_wifi_monitoring(NET_EVENT_NETWORK_DOWN);
					break;
				case WLAN_STATUS_UP:
					printf("up\n");
					break;
				default:
					printf("unknow\n");
					break;
			}
			printf("arg:%s\n", msg_cb_arg_p);
			break;
		case WIFI_MSG_ID_STA_CN_EVENT:
			printf("sta event (%d)\n",msg->data.event);
			switch(msg->data.event) {
				case WIFI_DISCONNECTED:
					printf("disconnect\n");
					if (NULL != s_cb) {
						s_cb(NET_EVENT_DISCONNECTED, NULL);
					}
                    // module_network_wifi_monitoring(NET_EVENT_DISCONNECTED);
					break;
				case WIFI_SCAN_STARTED:
					printf("scan started\n");
					break;
				case WIFI_SCAN_FAILED:
					printf("scan failed\n");
					if (NULL != s_cb) {
						s_cb(NET_EVENT_SCAN_FAILED, NULL);
					}
					break;
				case WIFI_SCAN_RESULTS:
					printf("scan results\n");
					if (NULL != s_cb) {
						s_cb(NET_EVENT_SCAN_SUCCESS, NULL);
					}
					break;
				case WIFI_NETWORK_NOT_FOUND:
					printf("network not found\n");
					break;
				case WIFI_PASSWORD_INCORRECT:
					printf("password incorrect\n");
					if (NULL != s_cb) {
						s_cb(NET_EVENT_PASSWORD_ERR, NULL);
					}
					break;
				case WIFI_4WAY_HANDSHAKE_FAILED:
					printf("4way handshake failed\n");
					if (NULL != s_cb) {
						s_cb(NET_EVENT_4WAY_HANDSHAKE_FAILED, NULL);
					}
					break;
				case WIFI_CONNECTED:
					printf("connected\n");
					if (NULL != s_cb) {
						s_cb(NET_EVENT_CONNECT, NULL);
					}
					break;
				case WIFI_CONNECT_TIMEOUT:
					printf("connect timeout\n");
					// if (NULL != s_cb) {
					// 	s_cb(NET_EVENT_CONNECT_FAILED, NULL);
					// }
					break;
				case WIFI_DHCP_START:
					printf("dhcp start\n");
					break;
				case WIFI_DHCP_TIMEOUT:
					printf("dhcp timeout\n");
					// if (NULL != s_cb) {
					// 	s_cb(NET_EVENT_CONNECT_FAILED, NULL);
					// }
					break;
				case WIFI_DHCP_SUCCESS:
					printf("dhcp success\n");
					if (NULL != s_cb) {
						s_cb(NET_EVENT_NETWORK_UP, NULL);
					}
                    // module_network_wifi_monitoring(NET_EVENT_NETWORK_UP);
					break;
				case WIFI_TERMINATING:
					printf("terminating\n");
					break;
				case WIFI_UNKNOWN:
				default:
					printf("unknow msg->data.event = %d.\n", msg->data.event);
					break;
			}
			break;
		case WIFI_MSG_ID_STA_STATE_CHANGE:
			printf("sta state (%d)\n", msg->data.state);
			switch(msg->data.state) {
				case WIFI_STA_IDLE:
					printf("idle\n");
					break;
				case WIFI_STA_CONNECTING:
					printf("connecting\n");
					if (NULL != s_cb) {
						s_cb(NET_EVENT_NETWORK_CONNECTING, NULL);
					}
					break;
				case WIFI_STA_CONNECTED:
					printf("connected\n");
					break;
				case WIFI_STA_OBTAINING_IP:
					printf("obtaining ip\n");
					break;
				case WIFI_STA_NET_CONNECTED:
					printf("net connected\n");
					break;
				case WIFI_STA_DHCP_TIMEOUT:
					printf("dhcp timeout\n");
					break;
				case WIFI_STA_DISCONNECTING:
					printf("disconnecting\n");
					break;
				case WIFI_STA_DISCONNECTED:
					printf("disconnected\n");
					if (NULL != s_cb) {
						s_cb(NET_EVENT_DISCONNECTED, NULL);
					}
                    // module_network_wifi_monitoring(NET_EVENT_DISCONNECTED);
					break;
				default:
					printf("unknow\n");
					break;
			}
			printf("arg:%s\n", msg_cb_arg_p);
			break;
		default:
			break;
	}
}
int wifi_adapter_init(void (*cb)(unsigned int event, void *arg))
{
    int ret = -1;

    wifimanager_init();

    ret = wifi_on(WIFI_STATION);
    if (ret) {
        LOG_ERR("wifi on sta failed, ret = %d\n", ret);
        goto exit;
    }
	ret = wifi_register_msg_cb(wifi_msg_cb, (void *)sta_msg_cb_char);
    if (ret) {
        LOG_ERR("register msg cb failed, ret = %d\n", ret);
        goto exit;
    }
	wifi_sta_auto_reconnect(false);
    s_cb = cb;
    s_inited = 1;

    ret = 0;
exit:
	return ret;
}

int wifi_adapter_deinit()
{
    if (s_inited == 1) {
        printf("wifi init\n");
        wifi_off();
        wifimanager_deinit();
        s_cb = NULL;
        s_inited = 0;
    }

    return 0;
}

int wifi_adapter_connect(char *ssid, char *pwd, wifi_sec_type_t sec_type)
{
    int ret = -1;
    wifi_sta_cn_para_t cn_para = {0};

    cn_para.ssid = ssid;
    cn_para.password = pwd;
    if (SECURITY_TYPE_OPEN == sec_type) {
        cn_para.sec = WIFI_SEC_NONE;
    } else if (SECURITY_TYPE_WPA_PSK == sec_type) {
        cn_para.sec = WIFI_SEC_WPA_PSK;
    } else if (SECURITY_TYPE_WPA2_PSK == sec_type) {
        cn_para.sec = WIFI_SEC_WPA2_PSK;
    } else if (SECURITY_TYPE_WPA3_PSK == sec_type) {
        cn_para.sec = WIFI_SEC_WPA3_PSK;
    }

	if (NULL != s_cb) {
		s_cb(NET_EVENT_NETWORK_CONNECTING, NULL);
	}

    ret = wifi_sta_connect(&cn_para);
	if (ret != 0) {
		if (NULL != s_cb) {
			s_cb(NET_EVENT_CONNECT_FAILED, NULL);
		}
	}

exit:	
    return ret;
}

int wifi_adapter_disconnect()
{
    return wifi_sta_disconnect();
}

static int wifi_adapter_get_signal_level(int rssi, int numlevels)
{
    int level = 0;
    if (rssi <= RSSI_MIN_VALUE) {
        level = 0;
    } else if (rssi >= RSSI_MAX_VALUE) {
        level = numlevels - 1;
    } else {
        level = (rssi - RSSI_MIN_VALUE) * (numlevels - 1) / (RSSI_MAX_VALUE - RSSI_MIN_VALUE);
    }
    return level;
}

int wifi_adapter_get_apinfo(wifi_ap_info_t *ap_info)
{
    int ret = -1;
	wifi_sta_info_t sta_info = {0};

    if (NULL == ap_info) {
        LOG_ERR("ap_info is NULL\n");
        goto exit;
    }
	
	// cmd_free(0, NULL); //临时加入用来复现唤醒之后调用wifi接口阻塞的问题，解决或者出正式固件删掉
	
    ret = wifi_sta_get_info(&sta_info);
    if (ret != 0) {
        LOG_ERR("get connect info failed.\n");
        goto exit;
    }
	
    memcpy(ap_info->ssid, sta_info.ssid, sizeof(ap_info->ssid));
	sprintf(ap_info->bssid, "%02x:%02x:%02x:%02x:%02x:%02x", sta_info.bssid[0], sta_info.bssid[1], 
															sta_info.bssid[2], sta_info.bssid[3], 
															sta_info.bssid[4], sta_info.bssid[5]);
	ap_info->rssi = sta_info.rssi;
	ap_info->level = wifi_adapter_get_signal_level(sta_info.rssi, SIGNAL_LEVELS);
	ap_info->freq = sta_info.freq;
	if (2484 == sta_info.freq) {
		ap_info->channel = 14;
	} else {    
		ap_info->channel = ((sta_info.freq - 2412) / 5) + 1;
	}
	if (WIFI_SEC_NONE == sta_info.sec) {
		ap_info->sec_type = SECURITY_TYPE_OPEN;
	} else if (WIFI_SEC_WPA_PSK== sta_info.sec) {
		ap_info->sec_type = SECURITY_TYPE_WPA_PSK;
	} else if (WIFI_SEC_WPA2_PSK == sta_info.sec) {
		ap_info->sec_type = SECURITY_TYPE_WPA2_PSK;
	} else if (WIFI_SEC_WPA3_PSK == sta_info.sec) {
		ap_info->sec_type = SECURITY_TYPE_WPA3_PSK;
	}
	
    ret = 0;

exit:
	
	return 0;
}

int wifi_adapter_scan(void)
{
    os_adapter()->msleep(200);
	if (NULL != s_cb) {
		s_cb(NET_EVENT_SCAN_SUCCESS, NULL);
	}
   	return 0;
}

int wifi_adapter_get_scan_result(wifi_scan_results_t *results)
{
    int ret = -1;
    int bss_num = 0;

    memset(&s_scan_info, 0, sizeof(s_scan_info));
	s_scan_info[0].scan_action = true;
    ret = wifi_get_scan_results(&s_scan_info, NULL, &bss_num, results->size);
    if (ret != 0) {
        goto exit;
    }

    results->num = bss_num > results->size ? results->size : bss_num;

    for (int i = 0; i < results->num; i++) {
        memcpy(results->ap[i].ssid, s_scan_info[i].ssid, SSID_MAX_LEN);
        sprintf(results->ap[i].bssid, "%02x:%02x:%02x:%02x:%02x:%02x", s_scan_info[i].bssid[0], s_scan_info[i].bssid[1], 
															s_scan_info[i].bssid[2], s_scan_info[i].bssid[3], 
															s_scan_info[i].bssid[4], s_scan_info[i].bssid[5]);
        results->ap[i].rssi = s_scan_info[i].rssi;
        results->ap[i].freq = s_scan_info[i].freq;
        if (2484 == results->ap[i].freq) {
            results->ap[i].channel = 14;
        } else {    
            results->ap[i].channel = ((s_scan_info[i].freq - 2412) / 5) + 1;
        }
		results->ap[i].level = wifi_adapter_get_signal_level(s_scan_info[i].rssi, SIGNAL_LEVELS);
        if (WIFI_SEC_NONE == s_scan_info[i].key_mgmt) {
            results->ap[i].sec_type = SECURITY_TYPE_OPEN;
        } else if (WIFI_SEC_WPA2_PSK & s_scan_info[i].key_mgmt) {
            results->ap[i].sec_type = SECURITY_TYPE_WPA2_PSK;
        } else if (WIFI_SEC_WPA_PSK == s_scan_info[i].key_mgmt) {
            results->ap[i].sec_type = SECURITY_TYPE_WPA_PSK;
        }  else if (WIFI_SEC_WPA3_PSK & s_scan_info[i].key_mgmt) {
            results->ap[i].sec_type = SECURITY_TYPE_WPA3_PSK;
        }
    }
    // printf("scan end = %ld, scan = %ld, start = %ld-----------\n", wrapper_get_handle()->os.get_time_ms(), scan, start);
exit:
    return ret;
}

int wifi_adapter_add_wifi_to_list(wifi_info_t *wifi_info)
{
    int ret = -1;

    if (!wifi_info) {
        printf("wifi_info is null\n");
        goto exit;
    }

    if (s_wifi_list.wifi_cnt >= MAX_WIFI_SAVE_CNT) {
        printf("wifi list is full\n");
        goto exit;
    }

    // 避免重复
    for (int i = 0; i < s_wifi_list.wifi_cnt; i++) {
        if (strcmp(wifi_info->ssid, s_wifi_list.wifi_info[i].ssid) == 0) {
            printf("update wifi info\n");
            memcpy(&s_wifi_list.wifi_info[i], wifi_info, sizeof(wifi_info_t));
            ret = 0;
            goto exit;
        }
    }

    memcpy(&s_wifi_list.wifi_info[s_wifi_list.wifi_cnt++], wifi_info, sizeof(wifi_info_t));
    ret = 0;
exit:
    return ret;
}

int wifi_adapter_get_save_list(wifi_list_t *wifi_list)
{
    int ret = -1;

    if (!wifi_list) {
        printf("wifi_list is null\n");
        goto exit;
    }

    memcpy(wifi_list, &s_wifi_list, sizeof(wifi_list_t));
    ret = 0;
exit:
    return ret;
}

int wifi_adapter_del_save_wifi(char *ssid)
{
    int ret = -1;

    if (!ssid) {
        printf("bssid is null\n");
        goto exit;
    }
    
    printf("going to del %s\n", ssid);
    for (int i = 0; i < s_wifi_list.wifi_cnt; i++) {
        if (strcmp(ssid, s_wifi_list.wifi_info[i].ssid) == 0) {
            if (i != s_wifi_list.wifi_cnt - 1) {
                strncpy(s_wifi_list.wifi_info[i].ssid, s_wifi_list.wifi_info[s_wifi_list.wifi_cnt - 1].ssid, SSID_MAX_LEN);
                strncpy(s_wifi_list.wifi_info[i].pwd, s_wifi_list.wifi_info[s_wifi_list.wifi_cnt - 1].pwd, PWD_MAX_LEN);
            }
            memset(&s_wifi_list.wifi_info[s_wifi_list.wifi_cnt - 1], 0, sizeof(wifi_info_t));
            s_wifi_list.wifi_cnt--;
        }
    }
    ret = 0;
exit:
    return ret;
}

