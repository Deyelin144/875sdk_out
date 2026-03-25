#include "stdio.h"
#include "string.h"
#include <stdarg.h>
#include "at_command_handler.h"
#include "at_parser.h"
#include "../drv_log.h"
#include <console.h>
#include "drv_uart_iface.h"
#include "kernel/os/os.h"
#include "mcu_at_device.h"
#include "../../realize/realize_mcu/realize_mcu.h"
#include "../rtc/dev_rtc.h"
#include "dev_mcu_upgrade.h"
#include "../robot/dev_robot.h"

#define _mcu_at_mutex_lock() mcu_at_mutex_lock(__FUNCTION__, __LINE__)
#define _mcu_at_mutex_unlock() mcu_at_mutex_unlock(__FUNCTION__, __LINE__)
#define TOUCH_KEY_FILTER_TIME    1000       //ms

static void at_cmd_touchkey(const char *args);
static void at_cmd_action_reply(const char *args);
static void at_cmd_gyro(const char *args);
static void at_cmd_bat_handler(const char *args);
static void at_cmd_msg_handler(const char *args);
#if AT_CMD_TEST_EN
static void at_cmd_TEST(const char *args);
#endif
static void at_resp_default_handler(const char *resp_data, void *output, uint8_t status);
static void at_resp_get_ischarge_handler(const char *resp_data, void *output, uint8_t status);
static void at_resp_get_ischargeFull_handler(const char *resp_data, void *output, uint8_t status);
static void at_resp_iap_upgrade_handler(const char *resp_data, void *output, uint8_t status);
static void at_resp_get_wakeSrc_handler(const char *resp_data, void *output, uint8_t status);
static void at_resp_version_handler(const char *resp_data, void *output, uint8_t status);
static void at_resp_sleep_handler(const char *resp_data, void *output, uint8_t status);

__attribute__((weak)) void sdk_battery_servo_set_active(bool active, unsigned long duration_ms)
{
    (void)active;
    (void)duration_ms;
}

/**
 * @brief 被动接收的命令表：用于匹配并处理MCU发来的 AT 命令
 * 
 */
static const AtCommand_Res_t at_res_commands[] = {
    {"AT+ROBOT", 8, at_cmd_action_reply},
    {"AT+TK", 5, at_cmd_touchkey},
    {"AT+GYRO", 7, at_cmd_gyro},
    {"AT+BAT=", 7, at_cmd_bat_handler},
    {"AT+MSG=", 7, at_cmd_msg_handler},
#if AT_CMD_TEST_EN
    {"AT+TEST", 7, at_cmd_TEST},
#endif
};
/**
 * @brief 主动发送的命令表：用于匹配并处理AT响应
 * @note XR875 XR_OS_TicksToMSecs(XR_OS_GetTicks())函数计时会有误差, 大概比实际时间要快20ms左右, 所以timeout时间可以适当增加20ms
 *          handler 注意mcu的回复,只回复OK的不用判断resp是否为NULL !!!
 */
