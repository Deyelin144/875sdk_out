#include <stdio.h>
#include <string.h>

#include "sdk_exception_runtime.h"

#define SDK_EXCEPTION_LOG_LIMIT (50 * 1024)

static unsigned char s_exception_status = 0;
static char s_exception_log[SDK_EXCEPTION_LOG_LIMIT];
static int s_exception_log_len = 0;

void exception_wrapper_set_ex_status(unsigned char status)
{
    s_exception_status = status;
}

unsigned char exception_wrapper_get_ex_status(void)
{
    return s_exception_status;
}

int exception_wrapper_save_log_append(char *buffer, int len, unsigned char finish)
{
    int copy_len;

    if (buffer == NULL || len <= 0) {
        return 0;
    }

    copy_len = len;
    if (copy_len > SDK_EXCEPTION_LOG_LIMIT - s_exception_log_len) {
        copy_len = SDK_EXCEPTION_LOG_LIMIT - s_exception_log_len;
    }

    if (copy_len > 0) {
        memcpy(s_exception_log + s_exception_log_len, buffer, copy_len);
        s_exception_log_len += copy_len;
    }

    if (finish) {
        printf("[sdk_exception] captured %d bytes of exception log\n", s_exception_log_len);
    }

    return (copy_len == len) ? 0 : -1;
}

void sdk_exception_reset_capture(void)
{
    s_exception_log_len = 0;
    memset(s_exception_log, 0, sizeof(s_exception_log));
}

int sdk_exception_get_capture_buffer(char **buffer, int *len)
{
    if (buffer != NULL) {
        *buffer = s_exception_log;
    }

    if (len != NULL) {
        *len = s_exception_log_len;
    }

    return 0;
}
