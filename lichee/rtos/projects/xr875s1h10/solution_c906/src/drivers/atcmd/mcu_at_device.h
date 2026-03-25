#ifndef __MCU_AT_DEVICE_H__
#define __MCU_AT_DEVICE_H__ 

#include "stdio.h"
#include "kernel/os/os_thread.h"
#include "kernel/os/os.h"

typedef enum {
    MCU_AT_CMD_DEV_CONTROL = 0,
    MCU_AT_CMD_MAX,
} MCU_AT_COMMAND_E;

typedef struct {
    MCU_AT_COMMAND_E cmd;   // 命令索引
    char *p_send;   // 发送字符串指针  
    uint32_t wait_ack_dalay1ms;  //发送后查询返回指令最大响应时间，1ms为单位。
	uint8_t  max_try_times;      //最大重试次数
} mcu_at_cmd_config_t;


/*此处枚举与mcu保持一致，需同步修改*/
typedef enum {
	WAKE_SRC_ERROR 				=   -1,			//获取唤醒源超时等错误
	WAKE_SRC_NULL				=	0,			//无唤醒
	WAKE_SRC_USB				=	1,			//USB 唤醒
	WAKE_SRC_TOUCHKEY			=	2,			//触摸按键唤醒
	WAKE_SRC_VOICE				=	3,			//语音模块唤醒
	WAKE_SRC_RTC				=	4,			//RTC唤醒
	WAKE_SRC_OTHER								//其他唤醒
} mcu_wake_src_t;

int mcu_at_device_init(void);
int drv_mcu_iface_irq_init_default();
uint8_t mcu_at_uart_is_sleep();
void mcu_at_uart_suspend(void);
void mcu_at_uart_resume(void);
int dev_mcu_wakeSrc_get(void);

int dev_mcu_ext_vcc_set(uint8_t val);
int dev_mcu_sys_vcc_set(uint8_t val);
int dev_mcu_lcd_reset_set(uint8_t val);
int dev_mcu_cam_pwdn_set(uint8_t val);
int dev_mcu_lcd_bl_lp_set(uint8_t val);
int dev_mcu_extvcc_control(uint8_t val);
int dev_mcu_charge_info_get(char *isCharge, char *isFull);
uint8_t dev_mcu_charge_det_get(void);
uint8_t dev_mcu_charge_full_det_get(void);
int dev_mcu_robot_vcc_set(uint8_t val);
int dev_mcu_irq_set(uint8_t val);
int dev_mcu_sleep_set(int mode,int arg);
int dev_mcu_version_get(char **version);
int dev_mcu_sync();
int dev_mcu_upgrade();

int mcu_at_mutex_lock(char *funcuton, int line);
int mcu_at_mutex_unlock(char *funcuton, int line);

#endif
