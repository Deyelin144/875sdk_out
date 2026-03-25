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

#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "xr_hidd_api.h"
#include "xr_gap_bt_api.h"
#include "btmg_log.h"
#include "btmg_common.h"
#include "bt_manager.h"
#include "xr_log.h"

/* Values for service_type */
#define NO_TRAFFIC 0
#define BEST_EFFORT 1
#define GUARANTEED 2

#define HIDD_TAG "HIDD_DEMO"

/* HID Report Map Values */
#define HID_RM_INPUT               0x80
#define HID_RM_OUTPUT              0x90
#define HID_RM_FEATURE             0xb0
#define HID_RM_COLLECTION          0xa0
#define HID_RM_END_COLLECTION      0xc0
#define HID_RM_USAGE_PAGE          0x04
#define HID_RM_LOGICAL_MINIMUM     0x14
#define HID_RM_LOGICAL_MAXIMUM     0x24
#define HID_RM_PHYSICAL_MINIMUM    0x34
#define HID_RM_PHYSICAL_MAXIMUM    0x44
#define HID_RM_UNIT_EXPONENT       0x54
#define HID_RM_UNIT                0x64
#define HID_RM_REPORT_SIZE         0x74
#define HID_RM_REPORT_ID           0x84
#define HID_RM_REPORT_COUNT        0x94
#define HID_RM_PUSH                0xa4
#define HID_RM_POP                 0xb4
#define HID_RM_USAGE               0x08
#define HID_RM_USAGE_MINIMUM       0x18
#define HID_RM_USAGE_MAXIMUM       0x28
#define HID_RM_DESIGNATOR_INDEX    0x38
#define HID_RM_DESIGNATOR_MINIMUM  0x48
#define HID_RM_DESIGNATOR_MAXIMUM  0x58
#define HID_RM_STRING_INDEX        0x78
#define HID_RM_STRING_MINIMUM      0x88
#define HID_RM_STRING_MAXIMUM      0x98
#define HID_RM_DELIMITER           0xa8

/* HID Usage Pages and Usages */
#define HID_USAGE_PAGE_GENERIC_DESKTOP 0x01
#define HID_USAGE_KEYBOARD             0x06
#define HID_USAGE_MOUSE                0x02
#define HID_USAGE_JOYSTICK             0x04
#define HID_USAGE_GAMEPAD              0x05

#define HID_USAGE_PAGE_CONSUMER_DEVICE 0x0C
#define HID_USAGE_CONSUMER_CONTROL     0x01

/* HID Appearances */
#define BTMG_HID_APPEARANCE_GENERIC    0x03C0
#define BTMG_HID_APPEARANCE_KEYBOARD   0x03C1
#define BTMG_HID_APPEARANCE_MOUSE      0x03C2
#define BTMG_HID_APPEARANCE_JOYSTICK   0x03C3
#define BTMG_HID_APPEARANCE_GAMEPAD    0x03C4

/* HID Report Types */
#define BTMG_HID_REPORT_TYPE_INPUT   1
#define BTMG_HID_REPORT_TYPE_OUTPUT  2
#define BTMG_HID_REPORT_TYPE_FEATURE 3

/* HID Protocol Modes */
#define BTMG_HID_PROTOCOL_MODE_BOOT   0x00 // Boot Protocol Mode
#define BTMG_HID_PROTOCOL_MODE_REPORT 0x01 // Report Protocol Mode

/* HID Usage Types */
typedef enum {
    BTMG_HID_USAGE_GENERIC =  0,
    BTMG_HID_USAGE_KEYBOARD = 1,
    BTMG_HID_USAGE_MOUSE =    2,
    BTMG_HID_USAGE_JOYSTICK = 4,
    BTMG_HID_USAGE_GAMEPAD =  8,
    BTMG_HID_USAGE_TABLET =   16,
    BTMG_HID_USAGE_CCONTROL = 32,
    BTMG_HID_USAGE_VENDOR =   64
} btmg_hid_usage_t;

typedef struct {
    uint8_t map_index; // the index of the report map
    uint8_t report_id; // the id of the report
    uint8_t report_type; // input, output or feature
    uint8_t protocol_mode; // boot or report
    btmg_hid_usage_t usage; // generic, keyboard, mouse, joystick or gamepad
    uint16_t value_len; // maximum len of value by report map
} hidd_report_item_t;

/**
 * @brief HID report item structure
 */
typedef struct {
    uint8_t map_index; /*!< HID report map index */
    uint8_t report_id; /*!< HID report id */
    uint8_t report_type; /*!< HID report type */
    uint8_t protocol_mode; /*!< HID protocol mode */
    btmg_hid_usage_t usage; /*!< HID usage type */
    uint16_t value_len; /*!< HID report length in bytes */
} hid_report_item_t;

/**
 * @brief HID parsed report map structure
 */
typedef struct {
    btmg_hid_usage_t
            usage; /*!< Dominant HID usage. (keyboard > mouse > joystick > gamepad > generic) */
    uint16_t appearance; /*!< Calculated HID Appearance based on the dominant usage */
    uint8_t reports_len; /*!< Number of reports discovered in the report map */
    hid_report_item_t *reports; /*!< Reports discovered in the report map */
} hid_report_map_t;

/**
 * @brief HID raw report map structure
 */
typedef struct {
    const uint8_t *data; /*!< Pointer to the HID report map data */
    uint16_t len; /*!< HID report map data length */
} hid_raw_report_map_t;

typedef struct {
    hid_raw_report_map_t reports_map;
    uint8_t reports_len;
    hidd_report_item_t *reports;
} hidd_dev_map_t;

/**
 * @brief HID device config structure
 */
