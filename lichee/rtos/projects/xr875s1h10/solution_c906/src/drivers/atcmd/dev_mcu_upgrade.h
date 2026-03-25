#ifndef __DEV_MCU_UPGRADE_H__
#define __DEV_MCU_UPGRADE_H__

#include <stdbool.h>
/*
    发送数据格式：
    HEAD(1) + LEN(1) + CMD(1) + ADDR(4) + DATA(240 可变) + CRC(2)

    DATA[0]     HEAD
    DATA[1]     LEN     //CMD(1) + ADDR(4) + DATA_LEN
    DATA[2]     CMD     
    DATA[3-6]   ADDR
    DATA[7 - (6+LEN)]    DATA
    ...
    DATA[(MAX_PAYLOAD_SIZE-1) - (MAX_PAYLOAD_SIZE)]    CRC


    接收数据格式：
    DATA[0]     ACK         //在接收到数据包后，根据完整性赋值
    DATA[1]     HEAD        //回包时默认赋值
    DATA[2]     LEN         //reply + DATA_LEN 
    DATA[3]     reply       //指令的操作结果码

    DATA[4  - (LEN +2 )]    // DATA

    DATA[(LEN+3) - (LEN+4)]     CRC
*/


// MCU应用程序起始地址（Flash地址） 
#define CSBOOT_APP_BASE    					((uint32_t)0x00002600)
#define CSBOOT_APP_SIZE    					((uint32_t)0x0000D800)

// 协议定义
#define PACKET_HEAD         0x55
#define RESPONSE_HEAD       0x00
#define MAX_PAYLOAD_SIZE    240
#define MAX_PACKET_SIZE     (1 + 1 + 1 + 4 + MAX_PAYLOAD_SIZE + 2) // HEAD+LEN+CMD+ADDR+DATA+CRC
#define RX_BUFFER_SIZE      MAX_PACKET_SIZE

// 命令定义
#define CMD_GET_ID          0x20
#define CMD_GET_VERSION     0x21
#define CMD_WRITE_MEMORY    0x23
#define CMD_READ_MEMORY     0x24
#define CMD_ERASE_PAGE      0x25
#define CMD_ERASE_CHIP      0x26
#define CMD_GO              0x27

// 普通命令基值
#define CONTROL_CMD_BASE    0xA0

// 响应码定义
#define RESP_SUCCESS        0x00
#define RESP_CMD_SUCCESS    0x00
#define RESP_INVALID_CMD    0x01
#define RESP_INVALID_PARAM  0x02
#define RESP_FAILED         0x04

// 升级状态定义
typedef enum {
    STATE_IDLE,
    STATE_GET_CHIP_ID,
    STATE_GET_VERSION,
    STATE_ERASE_CHIP,
    STATE_WRITE_FIRMWARE,
    STATE_VERIFY_FIRMWARE,
    STATE_JUMP_TO_APP,
    STATE_FINISHED,
    STATE_ERROR
} UpgradeState;

char* get_bootloader_version_string();
bool bootload_get_version(uint32_t *version);
bool mcu_jump_to_app(uint8_t arg);
int mcu_firmware_update();
int mcu_firmware_update_load_info(char *buffer,int size);
int mcu_firmware_update_get_offset();
int mcu_firmware_update_get_state();
int mcu_firmware_update_reset_state();
int mcu_recovery_at_control(uint8_t cmd,uint8_t *data, uint16_t data_len);
int mcu_recovery_at_control_for_reply(uint8_t cmd, uint8_t *data, uint16_t data_len);
int mcu_recovery_at_thread_start();
int mcu_clear_rx_buffer(void);
#endif
