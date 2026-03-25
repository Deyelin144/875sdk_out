#ifdef CONFIG_ARCH_RISCV_C906
#include "sunxi_amp.h"
char *get_arm_version(void)
{
    const char *version = NULL;
	version = (const char *)func_stub(RPCCALL_VERSIONINFO_ARM(get_arm_version), 1, 0, NULL);
    return version;
}

char *get_dsp_version(void)
{
    const char *version = NULL;
	version = (const char *)func_stub(RPCCALL_VERSIONINFO_DSP(get_dsp_version), 1, 0, NULL);
    return version;
}
#endif
