#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "dev_mcu_upgrade.h"
#include <console.h>
#include "drv_uart_iface.h"
#include "test_firmware_array.h"
#include <stdbool.h>
#include "mcu_at_device.h"
#include "../drv_log.h"
#include "../drv_common.h"


#define BUF_LOG 0

// 固件缓冲区
static uint8_t *firmware_buffer = NULL;

// 升级状态机变量
static UpgradeState current_state = STATE_IDLE;
static uint32_t firmware_size = 0;
static uint32_t write_offset = 0;
static uint32_t verify_offset = 0;

// 升级AT线程
static XR_OS_Thread_t s_recovery_at_thread;
static XR_OS_Mutex_t s_recovery_at_mutex;
static uint8_t s_recovery_at_init = 0;
static uint32_t s_bootloader_version = 0;

char* get_bootloader_version_string() 
{
    static char version_str[32];  // 静态数组，注意线程安全
    
    uint8_t main_ver = (s_bootloader_version >> 24) & 0xFF;
    uint8_t sub1_ver = (s_bootloader_version >> 16) & 0xFF;
    uint8_t sub2_ver = (s_bootloader_version >> 8) & 0xFF;
    uint8_t rc_ver = s_bootloader_version & 0xFF;
    
    snprintf(version_str, sizeof(version_str), "%u.%u.%u.%u", 
                main_ver, sub1_ver, sub2_ver, rc_ver);
    
    printf("mcu bootloader version: %s\n", version_str);
    return version_str;
}

// 计算求和校验
static uint16_t calculate_sum_check(const uint8_t *data, uint16_t len) {
    uint32_t sum = 0;
    for (uint16_t i = 0; i < len; i++) {
        sum += data[i];
    }
    return (uint16_t)sum;
}

// 构建并发送数据包
static bool send_packet(uint8_t cmd, uint32_t addr, const uint8_t *data, uint16_t data_len) {
    uint16_t len_field = 1 + 4 + data_len; // CMD(1) + ADDR(4) + DATA
    uint16_t i = 0;
    // 检查数据包长度
    if (len_field + 3 > MAX_PACKET_SIZE) { // +3 for HEAD, LEN, CRC
        drv_loge("Error: Packet too large");
        return false;
    }
    
    // 构建数据包
    static uint8_t tx_buffer[MAX_PACKET_SIZE];
    tx_buffer[0] = PACKET_HEAD;
    tx_buffer[1] = len_field;
    tx_buffer[2] = cmd;
    tx_buffer[3] = (addr >> 0) & 0xFF;  // MSB
    tx_buffer[4] = (addr >> 8) & 0xFF;
    tx_buffer[5] = (addr >> 16) & 0xFF;
    tx_buffer[6] = (addr >> 24) & 0xFF; // LSB
    
    if (data && data_len > 0) {
        memcpy(&tx_buffer[7], data, data_len);
    }
    
    // 计算校验和
    uint16_t checksum;
    // 注意：只包含 [HEAD + LEN + CMD + ADDR + DATA]
    checksum = calculate_sum_check(&tx_buffer[0], len_field + 2); // HEAD(1) + LEN(1) + CMD+ADDR+DATA(len_field)
    drv_logi("checksum=0x%x len_field=%d\n", checksum, len_field);
    
    tx_buffer[7 + data_len] = checksum & 0xFF;
    tx_buffer[8 + data_len] = (checksum >> 8) & 0xFF;
    
    // 发送数据包
    uint16_t packet_len = 1 + 1 + len_field + 2; // HEAD + LEN + (CMD+ADDR+DATA) + CRC
    drv_logi("send len=%d\n", packet_len);
    for (i = 0; i < packet_len; i++) {
#if BUF_LOG
        printf("0x%x ", tx_buffer[i]);
#endif
        if (drv_mcu_iface_send_data(&tx_buffer[i], 1) != 1) {
            drv_loge("Error: Send packet failed\n");
            return false;
        }
    }
#if BUF_LOG
    printf("\n");
#endif
    printf("Sent packet: CMD=0x%02X, Addr=0x%08X, DataLen=%d\n", cmd, addr, data_len);
    return true;
}