typedef struct {
    uint16_t vendor_id; /*!< HID Vendor ID */
    uint16_t product_id; /*!< HID Product ID */
    uint16_t version; /*!< HID Product Version */
    const char *device_name; /*!< HID Device Name */
    const char *manufacturer_name; /*!< HID Manufacturer */
    const char *serial_number; /*!< HID Serial Number */
    hid_raw_report_map_t *report_maps; /*!< Array of the raw HID report maps */
    uint8_t report_maps_len; /*!< number of raw report maps in the array */
} hid_device_config_t;

typedef struct {
    hid_device_config_t config;
    uint16_t appearance;
    bool registered;
    bool connected;
    xr_bd_addr_t remote_bda;
    uint8_t bat_level; // 0 - 100 - battery percentage
    uint8_t control; // 0x00 suspend, 0x01 suspend off
    uint8_t protocol_mode; // 0x00 boot, 0x01 report
    hidd_dev_map_t *devices;
    uint8_t devices_len;
} bt_hidd_dev_t;

typedef struct {
    bt_hidd_dev_t dev;
    xr_hidd_app_param_t app_param;
    xr_hidd_qos_param_t in_qos;
    xr_hidd_qos_param_t out_qos;
} hidd_param_t;

typedef struct {
    uint16_t appearance;
    uint8_t usage_mask;
    uint8_t reports_len;
    hid_report_item_t reports[64];
} temp_hid_report_map_t;

typedef struct {
    uint8_t cmd;
    uint8_t len;
    union {
        uint32_t value;
        uint8_t data[4];
    };
} hid_report_cmd_t;

typedef struct {
    uint16_t usage_page;
    uint16_t usage;
    uint16_t inner_usage_page;
    uint16_t inner_usage;
    uint8_t report_id;
    uint16_t input_len;
    uint16_t output_len;
    uint16_t feature_len;
} hid_report_params_t;

typedef enum {
    PARSE_WAIT_USAGE_PAGE,
    PARSE_WAIT_USAGE,
    PARSE_WAIT_COLLECTION_APPLICATION,
    PARSE_WAIT_END_COLLECTION,
} s_parse_step_t;

const unsigned char btmg_mouseReportMap[] = {
    0x05, 0x01, // USAGE_PAGE (Generic Desktop)
    0x09, 0x02, // USAGE (Mouse)
    0xa1, 0x01, // COLLECTION (Application)

    0x09, 0x01, //   USAGE (Pointer)
    0xa1, 0x00, //   COLLECTION (Physical)

    0x05, 0x09, //     USAGE_PAGE (Button)
    0x19, 0x01, //     USAGE_MINIMUM (Button 1)
    0x29, 0x03, //     USAGE_MAXIMUM (Button 3)
    0x15, 0x00, //     LOGICAL_MINIMUM (0)
    0x25, 0x01, //     LOGICAL_MAXIMUM (1)
    0x95, 0x03, //     REPORT_COUNT (3)
    0x75, 0x01, //     REPORT_SIZE (1)
    0x81, 0x02, //     INPUT (Data,Var,Abs)
    0x95, 0x01, //     REPORT_COUNT (1)
    0x75, 0x05, //     REPORT_SIZE (5)
    0x81, 0x03, //     INPUT (Cnst,Var,Abs)

    0x05, 0x01, //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30, //     USAGE (X)
    0x09, 0x31, //     USAGE (Y)
    0x09, 0x38, //     USAGE (Wheel)
    0x15, 0x81, //     LOGICAL_MINIMUM (-127)
    0x25, 0x7f, //     LOGICAL_MAXIMUM (127)
    0x75, 0x08, //     REPORT_SIZE (8)
    0x95, 0x03, //     REPORT_COUNT (3)
    0x81, 0x06, //     INPUT (Data,Var,Rel)

    0xc0, //   END_COLLECTION
    0xc0 // END_COLLECTION
};

static hid_raw_report_map_t bt_report_maps[] = {
    { .data = btmg_mouseReportMap, .len = sizeof(btmg_mouseReportMap) },
};

static hid_device_config_t bt_hid_config = { .vendor_id = 0x16C0,
                                             .product_id = 0x05DF,
                                             .version = 0x0100,
                                             .device_name = "XR BT HID1",
                                             .manufacturer_name = "Xradio",
                                             .serial_number = "1234567890",
                                             .report_maps = bt_report_maps,
                                             .report_maps_len = 1 };

//static bt_hidd_dev_t hidd;
static hidd_param_t s_hidd_param = { 0 };
static s_parse_step_t s_parse_step = PARSE_WAIT_USAGE_PAGE;
static uint8_t s_collection_depth = 0;
static hid_report_params_t s_report_params = {
    0,
};
static uint16_t s_report_size = 0;
static uint16_t s_report_count = 0;

static bool s_new_map = false;
static temp_hid_report_map_t *s_temp_hid_report_map;
static const char *s_unknown_str = "UNKNOWN";

const char *btmg_hid_usage_str(btmg_hid_usage_t usage)
{
    switch (usage) {
    case BTMG_HID_USAGE_GENERIC:
        return "GENERIC";
    case BTMG_HID_USAGE_KEYBOARD:
        return "KEYBOARD";
    case BTMG_HID_USAGE_MOUSE:
        return "MOUSE";
    case BTMG_HID_USAGE_JOYSTICK:
        return "JOYSTICK";
    case BTMG_HID_USAGE_GAMEPAD:
        return "GAMEPAD";
    case BTMG_HID_USAGE_CCONTROL:
        return "CCONTROL";
    case BTMG_HID_USAGE_VENDOR:
        return "VENDOR";
    default:
        break;
    }
    return s_unknown_str;
}

static int add_report(temp_hid_report_map_t *map, hid_report_item_t *item)
{
    if (map->reports_len >= 64) {
        BTMG_ERROR("reports overflow");
        return -1;
    }
    memcpy(&(map->reports[map->reports_len]), item, sizeof(hid_report_item_t));
    map->reports_len++;
    return 0;
}

