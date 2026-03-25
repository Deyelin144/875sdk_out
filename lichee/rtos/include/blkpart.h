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

#ifndef __BLKPART_H__
#define __BLKPART_H__

#include <sys/types.h>
#include <stdint.h>
#include <devfs.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SECTOR_SHIFT 9
#define SECTOR_SIZE (1 << SECTOR_SHIFT)

/*
 * flag: PARTFLAG_FORCE_RW
 *     in normal case, when do blkpart read/write, we don't need to care about
 *     align and erase, blkpart handle them automatically, like read-merge-erase-write.
 *     in special case, read write speed is more important, we can set PARTFLAG_FORCE_RW
 *     and have to handle align and erase.
 *     after set, no nor cache and auto erase.
 *     for example, to update rtos part in nor flash:
 *	   1. config flag PARTFLAG_FORCE_RW for part rtos
 *	      [function: blkpart_config_force_rw]
 *	   2. erase where we want to write, or erase whole part rtos
 *	      [function: blkpart_erase]
 *	   3. write to part rtos in multiple times(don't need to erase again, but should not write different data to same address)
 *	      [open /dev/rtos and write]
 *	   4. reboot or remove flag PARTFLAG_FORCE_RW for part rtos
 *	      [function: blkpart_config_force_rw]
 *
 *
 * flag: PARTFLAG_SKIP_ERASE_BEFORE_WRITE
 *     in normal case, devfs blkpart write will do erase first automatically.
 *     in special case, read write speed is more important, we can set PARTFLAG_SKIP_ERASE_BEFORE_WRITE
 *     and have to handle erase.
 *     after set, nor cache still work but no auto erase.
 *     for example, to update rtos part in nor flash:
 *	   1. config flag PARTFLAG_SKIP_ERASE _BEFORE_WRITE for part rtos
 *	      [function: blkpart_config_skip_erase_before_write]
 *	   2. erase where we want to write, or erase whole part rtos
 *	      [function: blkpart_erase]
 *	   3. write to part rtos in multiple times(don't need to erase again, but should not write different data to same address)
 *	      [open /dev/rtos and write]
 *	   4. reboot or remove flag PARTFLAG_SKIP_ERASE_BEFORE_WRITE for part rtos
 *	      [function: blkpart_config_skip_erase_before_write]
 */
#define PARTFLAG_FORCE_RW  (1 << 0)
#define PARTFLAG_SKIP_ERASE_BEFORE_WRITE (1 << 1)

struct part {
    /* public */
#define BLKPART_OFF_APPEND UINT32_MAX
    uint64_t off;
#define BLKPART_SIZ_FULL UINT32_MAX
    uint64_t bytes;
#define MAX_BLKNAME_LEN 16
    char name[MAX_BLKNAME_LEN];      /* name: UDISK */

    /* private */
    char devname[MAX_BLKNAME_LEN];   /* name: nor0p1 */
    struct blkpart *blk;
#ifdef CONFIG_COMPONENTS_AW_DEVFS
    struct devfs_node node;
#endif
    uint32_t n_part;
    uint32_t flag;
    void *private;
};

struct blkpart {
    /* public */
    const char *name;
    uint64_t total_bytes;
    uint32_t blk_bytes;
    uint32_t page_bytes;
    int (*erase)(unsigned int, unsigned int);
    int (*program)(unsigned int, char *, unsigned int);
    int (*read)(unsigned int, char *, unsigned int);
    int (*sync)(void);
    int (*noncache_erase)(unsigned int, unsigned int);
    int (*noncache_program)(unsigned int, char *, unsigned int);
    int (*noncache_read)(unsigned int, char *, unsigned int);

    /* if no any partition, the follow can be NULL */
    struct part *parts;
    uint32_t n_parts;

    /* private */
    int blk_cnt;
    struct part root;
    struct blkpart *next;
    uint32_t n_blk;
};

int add_blkpart(struct blkpart *blk);
void del_blkpart(struct blkpart *blk);

#define PARTINDEX_THE_LAST UINT32_MAX
struct blkpart *get_blkpart_by_name(const char *name);
struct part *get_part_by_index(const char *blk_name, uint32_t index);
struct part *get_part_by_name(const char *name);
ssize_t blkpart_devfs_read(struct devfs_node *node, uint32_t offset,
        uint32_t size, void *data);
ssize_t blkpart_devfs_write(struct devfs_node *node, uint32_t offset,
        uint32_t size, const void *data);
ssize_t blkpart_erase_write(struct part *, uint32_t, uint32_t, const void *);
int blkpart_erase(struct part *, uint32_t, uint32_t);
int blkpart_sync(struct part *);
int blkpart_config_force_rw(struct part *, uint32_t);
int blkpart_config_skip_erase_before_write(struct part *, uint32_t);


#ifdef __cplusplus
}
#endif

#endif
