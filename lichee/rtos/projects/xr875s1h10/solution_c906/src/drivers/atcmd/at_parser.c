#include "at_parser.h"
#include <string.h>

#define CMD_BUF_SIZE 64

static uint8_t cmd_buf[2][CMD_BUF_SIZE] = {0}; // 双缓冲
static uint8_t current_buf_index = 0;          // 当前使用的缓冲区索引
static uint8_t recv_complete_buf_index = 0;          // 接收完成的缓冲区索引
static uint8_t cmd_index = 0;
static uint8_t expect_AT = 1; // 是否正在等待 "AT"
static uint8_t resp_parsing = 0; // 是否正在解析响应

/**
 * @brief 解析AT数据
 * 
 * @param byte 
 * @return int 0: 正常处理 1: 收到AT指令 2: 收到AT响应数据
 */
int at_parser_feed(uint8_t byte)
{
    if (expect_AT) {
        // 切换到另一个缓冲区
        current_buf_index = (current_buf_index + 1) % 2;
        // printf("--> cur index=%d\n", current_buf_index);
        cmd_index = 0;

        // 可能开始AT指令或响应
        if (byte == 'A') {
            cmd_buf[current_buf_index][0] = 'A';
            cmd_index = 1;
            expect_AT = 0;
            resp_parsing = 0;
            return 0;
        } 
        // 开始响应解析（+RES:/OK/ERROR）
        else if (byte == '+' || byte == 'O' || byte == 'E') {
            cmd_buf[current_buf_index][0] = byte;
            cmd_index = 1;
            expect_AT = 0;
            resp_parsing = 1;
            return 0;
        }
        return 0;
    }

    uint8_t (*cmd_buf_current)[CMD_BUF_SIZE] = &cmd_buf[current_buf_index];

    if (resp_parsing) {
        // 存储响应数据
        if (cmd_index < CMD_BUF_SIZE - 1) {
            (*cmd_buf_current)[cmd_index++] = byte;
        }
        
        // 检测OK结束符
        if (cmd_index >= 4 && memcmp((*cmd_buf_current) + cmd_index - 4, "OK\r\n", 4) == 0) {
            (*cmd_buf_current)[cmd_index] = '\0';
            cmd_index = 0;
            expect_AT = 1;
            resp_parsing = 0;
            recv_complete_buf_index = current_buf_index;
            return 2;
        }
        
        // 检测ERROR结束符
        if (cmd_index >= 7 && memcmp((*cmd_buf_current) + cmd_index - 7, "ERROR\r\n", 7) == 0) {
            (*cmd_buf_current)[cmd_index] = '\0';
            cmd_index = 0;
            expect_AT = 1;
            resp_parsing = 0;
            recv_complete_buf_index = current_buf_index;
            return 2;
        }
        return 0;
    } 
    else {
        // AT指令解析逻辑
        if (cmd_index == 1 && byte == 'T') {
            (*cmd_buf_current)[1] = 'T';
            cmd_index = 2;
            return 0;
        }

        if (cmd_index == 2 && byte == '+') {
            (*cmd_buf_current)[2] = '+';
            cmd_index = 3;
            return 0;
        }

        if (byte == '\r' || byte == '\n') {
            (*cmd_buf_current)[cmd_index] = '\0';
            cmd_index = 0;
            expect_AT = 1;
            recv_complete_buf_index = current_buf_index;
            return 1;
        }

        if (cmd_index < CMD_BUF_SIZE - 1) {
            (*cmd_buf_current)[cmd_index++] = byte;
        } else {
            cmd_index = 0;
            expect_AT = 1;
            return 0;
        }
        return 0;
    }
}

/**
 * @brief 获取当前已接收到的AT命令或响应（只返回有数据的缓冲区）
 *
 * @return const char* 返回命令字符串指针，若无数据则返回 NULL
 */
const char* at_parser_get_cmd(void)
{
    // printf("--> complete index=%d\n", recv_complete_buf_index);
    return (const char *)cmd_buf[recv_complete_buf_index];
}
