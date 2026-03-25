#include "sdk_wifi_wrapper.h"

#include <stdio.h>
#include <string.h>

#include "console.h"
#include "kernel/os/os.h"
#include "wifimg.h"
#include "net/wlan/wlan.h"

#define SDK_WIFI_SCAN_MAX 32
#define SDK_WIFI_SCAN_WAIT_MS 2000

static int s_wifi_inited;
static wifi_scan_result_t s_scan_results[SDK_WIFI_SCAN_MAX];
static uint32_t s_scan_result_count;

static const char *sdk_wifi_sec_to_string(wifi_secure_t sec)
{
    switch (sec) {
    case WIFI_SEC_NONE:
        return "open";
    case WIFI_SEC_WEP:
        return "wep";
    case WIFI_SEC_WPA_PSK:
        return "wpa-psk";
    case WIFI_SEC_WPA2_PSK:
        return "wpa2-psk";
    case WIFI_SEC_WPA2_PSK_SHA256:
        return "wpa2-psk-sha256";
    case WIFI_SEC_WPA3_PSK:
        return "wpa3-psk";
    case WIFI_SEC_EAP:
        return "eap";
    default:
        return "unknown";
    }
}

static wifi_secure_t sdk_wifi_pick_security(wifi_sec key_mgmt)
{
    if (key_mgmt & WIFI_SEC_WPA3_PSK) {
        return WIFI_SEC_WPA3_PSK;
    }
    if (key_mgmt & WIFI_SEC_WPA2_PSK_SHA256) {
        return WIFI_SEC_WPA2_PSK_SHA256;
    }
    if (key_mgmt & WIFI_SEC_WPA2_PSK) {
        return WIFI_SEC_WPA2_PSK;
    }
    if (key_mgmt & WIFI_SEC_WPA_PSK) {
        return WIFI_SEC_WPA_PSK;
    }
    if (key_mgmt & WIFI_SEC_WEP) {
        return WIFI_SEC_WEP;
    }
    return WIFI_SEC_NONE;
}

static void sdk_wifi_print_ip(void)
{
    wifi_sta_info_t info = {0};

    if (wifi_sta_get_info(&info) != WMG_STATUS_SUCCESS) {
        return;
    }

    printf("wifi ip: %u.%u.%u.%u gw: %u.%u.%u.%u ssid: %s rssi: %d\n",
           info.ip_addr[0], info.ip_addr[1], info.ip_addr[2], info.ip_addr[3],
           info.gw_addr[0], info.gw_addr[1], info.gw_addr[2], info.gw_addr[3],
           info.ssid, info.rssi);
}

static void sdk_wifi_msg_cb(wifi_msg_data_t *msg)
{
    if (msg == NULL) {
        return;
    }

    switch (msg->id) {
    case WIFI_MSG_ID_DEV_STATUS:
        printf("wifi dev status: %d\n", msg->data.d_status);
        break;
    case WIFI_MSG_ID_STA_CN_EVENT:
        printf("wifi sta event: %d\n", msg->data.event);
        if (msg->data.event == WIFI_DHCP_SUCCESS) {
            sdk_wifi_print_ip();
        }
        break;
    case WIFI_MSG_ID_STA_STATE_CHANGE:
        printf("wifi sta state: %d\n", msg->data.state);
        break;
    default:
        break;
    }
}

static int sdk_wifi_refresh_scan_results(void)
{
    uint32_t bss_num = SDK_WIFI_SCAN_MAX;
    int ret;

    ret = wlan_sta_scan_once();
    if (ret != 0) {
        printf("wifi scan trigger failed: %d\n", ret);
        return ret;
    }

    XR_OS_MSleep(SDK_WIFI_SCAN_WAIT_MS);

    memset(s_scan_results, 0, sizeof(s_scan_results));
    ret = wifi_get_scan_results(s_scan_results, NULL, &bss_num, SDK_WIFI_SCAN_MAX);
    if (ret != WMG_STATUS_SUCCESS) {
        printf("wifi get scan results failed: %d\n", ret);
        s_scan_result_count = 0;
        return ret;
    }

    s_scan_result_count = bss_num;
    return 0;
}