static int handle_report(hid_report_params_t *report, bool first)
{
    if (s_temp_hid_report_map == NULL) {
        s_temp_hid_report_map = (temp_hid_report_map_t *)calloc(1, sizeof(temp_hid_report_map_t));
        if (s_temp_hid_report_map == NULL) {
            BTMG_ERROR("malloc failed");
            return -1;
        }
    }
    temp_hid_report_map_t *map = s_temp_hid_report_map;
    if (first) {
        memset(map, 0, sizeof(temp_hid_report_map_t));
    }

    if (report->usage_page == HID_USAGE_PAGE_GENERIC_DESKTOP &&
        report->usage == HID_USAGE_KEYBOARD) {
        //Keyboard
        map->usage_mask |= BTMG_HID_USAGE_KEYBOARD;
        if (report->input_len > 0) {
            hid_report_item_t item = {
                .usage = BTMG_HID_USAGE_KEYBOARD,
                .report_id = report->report_id,
                .report_type = BTMG_HID_REPORT_TYPE_INPUT,
                .protocol_mode = BTMG_HID_PROTOCOL_MODE_REPORT,
                .value_len = report->input_len / 8,
            };
            if (add_report(map, &item) != 0) {
                return -1;
            }

            item.protocol_mode = BTMG_HID_PROTOCOL_MODE_BOOT;
            item.value_len = 8;
            if (add_report(map, &item) != 0) {
                return -1;
            }
        }
        if (report->output_len > 0) {
            hid_report_item_t item = {
                .usage = BTMG_HID_USAGE_KEYBOARD,
                .report_id = report->report_id,
                .report_type = BTMG_HID_REPORT_TYPE_OUTPUT,
                .protocol_mode = BTMG_HID_PROTOCOL_MODE_REPORT,
                .value_len = report->output_len / 8,
            };
            if (add_report(map, &item) != 0) {
                return -1;
            }

            item.protocol_mode = BTMG_HID_PROTOCOL_MODE_BOOT;
            item.value_len = 1;
            if (add_report(map, &item) != 0) {
                return -1;
            }
        }
    } else if (report->usage_page == HID_USAGE_PAGE_GENERIC_DESKTOP &&
               report->usage == HID_USAGE_MOUSE) {
        //Mouse
        map->usage_mask |= BTMG_HID_USAGE_MOUSE;
        if (report->input_len > 0) {
            hid_report_item_t item = {
                .usage = BTMG_HID_USAGE_MOUSE,
                .report_id = report->report_id,
                .report_type = BTMG_HID_REPORT_TYPE_INPUT,
                .protocol_mode = BTMG_HID_PROTOCOL_MODE_REPORT,
                .value_len = report->input_len / 8,
            };
            if (add_report(map, &item) != 0) {
                return -1;
            }

            item.protocol_mode = BTMG_HID_PROTOCOL_MODE_BOOT;
            item.value_len = 3;
            if (add_report(map, &item) != 0) {
                return -1;
            }
        }
    } else {
        btmg_hid_usage_t cusage = BTMG_HID_USAGE_GENERIC;
        if (report->usage_page == HID_USAGE_PAGE_GENERIC_DESKTOP) {
            if (report->usage == HID_USAGE_JOYSTICK) {
                //Joystick
                map->usage_mask |= BTMG_HID_USAGE_JOYSTICK;
                cusage = BTMG_HID_USAGE_JOYSTICK;
            } else if (report->usage == HID_USAGE_GAMEPAD) {
                //Gamepad
                map->usage_mask |= BTMG_HID_USAGE_GAMEPAD;
                cusage = BTMG_HID_USAGE_GAMEPAD;
            }
        } else if (report->usage_page == HID_USAGE_PAGE_CONSUMER_DEVICE &&
                   report->usage == HID_USAGE_CONSUMER_CONTROL) {
            //Consumer Control
            map->usage_mask |= BTMG_HID_USAGE_CCONTROL;
            cusage = BTMG_HID_USAGE_CCONTROL;
        } else if (report->usage_page >= 0xFF) {
            //Vendor
            map->usage_mask |= BTMG_HID_USAGE_VENDOR;
            cusage = BTMG_HID_USAGE_VENDOR;
        }
        //Generic
        hid_report_item_t item = {
            .usage = cusage,
            .report_id = report->report_id,
            .report_type = BTMG_HID_REPORT_TYPE_INPUT,
            .protocol_mode = BTMG_HID_PROTOCOL_MODE_REPORT,
            .value_len = report->input_len / 8,
        };
        if (report->input_len > 0) {
            if (add_report(map, &item) != 0) {
                return -1;
            }
        }
        if (report->output_len > 0) {
            item.report_type = BTMG_HID_REPORT_TYPE_OUTPUT;
            item.value_len = report->output_len / 8;
            if (add_report(map, &item) != 0) {
                return -1;
            }
        }
        if (report->feature_len > 0) {
            item.report_type = BTMG_HID_REPORT_TYPE_FEATURE;
            item.value_len = report->feature_len / 8;
            if (add_report(map, &item) != 0) {
                return -1;
            }
        }
    }
    return 0;
}

static int parse_cmd(const uint8_t *data, size_t len, size_t index, hid_report_cmd_t **out)
{
    if (index == len) {
        return 0;
    }
    hid_report_cmd_t *cmd = (hid_report_cmd_t *)malloc(sizeof(hid_report_cmd_t));
    if (cmd == NULL) {
        return -1;
    }
    const uint8_t *dp = data + index;
    cmd->cmd = *dp & 0xFC;
    cmd->len = *dp & 0x03;
    cmd->value = 0;
    if (cmd->len == 3) {
        cmd->len = 4;
    }
    if ((len - index - 1) < cmd->len) {
        BTMG_ERROR("not enough bytes! cmd: 0x%02x, len: %u, index: %u", cmd->cmd, cmd->len, index);
        free(cmd);
        return -1;
    }
    memcpy(cmd->data, dp + 1, cmd->len);
    *out = cmd;
    return cmd->len + 1;
}

