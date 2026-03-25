#ifndef _DEV_SD_H_
#define _DEV_SD_H_

#include "../drv_common.h"

typedef enum {
    SD_STATE_REMOVE = 0,
    SD_STATE_INSERT,
} drv_sd_state_t;

typedef void (*sd_detect_cb)(drv_sd_state_t state);

/**
 * @brief  SD 卡检测初始化
 */
drv_status_t dev_sd_detect_init(void);

/**
 * @brief  SD 卡检测反初始化
 */
void dev_sd_detect_deinit(void);

/**
 * @brief  SD 卡检测状态
 * @return 1:已插入, 0:已弹出
 */
int dev_sd_detect_status(void);

/**
 * @brief  SD 卡检测回调函数注册
 */
void dev_sd_detect_cb_register(sd_detect_cb cb);

void dev_sd_suspend(void);
void dev_sd_resume(void);


#endif
