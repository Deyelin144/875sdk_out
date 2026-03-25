
#ifndef __SERIAL_TRANS_WRAPPER__H__
#define __SERIAL_TRANS_WRAPPER__H__

/** 
 * @brief 初始化，打开串口等操作
 * @param
 * @return 成功0，失败返回-1
 */
typedef int (*serial_trans_init_t)(void);

/**
 * @brief 读取一个字符
 * @param
 * @return 成功返回读取到的字符，失败返回-1
 */
typedef int (*serial_trans_recv_t)(char *path);

/**
 * @brief 发送一个字符
 * @param
 * @return 成功返回发送字符数，失败返回-1
 */
typedef int (*serial_trans_send_t)(char *path, unsigned char c);

/**
 * @brief 去初始化，关闭串口等操作
 * @param
 * @return 成功返回0，失败返回-1
 */
typedef int (*serial_trans_deinit_t)(void);

typedef struct {
    serial_trans_init_t serial_trans_init;
    serial_trans_recv_t serial_trans_recv;
    serial_trans_send_t serial_trans_send;
    serial_trans_deinit_t serial_trans_deinit;
} serial_trans_wrapper_t;

#endif