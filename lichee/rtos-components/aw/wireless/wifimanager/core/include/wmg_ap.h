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

#ifndef __WMG_AP_H__
#define __WMG_AP_H__

#include <wmg_common.h>
#ifdef OS_NET_FREERTOS_OS
#include <freertos/event.h>
#else
#include <linux/event.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define AP_CMD_ENABLE                  0x0
#define AP_CMD_DISABLE                 0x1
#define AP_CMD_GET_CONFIG              0x2
#define AP_CMD_SET_SCAN_PARAM          0x3
#define AP_CMD_GET_SCAN_RESULTS        0x4
#define AP_CMD_GET_STATE               0x5
#define AP_CMD_VENDOR_SEND_DATA        0x6
#define AP_CMD_VENDOR_REGISTER_RX_CB   0x7

typedef void(*ap_event_cb_t)(wifi_ap_event_t event);

typedef struct dev_node {
	uint8_t bssid[6];
	struct dev_node *next;
} dev_node_t;

//这里定义的是平台接口,与特定平台实现有关
typedef struct {
	wmg_bool_t ap_init_flag;
	wmg_bool_t ap_enable_flag;
	ap_event_cb_t ap_event_cb;
	event_handle_t *ap_event_handle;
	void *ap_private_data;
	dev_node_t *dev_list;
	uint32_t sta_num;
	wmg_status_t (*ap_inf_init)(ap_event_cb_t);
	wmg_status_t (*ap_inf_deinit)(void*);
	wmg_status_t (*ap_inf_enable)(void*);
	wmg_status_t (*ap_inf_disable)(void*);
	wmg_status_t (*ap_platform_extension)(int,void*,int*);
} wmg_ap_inf_object_t;

mode_object_t* wmg_ap_register_object(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __WMG_AP_H__ */
