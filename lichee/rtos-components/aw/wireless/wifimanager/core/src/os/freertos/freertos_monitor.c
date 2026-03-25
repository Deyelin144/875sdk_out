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
#include <stdbool.h>
#include <string.h>
//#include <net/if.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <wifi_log.h>
#include <wmg_monitor.h>
#include <freertos_monitor.h>
#include <freertos_common.h>
#include "wlan.h"
#include "net_init.h"
#include "net_ctrl.h"
#include "sysinfo.h"
#include "cmd_util.h"

static wmg_monitor_inf_object_t monitor_inf_object;
static observer_base *monitor_net_ob;

static void freertos_monitor_net_msg_process(uint32_t event, uint32_t data, void *arg)
{
	/* nothing to do for monitor mode in freertos */
}

static wmg_status_t freertos_monitor_mode_init()
{
	WMG_DEBUG("monitor mode init\n");
	/* nothing to do in freertos */
	return WMG_STATUS_SUCCESS;
}

static wmg_status_t freertos_monitor_mode_enable(void *para)
{
	WMG_DEBUG("monitor mode enable\n");

	xr_wlan_on(WLAN_MODE_MONITOR);

	monitor_net_ob = sys_callback_observer_create(CTRL_MSG_TYPE_NETWORK, NET_CTRL_MSG_ALL, freertos_monitor_net_msg_process, NULL);
	if (monitor_net_ob == NULL) {
		WMG_ERROR("freertos callback observer create failed\n");
		return WMG_STATUS_FAIL;
	}
	if (sys_ctrl_attach(monitor_net_ob) != 0) {
		WMG_ERROR("freertos callback observer attach failed\n");
		sys_callback_observer_destroy(monitor_net_ob);
		return WMG_STATUS_FAIL;
	} else {
		/* use sta_private_data to save observer_base point. */
		monitor_inf_object.monitor_private_data = monitor_net_ob;
	}

	struct sysinfo *sysinfo = sysinfo_get();
	if (sysinfo == NULL) {
		WMG_DEBUG("failed to get sysinfo %p\n", sysinfo);
		return WMG_STATUS_FAIL;
	}

	return WMG_STATUS_SUCCESS;
}

static wmg_status_t freertos_monitor_mode_disable()
{
	WMG_DEBUG("sta mode disable\n");

	monitor_net_ob = (observer_base *)monitor_inf_object.monitor_private_data;
	if(monitor_net_ob) {
		sys_ctrl_detach(monitor_net_ob);
		sys_callback_observer_destroy(monitor_net_ob);
		monitor_inf_object.monitor_private_data = NULL;
	}

	xr_wlan_off();

	return WMG_STATUS_SUCCESS;
}

static wmg_status_t freertos_monitor_mode_deinit()
{
	WMG_DEBUG("sta mode deinit\n");
	/* nothing to do in freertos */
	return WMG_STATUS_SUCCESS;
}

static wmg_status_t freertos_monitor_enable()
{
	return WMG_STATUS_SUCCESS;
}

static wmg_status_t freertos_monitor_disable()
{
	return WMG_STATUS_SUCCESS;
}

static wmg_status_t freertos_monitor_connect()
{
	return WMG_STATUS_SUCCESS;
}

static wmg_status_t freertos_monitor_set_channel(uint8_t channel)
{
	return WMG_STATUS_SUCCESS;
}

static wmg_status_t freertos_monitor_disconnect()
{
	return WMG_STATUS_SUCCESS;
}

static wmg_status_t freertos_platform_extension(int cmd, void* cmd_para,int *erro_code)
{
	switch (cmd) {
		case MONITOR_CMD_ENABLE:
			return freertos_monitor_enable();
		case MONITOR_CMD_DISABLE:
			return freertos_monitor_disable();
		case MONITOR_CMD_SET_CHANNL:
			return freertos_monitor_set_channel((uint8_t)(intptr_t)cmd_para);
		default:
			return WMG_STATUS_FAIL;
	}
}

static wmg_monitor_inf_object_t monitor_inf_object = {
	.monitor_init_flag = WMG_FALSE,
	.monitor_enable = WMG_FALSE,
	.monitor_pid = (void *)-1,
	.monitor_state  = NULL,
	.monitor_channel = 255,
	.monitor_private_data = NULL,

	.monitor_inf_init = freertos_monitor_mode_init,
	.monitor_inf_deinit = freertos_monitor_mode_deinit,
	.monitor_inf_enable = freertos_monitor_mode_enable,
	.monitor_inf_disable = freertos_monitor_mode_disable,
	.monitor_platform_extension = freertos_platform_extension,
};

wmg_monitor_inf_object_t * monitor_rtos_inf_object_register(void)
{
	return &monitor_inf_object;
}