// 接收并解析响应
static bool receive_response(uint8_t *resp_cmd, uint8_t *resp_data, uint16_t *resp_data_len) {
    static uint8_t rx_buffer[RX_BUFFER_SIZE];
    uint32_t timeout = 1000; // 1000ms超时
    uint16_t rx_len = 0;
    int i = 0;
    uint16_t data_len = 0;
    
    // 读取ACK
    uint8_t byte = 0;
    if (drv_mcu_iface_recv_data(&byte, 1, 200) == 1 && byte == RESP_SUCCESS) {
        rx_buffer[rx_len++] = byte;
        drv_logi("ack=0x%x\n", byte);
    } else {
        drv_loge("Error: Response ack timeout or invalid:0x%02x\n",byte);
        return false;
    }

    // 读取head字段
    if (drv_mcu_iface_recv_data(&byte, 1, 200) == 1 && byte == PACKET_HEAD) {
        rx_buffer[rx_len++] = byte;
        drv_logi("head=0x%x\n", byte);
    } else {
        drv_loge("Error: Response head timeout or invalid\n");
        return false;
    }
    
    // 读取len字段
    if (drv_mcu_iface_recv_data(&byte, 1, 200) == 1) {
        rx_buffer[rx_len++] = byte;
        drv_logi("len=0x%x\n", byte);
        data_len = byte - 1;
        *resp_data_len = byte;
    } else {
        drv_loge("Error: Response len timeout or invalid\n");
        return false;
    }

    // 读取协议层响应码字段
    if (drv_mcu_iface_recv_data(&byte, 1, 200) == 1 && byte == RESP_CMD_SUCCESS) {
        rx_buffer[rx_len++] = byte;
        *resp_cmd = byte;
        drv_logi("cmd ack=0x%x\n", byte);
    } else {
        drv_loge("Error: Response cmd timeout or invalid\n");
        return false;
    }
    // 读取data+crc字段
    if (drv_mcu_iface_recv_data(&rx_buffer[rx_len], data_len+2, timeout) == data_len+2) {
        rx_len += data_len;
        
        if (resp_data) {
            memcpy(resp_data, &rx_buffer[4], data_len);
        }
#if BUF_LOG
        printf("data:");
        for (i = 0; i < data_len; i++) {
            printf("0x%x  ", rx_buffer[4+i]);
        }
        printf("\n");
#endif
    } else {
        drv_loge("Error: Response data timeout\n");
        return false;
    }

    // 验证校验和
    uint16_t checksum = (rx_buffer[rx_len+1] << 8) | rx_buffer[rx_len];
    uint16_t calc_checksum =0;
    
    calc_checksum = calculate_sum_check(&rx_buffer[0], data_len + 4);
    drv_logi("calc_checksum: 0x%x checksum:0x%x 0x%x 0x%x\n", calc_checksum, checksum, rx_buffer[rx_len], rx_buffer[rx_len+1]);
    
    if (checksum != calc_checksum) {
        drv_loge("Error: Response CRC mismatch, expected=0x%04X, received=0x%04X\n", calc_checksum, checksum);
        return false;
    }
    
    printf("Received response: CMD=0x%02X, DataLen=%d\n", *resp_cmd, *resp_data_len);
    return true;
}

// 复位芯片
static bool reset_chip() {
    printf("复位芯片动作...\n");
    return send_packet(CMD_GO, 0, NULL, 0);
}

// 获取芯片ID
static bool get_chip_id(uint32_t *chip_id) {
    printf("获取芯片ID动作...\n");
    uint8_t resp_cmd;
    uint8_t resp_data[4];
    uint16_t resp_data_len;
    
    XR_OS_MutexLock(&s_recovery_at_mutex,XR_OS_WAIT_FOREVER);
    if (!send_packet(CMD_GET_ID, 0, NULL, 0)) {
        goto exit;
    }
    
    if (!receive_response(&resp_cmd, resp_data, &resp_data_len)) {
        goto exit;
    }
    
    if (resp_cmd != RESP_SUCCESS || resp_data_len < 4) {
        printf("获取芯片ID失败\n");
        goto exit;
    }

    XR_OS_MutexUnlock(&s_recovery_at_mutex);
    *chip_id = (resp_data[0] << 24) | (resp_data[1] << 16) | (resp_data[2] << 8) | resp_data[3];
    printf("芯片ID:0x%08X\n", *chip_id);
    return true;

exit:
    XR_OS_MutexUnlock(&s_recovery_at_mutex);
    return false;
}

