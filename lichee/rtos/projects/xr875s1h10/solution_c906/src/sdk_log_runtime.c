#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "include/mod_realize/realize_unit_log/realize_unit_log.h"
#include "include/mod_realize/realize_unit_log/realize_unit_log_upload.h"

#define SDK_LOG_BUFFER_SIZE 512
#define SDK_LOG_TYPE_COUNT  2

static realize_unit_log_ctx_t s_log_ctx;
static bool s_log_ctx_valid = false;
static unit_log_hook_t *s_log_hook = NULL;
static bool s_log_output_to_terminal = true;
static int s_log_levels[SDK_LOG_TYPE_COUNT] = {
    REALIZE_UNIT_LOG_LEVEL_INFO,
    REALIZE_UNIT_LOG_LEVEL_INFO,
};

static void sdk_log_output(const char *buffer)
{
    if (buffer == NULL || buffer[0] == '\0') {
        return;
    }

    if (s_log_output_to_terminal) {
        if (s_log_hook != NULL && s_log_hook->print != NULL) {
            s_log_hook->print((char *)buffer);
        } else {
            fputs(buffer, stdout);
        }
    }
}

void realize_unit_log_raw(const char *format, ...)
{
    char buffer[SDK_LOG_BUFFER_SIZE];
    va_list ap;

    if (format == NULL) {
        return;
    }

    va_start(ap, format);
    vsnprintf(buffer, sizeof(buffer), format, ap);
    va_end(ap);
    sdk_log_output(buffer);
}

void _realize_unit_log_add(realize_unit_log_type_t type,
                           realize_unit_log_level_t level,
                           const char *file,
                           int line,
                           const char *func,
                           const char *format,
                           ...)
{
    char buffer[SDK_LOG_BUFFER_SIZE];
    va_list ap;

    (void)type;
    (void)level;
    (void)file;
    (void)line;
    (void)func;

    if (format == NULL) {
        return;
    }

    va_start(ap, format);
    vsnprintf(buffer, sizeof(buffer), format, ap);
    va_end(ap);
    sdk_log_output(buffer);
}

void realize_unit_log_ctx_init(realize_unit_log_ctx_t *ctx)
{
    if (ctx == NULL) {
        memset(&s_log_ctx, 0, sizeof(s_log_ctx));
        s_log_ctx_valid = false;
        return;
    }

    s_log_ctx = *ctx;
    s_log_ctx_valid = true;
}

realize_unit_log_ctx_t *realize_unit_log_get_ctx(void)
{
    return s_log_ctx_valid ? &s_log_ctx : NULL;
}

void realize_unit_log_ctx_deinit(void)
{
    memset(&s_log_ctx, 0, sizeof(s_log_ctx));
    s_log_ctx_valid = false;
}

void realize_unit_log_set_output_to_terminal(bool enable)
{
    s_log_output_to_terminal = enable;
}

int realize_unit_log_flush_buffer(void)
{
    fflush(stdout);
    return 0;
}

int realize_unit_log_clean_file(const char *dir, int clean_type)
{
    (void)dir;
    (void)clean_type;
    return 0;
}

void realize_unit_log_set_level(int type, int level)
{
    if (type < 0 || type >= SDK_LOG_TYPE_COUNT) {
        return;
    }

    s_log_levels[type] = level;
}

int realize_unit_log_get_level(int type)
{
    if (type < 0 || type >= SDK_LOG_TYPE_COUNT) {
        return REALIZE_UNIT_LOG_LEVEL_INFO;
    }

    return s_log_levels[type];
}

unsigned char realize_unit_log_is_inited(void)
{
    return 0;
}

void realize_unit_log_set_hook(unit_log_hook_t *hook)
{
    s_log_hook = hook;
}

unit_log_hook_t *realize_unit_log_get_hook(void)
{
    return s_log_hook;
}

unsigned char realize_unit_log_get_write_status(void)
{
    return 0;
}

void _realize_unit_log_suspend(const char *func, unsigned char suspend)
{
    (void)func;
    (void)suspend;
}

void realize_unit_log_init(void)
{
}

int realize_unit_log_copy_to_buffer(char *buf, int len)
{
    (void)buf;
    (void)len;
    return 0;
}

void realize_unit_log_upload_set_abort(char abort)
{
    (void)abort;
}

char realize_unit_log_upload_get_init(void)
{
    return 0;
}

char realize_unit_log_upload_get_status(void)
{
    return 0;
}

void realize_unit_log_upload_inc_size(int size)
{
    (void)size;
}

int realize_unit_log_upload_get_max_buffer_size(void)
{
    return 1;
}

void realize_unit_log_upload_get_buffer(char *buffer, int max_len, int *len)
{
    if (buffer != NULL && max_len > 0) {
        buffer[0] = '\0';
    }

    if (len != NULL) {
        *len = 0;
    }
}

#if defined(CONFIG_ELOG_HANDLE_ENABLE) && CONFIG_ELOG_HANDLE_ENABLE
int realize_unit_log_upload_buffer(unit_log_upload_cfg *cfg, char *buffer, int len, int type)
{
    (void)cfg;
    (void)buffer;
    (void)len;
    (void)type;
    return 0;
}

int realize_unit_log_upload_control(unit_log_upload_cfg *cfg)
{
    (void)cfg;
    return 0;
}
#endif
