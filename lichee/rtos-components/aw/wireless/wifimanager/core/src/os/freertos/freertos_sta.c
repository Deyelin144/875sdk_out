/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the People's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>
#include <wpa_ctrl.h>
#include <freertos_sta.h>
#include <udhcpc.h>
#include <utils.h>
#include <wifi_log.h>
#include <unistd.h>
#include <wmg_sta.h>
#include <freertos/event.h>
#include <freertos/udhcpc.h>
#include <freertos/scan.h>
#include <freertos_common.h>
#include "tcpip_adapter.h"
#include "ethernetif.h"
#include "wlan.h"
#include "wlan_defs.h"
#include "net_ctrl.h"
#include "net_init.h"
#include "sysinfo.h"
#include "cmd_util.h"
#include "wlan_ext_req.h"
#include "lwip/tcpip.h"
#include "lwip/inet.h"
#include "lwip/dhcp.h"
#include "lwip/netifapi.h"

#define SLEEP_TIMES 15
#define CONNECT_TIMEOUT 100
#define FAST_CONNECT_TIMEOUT 300
#define SCAN_TIMEOUT 100
#define SCAN_NUM_PROBES 2
#define SCAN_PROBE_DELAY 100
#define SCAN_MIN_DWELL 100
#define SCAN_MAX_DWELL 125

static wmg_sta_inf_object_t sta_inf_object;
static observer_base *sta_net_ob;
static int call_disconnect = 0; /* "net sta disconnect" must mate "net sta conect". */
static int autoconn_disabled = 0; /* wlan_sta_set_autoconn(0) must mate wlan_sta_set_autoconn(1). */
static int scan_state = 0;
static int mac_state = 0;
static int connect_status = 0;

typedef struct bss_info {
	uint8_t  ssid[32];
	uint8_t  psk[32];
	uint32_t bss_size;
	uint8_t  bss[800];
	uint8_t  psk_ph[100];
	uint32_t flags;
} bss_info_t;

#if PRJCONF_SYSINFO_SAVE_TO_FLASH
#include <hal_fdcm.h>
#define BSS_FLASH_NUM         (0)
#define BSS_FLASH_ADDR        (1024 * 1024 * 12)
#define BSS_FLASH_SIZE        (8*1024)
#define FLAGS_BSS_USING_WPA3   (1U << (0))
char fast_sta_ssid[SSID_MAX_LEN * 2 + 1] ={0};
char fast_sta_psk[PSK_MAX_LEN *2 + 1] = {0};
bss_info_t g_bss_info;
static struct sysinfo g_sysinfo;
static fdcm_handle_t *g_fdcm_hdl;
#endif

static int wep_connect_timeout = 0;
static const char * const net_ctrl_msg_str[] = {
	"wifimg connected",
	"wifimg disconnected",
	"wifimg scan success",
	"wifimg scan failed",
	"wifimg 4way handshake failed",
	"wifimg ssid not found",
	"wifimg auth timeout",
	"wifimg disassoc",
	"wifimg associate timeout",
	"wifimg connect failed",
	"wifimg connect loss",
	"wifimg associate failed",
	"wifimg SAE auth-commit failed",
	"wifimg SAE auth-confirm failed",
	"wifimg dev hang",
	"wifimg ap-sta connected",
	"wifimg ap-sta disconnected",
	"network dhcp start",
	"network dhcp timeout",
	"network up",
	"network down",
#if (!defined(CONFIG_LWIP_V1) && LWIP_IPV6)
	"network IPv6 state",
#endif
};

static void sta_event_notify_to_sta_dev(wifi_sta_event_t event)
{
	if (sta_inf_object.sta_event_cb) {
		sta_inf_object.sta_event_cb(event);
	}
}

static void freertos_sta_net_msg_process(uint32_t event, uint32_t data, void *arg)
{
	uint16_t type = EVENT_SUBTYPE(event);
	struct netif *nif = g_wlan_netif;
	WMG_DEBUG("recv msg <%s>\n", net_ctrl_msg_str[type]);
	WMG_DEBUG("wifi state: %d, connect_status: %d\n", mac_state, connect_status);

	WMG_DEBUG("event : %d \n", type);
	wlan_ext_stats_code_get_t param;
	switch (type) {
	case NET_CTRL_MSG_WLAN_CONNECTED:
		sta_event_notify_to_sta_dev(WIFI_CONNECTED);
		mac_state = 1;
		break;
	case NET_CTRL_MSG_WLAN_DISCONNECTED:
		if (wep_connect_timeout)
			break;
		sta_event_notify_to_sta_dev(WIFI_DISCONNECTED);
		/* flush scan buf. */
		wlan_sta_bss_flush(0);
		connect_status = -1;
		mac_state = 0;
		break;
	case NET_CTRL_MSG_WLAN_SCAN_SUCCESS:
		WMG_DEBUG("%s() scan_state:%d, we will set it to 1!\n", __func__, scan_state);
		sta_event_notify_to_sta_dev(WIFI_SCAN_RESULTS);
		scan_state = 1;
		break;
	case NET_CTRL_MSG_WLAN_SCAN_FAILED:
		sta_event_notify_to_sta_dev(WIFI_SCAN_FAILED);
		break;
	case NET_CTRL_MSG_WLAN_4WAY_HANDSHAKE_FAILED:
		sta_event_notify_to_sta_dev(WIFI_4WAY_HANDSHAKE_FAILED);
		mac_state = 0;
		connect_status = -1;
		break;
	case NET_CTRL_MSG_WLAN_SSID_NOT_FOUND:
		sta_event_notify_to_sta_dev(WIFI_NETWORK_NOT_FOUND);
		mac_state = 0;
		break;
	case NET_CTRL_MSG_WLAN_AUTH_TIMEOUT:
		sta_event_notify_to_sta_dev(WIFI_AUTH_TIMEOUT);
		mac_state = 0;
		break;
	case NET_CTRL_MSG_WLAN_ASSOC_TIMEOUT:
		sta_event_notify_to_sta_dev(WIFI_AUTH_FAILED);
		mac_state = 0;
		break;
	case NET_CTRL_MSG_WLAN_CONNECT_FAILED:
		sta_event_notify_to_sta_dev(WIFI_CONNECT_FAILED);
		mac_state = 0;
		break;
	case NET_CTRL_MSG_WLAN_DEV_HANG:
		break;
	case NET_CTRL_MSG_WLAN_CONNECTION_LOSS:
		break;
	case NET_CTRL_MSG_WLAN_ASSOC_FAILED:
		sta_event_notify_to_sta_dev(WIFI_ASSOC_FAILED);
		mac_state = 0;
		break;
	case NET_CTRL_MSG_WLAN_SAE_COMMIT_FAILED:
		connect_status = -1;
		break;
	case NET_CTRL_MSG_WLAN_SAE_CONFIRM_FAILED:
		connect_status = -1;
		break;
	case NET_CTRL_MSG_WLAN_AP_STA_DISCONNECTED:
		break;
	case NET_CTRL_MSG_WLAN_DISASSOC:
		sta_event_notify_to_sta_dev(WIFI_DISASSOC);
		mac_state = 0;
		connect_status = -1;
		break;
	case NET_CTRL_MSG_WLAN_AP_STA_CONNECTED:
		break;
	case NET_CTRL_MSG_NETWORK_DHCP_START:
		sta_event_notify_to_sta_dev(WIFI_DHCP_START);
		break;
	case NET_CTRL_MSG_NETWORK_DHCP_TIMEOUT:
		sta_event_notify_to_sta_dev(WIFI_DHCP_TIMEOUT);
		connect_status = 0;
		break;
	case NET_CTRL_MSG_NETWORK_UP:
		connect_status = 1;
		if (scan_state) {
			scan_state = 0;
		}
		sta_event_notify_to_sta_dev(WIFI_DHCP_SUCCESS);
		
		break;
	case NET_CTRL_MSG_NETWORK_DOWN:
		connect_status = 0;
		break;
	default:
		sta_event_notify_to_sta_dev(WIFI_UNKNOWN);
		WMG_WARNG("unknown msg (%u, %u)\n", type, data);
		break;
	}
	WMG_DEBUG("deal msg <%s>\n", net_ctrl_msg_str[type]);
	WMG_DEBUG("wifi state: %d, connect_status: %d\n", mac_state, connect_status);
}

