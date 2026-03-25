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

#ifndef __XR_HIDD_API_H__
#define __XR_HIDD_API_H__

#include "xr_err.h"
#include "xr_bt_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/// subclass of hid device
#define XR_HID_CLASS_UNKNOWN      (0x00<<2)           /*!< unknown HID device subclass */
#define XR_HID_CLASS_JOS          (0x01<<2)           /*!< joystick */
#define XR_HID_CLASS_GPD          (0x02<<2)           /*!< game pad */
#define XR_HID_CLASS_RMC          (0x03<<2)           /*!< remote control */
#define XR_HID_CLASS_SED          (0x04<<2)           /*!< sensing device */
#define XR_HID_CLASS_DGT          (0x05<<2)           /*!< digitizer tablet */
#define XR_HID_CLASS_CDR          (0x06<<2)           /*!< card reader */
#define XR_HID_CLASS_KBD          (0x10<<2)           /*!< keyboard */
#define XR_HID_CLASS_MIC          (0x20<<2)           /*!< pointing device */
#define XR_HID_CLASS_COM          (0x30<<2)           /*!< combo keyboard/pointing */

/**
 * @brief HIDD handshake result code
 */
typedef enum {
    XR_HID_PAR_HANDSHAKE_RSP_SUCCESS = 0,                 /*!< successful */
    XR_HID_PAR_HANDSHAKE_RSP_NOT_READY = 1,               /*!< not ready, device is too busy to accept data */
    XR_HID_PAR_HANDSHAKE_RSP_ERR_INVALID_REP_ID = 2,      /*!< invalid report ID */
    XR_HID_PAR_HANDSHAKE_RSP_ERR_UNSUPPORTED_REQ = 3,     /*!< device does not support the request */
    XR_HID_PAR_HANDSHAKE_RSP_ERR_INVALID_PARAM = 4,       /*!< parameter value is out of range or inappropriate */
    XR_HID_PAR_HANDSHAKE_RSP_ERR_UNKNOWN = 14,            /*!< device could not identify the error condition */
    XR_HID_PAR_HANDSHAKE_RSP_ERR_FATAL = 15,              /*!< restart is essential to resume functionality */
} xr_hidd_handshake_error_t;

/**
 * @brief HIDD report types
 */
typedef enum {
    XR_HIDD_REPORT_TYPE_OTHER = 0,                   /*!< unknown report type */
    XR_HIDD_REPORT_TYPE_INPUT,                       /*!< input report */
    XR_HIDD_REPORT_TYPE_OUTPUT,                      /*!< output report */
    XR_HIDD_REPORT_TYPE_FEATURE,                     /*!< feature report */
    XR_HIDD_REPORT_TYPE_INTRDATA,                    /*!< special value for reports to be sent on interrupt channel, INPUT is assumed */
} xr_hidd_report_type_t;

/**
 * @brief HIDD connection state
 */
typedef enum {
    XR_HIDD_CONN_STATE_CONNECTED,                    /*!< HID connection established */
    XR_HIDD_CONN_STATE_CONNECTING,                   /*!< connection to remote Bluetooth device */
    XR_HIDD_CONN_STATE_DISCONNECTED,                 /*!< connection released */
    XR_HIDD_CONN_STATE_DISCONNECTING,                /*!< disconnecting to remote Bluetooth device*/
    XR_HIDD_CONN_STATE_UNKNOWN,                      /*!< unknown connection state */
} xr_hidd_connection_state_t;

/**
 * @brief HID device protocol modes
 */
typedef enum {
    XR_HIDD_REPORT_MODE = 0x00,                      /*!< Report Protocol Mode */
    XR_HIDD_BOOT_MODE = 0x01,                        /*!< Boot Protocol Mode */
    XR_HIDD_UNSUPPORTED_MODE = 0xff,                 /*!< unsupported */
} xr_hidd_protocol_mode_t;

/**
 * @brief HID Boot Protocol report IDs
 */
typedef enum {
    XR_HIDD_BOOT_REPORT_ID_KEYBOARD = 1,             /*!< report ID of Boot Protocol keyboard report */
    XR_HIDD_BOOT_REPORT_ID_MOUSE = 2,                /*!< report ID of Boot Protocol mouse report */
} xr_hidd_boot_report_id_t;

/**
 * @brief HID Boot Protocol report size including report ID
 */
enum {
    XR_HIDD_BOOT_REPORT_SIZE_KEYBOARD = 9,           /*!< report size of Boot Protocol keyboard report */
    XR_HIDD_BOOT_REPORT_SIZE_MOUSE = 4,              /*!< report size of Boot Protocol mouse report */
};

/**
 * @brief HID device characteristics for SDP server
 */