static const AtCommand_Req_t at_req_commands[AT_REQ_CMD_MAX] = {
    [AT_REQ_CMD_EXT_VCC_SET]                                = {"AT+CTRL=",    at_resp_default_handler,    120},
    [AT_REQ_CMD_SYS_VCC_SET]                                = {"AT+CTRL=",    at_resp_default_handler,    120},
    [AT_REQ_CMD_LCD_RESET_SET]                              = {"AT+CTRL=",    at_resp_default_handler,    120},
    [AT_REQ_CMD_TP_RESET_SET]                               = {"AT+CTRL=",    at_resp_default_handler,    500},
    [AT_REQ_CMD_CAMERA_PWDN_SET]                            = {"AT+CTRL=",    at_resp_default_handler,    120},
    [AT_REQ_CMD_PA_SET]                                     = {"AT+CTRL=",    at_resp_default_handler,    220},
    [AT_REQ_CMD_BATTERY_CHARGE_GET]                         = {"AT+CTRL=",    at_resp_get_ischarge_handler,    500},
    [AT_REQ_CMD_BATTERY_CHARGE_FULL_GET]                    = {"AT+CTRL=",    at_resp_get_ischargeFull_handler,    500},
    [AT_REQ_CMD_LCD_BL_LP_SET]                              = {"AT+CTRL=",    at_resp_default_handler,    120},
    [AT_REQ_CMD_ROBOT_VCC_SET]                              = {"AT+CTRL=",    at_resp_default_handler,    120},
    [AT_REQ_CMD_IRQ_SET]                                    = {"AT+CTRL=",    at_resp_default_handler,    120},
    [AT_REQ_CMD_IAP_UPGRADE]                                = {"AT+UPGRADE",     at_resp_iap_upgrade_handler,    500},
    [AT_REQ_CMD_ATI]                                        = {"AT+ATI",         at_resp_version_handler,    500},
    [AT_REQ_CMD_ROBOT]                                      = {"AT+ROBOT",      at_resp_default_handler,     220},
    [AT_REQ_CMD_TK]                                         = {"AT+TK",         at_resp_default_handler,     120},
    [AT_REQ_CMD_GYRO]                                       = {"AT+GYRO",       at_resp_default_handler,     120},
    [AT_REQ_CMD_SLEEP]                                      = {"AT+SLEEP=",      at_resp_sleep_handler,    500},
    [AT_REQ_CMD_WAKE_SRC_GET]                               = {"AT+WAKESRC=",    at_resp_get_wakeSrc_handler,    500},

#if AT_CMD_TEST_EN
    [AT_REQ_CMD_TEST]                                       = {"AT+TEST=",       at_resp_default_handler,    100},
#endif
};

static const uint8_t at_res_commands_count = sizeof(at_res_commands) / sizeof(at_res_commands[0]);
static char at_build_buffer[AT_BUILD_BUF_SIZE] = {0};

static AtRequestRecordQueue_t at_request_record_queue = {0};
static uint32_t s_mcu_tk_time = 0;


uint8_t at_cmd_queue_is_empty()
{
    return at_request_record_queue.count == 0;
}

static int at_request_record_queue_push(const AtRequestRcord_t *req)
{
    _mcu_at_mutex_lock();
    if (at_request_record_queue.count >= AT_REQUEST_RECORD_QUEUE_SIZE) {
        drv_loge("queue is full");
        _mcu_at_mutex_unlock();
        return -1;
    }
    int idx = at_request_record_queue.tail;
    at_request_record_queue.queue[at_request_record_queue.tail] = *req;
    // drv_logi("push pos: %d %d", at_request_record_queue.tail, req->cmd_idx);
    at_request_record_queue.tail = (at_request_record_queue.tail + 1) % AT_REQUEST_RECORD_QUEUE_SIZE;
    at_request_record_queue.count++;
    // drv_logi("push cnt: %d", at_request_record_queue.count);
    _mcu_at_mutex_unlock();
    return idx;
}

static AtRequestRcord_t * at_request_record_queue_pop()
{
    _mcu_at_mutex_lock();
    if (at_request_record_queue.count == 0) {
        drv_loge("queue is empty");
        _mcu_at_mutex_unlock();
        return NULL;
    }
    AtRequestRcord_t *req = &at_request_record_queue.queue[at_request_record_queue.head];
    // drv_logi("pop pos: %d %d", at_request_record_queue.head, at_request_record_queue.queue[at_request_record_queue.head].cmd_idx);
    at_request_record_queue.head = (at_request_record_queue.head + 1) % AT_REQUEST_RECORD_QUEUE_SIZE;
    at_request_record_queue.count--;
    _mcu_at_mutex_unlock();
    // drv_logi("pop cnt: %d", at_request_record_queue.count);
    return req;
}

/**
 * @brief 清理队列中所有非活跃的请求
 * 
 */
