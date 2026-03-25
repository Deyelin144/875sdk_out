// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "btc/btc_manage.h"
#include "btc_hh.h"
#include "xr_bt_main.h"
#include "xr_err.h"
#include "xr_hidh_api.h"
#include <string.h>

#if (defined BTC_HH_INCLUDED && BTC_HH_INCLUDED == TRUE)

xr_err_t xr_bt_hid_host_register_callback(xr_hh_cb_t callback)
{
    if (xr_bluedroid_get_status() != XR_BLUEDROID_STATUS_ENABLED) {
        return XR_ERR_INVALID_STATE;
    }

    if (callback == NULL) {
        return XR_FAIL;
    }

    btc_profile_cb_set(BTC_PID_HH, callback);
    return XR_OK;
}

xr_err_t xr_bt_hid_host_init(void)
{
    if (xr_bluedroid_get_status() != XR_BLUEDROID_STATUS_ENABLED) {
        return XR_ERR_INVALID_STATE;
    }

    btc_msg_t msg;

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_HH;
    msg.act = BTC_HH_INIT_EVT;

    bt_status_t stat = btc_transfer_context(&msg, NULL, 0, NULL);
    return (stat == BT_STATUS_SUCCESS) ? XR_OK : XR_FAIL;
}

xr_err_t xr_bt_hid_host_deinit(void)
{
    if (xr_bluedroid_get_status() != XR_BLUEDROID_STATUS_ENABLED) {
        return XR_ERR_INVALID_STATE;
    }

    btc_msg_t msg;

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_HH;
    msg.act = BTC_HH_DEINIT_EVT;

    bt_status_t stat = btc_transfer_context(&msg, NULL, 0, NULL);
    return (stat == BT_STATUS_SUCCESS) ? XR_OK : XR_FAIL;
}

xr_err_t xr_bt_hid_host_connect(xr_bd_addr_t bd_addr)
{
    if (xr_bluedroid_get_status() != XR_BLUEDROID_STATUS_ENABLED) {
        return XR_ERR_INVALID_STATE;
    }

    btc_msg_t msg;
    btc_hidh_args_t arg;

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_HH;
    msg.act = BTC_HH_CONNECT_EVT;

    memcpy(arg.connect.bd_addr, bd_addr, sizeof(xr_bd_addr_t));

    bt_status_t stat = btc_transfer_context(&msg, &arg, sizeof(btc_hidh_args_t), NULL);
    return (stat == BT_STATUS_SUCCESS) ? XR_OK : XR_FAIL;
}

xr_err_t xr_bt_hid_host_disconnect(xr_bd_addr_t bd_addr)
{
    if (xr_bluedroid_get_status() != XR_BLUEDROID_STATUS_ENABLED) {
        return XR_ERR_INVALID_STATE;
    }

    btc_msg_t msg;
    btc_hidh_args_t arg;

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_HH;
    msg.act = BTC_HH_DISCONNECT_EVT;

    memcpy(arg.disconnect.bd_addr, bd_addr, sizeof(xr_bd_addr_t));

    bt_status_t stat = btc_transfer_context(&msg, &arg, sizeof(btc_hidh_args_t), NULL);
    return (stat == BT_STATUS_SUCCESS) ? XR_OK : XR_FAIL;
}

xr_err_t xr_bt_hid_host_virtual_cable_unplug(xr_bd_addr_t bd_addr)
{
    if (xr_bluedroid_get_status() != XR_BLUEDROID_STATUS_ENABLED) {
        return XR_ERR_INVALID_STATE;
    }

    btc_msg_t msg;
    btc_hidh_args_t arg;

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_HH;
    msg.act = BTC_HH_UNPLUG_EVT;

    memcpy(arg.unplug.bd_addr, bd_addr, sizeof(xr_bd_addr_t));

    bt_status_t stat = btc_transfer_context(&msg, &arg, sizeof(btc_hidh_args_t), NULL);
    return (stat == BT_STATUS_SUCCESS) ? XR_OK : XR_FAIL;
}

xr_err_t xr_bt_hid_host_set_info(xr_bd_addr_t bd_addr, xr_hidh_hid_info_t *hid_info)
{
    if (xr_bluedroid_get_status() != XR_BLUEDROID_STATUS_ENABLED) {
        return XR_ERR_INVALID_STATE;
    }

    btc_msg_t msg;
    btc_hidh_args_t arg;

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_HH;
    msg.act = BTC_HH_SET_INFO_EVT;

    memcpy(arg.set_info.bd_addr, bd_addr, sizeof(xr_bd_addr_t));
    arg.set_info.hid_info = hid_info;

    bt_status_t stat = btc_transfer_context(&msg, &arg, sizeof(btc_hidh_args_t),
                                                btc_hh_arg_deep_copy);
    return (stat == BT_STATUS_SUCCESS) ? XR_OK : XR_FAIL;
}

xr_err_t xr_bt_hid_host_get_protocol(xr_bd_addr_t bd_addr)
{
    if (xr_bluedroid_get_status() != XR_BLUEDROID_STATUS_ENABLED) {
        return XR_ERR_INVALID_STATE;
    }

    btc_msg_t msg;
    btc_hidh_args_t arg;

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_HH;
    msg.act = BTC_HH_GET_PROTO_EVT;

    memcpy(arg.get_protocol.bd_addr, bd_addr, sizeof(xr_bd_addr_t));

    bt_status_t stat = btc_transfer_context(&msg, &arg, sizeof(btc_hidh_args_t), NULL);
    return (stat == BT_STATUS_SUCCESS) ? XR_OK : XR_FAIL;
}

