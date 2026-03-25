#ifndef _REALIZE_UNIT_DEVICE_H_ 
#define _REALIZE_UNIT_DEVICE_H_

typedef enum {
	DRV_DISP_ROT_NONE = 0,
    DRV_DISP_ROT_90,
    DRV_DISP_ROT_180,
    DRV_DISP_ROT_270,
    DRV_DISP_ROT_MAX
} drv_lcd_rotation_t;

typedef enum {
    DRV_BACKLIGHT_PWM = 0,
    DRV_BACKLIGHT_GPIO,
    DRV_BACKLIGHT_MAX
} drv_backlight_type_t;

/*******************************************************************************************/
/*******************************************音量接口*****************************************/
/*******************************************************************************************/
int realize_unit_drv_set_volume(unsigned short volume);
int realize_unit_drv_get_volume(void);

/*******************************************************************************************/
/******************************************开关机接口*****************************************/
/*******************************************************************************************/
void realize_unit_drv_poweroff(void);
void realize_unit_drv_poweron(void);
void realize_unit_drv_reboot(void);
unsigned char realize_unit_drv_get_poweroff_during(void);
void realize_unit_drv_set_poweroff_during(unsigned char state);
void realize_unit_drv_force_poweroff_init(void);

/*******************************************************************************************/
/****************************************熄屏背光接口*****************************************/
/*******************************************************************************************/
int realize_unit_drv_set_brightness(int brightness);
int realize_unit_drv_get_brightness(void);
int realize_unit_drv_switch_backlight(drv_backlight_type_t type);

/*******************************************************************************************/
/*******************************************rtc接口******************************************/
/*******************************************************************************************/
#define REALIZE_UNIT_DRV_RTC_NTP_SERVER_HOST  "pool.ntp.org"
#define REALIZE_UNIT_DRV_RTC_NTP_SERVER_HOST1  "ntp1.aliyun.com"
#define REALIZE_UNIT_DRV_RTC_NTP_SERVER_HOST2  "ntp2.aliyun.com"

int realize_unit_drv_cal_rtc(void);

/*******************************************************************************************/
/************************************获取chip id接口****************************************/
/*******************************************************************************************/
char *realize_unit_drv_get_chip_id(void);

/*******************************************************************************************/
/**************************************获取mac接口******************************************/
/*******************************************************************************************/
char *realize_unit_drv_get_mac(void);

/*******************************************************************************************/
/************************************设置屏幕旋转角度*****************************************/
/*******************************************************************************************/
int realize_unit_drv_set_disp_rotation(int angle);

/*******************************************************************************************/
/************************************设置左右手模式*****************************************/
/*******************************************************************************************/
int realize_unit_drv_set_handedness(int mode);

void realize_unit_drv_adb_control(int comd);

void realize_unit_drv_earphone_init();
int realize_unit_drv_get_earphone_state();
void realize_unit_drv_earphone_deinit();

int realize_unit_drv_get_time_ms();
void realize_unit_drv_deep_sleep(int time);
#endif

