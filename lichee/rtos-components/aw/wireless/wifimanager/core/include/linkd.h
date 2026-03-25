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

#ifndef __LINKD_H__
#define __LINKD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <wifimg.h>
#include <pthread.h>

#define WMG_LINKD_PROTO_MAX     (4)
#define DEFAULT_SECOND          180

typedef void (*proto_result_cb)(wifi_linkd_result_t *linkd_result);
typedef void *(*proto_main_loop)(void *arg);
typedef struct {
	proto_result_cb result_cb;
	void *private;
} proto_main_loop_para_t;

typedef enum {
	WMG_LINKD_IDEL,
	WMG_LINKD_RUNNING,
	WMG_LINKD_OFF,
} wmg_linkd_state_t;

typedef enum {
	WMG_LINKD_RESULT_SUCCESS,
	WMG_LINKD_RESULT_FAIL,
	WMG_LINKD_RESULT_INVALIN,
} wmg_linkd_result_state_t;

typedef struct {
	wmg_linkd_state_t linkd_state;
	wifi_linkd_mode_t linkd_mode_state;
	proto_main_loop *proto_main_loop_list;
	proto_main_loop_para_t main_loop_para;
	char ssid_result[SSID_MAX_LEN];
	char psk_result[PSK_MAX_LEN];
	wmg_linkd_result_state_t result_state;
	pthread_t thread;
	wmg_status_t (*linkd_init)(void);
	wmg_status_t (*linkd_protocol_enable)(wifi_linkd_mode_t mode, void *proto_function_param);
	wmg_status_t (*linkd_protocol_get_results)(wifi_linkd_result_t *linkd_result, int second);
	void (*linkd_deinit)(void);
} wmg_linkd_object_t;

wmg_status_t wmg_linkd_protocol(wifi_linkd_mode_t mode, void *params, int second, wifi_linkd_result_t *linkd_result);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LINKD_H__ */
