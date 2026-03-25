#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>

#ifndef DRV_LOG_SET_LEVEL
#define DRV_LOG_SET_LEVEL
typedef enum  {
    DRV_LOG_LEVEL_VERB = 1,
    DRV_LOG_LEVEL_DBUG = 2,
    DRV_LOG_LEVEL_INFO = 3,
    DRV_LOG_LEVEL_WARN = 4,
    DRV_LOG_LEVEL_EROR = 5,
} LOG_LEVEL_TYPE;

extern LOG_LEVEL_TYPE s_drv_log_level;
#endif

//void drv_log_set_level(LOG_LEVEL_TYPE level);
LOG_LEVEL_TYPE drv_log_get_level();

#define _drv_logv(level, fmt, ...) do {\
                                         if (level >= s_drv_log_level) {\
                                             printf("\e[0m[VERB] <%s %d> : "fmt"\e[0m\n", __func__, __LINE__, ##__VA_ARGS__);\
                                         }\
                                     } while(0)

#define _drv_logd(level, fmt, ...) do {\
                                         if (level >= s_drv_log_level) {\
                                             printf("\e[0m[DBUG] <%s %d> : "fmt"\e[0m\n", __func__, __LINE__, ##__VA_ARGS__);\
                                         }\
                                      } while(0)

#define _drv_logi(level, fmt, ...) do {\
                                          if (level >= s_drv_log_level) {\
                                              printf("\e[0;32m[INFO] <%s %d> : "fmt"\e[0m\n", __func__, __LINE__, ##__VA_ARGS__);\
                                          }\
                                      } while(0)

#define _drv_logw(level, fmt, ...) do {\
                                         if (level >= s_drv_log_level) {\
                                             printf("\e[0;33m[WARN] <%s %d> : "fmt"\e[0m\n", __func__, __LINE__, ##__VA_ARGS__);\
                                         }\
                                      } while(0)

#define _drv_loge(level, fmt, ...) do {\
                                          if (level >= s_drv_log_level) {\
                                              printf("\e[0;31m[EROR] <%s %d> : "fmt"\e[0m\n", __func__, __LINE__, ##__VA_ARGS__);\
                                          }\
                                      } while(0)

#define drv_logv(fmt, ...) _drv_logv(DRV_LOG_LEVEL_VERB, fmt, ##__VA_ARGS__)
#define drv_logd(fmt, ...) _drv_logd(DRV_LOG_LEVEL_DBUG, fmt, ##__VA_ARGS__)
#define drv_logi(fmt, ...) _drv_logi(DRV_LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define drv_logw(fmt, ...) _drv_logw(DRV_LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define drv_loge(fmt, ...) _drv_loge(DRV_LOG_LEVEL_EROR, fmt, ##__VA_ARGS__)

#endif
