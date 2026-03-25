#include "sdk_runtime.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "console.h"
#include "drivers/rtc/dev_rtc.h"
#include <hal_lcd_fb.h>

#define SDK_RUNTIME_DEFAULT_BACKLIGHT 100
#define SDK_RUNTIME_DEFAULT_P2P_IP "255.255.255.255"
#define SDK_RUNTIME_LCD_SCREEN_ID 0

static int sdk_runtime_percent_to_raw(int percent)
{
    if (percent <= 0) {
        return 0;
    }

    if (percent > 100) {
        percent = 100;
    }

    return (percent * 255 + 99) / 100;
}

static int sdk_runtime_raw_to_percent(int raw)
{
    if (raw <= 0) {
        return 0;
    }

    if (raw > 255) {
        raw = 255;
    }

    return (raw * 100 + 254) / 255;
}

typedef struct {
    int poweroff_status;
    int backlight_percent;
    char p2p_wakeup_ip[16];
} sdk_runtime_state_t;

static sdk_runtime_state_t s_runtime_state = {
    .poweroff_status = 0,
    .backlight_percent = SDK_RUNTIME_DEFAULT_BACKLIGHT,
    .p2p_wakeup_ip = SDK_RUNTIME_DEFAULT_P2P_IP,
};

int sdk_runtime_get_poweroff_status(void)
{
    return s_runtime_state.poweroff_status;
}

void sdk_runtime_set_poweroff_status(int status)
{
    if (status < 0 || status > 1) {
        return;
    }

    s_runtime_state.poweroff_status = status;
}

int sdk_runtime_get_backlight_percent(void)
{
    return s_runtime_state.backlight_percent;
}

int sdk_runtime_set_backlight_percent(int bright)
{
    int raw;
    int ret;

    if (bright <= 0) {
        bright = 1;
    } else if (bright > 100) {
        bright = 100;
    }

    s_runtime_state.backlight_percent = bright;
    raw = sdk_runtime_percent_to_raw(bright);
    ret = bsp_disp_lcd_set_bright(SDK_RUNTIME_LCD_SCREEN_ID, raw);
    if (ret != 0) {
        printf("[sdk_runtime] set backlight failed: percent=%d raw=%d ret=%d\n",
               bright, raw, ret);
    }

    return bright;
}

int sdk_runtime_get_backlight_raw(void)
{
    return bsp_disp_lcd_get_bright(SDK_RUNTIME_LCD_SCREEN_ID);
}

char *sdk_runtime_get_p2p_wakeup_ip(void)
{
    if (s_runtime_state.p2p_wakeup_ip[0] == '\0') {
        (void)sdk_runtime_set_p2p_wakeup_ip(SDK_RUNTIME_DEFAULT_P2P_IP);
    }

    return s_runtime_state.p2p_wakeup_ip;
}

int sdk_runtime_set_p2p_wakeup_ip(const char *ip)
{
    if (ip == NULL || ip[0] == '\0') {
        ip = SDK_RUNTIME_DEFAULT_P2P_IP;
    }

    strncpy(s_runtime_state.p2p_wakeup_ip, ip, sizeof(s_runtime_state.p2p_wakeup_ip) - 1);
    s_runtime_state.p2p_wakeup_ip[sizeof(s_runtime_state.p2p_wakeup_ip) - 1] = '\0';
    return 0;
}

int sdk_runtime_sync_time_from_rtc(void)
{
    drv_rtc_time_fmt_t rtc_tm = {0};
    int ret = dev_cali_rtc_get_time(&rtc_tm);
    if (ret != DRV_OK) {
        printf("[sdk_runtime] read rtc failed: %d\n", ret);
        return ret;
    }

    printf("[sdk_runtime] sync rtc time %04d-%02d-%02d %02d:%02d:%02d\n",
           rtc_tm.year, rtc_tm.month, rtc_tm.day,
           rtc_tm.hour, rtc_tm.minute, rtc_tm.second);

    ret = dev_rtc_set_time(&rtc_tm);
    if (ret != DRV_OK) {
        printf("[sdk_runtime] set rtc failed: %d\n", ret);
    }

    return ret;
}

static int cmd_sdk_backlight(int argc, char **argv)
{
    int raw;

    if (argc == 1) {
        raw = sdk_runtime_get_backlight_raw();
        printf("backlight: percent=%d raw=%d\n",
               sdk_runtime_get_backlight_percent(),
               raw);
        return 0;
    }

    if (argc != 2) {
        printf("usage: backlight [percent]\n");
        return -1;
    }

    sdk_runtime_set_backlight_percent(atoi(argv[1]));
    raw = sdk_runtime_get_backlight_raw();
    printf("backlight set: percent=%d raw=%d\n",
           sdk_runtime_get_backlight_percent(),
           raw);
    return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_sdk_backlight, backlight, get or set lcd backlight percent);
