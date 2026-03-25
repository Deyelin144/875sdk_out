#ifndef AT_COMMAND_HANDLER_H_
#define AT_COMMAND_HANDLER_H_

#include <stdint.h>

#define AT_CMD_TEST_EN  (0)

// 全局静态缓冲区，用于构建响应/发送字符串（仅占用一次栈空间）
#define AT_BUILD_BUF_SIZE (64)

#define AT_REQUEST_RECORD_QUEUE_SIZE (10)

#define AT_CMD_RESPONSE_OK      "OK\r\n"
#define AT_CMD_RESPONSE_ERROR   "ERROR\r\n"

// 响应状态码定义
#define AT_RESP_STATUS_OK       (0x00)
#define AT_RESP_STATUS_ERROR    (0x01)
#define AT_RESP_STATUS_TIMEOUT  (0x02)

typedef enum {
    AT_RESP_TYPE_NORMAL,   // 普通响应 "+RES:xxx\r\nOK\r\n"
    AT_RESP_TYPE_OK,       // 成功响应 "OK\r\n"
    AT_RESP_TYPE_ERROR     // 错误响应 "+RES:xxx\r\nERROR\r\n"
} AtResponseType;

/**
 * @brief 请求AT指令类型, R128 -> MCU
 * @note R128和mcu两端的枚举顺序是对应的，如有修改，需两端同步
 * 
 */
typedef enum {
	AT_REQ_CMD_EXT_VCC_SET				=	0,			//外部供电设置
	AT_REQ_CMD_SYS_VCC_SET				,				//系统供电设置
	AT_REQ_CMD_LCD_RESET_SET			,				//LCD 	复位引脚控制
	AT_REQ_CMD_TP_RESET_SET			,				    //TP  	复位引脚设置
	AT_REQ_CMD_CAMERA_PWDN_SET			,				//摄像头使能引脚设置
	AT_REQ_CMD_PA_SET					,				//PA	引脚设置
	AT_REQ_CMD_BATTERY_CHARGE_GET		,				//充电状态获取
	AT_REQ_CMD_BATTERY_CHARGE_FULL_GET	,				//满电状态获取
	AT_REQ_CMD_LCD_BL_LP_SET			,				//LCD  休眠设置
    AT_REQ_CMD_ROBOT_VCC_SET,                           //机器人舵机供电设置
    AT_REQ_CMD_IRQ_SET                  ,               //中断电平设置


	AT_REQ_CMD_IAP_UPGRADE				,				//IAP  升级
	AT_REQ_CMD_ATI				,				//
    AT_REQ_CMD_ROBOT                    ,
    AT_REQ_CMD_TK                       ,
    AT_REQ_CMD_GYRO                     ,              //陀螺仪消息上报
    AT_REQ_CMD_SLEEP                    ,              //休眠(0:正常工作 1:浅度休眠 2:深度休眠)
    AT_REQ_CMD_WAKE_SRC_GET             ,              //唤醒源获取 

#if AT_CMD_TEST_EN
    AT_REQ_CMD_TEST					,
#endif
    AT_REQ_CMD_MAX			            ,
} at_req_cmd_e;

// 响应处理回调类型
typedef void (*at_resp_handler)(const char *resp_data, void *output, uint8_t status);
// 命令处理回调类型
typedef void (*at_cmd_res_parser_handler)(const char *args);

typedef struct {
    const char *cmd;
    uint8_t cmd_len;     // 预先计算命令长度
    at_cmd_res_parser_handler handler;
} AtCommand_Res_t;

typedef struct {
    const char *cmd;
    at_resp_handler handler;
    uint32_t timeout;
} AtCommand_Req_t;


typedef struct {
    at_req_cmd_e cmd_idx;     // 指令索引
    void *output;             // 输出缓冲区
    uint32_t timeout;         // 超时时间戳
    uint8_t active;           // 是否活跃
    uint8_t block;            // 是否阻塞
    int update;          // 是否更新    
} AtRequestRcord_t;

typedef struct {
    AtRequestRcord_t queue[AT_REQUEST_RECORD_QUEUE_SIZE];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
} AtRequestRecordQueue_t;

void at_process_command(const char *cmd);
void at_transmit_data_ex(AtResponseType type, const char *format, ...);

uint8_t at_cmd_queue_is_empty();
void at_send_command(at_req_cmd_e cmd_idx, const char *send_arg, void *output,uint8_t block);
void at_check_response_timeout(void);
void at_process_response(const char *resp);

/**
 * @brief 发送 AT 命令
 *        1. 支持 AT+xxx=xxx 格式
 *        2. 支持 AT+xxx? 格式
 *        3. 支持 AT+xxx 格式
 * 
 */
#define AT_SEND_REQ(cmd_idx, send_arg, output,block)      do { at_send_command(cmd_idx, send_arg, output,block); } while(0)

/**
 * @brief 发送 AT 命令响应
 * 
 */
#define AT_SEND_RES(fmt, ...) \
    at_transmit_data_ex(AT_RESP_TYPE_NORMAL, "+RES:" fmt "\r\nOK\r\n", ##__VA_ARGS__)

/**
 * @brief 发送 AT OK命令响应
 *        成功响应 "OK\r\n"
 * 
 */
#define AT_SEND_OK()          at_transmit_data_ex(AT_RESP_TYPE_OK, NULL)

/**
 * @brief 发送 AT ERROR命令响应
 *        错误响应 "+RES:xxx\r\nERROR\r\n"
 * 
 */
#define AT_SEND_ERR(fmt, ...) \
    at_transmit_data_ex(AT_RESP_TYPE_NORMAL, "+RES:" fmt "\r\nERROR\r\n", ##__VA_ARGS__)


#endif /* AT_COMMAND_HANDLER_H_ */
