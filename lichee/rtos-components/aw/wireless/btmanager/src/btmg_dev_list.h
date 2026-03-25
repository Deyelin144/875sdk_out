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

#ifndef _BTMG_DEV_LIST_H_
#define _BTMG_DEV_LIST_H_

#include <pthread.h>
#include <stdbool.h>
#include "kernel/os/os_semaphore.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_BT_NAME_LEN 128
#define MAX_BT_ADDR_LEN 17

typedef struct dev_node_t {
    struct dev_node_t *front;
    struct dev_node_t *next;
    char dev_name[MAX_BT_NAME_LEN + 1];
    char dev_addr[MAX_BT_ADDR_LEN + 1];
    unsigned int profile;
} dev_node_t;

typedef struct dev_list_t {
    dev_node_t *head;
    dev_node_t *tail;
    int length;
    bool list_cleared;
    pthread_mutex_t lock;
    XR_OS_Semaphore_t sem;
    int sem_flag;
} dev_list_t;

int btmg_dev_list_add_device(dev_list_t *dev_list, const char *name, const char *addr,
                             unsigned int profile);
dev_node_t *btmg_dev_list_find_device(dev_list_t *dev_list, const char *addr);
bool btmg_dev_list_remove_device(dev_list_t *dev_list, const char *addr);
dev_list_t *btmg_dev_list_new();
void btmg_dev_list_clear(dev_list_t *list);
void btmg_dev_list_free(dev_list_t *list);

#ifdef __cplusplus
}; /*extern "C"*/
#endif

#endif
