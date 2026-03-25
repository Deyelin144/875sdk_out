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

#ifndef __XR_HIDH_API_H__
#define __XR_HIDH_API_H__

#include "xr_err.h"
#include "xr_bt_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/// maximum size of HID Device report descriptor
#define BTHH_MAX_DSC_LEN 884

/**
 * @brief HID host connection state
 */
typedef enum {
    XR_HIDH_CONN_STATE_CONNECTED = 0,            /*!< connected state */
    XR_HIDH_CONN_STATE_CONNECTING,               /*!< connecting state */
    XR_HIDH_CONN_STATE_DISCONNECTED,             /*!< disconnected state */
    XR_HIDH_CONN_STATE_DISCONNECTING,            /*!< disconnecting state */
    XR_HIDH_CONN_STATE_UNKNOWN                   /*!< unknown state (initial state) */
} xr_hidh_connection_state_t;

/**
 * @brief HID handshake error code and vendor-defined result code
 */
typedef enum {
    XR_HIDH_OK,                 /*!< successful */
    XR_HIDH_HS_HID_NOT_READY,   /*!< handshake error: device not ready */
    XR_HIDH_HS_INVALID_RPT_ID,  /*!< handshake error: invalid report ID */
    XR_HIDH_HS_TRANS_NOT_SPT,   /*!< handshake error: HID device does not support the request */
    XR_HIDH_HS_INVALID_PARAM,   /*!< handshake error: parameter value does not meet the expected criteria of called function or API */
    XR_HIDH_HS_ERROR,           /*!< handshake error: HID device could not identify the error condition */
    XR_HIDH_ERR,                /*!< general XR HID Host error */
    XR_HIDH_ERR_SDP,            /*!< SDP error */
    XR_HIDH_ERR_PROTO,          /*!< SET_PROTOCOL error, only used in XR_HIDH_OPEN_EVT callback */
    XR_HIDH_ERR_DB_FULL,        /*!< device database full, used in XR_HIDH_OPEN_EVT/XR_HIDH_ADD_DEV_EVT */
    XR_HIDH_ERR_TOD_UNSPT,      /*!< type of device not supported */
    XR_HIDH_ERR_NO_RES,         /*!< out of system resources */
    XR_HIDH_ERR_AUTH_FAILED,    /*!< authentication fail */
    XR_HIDH_ERR_HDL,            /*!< connection handle error */
    XR_HIDH_ERR_SEC,            /*!< encryption error */
    XR_HIDH_BUSY,               /*!< vendor-defined: temporarily can not handle this request */
    XR_HIDH_NO_DATA,            /*!< vendor-defined: no data. */
    XR_HIDH_NEED_INIT,          /*!< vendor-defined: HIDH module shall initialize first */
    XR_HIDH_NEED_DEINIT,        /*!< vendor-defined: HIDH module shall de-deinitialize first */
    XR_HIDH_NO_CONNECTION,      /*!< vendor-defined: connection may have been closed */
} xr_hidh_status_t;

/**
 * @brief HID host protocol modes
 */
typedef enum {
    XR_HIDH_BOOT_MODE = 0x00,        /*!< boot protocol mode */
    XR_HIDH_REPORT_MODE = 0x01,      /*!< report protocol mode */
    XR_HIDH_UNSUPPORTED_MODE = 0xff  /*!< unsupported protocol mode */
} xr_hidh_protocol_mode_t;

/**
 * @brief HID host report types
 */
typedef enum {
    XR_HIDH_REPORT_TYPE_OTHER = 0,  /*!< unsupported report type */
    XR_HIDH_REPORT_TYPE_INPUT,      /*!< input report type */
    XR_HIDH_REPORT_TYPE_OUTPUT,     /*!< output report type */
    XR_HIDH_REPORT_TYPE_FEATURE,    /*!< feature report type */
} xr_hidh_report_type_t;

/**
 * @brief HID host callback function events
 */
