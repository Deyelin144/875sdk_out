
#ifdef CONFIG_ARCH_DSP

#include "sunxi_amp.h"

extern char *dsp_get_version(void);

static char *get_dsp_version(void)
{
    return dsp_get_version();
}

sunxi_amp_func_table versioninfo_dsp_table[] ={
    {.func = (void *)&get_dsp_version, .args_num = 0, .return_type = RET_POINTER},
};
#endif