static void at_cleanup_inactive_requests(void)
{
    if (at_request_record_queue.count == 0) {
        return;
    }
    
    uint8_t new_tail = at_request_record_queue.head;
    uint8_t new_count = 0;
    
    // 遍历队列，只保留活跃的请求
    for (uint8_t i = 0; i < at_request_record_queue.count; i++) {
        uint8_t idx = (at_request_record_queue.head + i) % AT_REQUEST_RECORD_QUEUE_SIZE;
        AtRequestRcord_t *req = &at_request_record_queue.queue[idx];
        
        if (req->active) {
            // 如果请求活跃，保留它
            if (new_tail != idx) {
                // printf("clean tail:%d idx:%d\n", new_tail, idx);
                at_request_record_queue.queue[new_tail] = *req;
                if (req->block) {
                    req->update = new_tail;    //更新阻塞等待的序号
                    drv_logw("wait update\n");
                    while (req->update != -1) {
                        XR_OS_MSleep(1);
                    }
                    drv_logw("wait update end \n");
                }
            }
            new_tail = (new_tail + 1) % AT_REQUEST_RECORD_QUEUE_SIZE;
            new_count++;
        } else {
            // 如果请求不活跃，清理它
            drv_logw("Cleaned up inactive request cmd_idx: %d idx:%d", req->cmd_idx,idx);
        }
    }
    
    // 更新队列指针和计数
    at_request_record_queue.tail = new_tail;
    at_request_record_queue.count = new_count;
}

/**
 * @brief AT command 发送函数
 *        1. 支持 AT+xxx=xxx 格式
 *        2. 支持 AT+xxx? 格式
 *        3. 支持 AT+xxx 格式
 * @param cmd_idx AT指令索引
 * @param send_arg 发送参数
 * @param output 输出参数
 */
void at_send_command(at_req_cmd_e cmd_idx, const char *send_arg, void *output,uint8_t block)
{
    if (mcu_at_uart_is_sleep()){   
        printf("at sleep skip cmd: %d %s\n", cmd_idx,send_arg);
        return ;
    }
    static uint8_t is_first = 1;
    int idx = 0;
    int idx_tmp = 0;
    
    if (is_first) {
        is_first = 0;
        memset(&at_request_record_queue, 0, sizeof(AtRequestRecordQueue_t));
    }

    if (cmd_idx >= AT_REQ_CMD_MAX) {
        drv_loge("cmd_idx error");
        return;
    }

    // 添加AT前缀和换行符
    memset(at_build_buffer, 0, AT_BUILD_BUF_SIZE);
    if (send_arg) {
        snprintf(at_build_buffer, AT_BUILD_BUF_SIZE, "%s%s\r\n", at_req_commands[cmd_idx].cmd, send_arg);
    } else {
        snprintf(at_build_buffer, AT_BUILD_BUF_SIZE, "%s\r\n", at_req_commands[cmd_idx].cmd);
    }
    drv_logi("send cmd: %d %s", cmd_idx, at_build_buffer);

    // 发送指令
    if (strlen(at_build_buffer) != drv_mcu_iface_send_data((uint8_t *)at_build_buffer, strlen(at_build_buffer))) {
        drv_loge("send cmd error");
        return;
    }
    
    AtRequestRcord_t req = {
        .cmd_idx = cmd_idx,
        .output = output,
        .timeout = XR_OS_TicksToMSecs(XR_OS_GetTicks()) + at_req_commands[cmd_idx].timeout,
        .active = 1,
        .block = 0,
        .update = -1,
    };

    if (block) {
        req.block = 1;
    }
    // 将请求加入队列
    idx = at_request_record_queue_push(&req);
    if (idx < 0) {
        return ;
    }

    // 延时5ms, 避免连续发送过快, mcu端处理不过来. 暂时这样修改
    XR_OS_MSleep(5);

    if (block) {
        printf("wait cmd: %d idx:%d\n", cmd_idx,idx);
        while (1) {
            //更新索引
            if (at_request_record_queue.queue[idx].update != -1) {
                idx_tmp = at_request_record_queue.queue[idx].update;
                at_request_record_queue.queue[idx].update = -1;
                // printf("wait cmd change idx:%d --> idx_tmp:%d\n", idx,idx_tmp);
                idx = idx_tmp;
            }

            if ( !at_request_record_queue.queue[idx].active || (XR_OS_TicksToMSecs(XR_OS_GetTicks()) > req.timeout + 200)){
                break;
            } else {
                XR_OS_MSleep(1);
            }
        }
    
        at_request_record_queue.queue[idx].block = 0;
        printf("wait cmd end: %d idx:%d\n", cmd_idx,idx);
    }
}

/**
 * @brief 检查at cmd是否响应超时
 * 
 */
