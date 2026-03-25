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

#ifndef _BTMG_HFP_HF_H_
#define _BTMG_HFP_HF_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_BT_HFP_CLIENT_ENABLE
#include "bt_manager.h"

btmg_err bt_hfp_hf_init(void);
btmg_err bt_hfp_hf_deinit(void);
btmg_err bt_hfp_hf_disconnect(const char *addr);
btmg_err bt_hfp_hf_disconnect_audio(const char *addr);
btmg_err bt_hfp_hf_start_voice_recognition(void);
btmg_err bt_hfp_hf_stop_voice_recognition(void);
btmg_err bt_hfp_hf_spk_vol_update(int volume);
btmg_err bt_hfp_hf_mic_vol_update(int volume);
btmg_err bt_hfp_hf_dial(const char *number);
btmg_err bt_hfp_hf_dial_memory(int location);
btmg_err bt_hfp_hf_send_chld_cmd(btmg_hf_chld_type_t chld, int idx);
btmg_err bt_hfp_hf_send_btrh_cmd(btmg_hf_btrh_cmd_t btrh);
btmg_err bt_hfp_hf_answer_call(void);
btmg_err bt_hfp_hf_reject_call(void);
btmg_err bt_hfp_hf_query_calls(void);
btmg_err bt_hfp_hf_query_operator(void);
btmg_err bt_hfp_hf_query_number(void);
btmg_err bt_hfp_hf_send_dtmf(char code);
btmg_err bt_hfp_hf_request_last_voice_tag_number(void);
btmg_err bt_hfp_hf_send_nrec(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _BTMG_HFP_HF_H_ */