typedef enum {
    XR_HIDH_INIT_EVT = 0,   /*!< when HID host is initialized, the event comes */
    XR_HIDH_DEINIT_EVT,     /*!< when HID host is deinitialized, the event comes */
    XR_HIDH_OPEN_EVT,       /*!< when HID host connection opened, the event comes */
    XR_HIDH_CLOSE_EVT,      /*!< when HID host connection closed, the event comes */
    XR_HIDH_GET_RPT_EVT,    /*!< when Get_Report command is called, the event comes */
    XR_HIDH_SET_RPT_EVT,    /*!< when Set_Report command is called, the event comes */
    XR_HIDH_GET_PROTO_EVT,  /*!< when Get_Protocol command is called, the event comes */
    XR_HIDH_SET_PROTO_EVT,  /*!< when Set_Protocol command is called, the event comes */
    XR_HIDH_GET_IDLE_EVT,   /*!< when Get_Idle command is called, the event comes */
    XR_HIDH_SET_IDLE_EVT,   /*!< when Set_Idle command is called, the event comes */
    XR_HIDH_GET_DSCP_EVT,   /*!< when HIDH is initialized, the event comes */
    XR_HIDH_ADD_DEV_EVT,    /*!< when a device is added, the event comes */
    XR_HIDH_RMV_DEV_EVT,    /*!< when a device is removed, the event comes */
    XR_HIDH_VC_UNPLUG_EVT,  /*!< when virtually unplugged, the event comes */
    XR_HIDH_DATA_EVT,       /*!< when send data on interrupt channel, the event comes */
    XR_HIDH_DATA_IND_EVT,   /*!< when receive data on interrupt channel, the event comes */
    XR_HIDH_SET_INFO_EVT    /*!< when set the HID device descriptor, the event comes */
} xr_hidh_cb_event_t;

/**
 * @brief HID device information from HID Device Service Record and Device ID Service Record
 */
typedef enum {
    XR_HIDH_DEV_ATTR_VIRTUAL_CABLE = 0x0001,            /*!< whether Virtual Cables is supported */
    XR_HIDH_DEV_ATTR_NORMALLY_CONNECTABLE = 0x0002,     /*!< whether device is in Page Scan mode when there is no active connection */
    XR_HIDH_DEV_ATTR_RECONNECT_INITIATE = 0x0004,       /*!< whether the HID device inititates the reconnection process */
} xr_hidh_dev_attr_t;

/**
 * @brief application ID(non-zero) for each type of device
 */
typedef enum {
    XR_HIDH_APP_ID_MOUSE = 1,                 /*!< pointing device */
    XR_HIDH_APP_ID_KEYBOARD = 2,              /*!< keyboard */
    XR_HIDH_APP_ID_REMOTE_CONTROL = 3,        /*!< remote control */
    XR_HIDH_APP_ID_JOYSTICK = 5,              /*!< joystick */
    XR_HIDH_APP_ID_GAMEPAD = 6,               /*!< gamepad*/
} xr_hidh_dev_app_id_t;

/**
 * @brief HID device information from HID Device Service Record and Device ID Service Record
 */
typedef struct {
    int attr_mask;                            /*!< device attribute bit mask, refer to xr_hidh_dev_attr_t */
    uint8_t sub_class;                        /*!< HID device subclass */
    uint8_t app_id;                           /*!< application ID, refer to xr_hidh_dev_app_id_t */
    int vendor_id;                            /*!< Device ID information: vendor ID */
    int product_id;                           /*!< Device ID information: product ID */
    int version;                              /*!< Device ID information: version */
    uint8_t ctry_code;                        /*!< SDP attrbutes of HID devices: HID country code (https://www.usb.org/sites/default/files/hid1_11.pdf) */
    int dl_len;                               /*!< SDP attrbutes of HID devices: HID device descriptor length */
    uint8_t dsc_list[BTHH_MAX_DSC_LEN];       /*!< SDP attrbutes of HID devices: HID device descriptor definition */
} xr_hidh_hid_info_t;

/**
 * @brief HID host callback parameters union
 */
