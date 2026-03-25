#include "serial.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "interrupt.h"
#include <portmacro.h>
#include <autoconf.h>
#include <cli_console.h>

#include "tinatest.h"

#ifdef CONFIG_COMPONENT_ULOG
#include <ulog.h>
#endif

//#define SMP_KERNEL_TEST


#ifdef CONFIG_COMPONENTS_AW_TINATEST
static volatile int tinatest_init_ok = 0;
#endif

void cmd_tcpip_init(void);
void cpu1_app_entry(void * param)
{
#ifdef CONFIG_COMPONENT_FINSH_CLI
    extern int finsh_system_init(void);
    finsh_system_init();
#elif CONFIG_COMPONENTS_OPUS_TEST
	vCommandConsoleStart(0x10000 - 1, HAL_THREAD_PRIORITY_CLI, NULL);
#else
	vCommandConsoleStart(0x2000, HAL_THREAD_PRIORITY_CLI, NULL);
#endif

	vTaskDelete(NULL);
}

void cpu0_app_entry(void * param)
{
	(void)param;

	void bsp_init(void);
	bsp_init();

	int opts = 0;
   	int argc = 5;
	char optopt;
	char *argv[7] = {"memtest","1M", "-l", "-h", "-k", "-", "-s"};
    	while ((opts = getopt(argc, argv, ":lhk")) != EOF) {
  	    SMP_DBG("----------------\n");
	    switch (opts) {
	        case 'l': SMP_DBG("-l\n"); break;
	        case 'k': SMP_DBG("-k\n"); break;
	        case 'h': SMP_DBG("-h\n"); break;;
	        case '?': SMP_DBG("invalid option %c\n", optopt); break;
	        case ':': SMP_DBG("option -%c requires an argument\n", optopt); break;
	    }
    	}
  	SMP_DBG("fnish----------------\n");

	//tcpip stack init
	cmd_tcpip_init();
#ifdef CONFIG_DRIVER_XR829
	int sysinfo_init(void);
	sysinfo_init();
#endif

	printf("Welcome to Application\n");

#ifdef CONFIG_COMPONENTS_AW_TINATEST
	if (!access(TINATEST_OK_FILE, R_OK)) {
		printf("tinatest found %s\n", TINATEST_OK_FILE);
	} else {
		printf("start tinatest\n");
		int tinatest(int argc, char **argv);
		tinatest(1, NULL);
	}
	tinatest_init_ok = 1;
#endif

#ifndef configKERNEL_SUPPORT_SMP
#ifdef CONFIG_COMPONENTS_OPUS_TEST
	vCommandConsoleStart(0x8000, HAL_THREAD_PRIORITY_CLI, NULL);
#else
	vCommandConsoleStart(0x2000, HAL_THREAD_PRIORITY_CLI, NULL);
#endif
#endif

	vTaskDelete(NULL);
}