void at_check_response_timeout()
{
    uint32_t current_time = XR_OS_TicksToMSecs(XR_OS_GetTicks());
    uint8_t is_clean = 0;

    _mcu_at_mutex_lock();
    for (uint8_t i = 0; i < at_request_record_queue.count; i++) {
        uint8_t idx = (at_request_record_queue.head + i) % AT_REQUEST_RECORD_QUEUE_SIZE;
        AtRequestRcord_t *req = &at_request_record_queue.queue[idx];

        if (!req->active) {
            continue;
        }

        if (current_time > req->timeout) {
            is_clean = 1;
            drv_logw("AT timeout for cmd_idx %d idx:%d", req->cmd_idx, idx);
            req->active = 0;
            at_req_commands[req->cmd_idx].handler(NULL, req->output, AT_RESP_STATUS_TIMEOUT);
        }
    }
    if (is_clean) {
        at_cleanup_inactive_requests();
    }
    _mcu_at_mutex_unlock();

    return;
}

/**
 * @brief 响应解析MCU返回的数据
 * 
 * @param resp 
 */
void at_process_response(const char *resp)
{
    AtRequestRcord_t *req = NULL;

    req = at_request_record_queue_pop();
    if ( NULL == req || !req->active) {
        drv_loge("No active request to match");
        return;
    }

    const char *args = NULL;
    char *end_ptr = NULL;
    if (NULL != (args = strstr(resp, "+RES:"))) {
        args += 5;
    }

    if (NULL != (end_ptr = strstr(resp, "OK\r\n"))) {
        if (args) {
            *(end_ptr - 2) = '\0';
        }
        at_req_commands[req->cmd_idx].handler(args, req->output, AT_RESP_STATUS_OK);
    } else if (NULL != (end_ptr = strstr(resp, "ERROR\r\n"))) {
        if (args) {
            *(end_ptr - 2) = '\0';
        }
        at_req_commands[req->cmd_idx].handler(args, req->output, AT_RESP_STATUS_ERROR);
    }

    req->active = 0;
}

/**
 * @brief AT 传输数据到MCU
 * 
 * @param type 
 * @param format 
 * @param ... 
 */
void at_transmit_data_ex(AtResponseType type, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    memset(at_build_buffer, 0, AT_BUILD_BUF_SIZE);
    switch (type) {
        case AT_RESP_TYPE_NORMAL:
            vsnprintf(at_build_buffer, AT_BUILD_BUF_SIZE, format, args);
            drv_mcu_iface_send_data((uint8_t *)at_build_buffer, strlen(at_build_buffer));
            break;

        case AT_RESP_TYPE_OK:
            drv_mcu_iface_send_data((uint8_t *)AT_CMD_RESPONSE_OK, strlen(AT_CMD_RESPONSE_OK));
            break;

        case AT_RESP_TYPE_ERROR:
            snprintf(at_build_buffer, AT_BUILD_BUF_SIZE, "+RES:%s\r\n", format);
            drv_mcu_iface_send_data((uint8_t *)at_build_buffer, strlen(at_build_buffer));
            drv_mcu_iface_send_data((uint8_t *)AT_CMD_RESPONSE_ERROR, strlen(AT_CMD_RESPONSE_ERROR));
            break;
    }

    va_end(args);
}

/**
 * @brief 匹配MCU发送过来的AT指令
 * 
 * @param cmd 
 */
void at_process_command(const char *cmd)
{
    for (int i = 0; i < at_res_commands_count; i++) {
        const AtCommand_Res_t *at_cmd = &at_res_commands[i];
        if (strncmp(cmd, at_cmd->cmd, at_cmd->cmd_len) == 0) {

            const char *args = cmd + at_cmd->cmd_len;

            if (*args == '?' || *args == '=') {
                args++;
                at_cmd->handler(args);
            } else {
                at_cmd->handler(args);
            }
            return;
        }
    }

    AT_SEND_ERR("Unknown cmd");
}

/**
 * @brief 默认处理AT指令响应的函数
 * 
 * @param resp_data mcu响应的数据内容
 * @param status 响应状态
 */
static void at_resp_default_handler(const char *resp_data, void *output, uint8_t status)
{
    if (status == AT_RESP_STATUS_OK) {
        drv_logd("resp ok");
    } else if (status == AT_RESP_STATUS_ERROR) {
        drv_logd("resp err");
    } else if (status == AT_RESP_STATUS_TIMEOUT) {
        drv_logd("resp timeout");
    }
}