static wmg_status_t freertos_sta_mode_init(sta_event_cb_t sta_event_cb, void *para)
{
	WMG_DEBUG("sta mode init\n");
	scan_state = 0;
	mac_state = 0;
	connect_status = 0;
	if (sta_event_cb != NULL){
		sta_inf_object.sta_event_cb = sta_event_cb;
	}
	/* nothing to do in freertos */
	return WMG_STATUS_SUCCESS;
}

static wmg_status_t freertos_sta_mode_enable(void *para)
{
	WMG_DEBUG("sta mode enable\n");

	xr_wlan_on(WLAN_MODE_STA);

	sta_net_ob = sys_callback_observer_create(CTRL_MSG_TYPE_NETWORK, NET_CTRL_MSG_ALL, freertos_sta_net_msg_process, NULL);
	if (sta_net_ob == NULL) {
		WMG_ERROR("freertos callback observer create failed\n");
		return WMG_STATUS_FAIL;
	}
	if (sys_ctrl_attach(sta_net_ob) != 0) {
		WMG_ERROR("freertos callback observer attach failed\n");
		sys_callback_observer_destroy(sta_net_ob);
		return WMG_STATUS_FAIL;
	} else {
		/* use sta_private_data to save observer_base point. */
		sta_inf_object.sta_private_data = sta_net_ob;
	}

	/*set scan param.*/
	wlan_ext_scan_param_t param;
	param.num_probes = SCAN_NUM_PROBES;
	param.probe_delay = SCAN_PROBE_DELAY;
	param.min_dwell = SCAN_MIN_DWELL;
	param.max_dwell = SCAN_MAX_DWELL;
	wlan_ext_request(g_wlan_netif, WLAN_EXT_CMD_SET_SCAN_PARAM, (uint32_t)(uintptr_t)(&param));

	return WMG_STATUS_SUCCESS;
}

static wmg_status_t freertos_sta_mode_disable()
{
	WMG_DEBUG("sta mode disable\n");

	sta_net_ob = (observer_base *)sta_inf_object.sta_private_data;
	if(sta_net_ob) {
		sys_ctrl_detach(sta_net_ob);
		sys_callback_observer_destroy(sta_net_ob);
		sta_inf_object.sta_private_data = NULL;
	}

	xr_wlan_off();

	connect_status = 0;
	mac_state = 0;

	return WMG_STATUS_SUCCESS;
}

static wmg_status_t freertos_sta_mode_deinit()
{
	WMG_DEBUG("sta mode deinit\n");
	/* nothing to do in freertos */
	return WMG_STATUS_SUCCESS;
}

static wmg_status_t freertos_sta_disconnect()
{
	/* disconnect ap and report WIFI_DISCONNECTED to wifimg */
	WMG_DEBUG("sta disconnect\n");
	int sleep_times = SLEEP_TIMES;
	struct netif *nif = g_wlan_netif;
	wlan_sta_disconnect();
	call_disconnect = 1;
	wlan_sta_disable();
	while(mac_state) {
		if(sleep_times <= 0)
			break;
		sleep_times--;
		usleep(1000 * 1000);
	}

	/* release ip address when disconnect. */
	net_config(nif, 0);

	return WMG_STATUS_SUCCESS;
}

int wifi_wep_connect(const char *ssid, const char *passwd)
{
    uint8_t ssid_len;
	int sleep_times = SLEEP_TIMES;
    wlan_sta_config_t config;
    WMG_DEBUG("%s,ssid %s,passwd, %s\n", __func__, ssid, passwd);

    if (ssid)
        ssid_len = strlen(ssid);
    else
        goto err;


    if (ssid_len > WLAN_SSID_MAX_LEN)
        ssid_len = WLAN_SSID_MAX_LEN;

    memset(&config, 0, sizeof(config));

    /* ssid */
    config.field = WLAN_STA_FIELD_SSID;
    memcpy(config.u.ssid.ssid, ssid, ssid_len);
    config.u.ssid.ssid_len = ssid_len;
    if (wlan_sta_set_config(&config) != 0)
        goto err;

    /* WEP key0 */
    config.field = WLAN_STA_FIELD_WEP_KEY0;
    strlcpy((char *)config.u.wep_key, passwd, sizeof(config.u.wep_key));
    if (wlan_sta_set_config(&config) != 0)
        return -1;

    /* WEP key index */
    config.field = WLAN_STA_FIELD_WEP_KEY_INDEX;
    config.u.wep_tx_keyidx = 0;
    if (wlan_sta_set_config(&config) != 0)
        goto err;

    /* auth_alg: OPEN */
    config.field = WLAN_STA_FIELD_AUTH_ALG;
    config.u.auth_alg = WPA_AUTH_ALG_OPEN | WPA_AUTH_ALG_SHARED;
    if (wlan_sta_set_config(&config) != 0)
        goto err;

    /* key_mgmt: NONE */
    config.field = WLAN_STA_FIELD_KEY_MGMT;
    config.u.key_mgmt = WPA_KEY_MGMT_NONE;
    if (wlan_sta_set_config(&config) != 0)
        goto err;

	wep_connect_timeout = 1;
	if (connect_status == 0) {
		WMG_DEBUG("wep Connecting AP ......\n");
		wlan_sta_enable();
		if(call_disconnect || autoconn_disabled){
			wlan_sta_connect(); /* if call "waln_sta_disconnect, must call wlan_sta_connect". */
			call_disconnect = 0;
			autoconn_disabled = 0;
		}
	}

	while(sleep_times >= 6) {
		usleep(5000 * 1000);
		WMG_DEBUG("try wep connect\n");
		if (connect_status == 0) {
			wlan_sta_disable();
			wep_connect_timeout = 0;
			WMG_DEBUG("%s, WPA_AUTH_ALG_SHARED\n", __func__);
			config.field = WLAN_STA_FIELD_AUTH_ALG;
			config.u.auth_alg = WPA_AUTH_ALG_SHARED;
			if (wlan_sta_set_config(&config) != 0)
				goto err;
			break;
		}
		if (connect_status == 1) {
			wep_connect_timeout = 0;
			WMG_DEBUG("%s, WPA_AUTH_ALG_OPEN\n", __func__);
			break;
		}
		sleep_times--;
    }

	wep_connect_timeout = 0;

    return 0;

err:
    WMG_ERROR("connect ap failed\n");
    return -1;
}

