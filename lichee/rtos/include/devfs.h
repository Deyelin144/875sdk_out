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

#ifndef __DEVFS_H__
#define __DEVFS_H__

#include <stdint.h>
#include <stdio.h>

#ifdef CONFIG_COMPONENT_IO_MULTIPLEX
#include <poll.h>
#endif
struct devfs_file {
    struct devfs_node *node;
    uint32_t off;
    void *private;
};

struct devfs_node {
    const char *name;
    const char *alias;
#define NODE_SIZE_INFINITE UINT32_MAX
    uint64_t size;

    ssize_t (*write)(struct devfs_node *node, uint32_t addr, uint32_t size, const void *data);
    ssize_t (*read)(struct devfs_node *node, uint32_t addr, uint32_t size, void *data);
    /* device can do some initializion at this call back function */
    int (*open)(struct devfs_node *node);
    /* device can do some de-initializion at this call back function */
    int (*close)(struct devfs_node *node);
    /* if device has some other feature, make it come true here */
    int (*ioctl)(struct devfs_node *node, int cmd, void *arg);
    /* device can flush cache if it has at this call back function */
    int (*fsync)(struct devfs_node *node);
	struct devfs_node *next;
    void *private;

#ifdef CONFIG_COMPONENT_IO_MULTIPLEX
    vfs_t *fops;
    struct rt_wqueue wait_queue;
#endif
};

int devfs_add_node(struct devfs_node *new);
void devfs_del_node(struct devfs_node *node);
int devfs_mount(const char *mnt_point);

typedef uint8_t BYTE;
void ff_diskio_register_usb_msc(BYTE pdrv, struct devfs_node* card);
struct devfs_node * devfs_get_node(char *name);
//int vfs_fat_register(const char* base_path, const char* fat_drive, size_t max_files, FATFS** out_fs);
int vfs_fat_unregister_path(const char* base_path);
BYTE ff_usb_msc_diskio_get_pdrv_node(const struct devfs_node* node);
#endif

