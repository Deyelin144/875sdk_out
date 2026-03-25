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

#ifndef _RINGBUFF_H_
#define _RINGBUFF_H_

#include <stdint.h>
#include <stdbool.h>
#include "kernel/os/os.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RB_OK      (0)
#define RB_FAIL    (-1)
#define RB_DONE    (-2)
#define RB_ABORT   (-3)
#define RB_TIMEOUT (-4)

struct ringbuf {
    char *p_o;                  /**< Original pointer */
    char *volatile p_r;         /**< Read pointer */
    char *volatile p_w;         /**< Write pointer */
    volatile uint32_t fill_cnt; /**< Number of filled slots */
    uint32_t size;              /**< Buffer size */
    XR_OS_Semaphore_t can_read;
    XR_OS_Semaphore_t can_write;
    XR_OS_Mutex_t lock;
    bool abort_read;
    bool abort_write;
    bool is_done_write; /**< To signal that we are done writing */
};

typedef struct ringbuf* ringbuf_handle_t;

/**
 * @brief      Create ringbuffer with total size
 *
 * @param[in]  total_size   Size of ringbuffer
 *
 * @return     ringbuf_handle_t
 */
ringbuf_handle_t rb_create(int total_size);

/**
 * @brief      Cleanup and free all memory created by ringbuf_handle_t
 *
 * @param[in]  rb    The Ringbuffer handle
 *
 * @return
 *     - RB_OK
 *     - RB_FAIL
 */
int rb_destroy(ringbuf_handle_t rb);

/**
 * @brief      Reset ringbuffer, clear all values as initial state
 *
 * @param[in]  rb    The Ringbuffer handle
 *
 * @return
 *     - RB_OK
 *     - RB_FAIL
 */
int rb_reset(ringbuf_handle_t rb);

/**
 * @brief      Get total bytes available of Ringbuffer
 *
 * @param[in]  rb    The Ringbuffer handle
 *
 * @return     total bytes available
 */
int rb_get_remain_bytes(ringbuf_handle_t rb);

/**
 * @brief      Get the number of bytes that have filled the ringbuffer
 *
 * @param[in]  rb    The Ringbuffer handle
 *
 * @return     The number of bytes that have filled the ringbuffer
 */
int rb_get_filled_bytes(ringbuf_handle_t rb);

/**
 * @brief      Get total size of Ringbuffer (in bytes)
 *
 * @param[in]  rb    The Ringbuffer handle
 *
 * @return     total size of Ringbuffer
 */
int rb_get_total_size(ringbuf_handle_t rb);

/**
 * @brief      Read from Ringbuffer to `buf` with len and wait `tick_to_wait` ticks until enough bytes to read
 *             if the ringbuffer bytes available is less than `len`.
 *             If `buf` argument provided is `NULL`, then ringbuffer do pseudo reads by simply advancing pointers.
 *
 * @param[in]  rb             The Ringbuffer handle
 * @param      buf            The buffer pointer to read out data
 * @param[in]  len            The length request
 * @param[in]  timeout        The time to wait
 *
 * @return     Number of bytes read
 */
int rb_read(ringbuf_handle_t rb, uint8_t *buf, int buf_len, uint32_t timeout);

/**
 * @brief      Write to Ringbuffer from `buf` with `len` and wait `tick_to_wait` ticks until enough space to write
 *             if the ringbuffer space available is less than `len`
 *
 * @param[in]  rb             The Ringbuffer handle
 * @param      buf            The buffer
 * @param[in]  len            The length
 * @param[in]  timeout        The time to wait
 *
 * @return     Number of bytes written
 */
int rb_write(ringbuf_handle_t rb, uint8_t *buf, int buf_len, uint32_t timeout);

/**
 * @brief      unblock Read from Ringbuffer to `buf` with len
 *
 * @param[in]  rb             The Ringbuffer handle
 * @param      buf            The buffer pointer to read out data
 * @param[in]  len            The length request
 *
 * @return     Number of bytes read
 */
int rb_unblock_read(ringbuf_handle_t rb, uint8_t *buf, int buf_len);

/**
 * @brief      unblock Write to Ringbuffer from `buf` with len
 *
 * @param[in]  rb             The Ringbuffer handle
 * @param      buf            The buffer
 * @param[in]  len            The length
 *
 * @return     Number of bytes written
 */
int rb_unblock_write(ringbuf_handle_t rb, uint8_t *buf, int buf_len);

#ifdef __cplusplus
}
#endif

#endif