#if PRJCONF_SYSINFO_SAVE_TO_FLASH
/**
 * @brief Save bss info to flash
 * @return 0 on success, -1 on failure
 */
int save_bss_to_flash(bss_info_t *pbss_info)
{
	int ret = 0;
	uint32_t size;
	wlan_sta_bss_info_t bss_get;
	wlan_sta_ap_t ap_info;
	fdcm_handle_t *bss_fdcm_hdl;

	if (connect_status == 0) {
		WMG_WARNG("Please connect AP first!\n");
		ret = -1;
		return ret;
	}
	//Try to get current bss info size
	ret = wlan_sta_get_bss_size(&size);
	if (ret != 0) {
		WMG_ERROR("Get current bss info size failed!\n");
		return ret;
	}
	bss_get.size = size;
	bss_get.bss = malloc(size);

	//Try to get current bss info
	ret = wlan_sta_get_bss(&bss_get);
	if (ret != 0) {
		WMG_ERROR("Get current bss info failed!\n");
		return ret;
	}

	//Gererate 32 bytes HEX psk, so that we don't need to calcute next time
	wlan_gen_psk_param_t param;
	if (strlen(fast_sta_psk) == 64) {
		hex2bin(param.psk, fast_sta_psk, 32);
	} else if (fast_sta_psk[0] == '\0') {
		memset(param.psk, 0, sizeof(param.psk));
	} else {
		param.ssid_len = strlen(fast_sta_ssid);
		memcpy(param.ssid, fast_sta_ssid, param.ssid_len);
		strlcpy(param.passphrase, fast_sta_psk, sizeof(param.passphrase));
		ret = wlan_sta_gen_psk(&param);
		if (ret != 0) {
			WMG_ERROR("fail to generate psk\n");
			ret = -1;
			return ret;
		}
	}

	//Try to get connected ap info
	ret = wlan_sta_ap_info(&ap_info);
	if (ret != 0) {
		WMG_ERROR("Get ap info failed!\n");
		return ret;
	}
	WMG_DEBUG("get ap_info flags: 0x%x\n", ap_info.status);

	//Save current bss info to flash
	memset(pbss_info, 0, sizeof(bss_info_t));
	if (ap_info.status & WLAN_STA_AP_STATUS_USE_WPA3)
		pbss_info->flags |= FLAGS_BSS_USING_WPA3;
	else
		pbss_info->flags &= ~FLAGS_BSS_USING_WPA3;
	memcpy(pbss_info->ssid, fast_sta_ssid, strlen(fast_sta_ssid));
	memcpy(pbss_info->psk, param.psk, 32);

	memcpy(pbss_info->psk_ph, fast_sta_psk, 100);

	pbss_info->bss_size = size;
	memcpy(pbss_info->bss, bss_get.bss, size);
	bss_fdcm_hdl = fdcm_open(BSS_FLASH_NUM, BSS_FLASH_ADDR, BSS_FLASH_SIZE);
	if (bss_fdcm_hdl == NULL) {
		WMG_ERROR("fdcm open failed, hdl %p\n", bss_fdcm_hdl);
		ret = -1;
		return ret;
	}
	fdcm_write(bss_fdcm_hdl, pbss_info, sizeof(bss_info_t));
	fdcm_close(bss_fdcm_hdl);
	free(bss_get.bss);
	WMG_DEBUG("Save bss info done!\n");
	return ret;
}


int get_bss_from_flash(bss_info_t *pbss_info)
{
	int ret;
	uint32_t size;
	wlan_sta_bss_info_t bss_set;
	fdcm_handle_t *bss_fdcm_hdl;

	bss_fdcm_hdl = fdcm_open(BSS_FLASH_NUM, BSS_FLASH_ADDR, BSS_FLASH_SIZE);
	if (bss_fdcm_hdl == NULL) {
		WMG_ERROR("fdcm open failed, hdl %p\n", bss_fdcm_hdl);
		ret = -1;
		return ret;
	}
	size = fdcm_read(bss_fdcm_hdl, pbss_info, sizeof(bss_info_t));
	fdcm_close(bss_fdcm_hdl);
	if (size != sizeof(bss_info_t)) {
		WMG_ERROR("fdcm read failed, size %d\n", size);
		ret = -1;
		return ret;
	}
	if (pbss_info->bss_size == 0) {
		WMG_ERROR("empty bss info\n");
		ret = -1;
		return ret;
	}

	WMG_INFO("SSID:%s\n", pbss_info->ssid);
	WMG_INFO("FLAGS:0x%x\n", pbss_info->flags);
	WMG_INFO("PSK_PHRASE:%s\n", pbss_info->psk_ph);
	WMG_INFO("PSK:");
	for (int i = 1; i < 33; ++i)
		printf("%02x", pbss_info->psk[i-1]);
	WMG_INFO("\n");
	WMG_INFO("BSS size:%d\n", pbss_info->bss_size);

	WMG_DEBUG("Set current bss info!\n");
	bss_set.size = pbss_info->bss_size;
	bss_set.bss = malloc(bss_set.size);
	memcpy(bss_set.bss, pbss_info->bss, bss_set.size);
	for(int i = 0; i < bss_set.size; i++){
		printf("%02x ",bss_set.bss[i]);
	}
	ret = wlan_sta_set_bss(&bss_set);
	free(bss_set.bss);
	return ret;
}


int clear_bss_in_flash(void)
{
	int ret = 0;
	fdcm_handle_t *bss_fdcm_hdl;

	WMG_DEBUG("Clear bss info in flash!\n");
	memset(&g_bss_info, 0, sizeof(bss_info_t));
	bss_fdcm_hdl = fdcm_open(BSS_FLASH_NUM, BSS_FLASH_ADDR, BSS_FLASH_SIZE);
	if (bss_fdcm_hdl == NULL) {
		WMG_ERROR("fdcm open failed, hdl %p\n", bss_fdcm_hdl);
		ret = -1;
		return ret;
	}
	fdcm_write(bss_fdcm_hdl, &g_bss_info, sizeof(bss_info_t));
	fdcm_close(bss_fdcm_hdl);

	struct sysinfo *sysinfo = sysinfo_get();
	if (sysinfo == NULL) {
		WMG_ERROR("sysinfo %p\n", sysinfo);
		ret = -1;
		return ret;
	}

	WMG_DEBUG("Clear IP info inflash!\n");
	sysinfo->sta_use_dhcp = 1;
	sysinfo_save();
	return ret;
}