static int handle_cmd(hid_report_cmd_t *cmd)
{
    switch (s_parse_step) {
    case PARSE_WAIT_USAGE_PAGE: {
        if (cmd->cmd != HID_RM_USAGE_PAGE) {
            BTMG_ERROR("expected USAGE_PAGE, but got 0x%02x", cmd->cmd);
            return -1;
        }
        s_report_size = 0;
        s_report_count = 0;
        memset(&s_report_params, 0, sizeof(hid_report_params_t));
        s_report_params.usage_page = cmd->value;
        s_parse_step = PARSE_WAIT_USAGE;
        break;
    }
    case PARSE_WAIT_USAGE: {
        if (cmd->cmd != HID_RM_USAGE) {
            BTMG_ERROR("expected USAGE, but got 0x%02x", cmd->cmd);
            s_parse_step = PARSE_WAIT_USAGE_PAGE;
            return -1;
        }
        s_report_params.usage = cmd->value;
        s_parse_step = PARSE_WAIT_COLLECTION_APPLICATION;
        break;
    }
    case PARSE_WAIT_COLLECTION_APPLICATION: {
        if (cmd->cmd != HID_RM_COLLECTION) {
            BTMG_ERROR("expected COLLECTION, but got 0x%02x", cmd->cmd);
            s_parse_step = PARSE_WAIT_USAGE_PAGE;
            return -1;
        }
        if (cmd->value != 1) {
            BTMG_ERROR("expected APPLICATION, but got 0x%02x", cmd->value);
            s_parse_step = PARSE_WAIT_USAGE_PAGE;
            return -1;
        }
        s_report_params.report_id = 0;
        s_collection_depth = 1;
        s_parse_step = PARSE_WAIT_END_COLLECTION;
        break;
    }
    case PARSE_WAIT_END_COLLECTION: {
        if (cmd->cmd == HID_RM_REPORT_ID) {
            if (s_report_params.report_id && s_report_params.report_id != cmd->value) {
                //report id changed mid collection
                if (s_report_params.input_len & 0x7) {
                    BTMG_ERROR("ERROR: INPUT report does not amount to full bytes! %d (%d)",
                               s_report_params.input_len, s_report_params.input_len & 0x7);
                } else if (s_report_params.output_len & 0x7) {
                    BTMG_ERROR("ERROR: OUTPUT report does not amount to full bytes! %d (%d)",
                               s_report_params.output_len, s_report_params.output_len & 0x7);
                } else if (s_report_params.feature_len & 0x7) {
                    BTMG_ERROR("ERROR: FEATURE report does not amount to full bytes! %d (%d)",
                               s_report_params.feature_len, s_report_params.feature_len & 0x7);
                } else {
                    //SUCCESS!!!
                    int res = handle_report(&s_report_params, s_new_map);
                    if (res != 0) {
                        s_parse_step = PARSE_WAIT_USAGE_PAGE;
                        return -1;
                    }
                    s_new_map = false;

                    s_report_params.input_len = 0;
                    s_report_params.output_len = 0;
                    s_report_params.feature_len = 0;
                    s_report_params.usage = s_report_params.inner_usage;
                    s_report_params.usage_page = s_report_params.inner_usage_page;
                }
            }
            s_report_params.report_id = cmd->value;
        } else if (cmd->cmd == HID_RM_USAGE_PAGE) {
            s_report_params.inner_usage_page = cmd->value;
        } else if (cmd->cmd == HID_RM_USAGE) {
            s_report_params.inner_usage = cmd->value;
        } else if (cmd->cmd == HID_RM_REPORT_SIZE) {
            s_report_size = cmd->value;
        } else if (cmd->cmd == HID_RM_REPORT_COUNT) {
            s_report_count = cmd->value;
        } else if (cmd->cmd == HID_RM_INPUT) {
            s_report_params.input_len += (s_report_size * s_report_count);
        } else if (cmd->cmd == HID_RM_OUTPUT) {
            s_report_params.output_len += (s_report_size * s_report_count);
        } else if (cmd->cmd == HID_RM_FEATURE) {
            s_report_params.feature_len += (s_report_size * s_report_count);
        } else if (cmd->cmd == HID_RM_COLLECTION) {
            s_collection_depth += 1;
        } else if (cmd->cmd == HID_RM_END_COLLECTION) {
            s_collection_depth -= 1;
            if (s_collection_depth == 0) {
                if (s_report_params.input_len & 0x7) {
                    BTMG_ERROR("ERROR: INPUT report does not amount to full bytes! %d (%d)",
                               s_report_params.input_len, s_report_params.input_len & 0x7);
                } else if (s_report_params.output_len & 0x7) {
                    BTMG_ERROR("ERROR: OUTPUT report does not amount to full bytes! %d (%d)",
                               s_report_params.output_len, s_report_params.output_len & 0x7);
                } else if (s_report_params.feature_len & 0x7) {
                    BTMG_ERROR("ERROR: FEATURE report does not amount to full bytes! %d (%d)",
                               s_report_params.feature_len, s_report_params.feature_len & 0x7);
                } else {
                    //SUCCESS!!!
                    int res = handle_report(&s_report_params, s_new_map);
                    if (res != 0) {
                        s_parse_step = PARSE_WAIT_USAGE_PAGE;
                        return -1;
                    }
                    s_new_map = false;
                }
                s_parse_step = PARSE_WAIT_USAGE_PAGE;
            }
        }

        break;
    }
    default:
        s_parse_step = PARSE_WAIT_USAGE_PAGE;
        break;
    }
    return 0;
}

