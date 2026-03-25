#ifndef _DEVICE_WRAPPER_H_
#define _DEVICE_WRAPPER_H_

#include "../../../mod_realize/realize_unit_device/realize_unit_device.h"

typedef int (*device_get_volume_t)(void);
typedef int (*device_set_volume_t)(unsigned short volume);

typedef int (*device_set_rtc_t)(void *str_time);

typedef void (*device_poweroff_t)(void);
typedef void (*device_poweron_t)(void);
typedef void (*device_reboot_t)(void);
typedef void (*device_deep_sleep_t)(int time);

typedef int (*device_get_brightness_t)(void);
typedef int (*device_set_brightness_t)(int bright);
typedef int (*device_switch_backlight_t)(drv_backlight_type_t type);

typedef char *(*device_get_chip_id_t)(void);
typedef char *(*device_get_mac_t)(void);

typedef void (*device_init_earphone_t)(void);
typedef int (*device_get_earphone_state_t)(void);
typedef void (*device_deinit_earphone_t)(void);
typedef void (*device_adb_control_t)(int comd);
typedef char *(*device_get_sn_t)(void);
typedef char *(*device_get_version_t)(void);
typedef unsigned int (*device_get_cpu_usage_rate_t)(unsigned int duration_ms);
typedef struct {
	device_get_volume_t get_volume;
	device_set_volume_t set_volume;
	device_set_rtc_t set_rtc;
	device_poweroff_t poweroff;
	device_poweron_t poweron;
	device_reboot_t reboot;
	device_get_brightness_t get_brightness;
	device_set_brightness_t set_brightness;
	device_switch_backlight_t switch_backlight;
	device_get_chip_id_t get_chip_id;
	device_get_mac_t get_mac;
	device_init_earphone_t earphone_init;
	device_get_earphone_state_t get_earphone_state;
	device_deinit_earphone_t earphone_deinit;
	device_deep_sleep_t deep_sleep;
	device_adb_control_t adb_control;
    device_get_sn_t get_sn;
    device_get_version_t get_version;
    device_get_cpu_usage_rate_t get_cpu_usage_rate;
} device_wrapper_t;

#endif