/*fast connect func get ap info from flash.*/
static wmg_status_t connect_ap_fast_flash()
{
	WMG_INFO("sta fast connect get ap info from flash...\n");
	/* judge if fast connect mode, get ap info from flash. */
	if (get_bss_from_flash(&g_bss_info)){
		WMG_WARNG("Get old bss failed!\n");
	} else {
		WMG_INFO("Get ap info success from flash, Begin fast connection\n");
	}
	char psk_buf[64];
	char *p;
	int i;
	int sleep_times = FAST_CONNECT_TIMEOUT;
	p = psk_buf;

	WMG_INFO("Set old bss info!\n");
	if (g_bss_info.psk_ph[0] == '\0') { /* for open AP */
		wlan_sta_config((uint8_t *)g_bss_info.ssid, strlen((char *)g_bss_info.ssid),
		                g_bss_info.psk_ph, 0);
	} else { /* for wpa/wpa2 fast connect */
		for (i = 0; i < 32; i++) {
			sprintf(p, "%02x", g_bss_info.psk[i]);
			p += 2;
		}
		wlan_sta_config((uint8_t *)g_bss_info.ssid, strlen((char *)g_bss_info.ssid),
		                (uint8_t *)psk_buf, 0);
	}
	WMG_INFO("Try to connect AP\n");
	wlan_sta_enable();

	/* wait for wifi fast connected, until 50us * 300 = 15s timeout. */
	while (connect_status == 0) {
		if(sleep_times <= 0)
			break;
		sleep_times--;
		usleep(50 * 1000);
	}
	WMG_INFO("Fast connect AP success, sleep_times:%d\n", sleep_times);
	//Fast connect AP success, then set dns.
	if (connect_status == 1) {
		ip_addr_t dnsserver;
		ip_addr_set_ip4_u32_val(dnsserver, ipaddr_addr("180.76.76.76"));
		dns_setserver(0, &dnsserver);
		WMG_INFO("dns_setserver success...\n");
	}
	if(connect_status == 1){
		WMG_INFO("Fast connect successful.\n");
		return WMG_STATUS_SUCCESS;
	}else{
		WMG_ERROR("Fast connect failed.\n");
        /* notify connect_timeout, or will in connecting status. */
        sta_event_notify_to_sta_dev(WIFI_CONNECT_TIMEOUT);
        /* disable wlan sta, or driver will always try connect. */
        wlan_sta_disable();
        return WMG_STATUS_FAIL;
    }
}
#endif

#if CONFIG_COMPONENTS_WLAN_CSFC && CONFIG_DRIVERS_XRADIO
/*fast connect func get ap info from file.*/
int connect_ap_fast_by_bss_info(bss_info_t *pbss_info)
{
	int ret;
	wlan_sta_bss_info_t bss_set;
	char psk_buf[64];
	char *p;
	int i;
	p = psk_buf;
	
	bss_set.size = pbss_info->bss_size;
	bss_set.bss = malloc(bss_set.size);
	memcpy(bss_set.bss, pbss_info->bss, bss_set.size);
	ret = wlan_sta_set_bss(&bss_set);
	free(bss_set.bss);

	/* for wpa/wpa2 fast connect */
	for (i = 0; i < 32; i++) {
		sprintf(p, "%02x", pbss_info->psk[i]);
		p += 2;
	}
	wlan_sta_config((uint8_t *)pbss_info->ssid, strlen((char *)pbss_info->ssid),
	                (uint8_t *)psk_buf, 0);

	return 0;
}
#endif

static wmg_status_t connect_ap_fast_file(wifi_sta_cn_para_t *cn_para)
{
	WMG_INFO("sta fast connect get ap info from file...\n");
	int sleep_times = FAST_CONNECT_TIMEOUT;

	/* judge wifi encryption method */
	switch (cn_para->sec) {
		case WIFI_SEC_NONE:
			wlan_sta_set((uint8_t *)cn_para->ssid, strlen(cn_para->ssid),(uint8_t *)cn_para->password);
			break;
		case WIFI_SEC_WPA_PSK:
		case WIFI_SEC_WPA2_PSK:
			{
#if CONFIG_COMPONENTS_WLAN_CSFC && CONFIG_DRIVERS_XRADIO
				bss_info_t pbss_info;
				strcpy(pbss_info.ssid, cn_para->ssid);
				memcpy(pbss_info.psk, cn_para->psk, sizeof(pbss_info.psk));
				memcpy(pbss_info.bss, cn_para->bss, sizeof(pbss_info.bss));
				pbss_info.bss_size = cn_para->bss_size;
				connect_ap_fast_by_bss_info(&pbss_info);
				//set dns server, use static ip addr
				extern ip_addr_t csfc_dns_server[2];
				extern void dns_setserver(uint8_t, ip_addr_t *);
				dns_setserver(0, &csfc_dns_server[0]);
				dns_setserver(1, &csfc_dns_server[1]);
#else
				wlan_sta_set((uint8_t *)cn_para->ssid, strlen(cn_para->ssid),(uint8_t *)cn_para->password);
#endif
			}
			break;
		case WIFI_SEC_WPA3_PSK:
			wlan_sta_set((uint8_t *)cn_para->ssid, strlen(cn_para->ssid),(uint8_t *)cn_para->password);
			break;
		case WIFI_SEC_WEP:
			wifi_wep_connect(cn_para->ssid, cn_para->password);
			break;

		default:
			WMG_ERROR("unknown key mgmt\n");
			return WMG_STATUS_FAIL;
	}

	WMG_INFO("Try to connect AP\n");
	if (connect_status != 1)
		wlan_sta_enable();

	/* wait for wifi connected, until 50us * 300 = 15s timeout. */
	while(connect_status == 0) {
		if(sleep_times <= 0)
			break;
		sleep_times--;
		usleep(50 * 1000);
	}
	WMG_INFO("Fast connect AP success, sleep_times:%d\n", sleep_times);
	if(mac_state == 1 && connect_status == 1)
	{
		WMG_INFO("Fast connect successful!\n");
		return WMG_STATUS_SUCCESS;
	}else{
		wlan_sta_disable();
		usleep(50 * 1000);
		sleep_times = FAST_CONNECT_TIMEOUT;
		wlan_sta_enable();
		while(connect_status == 0) {
			if(sleep_times <= 0)
				break;
			sleep_times--;
			usleep(50 * 1000);
		}
		if(mac_state == 1 && connect_status == 1)
		{
			WMG_INFO("Fast connect successful second time!\n");
			return WMG_STATUS_SUCCESS;
		}else{
			WMG_INFO("Fast connect failed second time!\n");
			sta_event_notify_to_sta_dev(WIFI_CONNECT_TIMEOUT);
			return WMG_STATUS_SUCCESS;
		}
	}
	WMG_ERROR("Fast connect failed!\n");
	sta_event_notify_to_sta_dev(WIFI_CONNECT_TIMEOUT);
	return WMG_STATUS_FAIL;
}
//
static int hex_to_bin(char ch)
{
	if ((ch >= '0') && (ch <= '9'))
		return ch - '0';
	ch = tolower(ch);
	if ((ch >= 'a') && (ch <= 'f'))
		return ch - 'a' + 10;
	return -1;
}