// 获取版本信息
bool bootload_get_version(uint32_t *version) {
    printf("获取版本信息动作...\n");
    uint8_t resp_cmd;
    uint8_t resp_data[4];
    uint16_t resp_data_len;

    if (XR_OS_MutexIsValid(&s_recovery_at_mutex)){
        XR_OS_MutexLock(&s_recovery_at_mutex,XR_OS_WAIT_FOREVER);
    }
    
    if (!send_packet(CMD_GET_VERSION, 0, NULL, 0)) {
        goto exit;
    }
    
    if (!receive_response(&resp_cmd, resp_data, &resp_data_len)) {
        goto exit;
    }
    
    if (resp_cmd != RESP_SUCCESS || resp_data_len < 4) {
        printf("获取版本信息失败\n");
        goto exit;
    }

    if (XR_OS_MutexIsValid(&s_recovery_at_mutex)){
        XR_OS_MutexUnlock(&s_recovery_at_mutex);
    }

    *version = (resp_data[3] << 24) | (resp_data[2] << 16) | (resp_data[1] << 8) | resp_data[0];
    printf("版本信息：0x%08X\n", *version);
    s_bootloader_version = *version;        // 保存版本信息
    return true;

exit:
    if (XR_OS_MutexIsValid(&s_recovery_at_mutex)){
        XR_OS_MutexUnlock(&s_recovery_at_mutex);
    }
    return false;
}

// 擦除扇区
static bool erase_page(uint32_t addr) {
    printf("执行擦除扇区:0x%08X\n", addr);
    
    uint8_t resp_cmd = 0;
    uint16_t resp_data_len =0;
    
    XR_OS_MutexLock(&s_recovery_at_mutex,XR_OS_WAIT_FOREVER);
    if (!send_packet(CMD_ERASE_PAGE, addr, NULL, 0)) {
        goto exit;
    }
    
    if (!receive_response(&resp_cmd, NULL, &resp_data_len)) {
        goto exit;
    }
    
    if (resp_cmd != RESP_SUCCESS) {
        printf("扇区擦除失败\n");
        goto exit;
    }

    XR_OS_MutexUnlock(&s_recovery_at_mutex);
    return true;

exit:
    XR_OS_MutexUnlock(&s_recovery_at_mutex);
    return false;
}

// 擦除整个APP
static bool erase_chip() {
    printf("开始擦除动作...\n");
    
    // MCU应用区域地址范围
    uint32_t start_addr = CSBOOT_APP_BASE;
    uint32_t sector_size = 512;
    uint32_t end_addr = start_addr + CSBOOT_APP_SIZE;
    // uint32_t sector_size = 1024;
    
    bool success = true;
    uint32_t addr = start_addr;
    
    while (addr < end_addr) {
        if (!erase_page(addr)) {
            success = false;
            break;
        }
        addr += sector_size;
    }
    
    if (success) {
        printf("擦除动作完成\n");
    } else {
        printf("擦除动作失败\n");
    }
    
    return success;
}

// 写入固件数据
static bool write_firmware() {
    uint32_t addr = CSBOOT_APP_BASE + write_offset;
    uint16_t data_len = (firmware_size - write_offset > MAX_PAYLOAD_SIZE) ? MAX_PAYLOAD_SIZE : (firmware_size - write_offset);
    
    uint8_t resp_cmd;
    uint16_t resp_data_len;
    XR_OS_MutexLock(&s_recovery_at_mutex,XR_OS_WAIT_FOREVER);
    if (!send_packet(CMD_WRITE_MEMORY, addr, firmware_buffer+write_offset, data_len)) {
        goto exit;
    }
    
    if (!receive_response(&resp_cmd, NULL, &resp_data_len)) {
        goto exit;
    }
    
    if (resp_cmd != RESP_SUCCESS) {
        printf("数据写入失败\n");
        goto exit;
    }

    XR_OS_MutexUnlock(&s_recovery_at_mutex);
    write_offset += data_len;
    return true;

exit:
    XR_OS_MutexUnlock(&s_recovery_at_mutex);
    return false;
}