typedef union {
    /**
     * @brief XR_HIDH_INIT_EVT
     */
    struct hidh_init_evt_param {
        xr_hidh_status_t status;  /*!< status */
    } init;                       /*!< HIDH callback param of XR_HIDH_INIT_EVT */

    /**
     * @brief XR_HIDH_DEINIT_EVT
     */
    struct hidh_uninit_evt_param {
        xr_hidh_status_t status;  /*!< status */
    } deinit;                     /*!< HIDH callback param of XR_HIDH_DEINIT_EVT */

    /**
     * @brief XR_HIDH_OPEN_EVT
     */
    struct hidh_open_evt_param {
        xr_hidh_status_t status;                 /*!< operation status         */
        xr_hidh_connection_state_t conn_status;  /*!< connection status        */
        bool is_orig;                            /*!< indicate if host intiate the connection        */
        uint8_t handle;                          /*!< device handle            */
        xr_bd_addr_t bd_addr;                    /*!< device address           */
    } open;                                      /*!< HIDH callback param of XR_HIDH_OPEN_EVT */

    /**
     * @brief XR_HIDH_CLOSE_EVT
     */
    struct hidh_close_evt_param {
        xr_hidh_status_t status;                 /*!< operation status         */
        uint8_t reason;                          /*!< lower layer failed reason(ref hiddefs.h)       */
        xr_hidh_connection_state_t conn_status;  /*!< connection status        */
        uint8_t handle;                          /*!< device handle            */
    } close;                                     /*!< HIDH callback param of XR_HIDH_CLOSE_EVT */

    /**
     * @brief XR_HIDH_VC_UNPLUG_EVT
     */
    struct hidh_unplug_evt_param {
        xr_hidh_status_t status;                 /*!< operation status         */
        xr_hidh_connection_state_t conn_status;  /*!< connection status        */
        uint8_t handle;                          /*!< device handle            */
    } unplug;                                    /*!< HIDH callback param of XR_HIDH_VC_UNPLUG_EVT */

    /**
     * @brief XR_HIDH_GET_PROTO_EVT
     */
    struct hidh_get_proto_evt_param {
        xr_hidh_status_t status;             /*!< operation status         */
        uint8_t handle;                      /*!< device handle            */
        xr_hidh_protocol_mode_t proto_mode;  /*!< protocol mode            */
    } get_proto;                             /*!< HIDH callback param of XR_HIDH_GET_PROTO_EVT */

    /**
     * @brief XR_HIDH_SET_PROTO_EVT
     */
    struct hidh_set_proto_evt_param {
        xr_hidh_status_t status;  /*!< operation status         */
        uint8_t handle;           /*!< device handle            */
    } set_proto;                  /*!< HIDH callback param of XR_HIDH_SET_PROTO_EVT */

    /**
     * @brief XR_HIDH_GET_RPT_EVT
     */
    struct hidh_get_rpt_evt_param {
        xr_hidh_status_t status;  /*!< operation status         */
        uint8_t handle;           /*!< device handle            */
        uint16_t len;             /*!< data length              */
        uint8_t *data;            /*!< data pointer             */
    } get_rpt;                    /*!< HIDH callback param of XR_HIDH_GET_RPT_EVT */

    /**
     * @brief XR_HIDH_SET_RPT_EVT
     */
    struct hidh_set_rpt_evt_param {
        xr_hidh_status_t status;  /*!< operation status         */
        uint8_t handle;           /*!< device handle            */
    } set_rpt;                    /*!< HIDH callback param of XR_HIDH_SET_RPT_EVT */

    /**
     * @brief XR_HIDH_DATA_EVT
     */
    struct hidh_send_data_evt_param {
        xr_hidh_status_t status;  /*!< operation status         */
        uint8_t handle;           /*!< device handle            */
        uint8_t reason;           /*!< lower layer failed reason(ref hiddefs.h)       */
    } send_data;                  /*!< HIDH callback param of XR_HIDH_DATA_EVT */

    /**
     * @brief XR_HIDH_GET_IDLE_EVT
     */
    struct hidh_get_idle_evt_param {
        xr_hidh_status_t status;  /*!< operation status         */
        uint8_t handle;           /*!< device handle            */
        uint8_t idle_rate;        /*!< idle rate                */
    } get_idle;                   /*!< HIDH callback param of XR_HIDH_GET_IDLE_EVT */

    /**
     * @brief XR_HIDH_SET_IDLE_EVT
     */
    struct hidh_set_idle_evt_param {
        xr_hidh_status_t status;  /*!< operation status         */
        uint8_t handle;           /*!< device handle            */
    } set_idle;                   /*!< HIDH callback param of XR_HIDH_SET_IDLE_EVT */

    /**
     * @brief XR_HIDH_DATA_IND_EVT
     */
    struct hidh_data_ind_evt_param {
        xr_hidh_status_t status;             /*!< operation status         */
        uint8_t handle;                      /*!< device handle            */
        xr_hidh_protocol_mode_t proto_mode;  /*!< protocol mode            */
        uint16_t len;                        /*!< data length              */
        uint8_t *data;                       /*!< data pointer             */
    } data_ind;                              /*!< HIDH callback param of XR_HIDH_DATA_IND_EVT */

    /**
     * @brief XR_HIDH_ADD_DEV_EVT
     */
    struct hidh_add_dev_evt_param {
        xr_hidh_status_t status;  /*!< operation status         */
        uint8_t handle;           /*!< device handle            */
        xr_bd_addr_t bd_addr;    /*!< device address           */
    } add_dev;                    /*!< HIDH callback param of XR_HIDH_ADD_DEV_EVT */

    /**
     * @brief XR_HIDH_RMV_DEV_EVT
     */
    struct hidh_rmv_dev_evt_param {
        xr_hidh_status_t status;  /*!< operation status         */
        uint8_t handle;           /*!< device handle            */
        xr_bd_addr_t bd_addr;     /*!< device address           */
    } rmv_dev;                    /*!< HIDH callback param of XR_HIDH_RMV_DEV_EVT */

    /**
     * @brief XR_HIDH_GET_DSCP_EVT
     */
    struct hidh_get_dscp_evt_param {
        xr_hidh_status_t status;  /*!< operation status         */
        uint8_t handle;           /*!< device handle            */
        bool added;               /*!< Indicate if added        */
        uint16_t vendor_id;       /*!< Vendor ID */
        uint16_t product_id;      /*!< Product ID */
        uint16_t version;         /*!< Version */
        uint16_t ssr_max_latency; /*!< SSR max latency in slots */
        uint16_t ssr_min_tout;    /*!< SSR min timeout in slots */
        uint8_t ctry_code;        /*!< Country Code */
        uint16_t dl_len;          /*!< Device descriptor length */
        uint8_t *dsc_list;        /*!< Device descriptor pointer */
    } dscp;                       /*!< HIDH callback param of XR_HIDH_GET_DSCP_EVT */

    /**
     * @brief XR_HIDH_SET_INFO_EVT
     */
    struct hidh_set_info_evt_param {
        xr_hidh_status_t status;  /*!< operation status         */
        uint8_t handle;           /*!< device handle            */
        xr_bd_addr_t bd_addr;     /*!< device address           */
    } set_info;                   /*!< HIDH callback param of XR_HIDH_SET_INFO_EVT */
} xr_hidh_cb_param_t;

