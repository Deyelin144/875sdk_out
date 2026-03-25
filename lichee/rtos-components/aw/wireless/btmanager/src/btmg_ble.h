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

#ifndef _BTMG_BLE_H_
#define _BTMG_BLE_H_

#include "bt_manager.h"
#include "btmg_common.h"
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct bt_gatt_subscribe_params subscribe_params;
typedef struct char_node_t {
    struct char_node_t *front;
    struct char_node_t *next;
    uint16_t char_handle;
    uint8_t conn_id;
    subscribe_params *sub_param;
} char_node_t;

typedef struct char_list_t {
    char_node_t *head;
    char_node_t *tail;
    int length;
    bool list_cleared;
    pthread_mutex_t lock;
} char_list_t;

char_list_t *btmg_char_list_new();
btmg_err btmg_char_list_add_node(char_list_t *char_list, uint8_t conn_id, uint16_t handle, subscribe_params *sub_param);
char_node_t *btmg_char_list_find_node(char_list_t *char_list, uint8_t conn_id, uint16_t handle);
bool btmg_char_list_remove_node(char_list_t *char_list, uint8_t conn_id, uint16_t handle);
void btmg_char_list_clear(char_list_t *list);
void btmg_char_list_free(char_list_t *list);

typedef enum {
    BTMG_LE_EVENT_ERROR = 0,
    //ble connection
    BTMG_LE_EVENT_BLE_SCAN_CB,
    BTMG_LE_EVENT_BLE_CONNECTION,
    BTMG_LE_EVENT_BLE_LE_PARAM_UPDATED,
    BTMG_LE_EVENT_BLE_SECURITY_CHANGED,
    //le smp
    BTMG_LE_EVENT_SMP_AUTH_PASSKEY_DISPLAY,
    BTMG_LE_EVENT_SMP_AUTH_PASSKEY_CONFIRM,
    BTMG_LE_EVENT_SMP_AUTH_PASSKEY_ENTRY,
    BTMG_LE_EVENT_SMP_AUTH_PINCODE_ENTRY,
    BTMG_LE_EVENT_SMP_AUTH_PAIRING_OOB_DATA_REQUEST,
    BTMG_LE_EVENT_SMP_AUTH_CANCEL,
    BTMG_LE_EVENT_SMP_AUTH_PAIRING_CONFIRM,
    BTMG_LE_EVENT_SMP_AUTH_PAIRING_FAILED,
    BTMG_LE_EVENT_SMP_AUTH_PAIRING_COMPLETE,
    BTMG_LE_EVENT_SMP_PAIRING_ACCEPT,
    BTMG_LE_EVENT_SMP_BOND_DELETED,
    //gatt client
    BTMG_LE_EVENT_GATTC_EXCHANGE_MTU_CB,
    BTMG_LE_EVENT_GATTC_WRITE_CB,
    BTMG_LE_EVENT_GATTC_READ_CB,
    BTMG_LE_EVENT_GATTC_DISCOVER,
    BTMG_LE_EVENT_GATTC_NOTIFY,
    //gatt server
    BTMG_LE_EVENT_GATTS_CHAR_WRITE,
    BTMG_LE_EVENT_GATTS_CHAR_READ,
    BTMG_LE_EVENT_GATTS_CCC_CFG_CHANGED,
    BTMG_LE_EVENT_GATTS_GET_DB,
    BTMG_LE_EVENT_GATTS_INCIDATE_CB,
    //OTHER
    BTMG_LE_EVENT_MAX,
} btmg_le_event_t;

int btmg_le_stack_event_callback(btmg_le_event_t type, void *event, size_t len);
btmg_err btmg_ble_init();
#if defined(CONFIG_BT_DEINIT)
btmg_err btmg_ble_deinit();
#endif

#ifdef __cplusplus
}
#endif

#endif /* _BTMG_BLE_H_ */
