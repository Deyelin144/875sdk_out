/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the People's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <reent.h>
#include <sys/stat.h>

#include "sunxi_amp.h"

#include <hal_cache.h>

int dsp_open(const char *name, int flag, ...)
{
    void *args[3] = {0};
    int ret = -1;
    int nameSize = strlen(name) + 1;
    char *align_name = amp_align_malloc(nameSize);
    if (!align_name)
    {
        return -1;
    }
    memset(align_name, 0, nameSize);
    memcpy(align_name, name, nameSize);
    args[0] = align_name;
    args[1] = (void *)(unsigned long)nameSize;
    args[2] = (void *)(unsigned long)flag;
    hal_dcache_clean((unsigned long)align_name, nameSize);
    ret = func_stub(RPCCALL_FSYS(open), 1, ARRAY_SIZE(args), args);
    amp_align_free(align_name);
    return ret;
}

unsigned int dsp_read(int fd, void *buffer, unsigned int size)
{
    void *args[3] = {0};
    unsigned ret = -1;
    char *read_buffer = amp_align_malloc(size);
    if (read_buffer == NULL) {
        return -1;
    }
    memset(read_buffer, 0, size);
    args[0] = (void *)(unsigned long)fd;
    args[1] = read_buffer;
    args[2] = (void *)(unsigned long)size;
    hal_dcache_invalidate((unsigned long)read_buffer, size);
    ret = (unsigned int)func_stub(RPCCALL_FSYS(read), 1, ARRAY_SIZE(args), args);
    hal_dcache_invalidate((unsigned long)read_buffer, size);

    memcpy(buffer, read_buffer, size);
    amp_align_free(read_buffer);
    return ret;
}

int dsp_close(int fd)
{
    void *args[1] = {0};
    args[0] = (void *)(unsigned long)fd;
    return func_stub(RPCCALL_FSYS(close), 1, ARRAY_SIZE(args), args);
}

unsigned int dsp_write(int fd, const void *buffer, unsigned int size)
{
    void *args[3] = {0};
    char *write_buffer = amp_align_malloc(size);
    if (write_buffer == NULL) {
        return -1;
    }
    memcpy(write_buffer, buffer, size);
    args[0] = (void *)(unsigned long)fd;
    args[1] = write_buffer;
    args[2] = (void *)(unsigned long)size;

    hal_dcache_clean((unsigned long)write_buffer, size);
    return (unsigned int)func_stub(RPCCALL_FSYS(write), 1, ARRAY_SIZE(args), args);
}

unsigned long dsp_lseek(int fd, unsigned long offset, int whence)
{
    void *args[3] = {0};
    args[0] = (void *)(unsigned long)fd;
    args[1] = (void *)(unsigned long)offset;
    args[2] = (void *)(unsigned long)whence;
    return func_stub(RPCCALL_FSYS(lseek), 1, ARRAY_SIZE(args), args);
}

int dsp_fstat(int fd, struct stat *st)
{
    void *args[2] = {0};
    int ret = -1;
    struct stat *st_align = amp_align_malloc(sizeof(*st));
    if (!st_align)
    {
        return -1;
    }
    memset(st_align, 0, sizeof(*st));
    args[0] = (void *)(unsigned long)fd;
    args[1] = st_align;
    hal_dcache_clean((unsigned long)st_align, sizeof(struct stat));
    ret = func_stub(RPCCALL_FSYS(fstat), 1, ARRAY_SIZE(args), args);
    hal_dcache_invalidate((unsigned long)st_align, sizeof(struct stat));
    memcpy(st, st_align, sizeof(struct stat));
    amp_align_free(st_align);
    return ret;
}

int dsp_stat(const char *path, struct stat *st)
{
    void *args[3] = {0};
    int ret = -1;
    int pathSize = strlen(path) + 1;
    char *align_path = amp_align_malloc(pathSize);
    if (!align_path)
    {
        return -1;
    }
    memset(align_path, 0, pathSize);
    memcpy(align_path, path, pathSize);
    struct stat *st_align = amp_align_malloc(sizeof(*st));
    if (!st_align)
    {
        amp_align_free(align_path);
        return -1;
    }
    memset(st_align, 0, sizeof(*st));

    args[0] = align_path;
    args[1] = (void *)(unsigned long)pathSize;
    args[2] = st_align;

    hal_dcache_clean((unsigned long)align_path, pathSize);
    hal_dcache_clean((unsigned long)st_align, sizeof(struct stat));

    ret = func_stub(RPCCALL_FSYS(stat), 1, ARRAY_SIZE(args), args);

    hal_dcache_invalidate((unsigned long)st_align, sizeof(struct stat));
    memcpy(st, st_align, sizeof(struct stat));
    amp_align_free(st_align);
    amp_align_free(align_path);
    return ret;
}

int dsp_unlink(const char *path)
{
    void *args[2] = {0};
    int ret = -1;
    int pathSize = strlen(path) + 1;
    char *align_path = amp_align_malloc(pathSize);
    if (!align_path)
    {
        return -1;
    }
    memset(align_path, 0, pathSize);
    memcpy(align_path, path, pathSize);
    args[0] = align_path;
    args[1] = (void *)(unsigned long)pathSize;
    hal_dcache_clean((unsigned long)align_path, pathSize);
    ret = func_stub(RPCCALL_FSYS(unlink), 1, ARRAY_SIZE(args), args);
    amp_align_free(align_path);
    return ret;
}