/**
 * @brief       HID host callback function type
 * @param       event:      Event type
 * @param       param:      Point to callback parameter, currently is union type
 */
typedef void (*xr_hh_cb_t)(xr_hidh_cb_event_t event, xr_hidh_cb_param_t *param);

/**
 * @brief       This function is called to init callbacks with HID host module.
 *
 * @param[in]   callback:   pointer to the init callback function.
 *
 * @return
 *              - XR_OK: success
 *              - other: failed
 */
xr_err_t xr_bt_hid_host_register_callback(xr_hh_cb_t callback);

/**
 * @brief       This function initializes HID host. This function should be called after xr_bluedroid_enable() success,
 *              and should be called after xr_bt_hid_host_register_callback(). When the operation is complete the callback
 *              function will be called with XR_HIDH_INIT_EVT.
 *
 * @return
 *              - XR_OK: success
 *              - other: failed
 */
xr_err_t xr_bt_hid_host_init(void);

/**
 * @brief       Closes the interface. This function should be called after xr_bluedroid_enable() success,
 *              and should be called after xr_bt_hid_host_init(). When the operation is complete the callback
 *              function will be called with XR_HIDH_DEINIT_EVT.
 *
 * @return      - XR_OK: success
 *              - other: failed
 */
xr_err_t xr_bt_hid_host_deinit(void);

/**
 * @brief       Connect to HID device. When the operation is complete the callback
 *              function will be called with XR_HIDH_OPEN_EVT.
 *
 * @param[in]   bd_addr:  Remote device bluetooth device address.
 *
 * @return      - XR_OK: success
 *              - other: failed
 */
xr_err_t xr_bt_hid_host_connect(xr_bd_addr_t bd_addr);

/**
 * @brief       Disconnect from HID device. When the operation is complete the callback
 *              function will be called with XR_HIDH_CLOSE_EVT.
 *
 * @param[in]   bd_addr:  Remote device bluetooth device address.
 *
 * @return      - XR_OK: success
 *              - other: failed
 */
xr_err_t xr_bt_hid_host_disconnect(xr_bd_addr_t bd_addr);