typedef struct {
    const char *name;                                /*!< service name */
    const char *description;                         /*!< service description */
    const char *provider;                            /*!< provider name */
    uint8_t subclass;                                /*!< HID device subclass */
    uint8_t *desc_list;                              /*!< HID descriptor list */
    int desc_list_len;                               /*!< size in bytes of HID descriptor list */
} xr_hidd_app_param_t;

/**
 * @brief HIDD Quality of Service parameters negotiated over L2CAP
 */
typedef struct {
    uint8_t service_type;                            /*!< the level of service, 0 indicates no traffic */
    uint32_t token_rate;                             /*!< token rate in bytes per second, 0 indicates "don't care" */
    uint32_t token_bucket_size;                      /*!< limit on the burstness of the application data */
    uint32_t peak_bandwidth;                         /*!< bytes per second, value 0 indicates "don't care" */
    uint32_t access_latency;                         /*!< maximum acceptable delay in microseconds */
    uint32_t delay_variation;                        /*!< the difference in microseconds between the max and min delay */
} xr_hidd_qos_param_t;

/**
 * @brief HID device callback function events
 */
typedef enum {
    XR_HIDD_INIT_EVT = 0,        /*!< When HID device is initialized, the event comes */
    XR_HIDD_DEINIT_EVT,          /*!< When HID device is deinitialized, the event comes */
    XR_HIDD_REGISTER_APP_EVT,    /*!< When HID device application registered, the event comes */
    XR_HIDD_UNREGISTER_APP_EVT,  /*!< When HID device application unregistered, the event comes */
    XR_HIDD_OPEN_EVT,            /*!< When HID device connection to host opened, the event comes */
    XR_HIDD_CLOSE_EVT,           /*!< When HID device connection to host closed, the event comes */
    XR_HIDD_SEND_REPORT_EVT,     /*!< When HID device send report to lower layer, the event comes */
    XR_HIDD_REPORT_ERR_EVT,      /*!< When HID device report handshanke error to lower layer, the event comes */
    XR_HIDD_GET_REPORT_EVT,      /*!< When HID device receives GET_REPORT request from host, the event comes */
    XR_HIDD_SET_REPORT_EVT,      /*!< When HID device receives SET_REPORT request from host, the event comes */
    XR_HIDD_SET_PROTOCOL_EVT,    /*!< When HID device receives SET_PROTOCOL request from host, the event comes */
    XR_HIDD_INTR_DATA_EVT,       /*!< When HID device receives DATA from host on intr, the event comes */
    XR_HIDD_VC_UNPLUG_EVT,       /*!< When HID device initiates Virtual Cable Unplug, the event comes */
    XR_HIDD_API_ERR_EVT          /*!< When HID device has API error, the event comes */
} xr_hidd_cb_event_t;

typedef enum {
    XR_HIDD_SUCCESS,
    XR_HIDD_ERROR,          /*!< general XR HD error */
    XR_HIDD_NO_RES,         /*!< out of system resources */
    XR_HIDD_BUSY,           /*!< Temporarily can not handle this request. */
    XR_HIDD_NO_DATA,        /*!< No data. */
    XR_HIDD_NEED_INIT,      /*!< HIDD module shall init first */
    XR_HIDD_NEED_DEINIT,    /*!< HIDD module shall deinit first */
    XR_HIDD_NEED_REG,       /*!< HIDD module shall register first */
    XR_HIDD_NEED_DEREG,     /*!< HIDD module shall deregister first */
    XR_HIDD_NO_CONNECTION,  /*!< connection may have been closed */
} xr_hidd_status_t;

/**
 * @brief HID device callback parameters union
 */
