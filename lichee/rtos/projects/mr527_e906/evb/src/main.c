#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "interrupt.h"
#include <portmacro.h>
#include "FreeRTOS.h"
#include "task.h"

#include <console.h>

#ifdef CONFIG_DRIVERS_MSGBOX
#include <hal_msgbox.h>
#endif

#ifdef CONFIG_COMPONENTS_OPENAMP
extern int openamp_init(void);
#endif

void cpu0_app_entry(void *param)
{
    	(void)param;

#ifdef CONFIG_COMPONENTS_OPENAMP
	openamp_init();
#endif

#ifdef CONFIG_COMPONENT_CLI
    	vCommandConsoleStart(0x1000, HAL_THREAD_PRIORITY_CLI, NULL);
#endif

    	vTaskDelete(NULL);
}