// 读取固件数据
static bool read_firmware(uint32_t addr, uint8_t *data, uint16_t len) {
    uint8_t resp_cmd;
    uint16_t resp_data_len;

    XR_OS_MutexLock(&s_recovery_at_mutex,XR_OS_WAIT_FOREVER);
    if (!send_packet(CMD_READ_MEMORY, addr, (uint8_t*)&len, 1)) {
        goto exit;
    }
    
    if (!receive_response(&resp_cmd, data, &resp_data_len)) {
        goto exit;
    }
    
    if (resp_cmd != RESP_SUCCESS || resp_data_len != len) {
        printf("数据读取失败\n");
        goto exit;
    }
    
    XR_OS_MutexUnlock(&s_recovery_at_mutex);
    return true;

exit:
    XR_OS_MutexUnlock(&s_recovery_at_mutex);
    return false;
}

// 验证固件数据
static bool verify_firmware() {
    printf("开始校验动作...\n");
    
    uint32_t verify_size = MAX_PAYLOAD_SIZE; // 每次验证240字节
    bool success = true;
    
    while (verify_offset < firmware_size) {
        uint32_t addr = CSBOOT_APP_BASE + verify_offset;
        uint16_t verify_len = (firmware_size - verify_offset > verify_size) ? verify_size : (firmware_size - verify_offset);
        
        uint8_t mcu_data[MAX_PAYLOAD_SIZE];
        if (!read_firmware(addr, mcu_data, verify_len)) {
            success = false;
            break;
        }
        
        if (memcmp(firmware_buffer + verify_offset, mcu_data, verify_len) != 0) {
            char buffer[64];
            sprintf(buffer, "地址0x%08X处数据校验失败\n", addr);
            printf(buffer);
            success = false;
            break;
        }
        
        verify_offset += verify_len;
    }
    
    if (success) {
        printf("校验动作完成\n");
    } else {
        printf("校验动作失败\n");
    }
    
    return success;
}

/***************************************************************
  *  @brief    跳转至App   
  *  @param     @args:   0:擦除升级标志位 1:跳转到app
  *  @note      
  *  @Sample usage: 
  * 	
 **************************************************************/
bool mcu_jump_to_app(uint8_t arg) {
    uint8_t resp_cmd;
    uint16_t resp_data_len;
    
    if (XR_OS_MutexIsValid(&s_recovery_at_mutex)){
        XR_OS_MutexLock(&s_recovery_at_mutex,XR_OS_WAIT_FOREVER);
    }
    if (!send_packet(CMD_GO, 0, &arg, 1)) {
        goto exit;
    }
    
    if (!receive_response(&resp_cmd, NULL, &resp_data_len)) {
        goto exit;
    }
    
    if (resp_cmd != RESP_SUCCESS) {
        printf("mcu_jump_to_app error\n");
        goto exit;
    }

    if (XR_OS_MutexIsValid(&s_recovery_at_mutex)){
        XR_OS_MutexUnlock(&s_recovery_at_mutex);
    }
    
    if (!arg) {
        printf("mcu upgrade success,wait reboot cmd\n");
    }else{
        printf("mcu jump to app success\n");
    }

    return true;

exit:
    if (XR_OS_MutexIsValid(&s_recovery_at_mutex)){
        XR_OS_MutexUnlock(&s_recovery_at_mutex);
    }
    return false;
}

int mcu_clear_rx_buffer(void)
{
    char *data = calloc(1,128);
    if (data == NULL) {
        drv_loge("calloc failed\n");
        return -1;
    }

    memset(data,0,128);
    drv_mcu_iface_recv_data(data, 128, 150);
    drv_logw("filter recv data:%s\n",data);
    
    drv_safe_free(data);
    return 0;
}

int mcu_firmware_update_load_info(char *buffer,int size) 
{
    firmware_buffer = buffer;
    firmware_size = size;
    if (NULL == firmware_buffer){
        printf("mcu firmware buffer is null");
        return -1;
    }

    write_offset = 0;
    verify_offset = 0;
    current_state = STATE_GET_CHIP_ID;
    printf("加载固件成功，大小：%d字节\n", firmware_size);
    return 0;
}

