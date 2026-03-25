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

#ifndef _PM_WAKELOCK_H_
#define _PM_WAKELOCK_H_

#ifdef CONFIG_KERNEL_FREERTOS
#include <FreeRTOS.h>
#include <semphr.h>
#include <timers.h>
#else
#error "PM do not support the RTOS!!"
#endif

#include <hal/aw_list.h>

#ifdef __cplusplus
extern "C" {
#endif

enum pm_wakelock_t {
        PM_WL_TYPE_UNKOWN = 0,
        PM_WL_TYPE_WAIT_ONCE,
        PM_WL_TYPE_WAIT_INC,
        PM_WL_TYPE_WAIT_TIMEOUT,
};

struct wakelock {
        const char              *name;

        /* pm core use, do not edit */
        struct list_head        node;
        uint32_t                expires;
        uint16_t                ref;
        uint16_t                type;
};

#define PM_WAKELOCK_USE_GLOBE_CNT
#define OS_WAIT_FOREVER		0xffffffffU

void pm_wakelocks_init(void);
void pm_wakelocks_deinit(void);

void pm_wakelocks_setname(struct wakelock *wl, const char *name);
int  pm_wakelocks_acquire(struct wakelock *wl, enum pm_wakelock_t type, uint32_t timeout);
int  pm_wakelocks_release(struct wakelock *wl);

uint32_t pm_wakelocks_accumcnt(void); /*don't use it*/
uint32_t pm_wakelock_check_ignore(void);
void pm_wakelock_ignore(uint32_t ignore);
uint32_t pm_wakelocks_refercnt(int stash);
void     pm_wakelocks_showall(void);

void pm_wakelocks_block_hold_mutex(void);
void pm_wakelocks_give_mutex(void);

#ifdef __cplusplus
}
#endif

#endif /* __XRADIO_PM_H */
