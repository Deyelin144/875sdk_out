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

#include <stdint.h>

#include "xr_spp_api.h"
#include "btmg_spp_server.h"
#include "btmg_common.h"
#include "btmg_log.h"
#include "bt_manager.h"

static uint32_t conn_handle = 0;
#define SPP_MIN_SCN 1
#define SPP_MAX_SCN 30
static const xr_spp_sec_t sec_mask = XR_SPP_SEC_AUTHENTICATE;
static const xr_spp_role_t role_slave = XR_SPP_ROLE_SLAVE;

static void bt_spps_cb(xr_spp_cb_event_t event, xr_spp_cb_param_t *param)
{
    switch (event) {
    case XR_SPP_INIT_EVT:
        if (param->init.status == XR_SPP_SUCCESS) {
            BTMG_INFO("spp server init OK");
        } else {
            BTMG_ERROR("spp server init fail, status:%d", param->init.status);
        }
        break;
    case XR_SPP_START_EVT:
         if (param->start.status == XR_SPP_SUCCESS) {
            BTMG_INFO("spp server start, scn:%d", param->start.scn);
        } else {
            BTMG_ERROR("spp server start fail, status:%d", param->start.status);
        }
        break;
    case XR_SPP_SRV_STOP_EVT:
        if (param->srv_stop.status == XR_SPP_SUCCESS) {
            BTMG_INFO("spp server stop, scn:%d", param->srv_stop.scn);
        } else {
            BTMG_ERROR("spp server stop fail, status:%d", param->srv_stop.status);
        }
        break;
    case XR_SPP_DATA_IND_EVT:
        if (btmg_cb_p[CB_MAIN] && btmg_cb_p[CB_MAIN]->btmg_spps_cb.recvdata_cb)
            btmg_cb_p[CB_MAIN]->btmg_spps_cb.recvdata_cb(bda_str, param->data_ind.data, param->data_ind.len);
        break;
    case XR_SPP_CONG_EVT:
        BTMG_INFO("spp connection congestion");
        break;
    case XR_SPP_WRITE_EVT:
        BTMG_INFO("spp write complete");
        break;
    case XR_SPP_SRV_OPEN_EVT:
        bda2str(param->srv_open.rem_bda, bda_str);
        if (param->srv_open.status == XR_SPP_SUCCESS) {
            conn_handle = param->srv_open.handle;
            if (btmg_cb_p[CB_MAIN] && btmg_cb_p[CB_MAIN]->btmg_spps_cb.conn_state_cb)
                btmg_cb_p[CB_MAIN]->btmg_spps_cb.conn_state_cb(bda_str, BTMG_SPP_CONNECTED);
        } else if (param->srv_open.status == XR_SPP_FAILURE) {
            if (btmg_cb_p[CB_MAIN] && btmg_cb_p[CB_MAIN]->btmg_spps_cb.conn_state_cb)
                btmg_cb_p[CB_MAIN]->btmg_spps_cb.conn_state_cb(bda_str, BTMG_SPP_CONNECT_FAILED);
        }
        break;
    case XR_SPP_CLOSE_EVT:
        if (btmg_cb_p[CB_MAIN] && btmg_cb_p[CB_MAIN]->btmg_spps_cb.conn_state_cb) {
            if (param->close.status == XR_SPP_SUCCESS) {
                conn_handle = 0;
                btmg_cb_p[CB_MAIN]->btmg_spps_cb.conn_state_cb(bda_str, BTMG_SPP_DISCONNECTED);
            } else if (param->close.status == XR_SPP_FAILURE) {
                btmg_cb_p[CB_MAIN]->btmg_spps_cb.conn_state_cb(bda_str, BTMG_SPP_DISCONNEC_FAILED);
            }
        }
        break;
    case XR_SPP_UNINIT_EVT:
        BTMG_INFO("spp server deinitialized");
        break;
    default:
        break;
    }
}

/* bt spp server init*/
btmg_err bt_spps_init(void)
{
    xr_err_t ret;

    conn_handle = 0;
    if ((ret = xr_spp_register_callback(bt_spps_cb)) != XR_OK) {
        BTMG_ERROR("spp register cb failed: %d", ret);
        return BT_FAIL;
    }

    if ((ret = xr_spp_init(XR_SPP_MODE_CB)) != XR_OK) {
        BTMG_ERROR("spp server init failed: %d", ret);
        return BT_FAIL;
    }

    return BT_OK;
}

/* bt spp server deinit*/
btmg_err bt_spps_deinit(void)
{
    xr_err_t ret;

    if ((ret = xr_spp_deinit()) != XR_OK) {
        BTMG_ERROR("spp server deinit failed: %d", ret);
        return BT_FAIL;
    }

    return BT_OK;
}

btmg_err bt_spps_start(int scn)
{
    xr_err_t ret;

    if (scn < SPP_MIN_SCN || scn > SPP_MAX_SCN) {
        BTMG_ERROR("The value range of SCN is 1 to 30");
        return BT_ERR_INVALID_ARG;
    }

    if ((ret = xr_spp_start_srv(sec_mask, role_slave, scn, "btmg_spp_server")) != XR_OK) {
        BTMG_ERROR("spp server start failed: %d", ret);
        return BT_FAIL;
    }

    return BT_OK;
}

btmg_err bt_spps_stop(void)
{
    xr_err_t ret;

    if ((ret = xr_spp_stop_srv()) != XR_OK) {
        BTMG_ERROR("spp server stop failed: %d", ret);
        return BT_FAIL;
    }

    return BT_OK;
}

btmg_err bt_spps_write(char *data, uint32_t len)
{
    xr_err_t ret;

    if (conn_handle == 0) {
        BTMG_ERROR("no device connected, write fail");
        return BT_FAIL;
    }

    if ((ret = xr_spp_write(conn_handle, len, data)) != XR_OK) {
        BTMG_ERROR("spp write data failed: %d", ret);
        return BT_FAIL;
    }

    return BT_OK;
}

btmg_err bt_spps_disconnect(const char *dst)
{
    xr_err_t ret;

    if ((ret = xr_spp_disconnect(conn_handle)) != XR_OK) {
        BTMG_ERROR("spp write data failed: %d", ret);
        return BT_FAIL;
    }

    return BT_OK;
}