static wifi_secure_t sdk_wifi_guess_security(const char *ssid, const char *password)
{
    uint32_t i;

    if (ssid != NULL) {
        for (i = 0; i < s_scan_result_count; ++i) {
            if (strcmp(s_scan_results[i].ssid, ssid) == 0) {
                return sdk_wifi_pick_security(s_scan_results[i].key_mgmt);
            }
        }
    }

    if (password != NULL && password[0] != '\0') {
        return WIFI_SEC_WPA2_PSK;
    }

    return WIFI_SEC_NONE;
}

int sdk_wifi_wrapper_init(void)
{
    int ret;

    if (s_wifi_inited) {
        return 0;
    }

    ret = wifimanager_init();
    if (ret != WMG_STATUS_SUCCESS) {
        printf("wifimanager_init failed: %d\n", ret);
        return ret;
    }

    ret = wifi_on(WIFI_STATION);
    if (ret != WMG_STATUS_SUCCESS) {
        printf("wifi_on failed: %d\n", ret);
        return ret;
    }

    ret = wifi_register_msg_cb(sdk_wifi_msg_cb, NULL);
    if (ret != WMG_STATUS_SUCCESS) {
        printf("wifi_register_msg_cb failed: %d\n", ret);
        return ret;
    }

    ret = wifi_sta_auto_reconnect(WMG_FALSE);
    if (ret != WMG_STATUS_SUCCESS) {
        printf("wifi_sta_auto_reconnect failed: %d\n", ret);
        return ret;
    }

    s_wifi_inited = 1;
    printf("wifi wrapper ready\n");
    return 0;
}

int sdk_wifi_wrapper_scan(void)
{
    uint32_t i;
    int ret;

    ret = sdk_wifi_wrapper_init();
    if (ret != 0) {
        return ret;
    }

    ret = sdk_wifi_refresh_scan_results();
    if (ret != 0) {
        return ret;
    }

    printf("wifi scan result count: %u\n", (unsigned int)s_scan_result_count);
    for (i = 0; i < s_scan_result_count; ++i) {
        printf("[%02u] ssid=%s rssi=%d freq=%u sec=%s\n",
               (unsigned int)i,
               s_scan_results[i].ssid,
               s_scan_results[i].rssi,
               (unsigned int)s_scan_results[i].freq,
               sdk_wifi_sec_to_string(sdk_wifi_pick_security(s_scan_results[i].key_mgmt)));
    }

    return 0;
}

int sdk_wifi_wrapper_connect(const char *ssid, const char *password)
{
    wifi_sta_cn_para_t cn_para = {0};
    wifi_secure_t sec;
    int ret;

    if (ssid == NULL || ssid[0] == '\0') {
        printf("wifi ssid is empty\n");
        return -1;
    }

    ret = sdk_wifi_wrapper_init();
    if (ret != 0) {
        return ret;
    }

    ret = sdk_wifi_refresh_scan_results();
    if (ret != 0) {
        printf("wifi scan before connect failed, fallback to default security\n");
    }

    sec = sdk_wifi_guess_security(ssid, password);

    wifi_sta_disconnect();
    XR_OS_MSleep(100);

    cn_para.ssid = ssid;
    cn_para.password = (password != NULL && password[0] != '\0') ? password : NULL;
    cn_para.sec = sec;
    cn_para.fast_connect = false;

    printf("wifi connect ssid=%s sec=%s\n", ssid, sdk_wifi_sec_to_string(sec));
    ret = wifi_sta_connect(&cn_para);
    if (ret != WMG_STATUS_SUCCESS) {
        printf("wifi_sta_connect failed: %d\n", ret);
        return ret;
    }

    return 0;
}

static int cmd_wifi_scan(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    return sdk_wifi_wrapper_scan();
}
FINSH_FUNCTION_EXPORT_CMD(cmd_wifi_scan, wifi_scan, scan wifi ap list);

static int cmd_wifi_sta(int argc, char **argv)
{
    const char *password = "";

    if (argc != 2 && argc != 3) {
        printf("usage: wifi_sta <ssid> [password]\n");
        return -1;
    }

    if (argc == 3) {
        password = argv[2];
    }

    return sdk_wifi_wrapper_connect(argv[1], password);
}
FINSH_FUNCTION_EXPORT_CMD(cmd_wifi_sta, wifi_sta, connect wifi in sta mode);