hid_report_map_t *btmg_hid_parse_report_map(const uint8_t *hid_rm, size_t hid_rm_len)
{
    size_t index = 0;
    int res;
    s_new_map = true;

    while (index < hid_rm_len) {
        hid_report_cmd_t *cmd;
        res = parse_cmd(hid_rm, hid_rm_len, index, &cmd);
        if (res < 0) {
            BTMG_ERROR("Failed parsing the descriptor at index: %u", index);
            return NULL;
        }
        index += res;
        res = handle_cmd(cmd);
        free(cmd);
        if (res != 0) {
            return NULL;
        }
    }

    hid_report_map_t *out = (hid_report_map_t *)calloc(1, sizeof(hid_report_map_t));
    if (out == NULL) {
        BTMG_ERROR("hid_report_map malloc failed");
        free(s_temp_hid_report_map);
        s_temp_hid_report_map = NULL;
        return NULL;
    }

    temp_hid_report_map_t *map = s_temp_hid_report_map;

    hid_report_item_t *reports =
            (hid_report_item_t *)calloc(1, map->reports_len * sizeof(hid_report_item_t));
    if (reports == NULL) {
        BTMG_ERROR("hid_report_items malloc failed! %u maps", map->reports_len);
        free(out);
        free(s_temp_hid_report_map);
        s_temp_hid_report_map = NULL;
        return NULL;
    }

    if (map->usage_mask & BTMG_HID_USAGE_KEYBOARD) {
        out->usage = BTMG_HID_USAGE_KEYBOARD;
        out->appearance = BTMG_HID_APPEARANCE_KEYBOARD;
    } else if (map->usage_mask & BTMG_HID_USAGE_MOUSE) {
        out->usage = BTMG_HID_USAGE_MOUSE;
        out->appearance = BTMG_HID_APPEARANCE_MOUSE;
    } else if (map->usage_mask & BTMG_HID_USAGE_JOYSTICK) {
        out->usage = BTMG_HID_USAGE_JOYSTICK;
        out->appearance = BTMG_HID_APPEARANCE_JOYSTICK;
    } else if (map->usage_mask & BTMG_HID_USAGE_GAMEPAD) {
        out->usage = BTMG_HID_USAGE_GAMEPAD;
        out->appearance = BTMG_HID_APPEARANCE_GAMEPAD;
    } else if (map->usage_mask & BTMG_HID_USAGE_CCONTROL) {
        out->usage = BTMG_HID_USAGE_CCONTROL;
        out->appearance = BTMG_HID_APPEARANCE_KEYBOARD;
    } else {
        out->usage = BTMG_HID_USAGE_GENERIC;
        out->appearance = BTMG_HID_APPEARANCE_GENERIC;
    }
    out->reports_len = map->reports_len;
    memcpy(reports, map->reports, map->reports_len * sizeof(hid_report_item_t));
    out->reports = reports;
    free(s_temp_hid_report_map);
    s_temp_hid_report_map = NULL;

    return out;
}

static int bt_hidd_init_config(bt_hidd_dev_t *dev, const hid_device_config_t *config)
{
    if (config->report_maps == NULL || config->report_maps_len == 0 ||
        config->report_maps_len > 1) {
        return -1;
    }
    memset((uint8_t *)(&dev->config), 0, sizeof(hid_device_config_t));
    dev->config.vendor_id = config->vendor_id;
    dev->config.product_id = config->product_id;
    dev->config.version = config->version;
    if (config->device_name != NULL) {
        dev->config.device_name = strdup(config->device_name);
    }
    if (config->manufacturer_name != NULL) {
        dev->config.manufacturer_name = strdup(config->manufacturer_name);
    }
    if (config->serial_number != NULL) {
        dev->config.serial_number = strdup(config->serial_number);
    }
    dev->appearance = BTMG_HID_APPEARANCE_GENERIC;

    if (config->report_maps_len) {
        dev->devices = (hidd_dev_map_t *)malloc(config->report_maps_len * sizeof(hidd_dev_map_t));
        if (dev->devices == NULL) {
            BTMG_ERROR("devices malloc(%d) failed", config->report_maps_len);
            return -1;
        }
        memset(dev->devices, 0, config->report_maps_len * sizeof(hidd_dev_map_t));
        dev->devices_len = config->report_maps_len;
        for (uint8_t d = 0; d < dev->devices_len; d++) {
            //raw report map
            uint8_t *map = (uint8_t *)malloc(config->report_maps[d].len);
            if (map == NULL) {
                BTMG_ERROR("report map malloc(%d) failed", config->report_maps[d].len);
                return -1;
            }
            memcpy(map, config->report_maps[d].data, config->report_maps[d].len);

            dev->devices[d].reports_map.data = (const uint8_t *)map;
            dev->devices[d].reports_map.len = config->report_maps[d].len;

            hid_report_map_t *rmap = btmg_hid_parse_report_map(config->report_maps[d].data,
                                                               config->report_maps[d].len);
            if (rmap == NULL) {
                BTMG_ERROR("hid_parse_report_map[%d](%d) failed", d, config->report_maps[d].len);
                return -1;
            }
            dev->appearance = rmap->appearance;
            dev->devices[d].reports_len = rmap->reports_len;
            dev->devices[d].reports =
                    (hidd_report_item_t *)malloc(rmap->reports_len * sizeof(hidd_report_item_t));
            if (dev->devices[d].reports == NULL) {
                BTMG_ERROR("reports malloc(%d) failed",
                           rmap->reports_len * sizeof(hidd_report_item_t));
                free(rmap);
                return -1;
            }
            for (uint8_t r = 0; r < rmap->reports_len; r++) {
                dev->devices[d].reports[r].map_index = d;
                dev->devices[d].reports[r].report_id = rmap->reports[r].report_id;
                dev->devices[d].reports[r].protocol_mode = rmap->reports[r].protocol_mode;
                dev->devices[d].reports[r].report_type = rmap->reports[r].report_type;
                dev->devices[d].reports[r].usage = rmap->reports[r].usage;
                dev->devices[d].reports[r].value_len = rmap->reports[r].value_len;
            }
            free(rmap->reports);
            free(rmap);
        }
    }

    return 0;
}

