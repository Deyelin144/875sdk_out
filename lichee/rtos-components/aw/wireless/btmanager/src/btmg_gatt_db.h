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

#ifndef _BTMG_GATT_DB_H_
#define _BTMG_GATT_DB_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "ble/bluetooth/gatt.h"

#define GET_PRIV(base)             ((ll_stack_gatt_server_t *)__containerof(base, ll_stack_gatt_server_t, base))
#define GATT_ATTR_GET(base, index) base->get(base, index)
#define GATT_ATTR_DESTROY(base)    base->destroy(base)

typedef enum {
    GATT_ATTR_SERVICE,
    GATT_ATTR_CHARACTERISTIC,
    GATT_ATTR_CCC,
} gatt_attr_type;

typedef struct {
    gatt_attr_type type;
    uint8_t properties;
    struct bt_gatt_attr attr;
} ll_bt_gatt_attr_t;

typedef struct gatt_attr_base {
    int (*add)(struct gatt_attr_base *base, ll_bt_gatt_attr_t *param);
    struct bt_gatt_attr *(*get)(struct gatt_attr_base *base, uint32_t attr_index);
    void (*destroy)(struct gatt_attr_base *base);
} gatt_attr_base;

typedef union {
    struct bt_uuid_128 uuid;  //server
    struct bt_gatt_chrc chrc; //chrc
    struct _bt_gatt_ccc ccc;  //ccc
} gatt_attr_user_data_t;

typedef struct {
    uint32_t attrIndex;
    uint8_t *entry;
    struct bt_gatt_attr *attrs;
    struct bt_uuid_128 *uuids;
    gatt_attr_user_data_t *user_data;
    gatt_attr_base base;
    gatt_attr_base *attrBase;
    struct bt_gatt_service service;
    size_t max_attr_count;
} ll_stack_gatt_server_t;

int ll_gatt_attr_create(ll_stack_gatt_server_t *gatt_attr, uint32_t attr_num);
struct bt_gatt_attr *gatt_server_handle_to_attr(ll_stack_gatt_server_t *gatt_attr, uint16_t handle);

#ifdef __cplusplus
}
#endif

#endif /* _BTMG_GATT_DB_H_ */
