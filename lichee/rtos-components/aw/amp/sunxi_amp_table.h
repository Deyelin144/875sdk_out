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

#ifndef _SUNXI_AMP_TABLE_H
#define _SUNXI_AMP_TABLE_H

#include "sunxi_amp_msg.h"

#include "service/demo/demo_service.h"

typedef struct
{
    sunxi_amp_func_table open;
    sunxi_amp_func_table close;
    sunxi_amp_func_table read;
    sunxi_amp_func_table write;
    sunxi_amp_func_table lseek;
    sunxi_amp_func_table fstat;
    sunxi_amp_func_table stat;
    sunxi_amp_func_table unlink;
    sunxi_amp_func_table rename;
    sunxi_amp_func_table opendir;
    sunxi_amp_func_table readdir;
    sunxi_amp_func_table closedir;
    sunxi_amp_func_table mkdir;
    sunxi_amp_func_table rmdir;
    sunxi_amp_func_table access;
    sunxi_amp_func_table truncate;
    sunxi_amp_func_table statfs;
    sunxi_amp_func_table fstatfs;
    sunxi_amp_func_table fsync;
} RPCHandler_FSYS_t;

typedef struct
{
#ifdef CONFIG_ARCH_RISCV_RV64
	sunxi_amp_func_table wlan_set_file_path;
	sunxi_amp_func_table wlan_get_file_path;
	sunxi_amp_func_table wpa_ctrl_request;
#if (defined(CONFIG_WLAN_STA) && defined(CONFIG_WLAN_AP))
	sunxi_amp_func_table wlan_start;
	sunxi_amp_func_table wlan_stop;
#elif (defined(CONFIG_WLAN_STA))
	sunxi_amp_func_table wlan_start_sta;
	sunxi_amp_func_table wlan_stop_sta;
#elif (defined(CONFIG_WLAN_AP))
	sunxi_amp_func_table wlan_start_hostap;
	sunxi_amp_func_table wlan_stop_hostap;
#endif
	sunxi_amp_func_table wlan_get_mac_addr;
	sunxi_amp_func_table wlan_set_mac_addr;
	sunxi_amp_func_table wlan_set_ip_addr;
	sunxi_amp_func_table wlan_set_ps_mode;
	sunxi_amp_func_table wlan_set_appie;
	sunxi_amp_func_table wlan_set_channel;
	sunxi_amp_func_table net80211_mail_init;
	sunxi_amp_func_table net80211_mail_deinit;
	sunxi_amp_func_table net80211_ifnet_setup;
	sunxi_amp_func_table net80211_ifnet_release;
	sunxi_amp_func_table net80211_monitor_enable_rx;
	sunxi_amp_func_table wlan_send_raw_frame;
	sunxi_amp_func_table wlan_if_create;
	sunxi_amp_func_table wlan_if_delete;
	sunxi_amp_func_table wlan_ext_request;
	sunxi_amp_func_table wlan_get_sysinfo_m33;
	/* lmac */
	sunxi_amp_func_table xradio_drv_cmd;
	sunxi_amp_func_table ethernetif_request_output;
#elif (defined CONFIG_ARCH_ARM_ARMV8M)
	sunxi_amp_func_table wlan_ap_get_default_conf;
	sunxi_amp_func_table wlan_event_notify;
	sunxi_amp_func_table wlan_monitor_input;
	sunxi_amp_func_table ethernetif_raw_input;
	sunxi_amp_func_table ethernetif_get_mode;
	sunxi_amp_func_table ethernetif_get_state;
	sunxi_amp_func_table wlan_ext_temp_volt_event_input;
	sunxi_amp_func_table xr_get_sdd_file_path;
#endif
} RPCHandler_NET_t;

typedef struct
{
#ifdef CONFIG_ARCH_RISCV_RV64
	sunxi_amp_func_table xrbtc_init;
	sunxi_amp_func_table xrbtc_deinit;
	sunxi_amp_func_table xrbtc_enable;
	sunxi_amp_func_table xrbtc_disable;
	sunxi_amp_func_table xrbtc_hci_init;
	sunxi_amp_func_table xrbtc_hci_deinit;
	sunxi_amp_func_table xrbtc_hci_h2c;
	sunxi_amp_func_table xrbtc_hci_c2h_cb;
	sunxi_amp_func_table xrbtc_sdd_init;
	sunxi_amp_func_table xrbtc_sdd_write;
#elif (defined CONFIG_ARCH_ARM_ARMV8M)
	sunxi_amp_func_table bt_event_notify;
#endif
} RPCHandler_BT_t;