xr_err_t xr_bt_hid_host_set_protocol(xr_bd_addr_t bd_addr, xr_hidh_protocol_mode_t protocol_mode)
{
    if (xr_bluedroid_get_status() != XR_BLUEDROID_STATUS_ENABLED) {
        return XR_ERR_INVALID_STATE;
    }

    btc_msg_t msg;
    btc_hidh_args_t arg;

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_HH;
    msg.act = BTC_HH_SET_PROTO_EVT;

    memcpy(arg.set_protocol.bd_addr, bd_addr, sizeof(xr_bd_addr_t));
    arg.set_protocol.protocol_mode = protocol_mode;

    bt_status_t stat = btc_transfer_context(&msg, &arg, sizeof(btc_hidh_args_t), NULL);
    return (stat == BT_STATUS_SUCCESS) ? XR_OK : XR_FAIL;
}

xr_err_t xr_bt_hid_host_get_idle(xr_bd_addr_t bd_addr)
{
    if (xr_bluedroid_get_status() != XR_BLUEDROID_STATUS_ENABLED) {
        return XR_ERR_INVALID_STATE;
    }

    btc_msg_t msg;
    btc_hidh_args_t arg;

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_HH;
    msg.act = BTC_HH_GET_IDLE_EVT;

    memcpy(arg.get_idle.bd_addr, bd_addr, sizeof(xr_bd_addr_t));

    bt_status_t stat = btc_transfer_context(&msg, &arg, sizeof(btc_hidh_args_t), NULL);
    return (stat == BT_STATUS_SUCCESS) ? XR_OK : XR_FAIL;
}

xr_err_t xr_bt_hid_host_set_idle(xr_bd_addr_t bd_addr, uint16_t idle_time)
{
    if (xr_bluedroid_get_status() != XR_BLUEDROID_STATUS_ENABLED) {
        return XR_ERR_INVALID_STATE;
    }

    btc_msg_t msg;
    btc_hidh_args_t arg;

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_HH;
    msg.act = BTC_HH_SET_IDLE_EVT;

    memcpy(arg.set_idle.bd_addr, bd_addr, sizeof(xr_bd_addr_t));
    arg.set_idle.idle_time = idle_time;

    bt_status_t stat = btc_transfer_context(&msg, &arg, sizeof(btc_hidh_args_t), NULL);
    return (stat == BT_STATUS_SUCCESS) ? XR_OK : XR_FAIL;
}

xr_err_t xr_bt_hid_host_get_report(xr_bd_addr_t bd_addr, xr_hidh_report_type_t report_type, uint8_t report_id,
                                     int buffer_size)
{
    if (xr_bluedroid_get_status() != XR_BLUEDROID_STATUS_ENABLED) {
        return XR_ERR_INVALID_STATE;
    }

    btc_msg_t msg;
    btc_hidh_args_t arg;

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_HH;
    msg.act = BTC_HH_GET_REPORT_EVT;

    memcpy(arg.get_report.bd_addr, bd_addr, sizeof(xr_bd_addr_t));
    arg.get_report.report_type = report_type;
    arg.get_report.report_id = report_id;
    arg.get_report.buffer_size = buffer_size;

    bt_status_t stat = btc_transfer_context(&msg, &arg, sizeof(btc_hidh_args_t), NULL);
    return (stat == BT_STATUS_SUCCESS) ? XR_OK : XR_FAIL;
}

xr_err_t xr_bt_hid_host_set_report(xr_bd_addr_t bd_addr, xr_hidh_report_type_t report_type, uint8_t *report,
                                     size_t len)
{
    if (xr_bluedroid_get_status() != XR_BLUEDROID_STATUS_ENABLED) {
        return XR_ERR_INVALID_STATE;
    }

    btc_msg_t msg;
    btc_hidh_args_t arg;

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_HH;
    msg.act = BTC_HH_SET_REPORT_EVT;

    memcpy(arg.set_report.bd_addr, bd_addr, sizeof(xr_bd_addr_t));
    arg.set_report.report_type = report_type;
    arg.set_report.len = len;
    arg.set_report.report = report;

    bt_status_t stat = btc_transfer_context(&msg, &arg, sizeof(btc_hidh_args_t),
                                                btc_hh_arg_deep_copy);
    return (stat == BT_STATUS_SUCCESS) ? XR_OK : XR_FAIL;
}

xr_err_t xr_bt_hid_host_send_data(xr_bd_addr_t bd_addr, uint8_t *data, size_t len)
{
    if (xr_bluedroid_get_status() != XR_BLUEDROID_STATUS_ENABLED) {
        return XR_ERR_INVALID_STATE;
    }

    btc_msg_t msg;
    btc_hidh_args_t arg;

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_HH;
    msg.act = BTC_HH_SEND_DATA_EVT;

    memcpy(arg.send_data.bd_addr, bd_addr, sizeof(xr_bd_addr_t));
    arg.send_data.len = len;
    arg.send_data.data = data;

    bt_status_t stat = btc_transfer_context(&msg, &arg, sizeof(btc_hidh_args_t),
                                                btc_hh_arg_deep_copy);
    return (stat == BT_STATUS_SUCCESS) ? XR_OK : XR_FAIL;
}

#endif /* defined BTC_HH_INCLUDED && BTC_HH_INCLUDED == TRUE */
