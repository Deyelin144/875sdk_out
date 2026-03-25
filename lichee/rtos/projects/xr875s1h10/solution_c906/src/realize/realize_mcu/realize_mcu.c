#include "realize_mcu.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mod_realize/realize_unit_log/realize_unit_log.h"
#include <console.h>

#define MCU_JSON_BUFFER_SIZE 160

static realize_mcu_callback_t s_mcu_callback = NULL;
static realize_mcu_event_callback_t s_mcu_event_cb = NULL;

static char *realize_mcu_build_json(const char *type,
                                    const char *payload_fmt,
                                    int value1,
                                    int value2,
                                    int value3)
{
    char payload[96];
    char *json;
    int ret;

    if (type == NULL || payload_fmt == NULL) {
        return NULL;
    }

    ret = snprintf(payload, sizeof(payload), payload_fmt, value1, value2, value3);
    if (ret < 0 || ret >= (int)sizeof(payload)) {
        return NULL;
    }

    json = malloc(MCU_JSON_BUFFER_SIZE);
    if (json == NULL) {
        return NULL;
    }

    ret = snprintf(json, MCU_JSON_BUFFER_SIZE, "{\"type\":\"%s\",\"payload\":{%s}}", type, payload);
    if (ret < 0 || ret >= MCU_JSON_BUFFER_SIZE) {
        free(json);
        return NULL;
    }

    return json;
}

int realize_mcu_init(realize_mcu_callback_t callback)
{
    int ret = -1;

    if(callback){
        ret = 0;
    }
    s_mcu_callback = callback;
    return ret;
}

/**
 * @brief   向app发送消息
 * @param   state
 * @param   payload
 * @return  
 */
void realize_mcu_send_app_msg(char *type, int target, int describe)
{
    int event_type = -1;
    int event_id = -1;
    int event_data = -1;

    if (!s_mcu_callback && !s_mcu_event_cb) {
        realize_unit_log_info("mcu callback is not ready, skip message\n");
        return;
    }

    if (!type) {
        realize_unit_log_error("type is NULL!\n");
        return;
    }
    char *json = NULL;

    if(strcmp(type, "touch_key") == 0){
        json = realize_mcu_build_json(type, "\"KeyCode\":%d,\"KeyType\":%d", target, describe, 0);
        event_type = MCU_EVENT_TOUCHKEY;
        event_id = target;
        event_data = describe;
        
    }else if(strcmp(type, "robot_action_reply") == 0){
        json = realize_mcu_build_json(type, "\"ActionId\":%d,\"ActionStatus\":%d", target, describe, 0);
        event_type = MCU_EVENT_ACTION;
        event_id = target;
        event_data = describe;
    } else {
        json = realize_mcu_build_json(type, "\"Target\":%d,\"Describe\":%d", target, describe, 0);
    }

    realize_unit_log_info("json = %s\n", json);
    if (s_mcu_event_cb && event_type >= 0) {
        mcu_event_param_t mcu_event_param;
        mcu_event_param.event_type = event_type;
        mcu_event_param.id = event_id-1;
        mcu_event_param.data = event_data;
        s_mcu_event_cb(&mcu_event_param);
    }   
    
    if (s_mcu_callback && json) {
        s_mcu_callback(json);
    } else {
        realize_unit_log_error("callback func or json is NULL!\n");
    }

    free(json);
}

/**
 * @brief   向app发送机器人trim信息
 * @param   target action id
 * @param   trim_st 包含矫正数据的字符
 * @return  
 */
void mcu_robot_trim_info(int target, void *trim_st)
{
    if (!trim_st) {
        realize_unit_log_error("trim_st is NULL!\n");
        return;
    }
    if (s_mcu_event_cb) {
        mcu_event_param_t mcu_event_param;
        mcu_event_param.event_type = target;
        mcu_event_param.id = target;
        mcu_event_param.data = 0;
        mcu_event_param.data_st = trim_st;
        s_mcu_event_cb(&mcu_event_param);
    } 
}


/**
* @brief   向app发送陀螺仪相关消息
* @param   type
* @param   x_data
* @param   y_data
* @param   z_data
* @return  
*/
void realize_mcu_send_app_msg_gyro(char *type, int x_data, int y_data, int z_data)
{
    if (!type) {
        realize_unit_log_error("type is NULL!\n");
        return;
    }
    char *json = realize_mcu_build_json(type, "\"XData\":%d,\"YData\":%d,\"ZData\":%d", x_data, y_data, z_data);
    realize_unit_log_info("json = %s\n", json);
    if (s_mcu_callback && json) {
        s_mcu_callback(json);
    } else {
        realize_unit_log_error("callback func or json is NULL!\n");
    }

    free(json);
}

void realize_register_mcu_event_cb(realize_mcu_event_callback_t cb)
{
    s_mcu_event_cb = cb;
}


static void touchkey_test(int argc, char **argv)
{
    if (argc != 3) {
        realize_unit_log_error("Usage: touchkey_test Parameter error!\n");
        return;
    }

    int key_id = atoi(argv[1]);
    int event = atoi(argv[2]);
    realize_mcu_send_app_msg("touch_key", key_id, event);
}
FINSH_FUNCTION_EXPORT_CMD(touchkey_test, tk, touchkey cmd);