static int hex2bin(uint8_t *dst, const char *src, size_t count)
{
	while (count--) {
		int hi = hex_to_bin(*src++);
		int lo = hex_to_bin(*src++);

		if ((hi < 0) || (lo < 0))
			return -1;

		*dst++ = (hi << 4) | lo;
	}
	return 0;
}

#if CONFIG_COMPONENTS_WLAN_CSFC && CONFIG_DRIVERS_XRADIO
int get_bss_info (bss_info_t *pbss_info, const char *ssid, const char *psk)
{
	int ret = 0;
	uint32_t size;
	wlan_sta_bss_info_t bss_get;
	wlan_sta_ap_t ap_info;

	//Try to get current bss info size
	ret = wlan_sta_get_bss_size(&size);
	if (ret != 0) {
		WMG_ERROR("Get current bss info size failed!\n");
		return ret;
	}
	bss_get.size = size;
	bss_get.bss = malloc(size);

	//Try to get current bss info
	ret = wlan_sta_get_bss(&bss_get);
	if (ret != 0) {
		WMG_ERROR("Get current bss info failed!\n");
		return ret;
	}

	//Gererate 32 bytes HEX psk, so that we don't need to calcute next time
	wlan_gen_psk_param_t param;
	if (strlen(psk) == 64) {
		hex2bin(param.psk, psk, 32);
	} else if (psk[0] == '\0') {
		memset(param.psk, 0, sizeof(param.psk));
	} else {
		param.ssid_len = strlen(ssid);
		memcpy(param.ssid, ssid, param.ssid_len);
		strlcpy(param.passphrase, psk, sizeof(param.passphrase));
		ret = wlan_sta_gen_psk(&param);
		if (ret != 0) {
			WMG_ERROR("fail to generate psk\n");
			ret = -1;
			return ret;
		}
	}

	//Try to get connected ap info
	ret = wlan_sta_ap_info(&ap_info);
	if (ret != 0) {
		WMG_ERROR("Get ap info failed!\n");
		return ret;
	}
	WMG_DEBUG("get ap_info flags: 0x%x\n", ap_info.status);

	//get current bss info
	memset(pbss_info, 0, sizeof(bss_info_t));
	//if (ap_info.status & WLAN_STA_AP_STATUS_USE_WPA3)
	//	pbss_info->flags |= FLAGS_BSS_USING_WPA3;
	//else
	//	pbss_info->flags &= ~FLAGS_BSS_USING_WPA3;
	memcpy(pbss_info->ssid, ssid, strlen(ssid));
	memcpy(pbss_info->psk, param.psk, 32);
	memcpy(pbss_info->psk_ph, psk, 100);
	pbss_info->bss_size = size;
	memcpy(pbss_info->bss, bss_get.bss, size);

	return 0;
}
//
#endif

