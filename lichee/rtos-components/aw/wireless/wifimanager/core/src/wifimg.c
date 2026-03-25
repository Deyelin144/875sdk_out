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

#include <wifimg.h>
#include <wmg_common.h>
#include <wifi_log.h>
#include <string.h>

wmg_status_t wifimanager_init(void)
{
	return __wifimanager_init();
}

wmg_status_t wifimanager_deinit(void)
{
	return __wifimanager_deinit();
}

wmg_status_t wifi_on(wifi_mode_t mode)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_PLA_ID,WMG_PLA_ACT_WIFI_ON,1,1,&mode,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_off(void)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_PLA_ID,WMG_PLA_ACT_WIFI_OFF,1,1,NULL,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_sta_connect(wifi_sta_cn_para_t *cn_para)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_STA_ID,WMG_STA_ACT_CONNECT,1,1,cn_para,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_sta_disconnect(void)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_STA_ID,WMG_STA_ACT_DISCONNECT,1,1,NULL,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_sta_auto_reconnect(wmg_bool_t enable)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_STA_ID,WMG_STA_ACT_AUTO_RECONNECT,1,1,&enable,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_sta_auto_connect(char *ssid)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_STA_ID,WMG_STA_ACT_AUTO_CONNECT,1,1,ssid,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_sta_get_info(wifi_sta_info_t *sta_info)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_STA_ID,WMG_STA_ACT_GET_INFO,1,1,sta_info,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_sta_list_networks(wifi_sta_list_t *sta_list_networks)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_STA_ID,WMG_STA_ACT_LIST_NETWORKS,1,1,sta_list_networks,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_sta_remove_networks(char *ssid)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_STA_ID,WMG_STA_ACT_REMOVE_NETWORKS,1,1,ssid,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_ap_enable(wifi_ap_config_t *ap_config)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_AP_ID,WMG_AP_ACT_ENABLE,1,1,ap_config,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_ap_disable(void)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_AP_ID,WMG_AP_ACT_DISABLE,1,1,NULL,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_ap_get_config(wifi_ap_config_t *ap_config)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_AP_ID,WMG_AP_ACT_GET_CONFIG,1,1,ap_config,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_monitor_enable(uint8_t channel)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_MONITOR_ID,WMG_MONITOR_ACT_ENABLE,1,1,&channel,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_monitor_set_channel(uint8_t channel)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_MONITOR_ID,WMG_MONITOR_ACT_SET_CHANNEL,1,1,&channel,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_monitor_disable(void)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_MONITOR_ID,WMG_MONITOR_ACT_DISABLE,1,1,NULL,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_p2p_enable(wifi_p2p_config_t *p2p_config)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_P2P_ID,WMG_P2P_ACT_ENABLE,1,1,p2p_config,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_p2p_disable(void)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_P2P_ID,WMG_P2P_ACT_DISABLE,1,1,NULL,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_p2p_find(wifi_p2p_peers_t *p2p_peers, uint8_t find_second)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_P2P_ID,WMG_P2P_ACT_FIND,2,1,p2p_peers,&find_second,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_p2p_connect(uint8_t *p2p_mac_addr)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_P2P_ID,WMG_P2P_ACT_CONNECT,1,1,p2p_mac_addr,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_p2p_disconnect(uint8_t *p2p_mac_addr)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_P2P_ID,WMG_P2P_ACT_DISCONNECT,1,1,p2p_mac_addr,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_p2p_get_info(wifi_p2p_info_t *p2p_info)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_P2P_ID,WMG_P2P_ACT_GET_INFO,1,1,p2p_info,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_register_msg_cb(wifi_msg_cb_t msg_cb, void *msg_cb_arg)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_PLA_ID,WMG_PLA_ACT_WIFI_REGISTER_MSG_CB,1,2,&msg_cb,&cb_msg,msg_cb_arg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_set_scan_param(wifi_scan_param_t *scan_param)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_PLA_ID,WMG_PLA_ACT_SCAN_PARAM,1,1,scan_param,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_get_scan_results(wifi_scan_result_t *scan_results, const char *ssid, uint32_t *bss_num, uint32_t arr_size)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_PLA_ID,WMG_PLA_ACT_SCAN_RESULTS,4,1,
				(void *)scan_results,(void *)ssid,(void *)bss_num,(void *)&arr_size,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_set_mac(const char *ifname, uint8_t *mac_addr)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_PLA_ID,WMG_PLA_ACT_WIFI_SET_MAC,2,1,
				(void *)ifname,(void *)mac_addr,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_get_mac(const char *ifname, uint8_t *mac_addr)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_PLA_ID,WMG_PLA_ACT_WIFI_GET_MAC,2,1,
				(void *)ifname,(void *)mac_addr,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_send_80211_raw_frame(uint8_t *data, uint32_t len, void *priv)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_PLA_ID,WMG_PLA_ACT_WIFI_SEND_80211_RAW_FRAME,1,1,NULL,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_set_country_code(const char *country_code)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_PLA_ID,WMG_PLA_ACT_WIFI_SET_COUNTRY_CODE,1,1,country_code,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_get_country_code(char *country_code)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_PLA_ID,WMG_PLA_ACT_WIFI_GET_COUNTRY_CODE,1,1,country_code,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_set_ps_mode(wmg_bool_t enable)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_PLA_ID,WMG_PLA_ACT_WIFI_SET_PS_MODE,1,1,&enable,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

#ifdef OS_NET_XRLINK_OS
wmg_status_t wifi_vendor_send_data(uint8_t *data, uint32_t len)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	void *call_argv[2];
	void *cb_argv[1];

	call_argv[0] = (void*)data;
	call_argv[1] = (void*)len;
	cb_argv[0] = (void*)&cb_msg;

	return __wifi_vendor_send_data(call_argv, cb_argv);

}

wmg_status_t wifi_vendor_register_rx_cb(wifi_vend_cb_t cb)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_PLA_ID,WMG_PLA_ACT_VENDOR_REGISTER_RX_CB,1,1,cb,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}
#endif

wmg_status_t wifi_linkd_protocol(wifi_linkd_mode_t mode, void *params, int second, wifi_linkd_result_t *linkd_result)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_PLA_ID,WMG_PLA_ACT_WIFI_LINKD_PROTOCOL,4,1,
				(void *)&mode,(void *)params,(void *)&second,(void *)linkd_result,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_get_wmg_state(wifi_wmg_state_t *wmg_state)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_PLA_ID,WMG_PLA_ACT_GET_WMG_STATE,1,1,wmg_state,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}

wmg_status_t wifi_send_expand_cmd(char *expand_cmd, void *expand_cb)
{
	wmg_status_t cb_msg = WMG_STATUS_FAIL;
	if(act_transfer(&wmg_act_handle,WMG_ACT_TABLE_PLA_ID,WMG_PLA_ACT_SEND_EXPAND_CMD,2,1,expand_cmd,expand_cb,&cb_msg)){
		WMG_ERROR("act fail\n");
		return WMG_STATUS_FAIL;
	}
	return cb_msg;
}
