#include "sunxi_amp.h"
#include <hal_cache.h>

MAYBE_STATIC int tjpeg_decode(uint8_t *jpeg, int in_size,
                                uint8_t *output, int out_size, int format)
{
    int ret = -1;
    void *args[5] = {0};
    args[0] = jpeg;
    args[1] = (void *)(unsigned long)in_size;
    args[2] = output;
    args[3] = (void *)(unsigned long)out_size;;
    args[4] = (void *)(unsigned long)format;

    hal_dcache_clean((unsigned long)jpeg, in_size);
    hal_dcache_clean((unsigned long)output, out_size);
    ret = func_stub(RPCCALL_JPEG(tjpeg_decode), 1, ARRAY_SIZE(args), args);
    hal_dcache_invalidate((unsigned long)output, out_size);
    return ret;
}