static void at_resp_get_ischarge_handler(const char *resp_data, void *output, uint8_t status)
{
    if (output == NULL) {
        drv_loge("output is null");
        return;
    }

    if (status == AT_RESP_STATUS_OK && resp_data) {
        drv_logd("resp ok: %s", resp_data);

        int *bat = (int *)output;
        int ret = -1;

        char* ptr = strstr(resp_data, "bat:");
        if (NULL == ptr) {
            drv_loge("not found bat");
            return;
        }

        ret = sscanf(ptr + strlen("bat:"), "%d,%d", &bat[0], &bat[1]);
        if (2 != ret) {
            drv_loge("get isCharge err.(%d)",ret);
            ret = -2;
            return;
        }
        drv_logd("isCharge: %d isFull: %d", bat[0], bat[1]);
    } else if (status == AT_RESP_STATUS_ERROR) {
        drv_logd("resp err");
    } else if (status == AT_RESP_STATUS_TIMEOUT) {
        drv_logd("resp timeout");
    }
}


static void at_resp_get_ischargeFull_handler(const char *resp_data, void *output, uint8_t status)
{
    int ret = -1;

    if (output == NULL) {
        drv_loge("output is null");
        return;
    }

    if (status == AT_RESP_STATUS_OK && resp_data) {
        drv_logd("resp ok: %s", resp_data);

        int isChargeFull = 0;
        int ret = -1;

        char* ptr = strstr(resp_data, "isChargeFull:");
        if (NULL == ptr) {
            drv_loge("not found isChargeFull");
            return;
        }

        ret = sscanf(ptr + strlen("isChargeFull:"), "%d,", &isChargeFull);
        if (1 != ret) {
            drv_loge("get isChargeFull err.(%d)",ret);
            ret = -2;
            return;
        }
        memcpy(output, &isChargeFull, sizeof(int));
        drv_logd("isChargeFull: %d %d", isChargeFull, *((int*)output));
    } else if (status == AT_RESP_STATUS_ERROR) {
        drv_logd("resp err");
    } else if (status == AT_RESP_STATUS_TIMEOUT) {
        drv_logd("resp timeout");
    }
}

static void at_resp_iap_upgrade_handler(const char *resp_data, void *output, uint8_t status)
{
    int success = -1;
    if (status == AT_RESP_STATUS_OK) {
        drv_logd("resp ok: %s", resp_data);
        if (strstr(resp_data, "READY")) {
            success = 0;
        }
    } else if (status == AT_RESP_STATUS_ERROR) {
        drv_logd("resp err");
    } else if (status == AT_RESP_STATUS_TIMEOUT) {
        drv_logd("resp timeout");
    }
    memcpy(output, &success, sizeof(int));
}

static void at_resp_sleep_handler(const char *resp_data, void *output, uint8_t status)
{
    if (output == NULL) {
        drv_loge("output is null");
        return;
    }

    int success = -1;
    if (status == AT_RESP_STATUS_OK) {
        drv_logd("resp ok: %s", resp_data);
        success = 0;
    } else if (status == AT_RESP_STATUS_ERROR) {
        drv_logd("resp err");
    } else if (status == AT_RESP_STATUS_TIMEOUT) {
        drv_logd("resp timeout");
    }
    memcpy(output, &success, sizeof(int));
    drv_logd("success: %d %d", success, *((int*)output));
}


static void at_cmd_touchkey(const char *args)
{
    AT_SEND_RES("touchkey ok");
  	if (!args || args[0] == '\0') {	return;}

    int ret = -1;
	int key_id = 0;
    int event = 0;
	ret = sscanf(args,"%d,%d",&key_id, &event);
    if(ret<0)	{return ;}
#ifndef CONFIG_PROJECT_BUILD_RECOVERY
    realize_mcu_send_app_msg("touch_key", key_id, event);
    s_mcu_tk_time = xTaskGetTickCount();
    printf("touch time:%d\n",s_mcu_tk_time);
#endif /* CONFIG_PROJECT_BUILD_RECOVERY */
}

//休眠前检测是否近期有触摸事件发生
int touch_key_sleep_check()
{
    if (xTaskGetTickCount() - s_mcu_tk_time < TOUCH_KEY_FILTER_TIME) {
        return -1;
    }else{
        return 0;
    }
}

