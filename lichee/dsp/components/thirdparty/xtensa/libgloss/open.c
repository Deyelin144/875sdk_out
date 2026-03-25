/* open.c  -  open and possibly create a file or device */

/*
 * Copyright (c) 2011 Tensilica Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <errno.h>
#include <fcntl.h>
#include "gloss.h"

extern int dsp_open(const char *name, int flag, ...);
/*
 *  open
 *
 *  No support for opening files (only for "pre-opened" stdin/stdout/stderr).
 */
#define AMP_RDONLY        0
#define AMP_WRONLY        0x01
#define AMP_RDWR          0x02
#define AMP_CREAT         0x200
#define AMP_EXCL          0x800
#define AMP_TRUNC         0x400
#define AMP_APPEND        0x08

#ifdef CONFIG_AMP_FSYS_STUB_DSP
static int dsp_sys_format_change(int flags)
{
    int ret = -1;
    switch(flags) {
        case O_RDONLY:
            ret = AMP_RDONLY;
            break;
        case O_WRONLY:
            ret = AMP_WRONLY;
            break;
        case O_RDWR:
            ret = AMP_RDWR;
            break;
        case O_CREAT:
            ret = AMP_CREAT;
            break;
        case O_EXCL:
            ret = AMP_EXCL;
            break;
        case O_TRUNC:
            ret = AMP_TRUNC;
            break;
        case O_APPEND:
            ret = AMP_APPEND;
            break;
        default:
            break;
    }
    return ret;
}
#endif

int
_FUNC (open, const char *pathname, int flags, int mode)
{
#ifdef CONFIG_AMP_FSYS_STUB_DSP
    int c_flags = dsp_sys_format_change(flags);
    return dsp_open(pathname, c_flags);
#else
    return 0;
#endif
}