// 升级状态机处理
void upgrade_state_machine() 
{
    switch (current_state) {
        case STATE_IDLE:
            {

            } 
            break;
            
        case STATE_GET_CHIP_ID:
            {
                uint32_t chip_id;
                if (get_chip_id(&chip_id)) {
                    current_state = STATE_GET_VERSION;
                } else {
                    current_state = STATE_ERROR;
                }
            }
            break;
            
        case STATE_GET_VERSION:
            {
                uint32_t version;
                if (bootload_get_version(&version)) {
                    current_state = STATE_ERASE_CHIP;
                } else {
                    current_state = STATE_ERROR;
                }
            }
            break;
            
        case STATE_ERASE_CHIP:
            if (erase_chip()) {
                current_state = STATE_WRITE_FIRMWARE;
            } else {
                current_state = STATE_ERROR;
            }
            break;
            
        case STATE_WRITE_FIRMWARE:
            if (write_offset < firmware_size) {
                if (!write_firmware()) {
                    current_state = STATE_ERROR;
                }
            } else {
                // current_state = STATE_VERIFY_FIRMWARE;
                current_state = STATE_JUMP_TO_APP;
            }
            break;
            
        case STATE_VERIFY_FIRMWARE:
            if (verify_firmware()) {
                current_state = STATE_JUMP_TO_APP;
            } else {
                current_state = STATE_ERROR;
            }
            break;
            
        case STATE_JUMP_TO_APP:
            if (mcu_jump_to_app(0)) {
                current_state = STATE_FINISHED;
            } else {
                current_state = STATE_ERROR;
            }
            break;
            
        case STATE_FINISHED:
            // printf("固件升级成功!");
            break;
            
        case STATE_ERROR:
            // printf("固件升级失败!");
            break;
            
        default:
            current_state = STATE_IDLE;
            break;
    }
}

/***************************************************************
  *  @brief     MCU 升级写入偏移量获取
  *  @param  
  *  @note      
  *  @Sample usage: 
 **************************************************************/
int mcu_firmware_update_get_offset()
{
    if (current_state == STATE_ERROR) {
        return -1;
    }

    return write_offset;
}

/***************************************************************
  *  @brief     MCU 升级状态获取
  *  @param  
  *  @note      -1 : 升级失败
  *             0 : 升级完成
  *             1 : 升级中
  *  @Sample usage: 
 **************************************************************/
int mcu_firmware_update_get_state()
{
    if (current_state == STATE_ERROR) {
        return -1;
    } else if(current_state == STATE_FINISHED) {
        return 0;
    } else {
        return 1;
    }
}

/***************************************************************
  *  @brief     MCU 重置升级状态
  *  @param  
  *  @note      
  *  @Sample usage: 
 **************************************************************/
int mcu_firmware_update_reset_state()
{
    current_state = STATE_IDLE;
}

//mcu recovery 指令
/***************************************************************
  *  @brief     MCU recovery at 控制指令
  *  @param     cmd:指令码,内部会加上控制指令基值
  *  @param     data:数据
  *  @param     resp_data_len:数据长度
  *  @note      不需要获取数据回执的通用控制接口
  *  @Sample usage: mcu_recovery_at_control(AT_REQ_CMD_TP_RESET_SET, &val,1);
 **************************************************************/
int mcu_recovery_at_control(uint8_t cmd,uint8_t *data, uint16_t data_len)
{
    XR_OS_MutexLock(&s_recovery_at_mutex,XR_OS_WAIT_FOREVER);
    drv_logd("mcu_recovery_at_control cmd: %d...\n", cmd);
    uint8_t resp_cmd;
    uint8_t resp_data[4];
    uint16_t resp_data_len;
    
    if (!send_packet(CONTROL_CMD_BASE, 0, data, data_len)) {
        goto error;
    }
    
    if (!receive_response(&resp_cmd, resp_data, &resp_data_len)) {
        goto error;
    }
    
    if (resp_cmd != RESP_SUCCESS ) {
        goto error;
    }
    
    XR_OS_MutexUnlock(&s_recovery_at_mutex);
    return 0;

error:
    drv_loge("mcu_recovery_at_control [%d] error\n", cmd);
    XR_OS_MutexUnlock(&s_recovery_at_mutex);
    return -1;
}

/***************************************************************
  *  @brief     MCU recovery at 控制指令
  *  @param     cmd:指令码,内部会加上控制指令基值
  *  @param     data:数据
  *  @param     data_len:数据长度
  *  @note     
  *  @Sample usage: 
 **************************************************************/