/*normal connect func.*/
static wmg_status_t connect_ap_normal(wifi_sta_cn_para_t *cn_para)
{
	WMG_DEBUG("sta normal connect...\n");
	struct sysinfo *sysinfo = sysinfo_get();

	if(cn_para->ssid == NULL) {
		WMG_ERROR("freertos os unsupport bssid connect\n");
		sta_event_notify_to_sta_dev(WIFI_DISCONNECTED);
		return WMG_STATUS_UNSUPPORTED;
	}
	int sleep_times = CONNECT_TIMEOUT;
#ifdef CONFIG_ARCH_SUN20IW2P1
	int ret;
	char *sec_buf;
	WMG_DEBUG("wifi state: %d\n", mac_state);
	if (mac_state != 0){
		WMG_DEBUG("need disconnect before connect.\n");
		freertos_sta_disconnect();
		sleep_times += 50;
	}

	/* judge wifi encryption method */
	switch (cn_para->sec) {
		case WIFI_SEC_NONE:
			ret = wlan_sta_set((uint8_t *)cn_para->ssid, strlen(cn_para->ssid),(uint8_t *)cn_para->password);
			sec_buf = "WIFI_SEC_NONE";
			break;
		case WIFI_SEC_WPA_PSK:
			ret = wlan_sta_set((uint8_t *)cn_para->ssid, strlen(cn_para->ssid),(uint8_t *)cn_para->password);
			sec_buf = "WIFI_SEC_WPA_PSK";
			break;
		case WIFI_SEC_WPA2_PSK:
			ret = wlan_sta_set((uint8_t *)cn_para->ssid, strlen(cn_para->ssid),(uint8_t *)cn_para->password);
			sec_buf = "WIFI_SEC_WPA2_PSK";
			break;
		case WIFI_SEC_WPA3_PSK:
			ret = wlan_sta_set((uint8_t *)cn_para->ssid, strlen(cn_para->ssid),(uint8_t *)cn_para->password);
			sec_buf = "WIFI_SEC_WPA3_PSK";
			break;
		case WIFI_SEC_WEP:
			wifi_wep_connect(cn_para->ssid, cn_para->password);
			sec_buf = "WIFI_SEC_WEP";
			break;

		default:
			WMG_ERROR("unknown key mgmt\n");
			return WMG_STATUS_FAIL;
	}

	//normal connect use dhcp
	sysinfo->sta_use_dhcp = 1;

	if (connect_status == 0) {
		WMG_DEBUG("Connecting AP ......\n");
		wlan_sta_enable();
		wlan_sta_connect();
		if(call_disconnect || autoconn_disabled){
			wlan_sta_connect(); /* if call "waln_sta_disconnect, must call wlan_sta_connect". */
			call_disconnect = 0;
			autoconn_disabled = 0;
		}
	}

	/* wait for wifi connected, until 50us * 200 = 10s timeout. */
	while(connect_status == 0) {
		if(sleep_times <= 0) {
			WMG_ERROR("Connect AP timeout\n");
			break;
		}
		sleep_times--;
		usleep(50 * 1000);
	}

	WMG_DEBUG("Connect AP break, then save ap info.\n");
	/* wifi connected, save ap info. */
	if(connect_status == 1){
		/* save ap info in file, /data/wpa_supplicant.conf. */
		/* if file existed, we should not create new one */
		int fd;
		int n_write;
		int n_read;
		char *buf_temp = ":";
		char *read_buf;
		bss_info_t pbss_info;

		if(!access("/data/wpa_supplicant.conf", R_OK))
		{
			WMG_DEBUG("wpa_supplicant.conf file is existed\n");
			fd = open("/data/wpa_supplicant.conf", O_RDWR | O_TRUNC);
			n_write = 0;
			n_write += write(fd, cn_para->ssid, strlen(cn_para->ssid));
			n_write += write(fd, buf_temp, strlen(buf_temp));
			n_write += write(fd, cn_para->password, strlen(cn_para->password));
			n_write += write(fd, buf_temp, strlen(buf_temp));
			n_write += write(fd, sec_buf, strlen(sec_buf));
			n_write += write(fd, buf_temp, strlen(buf_temp));
#if CONFIG_COMPONENTS_WLAN_CSFC && CONFIG_DRIVERS_XRADIO
			//
			struct netif *nif = g_wlan_netif;
			n_write += write(fd, &nif->ip_addr, sizeof(ip_addr_t));
			n_write += write(fd, &nif->gw, sizeof(ip_addr_t));
			n_write += write(fd, &nif->netmask, sizeof(ip_addr_t));
			const ip_addr_t *csfc_dns_server[2];
			extern ip_addr_t * dns_getserver(uint8_t);
			csfc_dns_server[0] = dns_getserver(0);
			csfc_dns_server[1] = dns_getserver(1);
			n_write += write(fd, csfc_dns_server[0], sizeof(ip_addr_t));
			n_write += write(fd, csfc_dns_server[1], sizeof(ip_addr_t));
			get_bss_info (&pbss_info, cn_para->ssid, cn_para->password);
			n_write += write(fd, pbss_info.psk, 32);
			n_write += write(fd, &pbss_info.bss_size, sizeof(uint32_t));
			n_write += write(fd, pbss_info.bss, pbss_info.bss_size);
			//
#endif
			if(n_write != -1)
				WMG_DEBUG("write %d byte to wpa_supplicant.conf\n", n_write);
			close(fd);
		}else{
			WMG_DEBUG("wpa_supplicant.conf file is not existed\n");
			fd = open("/data/wpa_supplicant.conf", O_RDWR | O_CREAT | O_WRONLY | O_TRUNC, 0666);
			n_write = 0;
			n_write += write(fd, cn_para->ssid, strlen(cn_para->ssid));
			n_write += write(fd, buf_temp, strlen(buf_temp));
			n_write += write(fd, cn_para->password, strlen(cn_para->password));
			n_write += write(fd, buf_temp, strlen(buf_temp));
			n_write += write(fd, sec_buf, strlen(sec_buf));
			n_write += write(fd, buf_temp, strlen(buf_temp));
#if CONFIG_COMPONENTS_WLAN_CSFC && CONFIG_DRIVERS_XRADIO
			//
			struct netif *nif = g_wlan_netif;
			n_write += write(fd, &nif->ip_addr, sizeof(ip4_addr_t));
			n_write += write(fd, &nif->gw, sizeof(ip4_addr_t));
			n_write += write(fd, &nif->netmask, sizeof(ip4_addr_t));
			const ip_addr_t *csfc_dns_server[2];
			extern ip_addr_t * dns_getserver(uint8_t);
			csfc_dns_server[0] = dns_getserver(0);
			csfc_dns_server[1] = dns_getserver(1);
			n_write += write(fd, csfc_dns_server[0], sizeof(ip_addr_t));
			n_write += write(fd, csfc_dns_server[1], sizeof(ip_addr_t));
			get_bss_info (&pbss_info, cn_para->ssid, cn_para->password);
			n_write += write(fd, pbss_info.psk, 32);
			n_write += write(fd, &pbss_info.bss_size, sizeof(uint32_t));
			n_write += write(fd, pbss_info.bss, pbss_info.bss_size);
			//
#endif
			if(n_write != -1)
				WMG_DEBUG("write %d byte to wpa_supplicant.conf\n", n_write);
			close(fd);
		}

		WMG_DEBUG("ssid: %s password: %s\n", cn_para->ssid, cn_para->password);
		fd = open("/data/wpa_supplicant.conf", O_RDWR);
		read_buf = (char *)malloc(n_write+1);
		memset(read_buf, 0, n_write + 1);
		n_read = read(fd, read_buf, n_write);
		WMG_DEBUG("read %d, context:%s\n", n_read, read_buf);
		close(fd);

#if PRJCONF_SYSINFO_SAVE_TO_FLASH
		/*Save new bss info to flash*/
		strcpy(fast_sta_ssid, cn_para->ssid);
		strcpy(fast_sta_psk, cn_para->password);
		WMG_DEBUG("fast_sta_ssid: %s, fast_sta_psk: %s\n", fast_sta_ssid, fast_sta_psk);
		clear_bss_in_flash();
		save_bss_to_flash(&g_bss_info);

		if (sysinfo == NULL) {
			printf("sysinfo %p\n", sysinfo);
			return;
		}
		/*Save IP info to flash.*/
		struct netif *nif = g_wlan_netif;
		WMG_INFO("ip_addr:%"U16_F".%"U16_F".%"U16_F".%"U16_F"\n", \
				ip4_addr1_16(netif_ip4_addr(g_wlan_netif)), ip4_addr2_16(netif_ip4_addr(g_wlan_netif)), \
				ip4_addr3_16(netif_ip4_addr(g_wlan_netif)), ip4_addr4_16(netif_ip4_addr(g_wlan_netif)));
		WMG_INFO("gw_addr:%"U16_F".%"U16_F".%"U16_F".%"U16_F"\n", \
				ip4_addr1_16(netif_ip4_gw(g_wlan_netif)), ip4_addr2_16(netif_ip4_gw(g_wlan_netif)), \
				ip4_addr3_16(netif_ip4_gw(g_wlan_netif)), ip4_addr4_16(netif_ip4_gw(g_wlan_netif)));
		WMG_INFO("netmask:%"U16_F".%"U16_F".%"U16_F".%"U16_F"\n", \
				ip4_addr1_16(netif_ip4_netmask(g_wlan_netif)), ip4_addr2_16(netif_ip4_netmask(g_wlan_netif)), \
				ip4_addr3_16(netif_ip4_netmask(g_wlan_netif)), ip4_addr4_16(netif_ip4_netmask(g_wlan_netif)));
		/*use static ip for fast connect.*/
		sysinfo->sta_use_dhcp = 0;
		memcpy(&sysinfo->netif_sta_param.ip_addr, &nif->ip_addr, sizeof(ip_addr_t));
		memcpy(&sysinfo->netif_sta_param.gateway, &nif->gw, sizeof(ip_addr_t));
		memcpy(&sysinfo->netif_sta_param.net_mask, &nif->netmask, sizeof(ip_addr_t));
		sysinfo_save();
#endif
		return WMG_STATUS_SUCCESS;
	}
	/* connect failed, print state then disable wifi. */
	WMG_DEBUG("wifi state: %d, connect_status: %d\n", mac_state, connect_status);
    /* notify connect_timeout, or will in connecting status. */
	sta_event_notify_to_sta_dev(WIFI_CONNECT_TIMEOUT);
	/* disable wlan sta, or driver will always try connect. */
	wlan_sta_disable();
	return WMG_STATUS_FAIL;
#endif /* CONFIG_ARCH_SUN20IW2P1 */

#ifdef CONFIG_ARCH_SUN8IW18P1
	WMG_ERROR("ssid:%s, passwd:%s\n", cn_para->ssid, cn_para->password);
	wifi_connect(cn_para->ssid, cn_para->password);
	sta_event_notify_to_sta_dev(WIFI_CONNECTED);
#endif /* CONFIG_ARCH_SUN20IW2P1 */
}

