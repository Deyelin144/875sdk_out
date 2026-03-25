#ifndef _DRV_COMMON_H_
#define _DRV_COMMON_H_

#include "drv_log.h"

typedef void (*irq_callback)(void *arg);

/* Driver Status value */
typedef enum {
    DRV_OK      = 0,	/* success */
    DRV_ERROR   = -1,	/* general error */
    DRV_BUSY    = -2,	/* device or resource busy */
    DRV_TIMEOUT = -3,	/* wait timeout */
    DRV_INVALID = -4,	/* invalid argument */
    DRV_AGAIN   = -5,   /* try again */
    DRV_NONE    = -6,   /* nothing need to do */
} drv_status_t;


/*********************************LED灯********************************************/
/*
 * LED灯模式
*/
#define CFG_LED_MODE_GPIO                           1
#define CFG_LED_MODE_PWM                            2
#define CFG_LED_MODE_RGB                            3

#define CFG_LED_EN 1
#define CFG_LED_MODE_TYPE   CFG_LED_MODE_RGB
 
 
#define drv_safe_free(p) do { if (p) { free(p); p = NULL; } } while(0)

#endif