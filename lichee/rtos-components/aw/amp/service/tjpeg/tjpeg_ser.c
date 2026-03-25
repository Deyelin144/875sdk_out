#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "sunxi_amp.h"
#include "hal_cache.h"
#include "turbojpeg.h"

static int __tjpeg_decode(uint8_t *jpeg, int in_size,
                                uint8_t *output, int out_size, int format)
{
    tjhandle handle = NULL;
    int ret = -1;
    int img_width, img_height, img_subsamp, img_colorspace;

    hal_dcache_invalidate((unsigned long)jpeg, in_size);
    handle = tjInitDecompress();
    if (NULL == handle)  {
        return -1;
    }
    
    if (format == 0) {
        ret = tjDecompressToYUV(handle, jpeg, in_size, output, 0);
    } else {
        ret = tjDecompressHeader3(handle, jpeg, in_size,\
                                    &img_width, &img_height,\
                                    &img_subsamp,&img_colorspace);
        if (0 != ret) {
            tjDestroy(handle);
            return -1;
        }

        ret = tjDecompress2(handle, jpeg, in_size, output, img_width, 0,
                            img_height, TJPF_RGB, 0);
    }

    tjDestroy(handle);
    hal_dcache_clean((unsigned long)output, out_size);

    return ret;
}

sunxi_amp_func_table jpeg_table[] =
{
    {.func = (void *)&__tjpeg_decode,      .args_num = 5, .return_type = RET_POINTER},
};