static wmg_status_t freertos_sta_connect(wifi_sta_cn_para_t *cn_para)
{
	connect_status = 0;

	WMG_INFO("sta connect {cn_para->fast_connect :%d, cn_para->ssid: [%s], cn_para->password: [%s] cn_para->sec: %d}\n", cn_para->fast_connect, cn_para->ssid, cn_para->password, cn_para->sec);
	if (cn_para->fast_connect){
#if PRJCONF_SYSINFO_SAVE_TO_FLASH
		return connect_ap_fast_flash();
#endif
		return connect_ap_fast_file(cn_para);
	}
	return connect_ap_normal(cn_para);
}

static wmg_status_t freertos_sta_auto_connect(bool *autoconnect)
{
	int sleep_times = CONNECT_TIMEOUT;
	/* set wifi auto connect disable. */
	if (!(*autoconnect)){
		WMG_DEBUG("sta set auto connect disable\n");
		wlan_sta_set_autoconnect(0);
		autoconn_disabled = 1;
		return WMG_STATUS_SUCCESS;
	}

	/* set wifi auto connect enable. */
	if(mac_state && connect_status == 1){
		WMG_DEBUG("wifi has already connected!\n");
		return WMG_STATUS_SUCCESS;
	}
	/* check /data/wpa_supplicant.conf file is exist. */
	if(access("/data/wpa_supplicant.conf", R_OK))
	{
		WMG_DEBUG("wpa_supplicant.conf file is not existed\n");
		return WMG_STATUS_FAIL;
	}

	/* check /data/wpa_supplicant.conf file is empty. */
	FILE* file1 = fopen("/data/wpa_supplicant.conf", "r");
	int c = fgetc(file1);
	if (c == EOF) {
		WMG_DEBUG("wpa_supplicant.conf file is empty\n");
		fclose(file1);
		return WMG_STATUS_FAIL;
	}
	fclose(file1);

	/* start auto connect. */
	WMG_DEBUG("sta set auto connect enable\n");
	wlan_sta_set_autoconnect(1);
	autoconn_disabled = 0;
	wlan_sta_enable();
	wlan_sta_connect();

	/* wait for wifi dhcp successed, until 50us * 200 = 10s timeout. */
	while(connect_status == 0) {
		if(sleep_times <= 0)
			break;
		sleep_times--;
		usleep(50 * 1000);
	}

	if(mac_state && connect_status){
		WMG_DEBUG("wifi reconnect success!\n");
		return WMG_STATUS_SUCCESS;
	}else {
		WMG_DEBUG("wifi reconnect failed!\n");
		return WMG_STATUS_FAIL;
	}
}

static wmg_status_t freertos_sta_get_info(wifi_sta_info_t *sta_info)
{
	WMG_DEBUG("sta get info\n");
	int ret;
	struct netif *nif = g_wlan_netif;
	wlan_ext_signal_t signal;

	wlan_sta_ap_t *ap = cmd_malloc(sizeof(wlan_sta_ap_t));
	if (ap == NULL) {
		WMG_ERROR("no mem\n");
		return WMG_STATUS_FAIL;
	}
	ret = wlan_sta_ap_info(ap);
	if (ret == 0) {
		sta_info->id = -1;
		sta_info->freq = ap->freq;
		// sta_info->rssi = ap->level;
		memcpy(sta_info->bssid, ap->bssid, (sizeof(uint8_t) * 6));
		memcpy(sta_info->ssid, ap->ssid.ssid, ap->ssid.ssid_len);
		//连接上的网络根据收发包信号强度更新
		wlan_ext_request(g_wlan_netif, WLAN_EXT_CMD_GET_SIGNAL, (int)(&signal));
		sta_info->rssi = signal.rssi / 2 + signal.noise;

		if (NET_IS_IP4_VALID(nif) && netif_is_link_up(nif)) {
			sta_info->ip_addr[0] = (nif->ip_addr.addr & 0xff);
			sta_info->ip_addr[1] = ((nif->ip_addr.addr >> 8) & 0xff);
			sta_info->ip_addr[2] = ((nif->ip_addr.addr >> 16) & 0xff);
			sta_info->ip_addr[3] = ((nif->ip_addr.addr >> 24) & 0xff);

			sta_info->gw_addr[0] = (nif->gw.addr & 0xff);
			sta_info->gw_addr[1] = ((nif->gw.addr >> 8) & 0xff);
			sta_info->gw_addr[2] = ((nif->gw.addr >> 16) & 0xff);
			sta_info->gw_addr[3] = ((nif->gw.addr >> 24) & 0xff);

		}

		if(ap->wpa_flags & WPA_FLAGS_WPA) {
			sta_info->sec = WIFI_SEC_WPA_PSK;
		} else if ((ap->wpa_flags & (WPA_FLAGS_RSN | WPA_FLAGS_SAE)) == WPA_FLAGS_RSN) {
			sta_info->sec = WIFI_SEC_WPA2_PSK;
		} else if ((ap->wpa_flags & (WPA_FLAGS_RSN | WPA_FLAGS_SAE)) == (WPA_FLAGS_RSN | WPA_FLAGS_SAE)) {
			sta_info->sec = WIFI_SEC_WPA3_PSK;
		} else if (ap->wpa_flags & WPA_FLAGS_WEP) {
			sta_info->sec = WIFI_SEC_WEP;
		} else {
			sta_info->sec = WIFI_SEC_NONE;
		}
	}
	cmd_free(ap);
	return WMG_STATUS_SUCCESS;
}

