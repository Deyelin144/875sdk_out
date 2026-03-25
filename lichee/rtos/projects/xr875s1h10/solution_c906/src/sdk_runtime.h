#ifndef __SDK_RUNTIME_H__
#define __SDK_RUNTIME_H__

int sdk_runtime_get_poweroff_status(void);
void sdk_runtime_set_poweroff_status(int status);

int sdk_runtime_get_backlight_percent(void);
int sdk_runtime_set_backlight_percent(int bright);
int sdk_runtime_get_backlight_raw(void);

char *sdk_runtime_get_p2p_wakeup_ip(void);
int sdk_runtime_set_p2p_wakeup_ip(const char *ip);

int sdk_runtime_sync_time_from_rtc(void);

#endif
