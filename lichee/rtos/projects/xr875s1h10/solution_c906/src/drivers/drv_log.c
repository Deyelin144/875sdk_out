#include <stdlib.h>
#include <console.h>
#include "drv_log.h"

#define DRV_LOG_CUSTOM_LEVEL 0

#if DRV_LOG_CUSTOM_LEVEL
LOG_LEVEL_TYPE s_drv_log_level = DRV_LOG_CUSTOM_LEVEL;
#else
LOG_LEVEL_TYPE s_drv_log_level = DRV_LOG_LEVEL_EROR;
#endif
void drv_log_set_level(int argc, const char **argv)
{
    LOG_LEVEL_TYPE level;
    if (2 != argc) {
        return;
    }
    level = (LOG_LEVEL_TYPE)atoi(argv[1]);
    if (level > DRV_LOG_LEVEL_EROR) {
        level = DRV_LOG_LEVEL_EROR;
    } else if (level < DRV_LOG_LEVEL_VERB) {
        level = DRV_LOG_LEVEL_VERB;
    }

    s_drv_log_level = level;
}

LOG_LEVEL_TYPE drv_log_get_level()
{
    return s_drv_log_level;
}

FINSH_FUNCTION_EXPORT_CMD(drv_log_set_level, drv_log, driver log level set);