int mcu_recovery_at_control_for_reply(uint8_t cmd, uint8_t *data, uint16_t data_len)
{
    XR_OS_MutexLock(&s_recovery_at_mutex,XR_OS_WAIT_FOREVER);
    uint8_t resp_cmd;
    uint8_t resp_data[4];
    uint16_t resp_data_len;
    
    if (!send_packet(CONTROL_CMD_BASE, 0, data, data_len)) {
        goto error;
    }
    
    if (!receive_response(&resp_cmd, resp_data, &resp_data_len)) {
        goto error;
    }
    
    if (resp_cmd != RESP_SUCCESS || resp_data_len < 1) {
        goto error;
    }

    data[2] = resp_data[0];
    XR_OS_MutexUnlock(&s_recovery_at_mutex);
    return 0;

error:
    drv_loge("mcu_recovery_at_control_for_reply [%d] error\n", cmd);
    XR_OS_MutexUnlock(&s_recovery_at_mutex);
    return -1;
}

/***************************************************************
  *  @brief     MCU 同步到bootloader系统
  *  @param  
  *  @note      
  *  @Sample usage: 
 **************************************************************/
int mcu_sync_bootloader()
{
    uint32_t bootload_version;
    char *data = calloc(1,128);
    if (data == NULL) {
        drv_loge("calloc failed\n");
        goto exit;
    }

    for (int i = 0; i < 4; i++) {
        memset(data,0,128);
        drv_mcu_iface_recv_data(data, 128, 150);
        drv_logw("filter recv data:%s\n",data);
        if (bootload_get_version(&bootload_version)) {     //mcu在 bootloader,同步成功,正常启动
            drv_loge("mcu in bootloader, sync success");
            goto exit;
        } 

        //清除缓冲区
        memset(data,0,128);
        drv_mcu_iface_recv_data(data, 128, 150);
        drv_logw("filter recv data:%s\n",data);

        //mcu可能在app,尝试切换到bootloader
        drv_mcu_iface_send_data("AT+UPGRADE\r\n",strlen("AT+UPGRADE\r\n"));
        drv_mcu_iface_recv_data(data, 128, 150);
        drv_logw("recv data:%s\n",data);
        if(strstr(data, "READY") != NULL) {
            drv_loge("mcu goto bootloader success,wait sync");
        }
    }

    drv_loge("mcu sync failed,please call developer!!!!!!\n");
    drv_loge("mcu sync failed,please call developer!!!!!!\n");
    drv_loge("mcu sync failed,please call developer!!!!!!\n");

    while (1) {
        XR_OS_MSleep(5000);
        drv_loge("mcu sync failed,please call developer!!!!!!\n");
    }

exit:
    drv_safe_free(data);
    return 0;
}

int mcu_recovery_at_thread()
{
    int status = -1;
    int retry = 0;
    int val = 0;

    //重置mcu升级状态,并确认已经切到bootload
    printf("AT step [0]: Reset upgrade && Confirm bootload\n");

    //同步mcu 状态
    mcu_sync_bootloader();

    //开始处理mcu指令
    printf("AT step [1]: Handle CMD\n");

    // 初始化升级状态机
    s_recovery_at_init = 1;
    current_state = STATE_IDLE;

    // 主循环
    while (1) {
        // 处理升级状态机
        upgrade_state_machine();
        
        XR_OS_MSleep(10);
    }

exit:
    if (XR_OS_MutexIsValid(&s_recovery_at_mutex)){
        XR_OS_MutexDelete(&s_recovery_at_mutex);
    }
    return 0;
}

int mcu_recovery_at_thread_start()
{
    XR_OS_Status status = XR_OS_OK;
    if (XR_OS_OK != (status = XR_OS_MutexCreate(&s_recovery_at_mutex))) {
        drv_loge("[s_recovery_at_mutex] create mutex fail");
        goto exit;
    }

    status = XR_OS_ThreadCreate(&s_recovery_at_thread,
                                    "mcu_recovery_at_thread",
                                    mcu_recovery_at_thread,
                                    NULL,
                                    XR_OS_PRIORITY_NORMAL,
                                    (10 * 1024));
    if (XR_OS_OK != status) {
        drv_loge("[guliteos] init task create fail.\n");
        goto exit;
    }

    //等待bootload确认完成
    while(s_recovery_at_init == 0) {
        printf("wait mcu recovery at init\n");
        XR_OS_MSleep(100);
    }

    return 0;

exit:
    if (XR_OS_MutexIsValid(&s_recovery_at_mutex)){
        XR_OS_MutexDelete(&s_recovery_at_mutex);
    }
    return -1;
}