typedef struct
{
	sunxi_amp_func_table _rpc_pm_wakelocks_getcnt_dsp;
	sunxi_amp_func_table _rpc_pm_msgtodsp_trigger_notify;
	sunxi_amp_func_table _rpc_pm_msgtodsp_trigger_suspend;
	sunxi_amp_func_table _rpc_pm_msgtodsp_check_subsys_assert;
	sunxi_amp_func_table _rpc_pm_msgtodsp_check_wakesrc_num;
} RPCHandler_PMOFDSP_t;

typedef struct
{
	sunxi_amp_func_table _rpc_pm_wakelocks_getcnt_riscv;
	sunxi_amp_func_table _rpc_pm_msgtorv_trigger_notify;
	sunxi_amp_func_table _rpc_pm_msgtorv_trigger_suspend;
	sunxi_amp_func_table _rpc_pm_msgtorv_check_subsys_assert;
	sunxi_amp_func_table _rpc_pm_msgtorv_check_wakesrc_num;
} RPCHandler_PMOFRV_t;

typedef struct
{
	sunxi_amp_func_table _rpc_pm_set_wakesrc;
	sunxi_amp_func_table _rpc_pm_trigger_suspend;
	sunxi_amp_func_table _rpc_pm_report_subsys_action;
	sunxi_amp_func_table _rpc_pm_subsys_soft_wakeup;
} RPCHandler_PMOFM33_t;

typedef struct
{
    sunxi_amp_func_table exe_cmd;
} RPCHandler_ARM_CONSOLE_t;

typedef struct
{
    sunxi_amp_func_table exe_cmd;
} RPCHandler_DSP_CONSOLE_t;

typedef struct
{
    sunxi_amp_func_table exe_cmd;
} RPCHandler_RV_CONSOLE_t;

#endif

typedef struct
{
    sunxi_amp_func_table nor_read;
    sunxi_amp_func_table nor_write;
    sunxi_amp_func_table nor_erase;
    sunxi_amp_func_table nor_ioctrl;
} RPCHandler_FLASHC_t;

typedef struct
{
    sunxi_amp_func_table misc_ioctrl;
} RPCHandler_RV_MISC_t;

typedef struct
{
    sunxi_amp_func_table misc_ioctrl;
} RPCHandler_M33_MISC_t;

typedef struct
{
    sunxi_amp_func_table misc_ioctrl;
} RPCHandler_DSP_MISC_t;

typedef struct
{
	sunxi_amp_func_table audio_test1;
	sunxi_amp_func_table audio_test2;
	sunxi_amp_func_table audio_test3;
	sunxi_amp_func_table audio_test4;
	/* AudioTrack */
	sunxi_amp_func_table AudioTrackCreateRM;
	sunxi_amp_func_table AudioTrackDestroyRM;
	sunxi_amp_func_table AudioTrackControlRM;
	sunxi_amp_func_table AudioTrackSetupRM;
	sunxi_amp_func_table AudioTrackWriteRM;
	/* AudioRecord */
	sunxi_amp_func_table AudioRecordCreateRM;
	sunxi_amp_func_table AudioRecordDestroyRM;
	sunxi_amp_func_table AudioRecordControlRM;
	sunxi_amp_func_table AudioRecordSetupRM;
	sunxi_amp_func_table AudioRecordReadRM;
} RPCHandler_AUDIO_t;

typedef struct
{
	sunxi_amp_func_table rpdata_test1;
	sunxi_amp_func_table rpdata_ioctl;
} RPCHandler_RPDATA_t;

typedef struct
{
	sunxi_amp_func_table tfm_sunxi_flashenc_set_region;
	sunxi_amp_func_table tfm_sunxi_flashenc_set_key;
	sunxi_amp_func_table tfm_sunxi_flashenc_set_ssk_key;
	sunxi_amp_func_table tfm_sunxi_flashenc_enable;
	sunxi_amp_func_table tfm_sunxi_flashenc_disable;
} RPCHandler_TFM_t;


typedef struct
{
	sunxi_amp_func_table get_arm_version;
} RPCHandler_VERSIONINFO_ARM_t;

typedef struct
{
	sunxi_amp_func_table get_dsp_version;
} RPCHandler_VERSIONINFO_DSP_t;

typedef struct
{
	sunxi_amp_func_table tjpeg_decode;
} RPCHandler_TJPEG_t;

typedef struct
{
	sunxi_amp_func_table VideoDecInit;
	sunxi_amp_func_table VideoDecDeinit;
	sunxi_amp_func_table Mp4VideoDecFrame;
} RPCHandler_XVID_t;