static void bt_hid_free_config(bt_hidd_dev_t *dev)
{
    for (uint8_t d = 0; d < dev->devices_len; d++) {
        free((void *)dev->devices[d].reports);
        free((void *)dev->devices[d].reports_map.data);
        dev->devices[d].reports = NULL;
        dev->devices[d].reports_map.data = NULL;
    }

    if (dev->devices) {
        free((void *)dev->devices);
        dev->devices = NULL;
    }

    if (dev->config.device_name) {
        free((void *)dev->config.device_name);
        dev->config.device_name = NULL;
    }

    if (dev->config.manufacturer_name) {
        free((void *)dev->config.manufacturer_name);
        dev->config.manufacturer_name = NULL;
    }

    if (dev->config.serial_number) {
        free((void *)dev->config.serial_number);
        dev->config.serial_number = NULL;
    }
}

static hidd_report_item_t *get_report_by_id_and_type(bt_hidd_dev_t *dev, uint8_t id, uint8_t type,
                                                     uint8_t *index)
{
    hidd_report_item_t *rpt = NULL;
    for (uint8_t idx = 0; idx < dev->devices_len; idx++) {
        for (uint8_t i = 0; i < dev->devices[idx].reports_len; i++) {
            rpt = &dev->devices[idx].reports[i];
            if (rpt->report_id == id && rpt->report_type == type &&
                rpt->protocol_mode == dev->protocol_mode) {
                if (index) {
                    *index = idx;
                }
                return rpt;
            }
        }
    }
    return NULL;
}

static void bt_hidd_cb(xr_hidd_cb_event_t event, xr_hidd_cb_param_t *param)
{
    uint8_t map_index = 0;
    hidd_report_item_t *p_rpt = NULL;

    switch (event) {
    case XR_HIDD_INIT_EVT: {
        if (param->init.status == XR_HIDD_SUCCESS) {
            BTMG_DEBUG("Setting hid parameters in_qos:%d, out_qos:%d\n",
                       s_hidd_param.in_qos.token_bucket_size,
                       s_hidd_param.out_qos.token_bucket_size);
            xr_bt_hid_device_register_app(&s_hidd_param.app_param, &s_hidd_param.in_qos,
                                          &s_hidd_param.out_qos);
        } else {
            BTMG_ERROR("Init hidd failed (%d)!\n", param->init.status);
            bt_hid_free_config(&s_hidd_param.dev);
        }
        break;
    }
    case XR_HIDD_DEINIT_EVT: {
        if (param->deinit.status == XR_HIDD_SUCCESS) {
            bt_hid_free_config(&s_hidd_param.dev);
        } else {
            BTMG_ERROR("Deinit hidd failed (%d)!\n", param->deinit.status);
        }
        break;
    }
    case XR_HIDD_REGISTER_APP_EVT: {
        if (param->register_app.status == XR_HIDD_SUCCESS) {
            BTMG_DEBUG("Setting hid parameters success!\n");
            if (param->register_app.in_use) {
                BTMG_WARNG("Start virtual cable plug!\n");
                xr_bt_hid_device_connect(param->register_app.bd_addr);
            }

            s_hidd_param.dev.registered = true;
        } else {
            BTMG_ERROR("Setting hid parameters failed (%d), now deint!\n",
                       param->register_app.status);
            xr_bt_hid_device_deinit();
        }
        break;
    }
    case XR_HIDD_UNREGISTER_APP_EVT: {
        break;
    }
    case XR_HIDD_OPEN_EVT: {
        if (param->open.conn_status == XR_HIDD_CONN_STATE_CONNECTING) {
            break;
        }
        if (param->open.status == XR_HIDD_SUCCESS &&
            param->open.conn_status == XR_HIDD_CONN_STATE_CONNECTED) {
            BTMG_DEBUG("Connected to %02x:%02x:%02x:%02x:%02x:%02x\n", param->open.bd_addr[0],
                       param->open.bd_addr[1], param->open.bd_addr[2], param->open.bd_addr[3],
                       param->open.bd_addr[4], param->open.bd_addr[5]);
            s_hidd_param.dev.connected = true;
            memcpy(s_hidd_param.dev.remote_bda, param->open.bd_addr, XR_BD_ADDR_LEN);
        } else {
            BTMG_ERROR("Connect failed (%d)!\n", param->open.status);
        }
        break;
    }
    case XR_HIDD_CLOSE_EVT: {
        if (param->close.conn_status == XR_HIDD_CONN_STATE_DISCONNECTING) {
            break;
        }
        if (param->close.status == XR_HIDD_SUCCESS &&
            param->close.conn_status == XR_HIDD_CONN_STATE_DISCONNECTED) {
            BTMG_DEBUG("HIDD disconnect!\n");
            s_hidd_param.dev.connected = false;
            memset(s_hidd_param.dev.remote_bda, 0, XR_BD_ADDR_LEN);
        } else {
            BTMG_ERROR("Disconnect failed (%d)!\n", param->close.status);
        }
        break;
    }
    case XR_HIDD_SEND_REPORT_EVT:
        break;
    case XR_HIDD_REPORT_ERR_EVT:
        break;
    case XR_HIDD_GET_REPORT_EVT: {
        uint8_t *data_ptr = NULL;
        p_rpt = get_report_by_id_and_type(&s_hidd_param.dev, param->get_report.report_id,
                                          param->get_report.report_type, &map_index);
        if (p_rpt == NULL) {
            BTMG_ERROR("Can not find report!\n");
            xr_bt_hid_device_report_error(XR_HID_PAR_HANDSHAKE_RSP_ERR_INVALID_REP_ID);
            break;
        }
        if (param->get_report.buffer_size > p_rpt->value_len) {
            BTMG_ERROR("Data size over %d!\n", p_rpt->value_len);
            xr_bt_hid_device_report_error(XR_HID_PAR_HANDSHAKE_RSP_ERR_INVALID_PARAM);
            break;
        }

        if (param->get_report.buffer_size) {
            *data_ptr++ = (uint8_t)param->get_report.buffer_size;
            *data_ptr++ = (uint8_t)(param->get_report.buffer_size >> 8);
        }

        BTMG_DEBUG("FEATURE[%u]: %8s ID: %2u, Len: %d\n", map_index,
                   btmg_hid_usage_str(p_rpt->usage), p_rpt->report_id,
                   param->get_report.buffer_size ? 2 : 0);
        xr_log_buffer_hex(HIDD_TAG, data_ptr, param->get_report.buffer_size ? 2 : 0);
        break;
    }
    case XR_HIDD_SET_REPORT_EVT: {
        p_rpt = get_report_by_id_and_type(&s_hidd_param.dev, param->set_report.report_id,
                                          param->set_report.report_type, &map_index);
        if (p_rpt == NULL) {
            BTMG_ERROR("Can not find report!\n");
            xr_bt_hid_device_report_error(XR_HID_PAR_HANDSHAKE_RSP_ERR_INVALID_REP_ID);
            break;
        }
        if (param->set_report.len > p_rpt->value_len) {
            BTMG_ERROR("Data size over %d!\n", p_rpt->value_len);
            xr_bt_hid_device_report_error(XR_HID_PAR_HANDSHAKE_RSP_ERR_INVALID_PARAM);
            break;
        }

        BTMG_DEBUG("FEATURE[%u]: %8s ID: %2u, Len: %d\n", map_index,
                   btmg_hid_usage_str(p_rpt->usage), p_rpt->report_id, param->set_report.len);
        xr_log_buffer_hex(HIDD_TAG, param->set_report.data, param->set_report.len);
        break;
    }
    case XR_HIDD_SET_PROTOCOL_EVT: {
        if (param->set_protocol.protocol_mode != XR_HIDD_UNSUPPORTED_MODE) {
            if (s_hidd_param.dev.protocol_mode == param->set_protocol.protocol_mode) {
                break;
            }
            s_hidd_param.dev.protocol_mode = param->set_protocol.protocol_mode;
            BTMG_DEBUG("PROTOCOL : %s\n", s_hidd_param.dev.protocol_mode ? "REPORT" : "BOOT");
        } else {
            BTMG_ERROR("Unsupported protocol mode!\n");
            break;
        }
        break;
    }
    case XR_HIDD_INTR_DATA_EVT: {
        p_rpt = get_report_by_id_and_type(&s_hidd_param.dev, param->intr_data.report_id,
                                          BTMG_HID_REPORT_TYPE_OUTPUT, &map_index);
        if (p_rpt == NULL) {
            BTMG_ERROR("Can not find report!\n");
            break;
        }

        BTMG_DEBUG("OUTPUT[%u]: %8s ID: %2u, Len: %d, Data:", map_index,
                   btmg_hid_usage_str(p_rpt->usage), p_rpt->report_id, param->intr_data.len);
        xr_log_buffer_hex(HIDD_TAG, param->intr_data.data, param->intr_data.len);
        break;
    }
    default:
        break;
    }
}

