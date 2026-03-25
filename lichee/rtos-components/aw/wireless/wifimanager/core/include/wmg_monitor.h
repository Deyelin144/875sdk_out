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

#ifndef __WMG_MONITOR_H__
#define __WMG_MONITOR_H__

#include <wmg_common.h>
#ifdef OS_NET_FREERTOS_OS
#include <freertos/event.h>
#else
#include <linux/event.h>
#endif
#include <wifimg.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MON_BUF_SIZE
#define MON_BUF_SIZE    4096
#endif

#ifndef MON_IF_SIZE
#define MON_IF_SIZE     20
#endif

#define MONITOR_CMD_ENABLE                 0x0
#define MONITOR_CMD_DISABLE                0x1
#define MONITOR_CMD_SET_CHANNL             0x2
#define MONITOR_CMD_GET_STATE              0x3
#define MONITOR_CMD_VENDOR_SEND_DATA       0x4
#define MONITOR_CMD_VENDOR_REGISTER_RX_CB  0x5

typedef void(*monitor_data_frame_cb_t)(wifi_monitor_data_t *frame);

typedef struct {
	struct nl_sock *nl_sock;
	int nl80211_id;
} nl80211_state_t;

typedef struct {
	wmg_bool_t monitor_init_flag;
	wmg_bool_t monitor_enable;
	os_net_thread_t monitor_pid;
	nl80211_state_t *monitor_state;
	monitor_data_frame_cb_t monitor_data_frame_cb;
	uint8_t monitor_channel;
	void *monitor_private_data;
	char monitor_if[MON_IF_SIZE];
	wmg_status_t (*monitor_inf_init)(monitor_data_frame_cb_t,void*);
	wmg_status_t (*monitor_inf_deinit)(void*);
	wmg_status_t (*monitor_inf_enable)(void*);
	wmg_status_t (*monitor_inf_disable)(void*);
	wmg_status_t (*monitor_platform_extension)(int,void*,int*);
} wmg_monitor_inf_object_t;

mode_object_t* wmg_monitor_register_object(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __WMG_MONITOR_H__ */