typedef union {
    /**
     * @brief XR_HIDD_INIT_EVT
     */
    struct hidd_init_evt_param {
        xr_hidd_status_t status;  /*!< operation status */
    } init;                       /*!< HIDD callback param of XR_HIDD_INIT_EVT */

    /**
     * @brief XR_HIDD_DEINIT_EVT
     */
    struct hidd_deinit_evt_param {
        xr_hidd_status_t status;  /*!< operation status */
    } deinit;                     /*!< HIDD callback param of XR_HIDD_DEINIT_EVT */

    /**
     * @brief XR_HIDD_REGISTER_APP_EVT
     */
    struct hidd_register_app_evt_param {
        xr_hidd_status_t status;  /*!< operation status */
        bool in_use;              /*!< indicate whether use virtual cable plug host address */
        xr_bd_addr_t bd_addr;     /*!< host address */
    } register_app;               /*!< HIDD callback param of XR_HIDD_REGISTER_APP_EVT */

    /**
     * @brief XR_HIDD_UNREGISTER_APP_EVT
     */
    struct hidd_unregister_app_evt_param {
        xr_hidd_status_t status;  /*!< operation status         */
    } unregister_app;             /*!< HIDD callback param of XR_HIDD_UNREGISTER_APP_EVT */

    /**
     * @brief XR_HIDD_OPEN_EVT
     */
    struct hidd_open_evt_param {
        xr_hidd_status_t status;                 /*!< operation status         */
        xr_hidd_connection_state_t conn_status;  /*!< connection status */
        xr_bd_addr_t bd_addr;                    /*!< host address */
    } open;                                      /*!< HIDD callback param of XR_HIDD_OPEN_EVT */

    /**
     * @brief XR_HIDD_CLOSE_EVT
     */
    struct hidd_close_evt_param {
        xr_hidd_status_t status;                 /*!< operation status         */
        xr_hidd_connection_state_t conn_status;  /*!< connection status        */
    } close;                                     /*!< HIDD callback param of XR_HIDD_CLOSE_EVT */

    /**
     * @brief XR_HIDD_SEND_REPORT_EVT
     */
    struct hidd_send_report_evt_param {
        xr_hidd_status_t status;            /*!< operation status         */
        uint8_t reason;                     /*!< lower layer failed reason(ref hiddefs.h)       */
        xr_hidd_report_type_t report_type;  /*!< report type        */
        uint8_t report_id;                  /*!< report id         */
    } send_report;                          /*!< HIDD callback param of XR_HIDD_SEND_REPORT_EVT */

    /**
     * @brief XR_HIDD_REPORT_ERR_EVT
     */
    struct hidd_report_err_evt_param {
        xr_hidd_status_t status;  /*!< operation status         */
        uint8_t reason;           /*!< lower layer failed reason(ref hiddefs.h)           */
    } report_err;                 /*!< HIDD callback param of XR_HIDD_REPORT_ERR_EVT */

    /**
     * @brief XR_HIDD_GET_REPORT_EVT
     */
    struct hidd_get_report_evt_param {
        xr_hidd_report_type_t report_type;  /*!< report type        */
        uint8_t report_id;                  /*!< report id         */
        uint16_t buffer_size;               /*!< buffer size         */
    } get_report;                           /*!< HIDD callback param of XR_HIDD_GET_REPORT_EVT */

    /**
     * @brief XR_HIDD_SET_REPORT_EVT
     */
    struct hidd_set_report_evt_param {
        xr_hidd_report_type_t report_type;  /*!< report type        */
        uint8_t report_id;                  /*!< report id         */
        uint16_t len;                       /*!< set_report data length         */
        uint8_t *data;                      /*!< set_report data pointer         */
    } set_report;                           /*!< HIDD callback param of XR_HIDD_SET_REPORT_EVT */

    /**
     * @brief XR_HIDD_SET_PROTOCOL_EVT
     */
    struct hidd_set_protocol_evt_param {
        xr_hidd_protocol_mode_t protocol_mode;  /*!< protocol mode        */
    } set_protocol;                             /*!< HIDD callback param of XR_HIDD_SET_PROTOCOL_EVT */

    /**
     * @brief XR_HIDD_INTR_DATA_EVT
     */
    struct hidd_intr_data_evt_param {
        uint8_t report_id; /*!< interrupt channel report id         */
        uint16_t len;      /*!< interrupt channel report data length         */
        uint8_t *data;     /*!< interrupt channel report data pointer         */
    } intr_data;           /*!< HIDD callback param of XR_HIDD_INTR_DATA_EVT */

    /**
     * @brief XR_HIDD_VC_UNPLUG_EVT
     */
    struct hidd_vc_unplug_param {
        xr_hidd_status_t status;                 /*!< operation status         */
        xr_hidd_connection_state_t conn_status;  /*!< connection status        */
    } vc_unplug;                                 /*!< HIDD callback param of XR_HIDD_VC_UNPLUG_EVT */
} xr_hidd_cb_param_t;

/**
 * @brief           HID device callback function type.
 * @param           event: Event type
 * @param           param: Point to callback parameter, currently is union type
 */
typedef void (*xr_hd_cb_t)(xr_hidd_cb_event_t event, xr_hidd_cb_param_t *param);

/**
 * @brief           This function is called to init callbacks with HID device module.
 *
 * @param[in]       callback: pointer to the init callback function.
 *
 * @return
 *                  - XR_OK: success
 *                  - other: failed
 */
xr_err_t xr_bt_hid_device_register_callback(xr_hd_cb_t callback);

/**
 * @brief           Initializes HIDD interface. This function should be called after
 *                  xr_bluedroid_enable() success, and should be called after xr_bt_hid_device_register_callback.
 *                  When the operation is complete, the callback function will be called with XR_HIDD_INIT_EVT.
 *
 * @return
 *                  - XR_OK: success
 *                  - other: failed
 */
xr_err_t xr_bt_hid_device_init(void);

