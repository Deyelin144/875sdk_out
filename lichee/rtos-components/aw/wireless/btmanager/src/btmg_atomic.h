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

#ifndef _BTMG_ATOMIC_H_
#define _BTNG_ATOMIC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdatomic.h>

typedef atomic_uint bt_atomic_t;

#define BT_ATOMIC_BITS      (sizeof(bt_atomic_t) * 8)
#define BT_ATOMIC_MASK(bit) (1 << ((unsigned int)(bit) & (BT_ATOMIC_BITS - 1)))

static inline bool bt_atomic_test_bit(bt_atomic_t *target, int bit)
{
    unsigned int val = *target;
    return (1 & (val >> (bit & (BT_ATOMIC_BITS - 1)))) != 0;
}

static inline void bt_atomic_clear_bit(bt_atomic_t *target, int bit)
{
    unsigned int mask = BT_ATOMIC_MASK(bit);
    atomic_fetch_and(target, ~mask);
}

static inline void bt_atomic_set_bit(bt_atomic_t *target, int bit)
{
    unsigned int mask = BT_ATOMIC_MASK(bit);
    atomic_fetch_or(target, mask);
}

static inline unsigned int bt_atomic_set_val(bt_atomic_t *target, unsigned int ulCount)
{
    unsigned int ulCurrent = 0;
    atomic_store(target, ulCount);
    return ulCurrent;
}

#ifdef __cplusplus
}
#endif

#endif /* _BTMG_ATOMIC_H_ */
