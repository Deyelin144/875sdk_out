#ifndef SDK_WIFI_WRAPPER_H
#define SDK_WIFI_WRAPPER_H

int sdk_wifi_wrapper_init(void);
int sdk_wifi_wrapper_scan(void);
int sdk_wifi_wrapper_connect(const char *ssid, const char *password);

#endif
