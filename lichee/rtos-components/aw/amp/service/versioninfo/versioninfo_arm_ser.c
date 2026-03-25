
#if (defined CONFIG_ARCH_ARM_ARMV8M && !defined CONFIG_PROJECT_BUILD_RECOVERY)

#include "sunxi_amp.h"

extern char *arm_get_version(void);

static char *get_arm_version(void)
{
    return arm_get_version();
}

sunxi_amp_func_table versioninfo_arm_table[] ={
    {.func = (void *)&get_arm_version, .args_num = 0, .return_type = RET_POINTER},
};
#endif
