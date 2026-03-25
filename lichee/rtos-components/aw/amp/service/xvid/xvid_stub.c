#include "sunxi_amp.h"
#include <hal_cache.h>

static unsigned g_draw_size = 0;

MAYBE_STATIC int VideoDecInit(unsigned width, unsigned higth)
{
    void *args[2] = {0};
    int ret = -1;
    args[0] = (void *)(unsigned long)width;
    args[1] = (void *)(unsigned long)higth;
    g_draw_size = width * higth * 3 / 2;
    ret = func_stub(RPCCALL_XVID(VideoDecInit), 1, ARRAY_SIZE(args), args);
    return ret;
}

MAYBE_STATIC int VideoDecDeinit(void)
{
    int ret = -1;
    ret = func_stub(RPCCALL_XVID(VideoDecDeinit), 1, 0, NULL);
    g_draw_size = 0;
    return ret;
}


MAYBE_STATIC int Mp4VideoDecFrame(unsigned char *input, unsigned in_size, unsigned char *output)
{
    int ret = -1;
    void *args[3] = {0};
    args[0] = input;
    args[1] = (void *)(unsigned long)in_size;
    args[2] = output;

    hal_dcache_clean((unsigned long)input, in_size);
    ret = func_stub(RPCCALL_XVID(Mp4VideoDecFrame), 1, ARRAY_SIZE(args), args);
    hal_dcache_invalidate((unsigned long)output, g_draw_size);
    return ret;
}