static void at_cmd_action_reply(const char *args)
{
    AT_SEND_RES("action reply ok");
  	if (!args || args[0] == '\0') {	return;}

    int ret = -1;
	int action_id = 0;
    int status = 0;
	ret = sscanf(args,"%d,%d",&action_id, &status);

    sdk_battery_servo_set_active(false, 0);

    if(ret<0)	{return ;}
    if(action_id == TEST_ROBOT_CMD){
        mcu_robot_trim_info(action_id, args);
        return;
    }
#ifndef CONFIG_PROJECT_BUILD_RECOVERY
    dev_robot_action_over();
    realize_mcu_send_app_msg("robot_action_reply", action_id, status);
#endif /* CONFIG_PROJECT_BUILD_RECOVERY */
}

static void at_cmd_gyro(const char *args)
{
    AT_SEND_RES("gyro report ok");
    if (!args || args[0] == '\0') {	return;}

    int ret = -1;
    int x_data = 0;
    int y_data = 0;
    int z_data = 0;
    ret = sscanf(args,"%d,%d,%d",&x_data, &y_data, &z_data);
    if(ret<0)	{return ;}
#ifndef CONFIG_PROJECT_BUILD_RECOVERY
    //realize_mcu_send_app_msg_gyro("gyro_report", x_data, y_data, z_data);
#endif /* CONFIG_PROJECT_BUILD_RECOVERY */
}

extern void dev_battery_bat_info_set(char isCharge, char isChargeFull);

static void at_resp_get_wakeSrc_handler(const char *resp_data, void *output, uint8_t status)
{
    if (output == NULL) {
        drv_loge("output is null");
        return;
    }

    if (status == AT_RESP_STATUS_OK && resp_data) {
        drv_logd("resp ok: %s", resp_data);

        int wakeSrc = 0;
        int ret = -1;


        char* ptr = strstr(resp_data, "wakeSrc:");
        if (NULL == ptr) {
            drv_loge("not found wakeSrc");
            return;
        }

        ret = sscanf(ptr + strlen("wakeSrc:"), "%d,", &wakeSrc);
        if (1 != ret) {
            drv_loge("get wakeSrc err.(%d)",ret);
            ret = -2;
            return;
        }
        memcpy(output, &wakeSrc, sizeof(int));
        drv_logd("wakeSrc: %d %d", wakeSrc, *((int*)output));
    } else if (status == AT_RESP_STATUS_ERROR) {
        drv_logd("resp err");
    } else if (status == AT_RESP_STATUS_TIMEOUT) {
        drv_logd("resp timeout");
    }
}

static void at_cmd_bat_handler(const char *args)
{
    AT_SEND_RES("bat ok");
  	if (!args || args[0] == '\0') {	return;}

    int ret = -1;
	int isCharge = 0;
    int isChargeFull = 0;
	ret = sscanf(args,"%d,%d",&isCharge, &isChargeFull);
    if(ret<=0)	{return ;}

    dev_battery_bat_info_set(isCharge, isChargeFull);
}

static void at_cmd_msg_handler(const char *args)
{
    AT_SEND_RES("msg ok");
  	if (!args || args[0] == '\0') {	return;}

	if (memcmp(args, "boot", strlen("boot")) == 0) {
        drv_loge("mcu boot, args: %s", args);

        // mcu在运行中, 中途复位时, 会导致部分外设异常, 如屏幕显示等, 需要对这些外设重新进行初始化才可以正常工作, 此处先直接复位128处理
        // 后续可以考虑单独针对每个和mcu相关的外设进行异常处理
        cmd_reboot(1, NULL);
    }
}

static void at_resp_version_handler(const char *resp_data, void *output, uint8_t status)
{
    if (output == NULL) {
        drv_loge("output is null");
        return;
    }

    if (status == AT_RESP_STATUS_OK && resp_data) {
        char* ptr = strstr(resp_data, "mcu_version:");
        if (NULL == ptr) {
            drv_loge("not mcu_version");
            return;
        }

        memcpy(output, ptr + strlen("mcu_version:"), strlen(ptr + strlen("mcu_version:")));
    } else if (status == AT_RESP_STATUS_ERROR) {
        drv_logd("resp err");
    } else if (status == AT_RESP_STATUS_TIMEOUT) {
        drv_logd("resp timeout");
    }
}

