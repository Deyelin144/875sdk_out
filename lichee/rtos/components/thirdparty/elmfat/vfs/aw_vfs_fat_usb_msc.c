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
#include <string.h>
#include "awlog.h"
#include "vfs.h"
#include <devfs.h>

#include <diskio_impl.h>

#include <ff.h>

extern int vfs_fat_register(const char* base_path, const char* fat_drive, size_t max_files, FATFS** out_fs);
int usb_msc_elmfat_mount(const char *dev, const char *base_path)
{
    FATFS* fs = NULL;
    int err;
	int pdrv = 0;
	struct devfs_node *node = devfs_get_node((void *)dev);

    ff_diskio_register_usb_msc(pdrv, node);
    pr_debug(TAG, "using pdrv=%i", pdrv);
    char drv[3] = {(char)('0' + pdrv), ':', 0};

    // connect FATFS to VFS
    err = vfs_fat_register(base_path, drv, 16, &fs);
    if (err == -1) {
        // it's okay, already registered with VFS
    } else if (err != 0) {
        pr_debug(TAG, "esp_vfs_fat_register failed 0x(%x)", err);
        goto fail;
    }

    // Try to mount partition
    FRESULT res = f_mount(fs, drv, 1);
    if (res != FR_OK) {
        err = -1;
        pr_err("failed to mount node (%d)", res);
        if (!((res == FR_NO_FILESYSTEM || res == FR_INT_ERR))) {
            goto fail;
        }
    }
    printf("mount usb mass storage successull!!\n");
    return 0;

fail:
    if (fs) {
        f_mount(NULL, drv, 0);
    }
	vfs_fat_unregister_path(base_path);
	ff_diskio_unregister(pdrv);
    return err;
}

static void local_node_remove(void)
{
}

static int unmount_node_core(const char *base_path, struct devfs_node *node)
{
    BYTE pdrv = ff_usb_msc_diskio_get_pdrv_node(node);
    if (pdrv == 0xff) {
        return -1;
    }

    // unmount
    char drv[3] = {(char)('0' + pdrv), ':', 0};
    f_mount(0, drv, 0);
    // release SD driver
    ff_diskio_unregister(pdrv);

    /*
     * When connect peripherals, Disk driver will allocate and release memory.
     * Request memory : DiskProbe -> UsbBlkDevAllocInit
     * Release memory : DiskRemove -> UsbBlkDevFree
     * So, it is totally unnecessary to free node.
     * free(node);
     */

    int err = vfs_fat_unregister_path(base_path);
    return err;
}

int usb_msc_elmfat_unmount(const char *base_path, struct devfs_node *node)
{
    int err = unmount_node_core(base_path, node);
    return err;
}