/**
 * @brief       Virtual UnPlug (VUP) the specified HID device. When the operation is complete the callback
 *              function will be called with XR_HIDH_VC_UNPLUG_EVT.
 *
 * @param[in]   bd_addr:  Remote device bluetooth device address.
 *
 * @return      - XR_OK: success
 *              - other: failed
 */
xr_err_t xr_bt_hid_host_virtual_cable_unplug(xr_bd_addr_t bd_addr);

/**
 * @brief       Set the HID device descriptor for the specified HID device. When the operation is complete the callback
 *              function will be called with XR_HIDH_SET_INFO_EVT.
 *
 * @param[in]   bd_addr:  Remote device bluetooth device address.
 * @param[in]   hid_info:  HID device descriptor structure.
 *
 * @return      - XR_OK: success
 *              - other: failed
 */
xr_err_t xr_bt_hid_host_set_info(xr_bd_addr_t bd_addr, xr_hidh_hid_info_t *hid_info);

/**
 * @brief       Get the HID proto mode. When the operation is complete the callback
 *              function will be called with XR_HIDH_GET_PROTO_EVT.
 *
 * @param[in]   bd_addr:  Remote device bluetooth device address.
 *
 * @return
 *              - XR_OK: success
 *              - other: failed
 */
xr_err_t xr_bt_hid_host_get_protocol(xr_bd_addr_t bd_addr);

/**
 * @brief       Set the HID proto mode. When the operation is complete the callback
 *              function will be called with XR_HIDH_SET_PROTO_EVT.
 *
 * @param[in]   bd_addr:  Remote device bluetooth device address.
 * @param[in]   protocol_mode:  Protocol mode type.
 *
 * @return
 *              - XR_OK: success
 *              - other: failed
 */
xr_err_t xr_bt_hid_host_set_protocol(xr_bd_addr_t bd_addr, xr_hidh_protocol_mode_t protocol_mode);

/**
 * @brief       Get the HID Idle Time. When the operation is complete the callback
 *              function will be called with XR_HIDH_GET_IDLE_EVT.
 *
 * @param[in]   bd_addr:  Remote device bluetooth device address.
 *
 * @return
 *              - XR_OK: success
 *              - other: failed
 */
xr_err_t xr_bt_hid_host_get_idle(xr_bd_addr_t bd_addr);

/**
 * @brief       Set the HID Idle Time. When the operation is complete the callback
 *              function will be called with XR_HIDH_SET_IDLE_EVT.
 *
 * @param[in]   bd_addr:  Remote device bluetooth device address.
 * @param[in]   idle_time:  Idle time rate
 *
 * @return    - XR_OK: success
 *            - other: failed
 */
xr_err_t xr_bt_hid_host_set_idle(xr_bd_addr_t bd_addr, uint16_t idle_time);

/**
 * @brief       Send a GET_REPORT to HID device. When the operation is complete the callback
 *              function will be called with XR_HIDH_GET_RPT_EVT.
 *
 * @param[in]   bd_addr:  Remote device bluetooth device address.
 * @param[in]   report_type:  Report type
 * @param[in]   report_id:  Report id
 * @param[in]   buffer_size:  Buffer size
 *
 * @return      - XR_OK: success
 *              - other: failed
 */
xr_err_t xr_bt_hid_host_get_report(xr_bd_addr_t bd_addr, xr_hidh_report_type_t report_type, uint8_t report_id,
                                     int buffer_size);

/**
 * @brief       Send a SET_REPORT to HID device. When the operation is complete the callback
 *              function will be called with XR_HIDH_SET_RPT_EVT.
 *
 * @param[in]   bd_addr:  Remote device bluetooth device address.
 * @param[in]   report_type:  Report type
 * @param[in]   report:  Report data pointer
 * @param[in]   len:  Report data length
 *
 * @return      - XR_OK: success
 *              - other: failed
 */
xr_err_t xr_bt_hid_host_set_report(xr_bd_addr_t bd_addr, xr_hidh_report_type_t report_type, uint8_t *report,
                                     size_t len);

/**
 * @brief       Send data to HID device. When the operation is complete the callback
 *              function will be called with XR_HIDH_DATA_EVT.
 *
 * @param[in]   bd_addr:  Remote device bluetooth device address.
 * @param[in]   data:  Data pointer
 * @param[in]   len:  Data length
 *
 * @return      - XR_OK: success
 *              - other: failed
 */
xr_err_t xr_bt_hid_host_send_data(xr_bd_addr_t bd_addr, uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* __XR_HIDH_API_H__ */