static uint8_t get_subclass_by_appearance(uint16_t appearance)
{
    uint8_t ret = XR_HID_CLASS_UNKNOWN;
    switch (appearance) {
    case BTMG_HID_APPEARANCE_KEYBOARD:
        ret = XR_HID_CLASS_KBD;
        break;
    case BTMG_HID_APPEARANCE_MOUSE:
        ret = XR_HID_CLASS_MIC;
        break;
    case BTMG_HID_APPEARANCE_JOYSTICK:
        ret = XR_HID_CLASS_JOS;
        break;
    case BTMG_HID_APPEARANCE_GAMEPAD:
        ret = XR_HID_CLASS_GPD;
        break;
    default:
        ret = XR_HID_CLASS_UNKNOWN;
        break;
    }
    return ret;
}

static void build_default_in_qos(xr_hidd_qos_param_t *in_qos, uint32_t value_len)
{
    if (value_len > 0) {
        in_qos->service_type = GUARANTEED;
        in_qos->token_rate = value_len * 100;
        in_qos->token_bucket_size = value_len;
        in_qos->peak_bandwidth = value_len * 100;
        in_qos->access_latency = 10;
        in_qos->delay_variation = 10;
    } else {
        memset(in_qos, 0, sizeof(xr_hidd_qos_param_t));
    }
}

static void build_default_out_qos(xr_hidd_qos_param_t *out_qos, uint32_t value_len)
{
    if (value_len > 0) {
        out_qos->service_type = GUARANTEED;
        out_qos->token_rate = value_len * 100;
        out_qos->token_bucket_size = value_len;
        out_qos->peak_bandwidth = value_len * 100;
        out_qos->access_latency = 10;
        out_qos->delay_variation = 10;
    } else {
        memset(out_qos, 0, sizeof(xr_hidd_qos_param_t));
    }
}

static uint32_t get_value_len_by_type_protocol(bt_hidd_dev_t *dev, uint8_t report_type,
                                               uint8_t protocol_mode)
{
    uint32_t value_len = 0;
    hidd_report_item_t *rpt = NULL;
    for (uint8_t d = 0; d < dev->devices_len; d++) {
        for (uint8_t i = 0; i < dev->devices[d].reports_len; i++) {
            rpt = &dev->devices[d].reports[i];
            if (rpt->report_type == report_type && rpt->protocol_mode == dev->protocol_mode) {
                value_len += rpt->value_len;
            }
        }
    }
    return value_len;
}

