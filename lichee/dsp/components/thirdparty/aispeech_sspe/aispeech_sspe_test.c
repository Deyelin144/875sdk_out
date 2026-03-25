#include <stdlib.h>
#include <stdio.h>
#include <backtrace.h>
#include <console.h>

#include "aispeech_sspe.h"

int cmd_ais_sspe_init(int argc, char **argv)
{
    printf("%s\n", __func__);

    aispeech_sspe_init();
    return 0;
}

FINSH_FUNCTION_EXPORT_CMD(cmd_ais_sspe_init, ais_sspe_init, aispeech sspe test);