/***************************************************************
  *  @brief     测试指令
  *  @param     
  *  @note      
  *  @Sample usage: 
  *         mcu_test cmd 0 5 1     //打开pa
  *         mcu_test cmd 1 6       //获取充电信息
  *         mcu_test bootload       //mcu进入bootload模式
  *         mcu_test recovery       // 128 跳到recovery模式
 **************************************************************/
int dev_mcu_test(int argc, const char **argv)
{
    if (0 == memcmp(argv[1], "demo", strlen("demo"))) {
    } else if (0 == memcmp(argv[1], "cmd", strlen("cmd"))) {
        int isResp = atoi(argv[2]);     //是否需要回复
        int cmd_idx = atoi(argv[3]);    //指令索引
        int val = 0;
        if(argv[4]){
            val = atoi(argv[4]);        //设置值
        }
        int output = 0;
        char cmd[8]={0};
        if(isResp){
            if(argv[4]){
                sprintf(cmd, "%d\r\n",val);
            }else{
                sprintf(cmd, "%d\r\n",cmd_idx);
            }
            AT_SEND_REQ(cmd_idx, cmd, &output,1);
        }else{
            sprintf(cmd, "%d,%d\r\n",cmd_idx,val);
            AT_SEND_REQ(cmd_idx, cmd, NULL,0);
        }
    } else if (0 == memcmp(argv[1], "at", strlen("at"))) {
        printf("at:%s %d\n",argv[2], strlen(argv[2]));
        drv_mcu_iface_send_data(argv[2], strlen(argv[2]));
        drv_mcu_iface_send_data("\r\n", 2);
    } else if (0 == memcmp(argv[1], "lcd_reset", strlen("lcd_reset"))) {
        dev_mcu_lcd_reset_set(atoi(argv[2]));
    }else if (0 == memcmp(argv[1], "recovery", strlen("recovery"))) {
        char *env_argv[] = {
            "fw_setenv",
            "loadparts",
            "arm-b@:config@0xc000000:"};
        // wifi_wrapper_disconnect(); //重启前先断开wifi，解决华为路由器重启后拒绝连接的异常。
        cmd_fw_setenv(3, env_argv);
        cmd_reboot(1, NULL);
    }else if (0 == memcmp(argv[1], "bootload", strlen("bootload"))) {
        // dev_mcu_upgrade();  //mcu to bootload
        uint8_t data[128] = {0};  
        if(drv_mcu_iface_send_data("AT+UPGRADE\r\n",strlen("AT+UPGRADE\r\n"))) {
            drv_mcu_iface_recv_data(data, 64, 300);
            printf("len:%d mcu data:%s\n",strlen(data),data);
        }
    }else if (0 == memcmp(argv[1], "app", strlen("app"))) {
        uint8_t data[128] = {0};  
        if( mcu_jump_to_app(1)){
            drv_loge("mcu jump to app ok\n");
        }else{
            drv_mcu_iface_recv_data(data, 128, 150);
            printf("len:%d mcu data:%s\n",strlen(data),data);
        }
    }else if (0 == memcmp(argv[1], "ATI", strlen("ATI"))) { 
        uint8_t data[128] = {0};  
        uint32_t bootload_version;
        if(drv_mcu_iface_send_data("AT+ATI\r\n",strlen("AT+ATI\r\n"))) {
            drv_mcu_iface_recv_data(data, 128, 150);
        }
        
        if (strstr(data, "mcu_version") != NULL) {
            drv_loge("mcu in app,%s\n",data);  //mcu在 app,正常启动
        }else{
            drv_loge("mcu AT+ATI ERROR\n");

            memset(data,0,128);
            if (bootload_get_version(&bootload_version)) {  
                drv_loge("mcu in bootloader\n");  
            }else{
                drv_mcu_iface_recv_data(data, 128, 150);
                printf("len:%d mcu data:%s\n",strlen(data),data);
            }
        }
    }
    
    return 0;
}


FINSH_FUNCTION_EXPORT_CMD(dev_mcu_test, mcu_test, mcu options);

#if AT_CMD_TEST_EN
static void at_cmd_TEST(const char *args)
{
    AT_SEND_RES("test ok");
}
#endif