static void bt_hidd_init_app(void)
{
    hid_device_config_t *p_config = &s_hidd_param.dev.config;
    s_hidd_param.app_param.name = p_config->device_name;
    s_hidd_param.app_param.description = p_config->device_name;
    s_hidd_param.app_param.provider = p_config->manufacturer_name;
    s_hidd_param.app_param.subclass = get_subclass_by_appearance(s_hidd_param.dev.appearance);
    s_hidd_param.app_param.desc_list = (uint8_t *)s_hidd_param.dev.devices[0].reports_map.data;
    s_hidd_param.app_param.desc_list_len = s_hidd_param.dev.devices[0].reports_map.len;
}

static void bt_hidd_init_qos(void)
{
    uint32_t value_len = 0;
    value_len = get_value_len_by_type_protocol(&s_hidd_param.dev, BTMG_HID_REPORT_TYPE_INPUT,
                                               s_hidd_param.dev.protocol_mode);
    build_default_in_qos(&s_hidd_param.in_qos, value_len);

    value_len = get_value_len_by_type_protocol(&s_hidd_param.dev, BTMG_HID_REPORT_TYPE_INPUT,
                                               s_hidd_param.dev.protocol_mode);
    build_default_out_qos(&s_hidd_param.out_qos, value_len);
}

static hidd_report_item_t *get_report_by_idx_id_type(bt_hidd_dev_t *dev, size_t index, uint8_t id,
                                                     uint8_t type)
{
    hidd_report_item_t *rpt = NULL;
    if (index >= dev->devices_len) {
        BTMG_ERROR("index out of range[0-%d]", dev->devices_len - 1);
        return NULL;
    }
    for (uint8_t i = 0; i < dev->devices[index].reports_len; i++) {
        rpt = &dev->devices[index].reports[i];
        if (rpt->report_id == id && rpt->report_type == type &&
            rpt->protocol_mode == dev->protocol_mode) {
            return rpt;
        }
    }
    return NULL;
}

btmg_err bt_hid_device_init(void)
{
    xr_err_t ret;

    // setting cod major, peripheral
    xr_bt_cod_t cod = { 0 };
    cod.major = XR_BT_COD_MAJOR_DEV_PERIPHERAL;
    cod.minor = 0x20; // refer to <<cod_definition.pdf>>
    xr_bt_gap_set_cod(cod, XR_BT_SET_COD_MAJOR_MINOR);

    //[1] Reset the hid device target environment
    s_hidd_param.dev.connected = false;
    s_hidd_param.dev.registered = false;
    s_hidd_param.dev.bat_level = 100;
    s_hidd_param.dev.protocol_mode = BTMG_HID_PROTOCOL_MODE_REPORT;

    //[2] parse hid descriptor
    if ((ret = bt_hidd_init_config(&s_hidd_param.dev, &bt_hid_config)) != XR_OK) {
        BTMG_ERROR("hid device register callback return failed: %d\n", ret);
    }

    //[3] configure hidd app param and qos param
    bt_hidd_init_app();
    bt_hidd_init_qos();

    if ((ret = xr_bt_hid_device_register_callback(&bt_hidd_cb)) != XR_OK) {
        BTMG_ERROR("hid device register callback return failed: %d\n", ret);
        bt_hid_free_config(&s_hidd_param.dev);
        return BT_FAIL;
    }

    if ((ret = xr_bt_hid_device_init()) != XR_OK) {
        BTMG_ERROR("return failed: %d\n", ret);
        bt_hid_free_config(&s_hidd_param.dev);
        return BT_FAIL;
    }
    return BT_OK;
}

btmg_err bt_hid_device_deinit(void)
{
    xr_err_t ret;
    if ((ret = xr_bt_hid_device_deinit()) != XR_OK) {
        BTMG_ERROR("return failed: %d\n", ret);
        return BT_FAIL;
    }
    return BT_OK;
}

btmg_err bt_hid_device_connect(const char *addr)
{
    xr_err_t ret;
    xr_bd_addr_t remote_bda = { 0 };

    str2bda(addr, remote_bda);

    if ((ret = xr_bt_hid_device_connect(remote_bda)) != XR_OK) {
        BTMG_ERROR("return failed: %d\n", ret);
        return BT_FAIL;
    }

    return BT_OK;
}

btmg_err bt_hid_device_disconnect(const char *addr)
{
    //unused addr

    xr_err_t ret;
    if ((ret = xr_bt_hid_device_disconnect()) != XR_OK) {
        BTMG_ERROR("return failed: %d\n", ret);
        return BT_FAIL;
    }
    return BT_OK;
}

#define HID_MOUSE_IN_RPT_LEN 4
// send the buttons, change in x, and change in y
btmg_err bt_hid_device_send_mouse(uint8_t buttons, char dx, char dy, char wheel)
{
    xr_err_t ret = 0;
    hidd_report_item_t *p_rpt;
    static uint8_t buffer[HID_MOUSE_IN_RPT_LEN] = { 0 };
    buffer[0] = buttons;
    buffer[1] = dx;
    buffer[2] = dy;
    buffer[3] = wheel;

    p_rpt = get_report_by_idx_id_type(&s_hidd_param.dev, 0, 0, BTMG_HID_REPORT_TYPE_INPUT);
    if (p_rpt == NULL /* || (p_rpt && (p_rpt->value_len < HID_MOUSE_IN_RPT_LEN)) */) {
        BTMG_ERROR("HID device not connected! p_rpt:%p\n", p_rpt);
        ret = -1;
    }

    if (0 == ret) {
        ret = xr_bt_hid_device_send_report(XR_HIDD_REPORT_TYPE_INTRDATA, 0, HID_MOUSE_IN_RPT_LEN,
                                           buffer);
    }

    return ret;
}