static wmg_status_t freertos_sta_scan_networks(get_scan_results_para_t *sta_scan_results_para)
{
	WMG_DEBUG("sta scan\n");
	int ret, i;
	int sleep_times = SCAN_TIMEOUT;
	int scan_default_size = 50;

	wlan_sta_scan_results_t results;
	results.ap = cmd_malloc(scan_default_size * sizeof(wlan_sta_ap_t));
	if (results.ap == NULL) {
		WMG_ERROR("no mem\n");
		return WMG_STATUS_FAIL;
    }
	results.size = scan_default_size;

    /* scan hidden ssid. */
    if (sta_scan_results_para->ssid != NULL) {
		WMG_DEBUG("sta scan hidden ssid: %s\n", sta_scan_results_para->ssid);
        wlan_sta_config(sta_scan_results_para->ssid, strlen(sta_scan_results_para->ssid), NULL, WLAN_STA_CONF_FLAG_WPA3);
	}

	/* set scan num max, default 50 */
	if ((sta_scan_results_para->arr_size != 0) && (sta_scan_results_para->arr_size < scan_default_size))
	{
		WMG_DEBUG("sta scan bss_num_max=%d\n", sta_scan_results_para->arr_size);
		scan_default_size = sta_scan_results_para->arr_size;
	}
	wlan_sta_bss_max_count((uint8_t)scan_default_size);

	/* judge need to scan once, or directly get last scan results. */
	WMG_INFO("sta scan_action = %d\n", sta_scan_results_para->scan_results->scan_action);
	if (sta_scan_results_para->scan_results->scan_action == 1){
		//add: 强制清除缓存，解决关闭热点后第一次扫描还会存在缓存的问题
		wlan_sta_bss_flush(0);
		wlan_sta_scan_once();
		scan_state = 0;
		WMG_DEBUG("%s->%d, scan state: %d\n", __func__, __LINE__, scan_state);
		/* wait for scan success event, until 50us * 100 = 5s timeout. */
		while(!scan_state) {
			if(sleep_times <= 0)
				break;
			sleep_times--;
			usleep(50 * 1000);
		}
	}
	printf("[ %s ][ %d ]   \n",__FUNCTION__,__LINE__);
	/* scan success, but no results, print info then return.*/
	wlan_sta_scan_result(&results);
	WMG_DEBUG("%s->%d, scan state: %d, we will clear it to 0!\n", __func__, __LINE__, scan_state);
	scan_state = 0;
	if (results.num == 0){
		WMG_DEBUG("sta scan success, but results is 0\n");
		cmd_free(results.ap);
		return WMG_STATUS_SUCCESS;
	}

	/* cp scan driver results to wifimg.*/
	*(sta_scan_results_para->bss_num) = results.num;
	for(i = 0; i < results.num; ++i) {
		WMG_DEBUG("driver scan one %s\n", results.ap[i].ssid.ssid);
		memcpy(sta_scan_results_para->scan_results[i].bssid, results.ap[i].bssid, (sizeof(uint8_t) * 6));
		memcpy(sta_scan_results_para->scan_results[i].ssid, results.ap[i].ssid.ssid, SSID_MAX_LEN);
		sta_scan_results_para->scan_results[i].ssid[SSID_MAX_LEN] = '\0';
		sta_scan_results_para->scan_results[i].freq = (uint32_t)(results.ap[i].freq);
		sta_scan_results_para->scan_results[i].rssi = results.ap[i].level;
		sta_scan_results_para->scan_results[i].key_mgmt = WIFI_SEC_NONE;
		if(results.ap[i].wpa_flags & WPA_FLAGS_WPA) {
			sta_scan_results_para->scan_results[i].key_mgmt = WIFI_SEC_WPA_PSK;
		}
		if ((results.ap[i].wpa_flags & (WPA_FLAGS_RSN | WPA_FLAGS_SAE)) == WPA_FLAGS_RSN) {
			sta_scan_results_para->scan_results[i].key_mgmt |= WIFI_SEC_WPA2_PSK;
		}
		if ((results.ap[i].wpa_flags & (WPA_FLAGS_RSN | WPA_FLAGS_SAE)) == (WPA_FLAGS_RSN | WPA_FLAGS_SAE)) {
			sta_scan_results_para->scan_results[i].key_mgmt |= WIFI_SEC_WPA3_PSK;
		}
		if (results.ap[i].wpa_flags & WPA_FLAGS_WEP) {
			sta_scan_results_para->scan_results[i].key_mgmt |= WIFI_SEC_WEP;
		}
	}
	cmd_free(results.ap);
	return WMG_STATUS_SUCCESS;
}

static wmg_status_t freertos_sta_remove_network(char *ssid)
{
	WMG_DEBUG("sta remove network\n");
	// check /data/wpa_supplicant.conf file is exist.
	if(access("/data/wpa_supplicant.conf", R_OK))
	{
		WMG_DEBUG("wpa_supplicant.conf file is not existed\n");
		return WMG_STATUS_FAIL;
	}

	// check /data/wpa_supplicant.conf file is empty.
	FILE* file1 = fopen("/data/wpa_supplicant.conf", "r");
	int c = fgetc(file1);
	if (c == EOF) {
		WMG_DEBUG("wpa_supplicant.conf file is empty\n");
		fclose(file1);
		return WMG_STATUS_FAIL;
	}
	fclose(file1);

	/*ToDo: check ap is exist in wpa_supplicant.conf */
	int file2;
	if (ssid != NULL) {
		file2 = open("/data/wpa_supplicant.conf", O_RDWR | O_TRUNC);
		WMG_DEBUG("remove network (%s) in /data/wpa_supplicant.conf file\n", ssid);
	} else {
		file2 = open("/data/wpa_supplicant.conf", O_RDWR | O_TRUNC);
		WMG_DEBUG("remove all networks in /data/wpa_supplicant.conf file\n");
	}
	wlan_sta_disconnect();
	call_disconnect = 1;
	wlan_sta_disable();
	close(file2);
	return WMG_STATUS_SUCCESS;
}

static wmg_status_t freertos_platform_extension(int cmd, void* cmd_para,int *erro_code)
{
	WMG_DEBUG("platform extension\n");
	switch (cmd) {
	case STA_CMD_GET_INFO:
		return freertos_sta_get_info((wifi_sta_info_t *)cmd_para);
	case STA_CMD_CONNECT:
		return freertos_sta_connect((wifi_sta_cn_para_t *)cmd_para);
	case STA_CMD_DISCONNECT:
			return freertos_sta_disconnect();
	case STA_CMD_LIST_NETWORKS:
			return WMG_STATUS_UNSUPPORTED;
	case STA_CMD_REMOVE_NETWORKS:
			return freertos_sta_remove_network((char *)cmd_para);
	case STA_CMD_SET_AUTO_RECONN:
			return freertos_sta_auto_connect((bool *)cmd_para);
	case STA_CMD_GET_SCAN_RESULTS:
			return freertos_sta_scan_networks((get_scan_results_para_t *)cmd_para);
	default:
			return WMG_FALSE;
	}
	return WMG_FALSE;
}

static wmg_sta_inf_object_t sta_inf_object = {
	.sta_init_flag = WMG_FALSE,
	.sta_auto_reconn = WMG_FALSE,
	.sta_event_cb = NULL,
	.sta_private_data = NULL,

	.sta_inf_init = freertos_sta_mode_init,
	.sta_inf_deinit = freertos_sta_mode_deinit,
	.sta_inf_enable = freertos_sta_mode_enable,
	.sta_inf_disable = freertos_sta_mode_disable,
	.sta_platform_extension = freertos_platform_extension,
};

wmg_sta_inf_object_t* sta_rtos_inf_object_register(void)
{
	return &sta_inf_object;
}