/**
 * @brief           De-initializes HIDD interface. This function should be called after
 *                  xr_bluedroid_enable() success, and should be called after xr_bt_hid_device_init().
 *                  When the operation is complete, the callback function will be called with XR_HIDD_DEINIT_EVT.
 *
 * @return
 *                  - XR_OK: success
 *                  - other: failed
 */
xr_err_t xr_bt_hid_device_deinit(void);

/**
 * @brief           Registers HIDD parameters with SDP and sets l2cap Quality of Service. This function should be
 *                  called after xr_bluedroid_enable() success, and should be called after xr_bt_hid_device_init().
 *                  When the operation is complete, the callback function will be called with XR_HIDD_REGISTER_APP_EVT.
 *
 * @param[in]       app_param: HIDD parameters
 * @param[in]       in_qos: incoming QoS parameters
 * @param[in]       out_qos: outgoing QoS parameters
 *
 * @return
 *                  - XR_OK: success
 *                  - other: failed
 */
xr_err_t xr_bt_hid_device_register_app(xr_hidd_app_param_t *app_param, xr_hidd_qos_param_t *in_qos,
                                         xr_hidd_qos_param_t *out_qos);

/**
 * @brief           Removes HIDD parameters from SDP and resets l2cap Quality of Service. This function should be
 *                  called after xr_bluedroid_enable() success, and should be called after xr_bt_hid_device_init().
 *                  When the operation is complete, the callback function will be called with XR_HIDD_UNREGISTER_APP_EVT.
 *
 * @return
 *                  - XR_OK: success
 *                  - other: failed
 */
xr_err_t xr_bt_hid_device_unregister_app(void);

/**
 * @brief           Connects to the peer HID Host with virtual cable. This function should be called after
 *                  xr_bluedroid_enable() success, and should be called after xr_bt_hid_device_init().
 *                  When the operation is complete, the callback function will be called with XR_HIDD_OPEN_EVT.
 *
 * @param[in]       bd_addr: Remote host bluetooth device address.
 *
 * @return
 *                  - XR_OK: success
 *                  - other: failed
 */
xr_err_t xr_bt_hid_device_connect(xr_bd_addr_t bd_addr);

/**
 * @brief           Disconnects from the currently connected HID Host. This function should be called after
 *                  xr_bluedroid_enable() success, and should be called after xr_bt_hid_device_init().
 *                  When the operation is complete, the callback function will be called with XR_HIDD_CLOSE_EVT.
 *
 * @note            The disconnect operation will not remove the virtually cabled device. If the connect request from the
 *                  different HID Host, it will reject the request.
 *
 * @return
 *                  - XR_OK: success
 *                  - other: failed
 */
xr_err_t xr_bt_hid_device_disconnect(void);

/**
 * @brief           Sends HID report to the currently connected HID Host. This function should be called after
 *                  xr_bluedroid_enable() success, and should be called after xr_bt_hid_device_init().
 *                  When the operation is complete, the callback function will be called with XR_HIDD_SEND_REPORT_EVT.
 *
 * @param[in]       type: type of report
 * @param[in]       id: report id as defined by descriptor
 * @param[in]       len: length of report
 * @param[in]       data: report data
 *
 * @return
 *                  - XR_OK: success
 *                  - other: failed
 */
xr_err_t xr_bt_hid_device_send_report(xr_hidd_report_type_t type, uint8_t id, uint16_t len, uint8_t *data);

/**
 * @brief           Sends HID Handshake with error info for invalid set_report to the currently connected HID Host.
 *                  This function should be called after xr_bluedroid_enable() success, and should be called after
 *                  xr_bt_hid_device_init(). When the operation is complete, the callback function will be called
 *                  with XR_HIDD_REPORT_ERR_EVT.
 *
 * @param[in]       error: type of error
 *
 * @return
 *                  - XR_OK: success
 *                  - other: failed
 */
xr_err_t xr_bt_hid_device_report_error(xr_hidd_handshake_error_t error);

/**
 * @brief           Remove the virtually cabled device. This function should be called after xr_bluedroid_enable() success,
 *                  and should be called after xr_bt_hid_device_init(). When the operation is complete, the callback function
 *                  will be called with XR_HIDD_VC_UNPLUG_EVT.
 *
 * @note            If the connection exists, then HID Device will send a `VIRTUAL_CABLE_UNPLUG` control command to
 *                  the peer HID Host, and the connection will be destroyed. If the connection does not exist, then HID
 *                  Device will only unplug on it's single side. Once the unplug operation is success, the related
 *                  pairing and bonding information will be removed, then the HID Device can accept connection request
 *                  from the different HID Host,
 *
 * @return          - XR_OK: success
 *                  - other: failed
 */
xr_err_t xr_bt_hid_device_virtual_cable_unplug(void);

#ifdef __cplusplus
}
#endif

#endif /* __XR_HIDD_API_H__ */
