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

#include <stdio.h>
#include <string.h>
#include "kernel/os/os.h"

#include "ringbuff.h"
#include "btmg_log.h"
#include "bt_manager.h"

static uint8_t destroy_flag = 0;
static XR_OS_Mutex_t rb_mutex;

static void *rb_buf_calloc(int size)
{
    void *data = NULL;
    int n_blocks;
    int block_size = 1024;

    n_blocks = size / block_size;
    data = calloc(n_blocks, block_size);

    return data;
}

ringbuf_handle_t rb_create(int size)
{
    if (size < 2) {
        BTMG_ERROR("Invalid size");
        return NULL;
    }

    ringbuf_handle_t rb;

    char *buf = NULL;

    rb = (struct ringbuf *)malloc(sizeof(struct ringbuf));
    if (rb == NULL) {
        BTMG_ERROR("rb_malloc fail");
        return NULL;
    }

    memset(rb, 0, sizeof(struct ringbuf));

    buf = rb_buf_calloc(size);
    if (buf == NULL) {
        BTMG_ERROR("rb_buf_calloc fail");
        free(rb);
        return NULL;
    }

    if (XR_OS_SemaphoreCreateBinary(&rb->can_read) != XR_OS_OK) {
        BTMG_ERROR("semaphore create fail");
        free(buf);
        free(rb);
        return NULL;
    }

    if (rb_mutex.handle == NULL) {
        BTMG_INFO("");
        if (XR_OS_MutexCreate(&rb_mutex) != XR_OS_OK) {
            BTMG_ERROR("mutex create fail");
            free(buf);
            free(rb);
            XR_OS_SemaphoreDelete(&rb->can_read);
            return NULL;
        }
    }

    if (XR_OS_MutexCreate(&rb->lock) != XR_OS_OK) {
        BTMG_ERROR("mutex create fail");
        free(buf);
        free(rb);
        XR_OS_MutexDelete(&rb_mutex);
        XR_OS_SemaphoreDelete(&rb->can_read);
        return NULL;
    }

    if (XR_OS_SemaphoreCreateBinary(&rb->can_write) != XR_OS_OK) {
        BTMG_ERROR("semaphore create fail");
        free(buf);
        free(rb);
        XR_OS_MutexDelete(&rb->lock);
        XR_OS_SemaphoreDelete(&rb->can_read);
        return NULL;
    }

    rb->p_o = rb->p_r = rb->p_w = buf;
    rb->fill_cnt = 0;
    rb->size = size;
    rb->is_done_write = false;
    rb->abort_read = false;
    rb->abort_write = false;
    destroy_flag = 0;

    return rb;
_rb_init_failed:
    rb_destroy(rb);
    return NULL;
}

int rb_destroy(ringbuf_handle_t rb)
{
    BTMG_INFO("enter");
    XR_OS_MutexLock(&rb_mutex, XR_OS_WAIT_FOREVER);
    destroy_flag = 1;
    if (rb == NULL) {
        XR_OS_MutexUnlock(&rb_mutex);
        return BT_ERR_INVALID_ARG;
    }
    if (rb->p_o) {
        free(rb->p_o);
        rb->p_o = NULL;
    }

    XR_OS_SemaphoreDelete(&rb->can_read);
    XR_OS_SemaphoreDelete(&rb->can_write);
    XR_OS_MutexDelete(&rb->lock);
    free(rb);
    XR_OS_MutexUnlock(&rb_mutex);
    BTMG_INFO("rb_destroy finish");

    return RB_OK;
}

int rb_reset(ringbuf_handle_t rb)
{
    if (rb == NULL) {
        return RB_FAIL;
    }

    rb->p_r = rb->p_w = rb->p_o;
    memset(rb->p_r, 0, rb->size);
    rb->fill_cnt = 0;
    rb->is_done_write = false;

    rb->abort_read = false;
    rb->abort_write = false;

    destroy_flag = 0;

    return RB_OK;
}

int rb_get_remain_bytes(ringbuf_handle_t rb)
{
    return (rb->size - rb->fill_cnt);
}

int rb_get_filled_bytes(ringbuf_handle_t rb)
{
    if (rb != NULL) {
        return rb->fill_cnt;
    }
    return RB_FAIL;
}

int rb_get_total_size(ringbuf_handle_t rb)
{
    if (rb == NULL) {
        return BT_ERR_INVALID_ARG;
    }
    return rb->size;
}

int rb_read(ringbuf_handle_t rb, uint8_t *buf, int buf_len, uint32_t timeout)
{
    int ret_val = 0;
    int read_size = 0;
    int total_read_size = 0;

    if (destroy_flag == 1) {
        printf("BT(R_RB_INVALID)\n");
        return RB_FAIL;
    }

    if (rb == NULL) {
        return RB_FAIL;
    }

    while (buf_len) {
        //take buffer lock
        if (XR_OS_MutexLock(&rb->lock, XR_OS_WAIT_FOREVER) != RB_OK) {
            ret_val = RB_TIMEOUT;
            goto read_err;
        }
        if (rb->fill_cnt < buf_len) {
            read_size = rb->fill_cnt;
        } else {
            read_size = buf_len;
        }
        //no data to read, release thread block to allow other threads to write data
        if (read_size == 0) {
            if (rb->abort_read) {
                ret_val = RB_ABORT;
                //printf("BT(R_RB_EMPTY)\n");
                XR_OS_MutexUnlock(&rb->lock);
                goto read_err;
            }

            XR_OS_MutexUnlock(&rb->lock);
            XR_OS_SemaphoreRelease(&rb->can_write);
            //wait till some data available to read
            if (XR_OS_SemaphoreWait(&rb->can_read, timeout) != RB_OK) {
                ret_val = RB_TIMEOUT;
                printf("BT(R_RB_TIMEOUT)\n");
                goto end;
            }
            continue;
        }
        if ((rb->p_r + read_size) > (rb->p_o + rb->size)) {
            int rlen1 = rb->p_o + rb->size - rb->p_r;
            int rlen2 = read_size - rlen1;
            if (buf) {
                memcpy(buf, rb->p_r, rlen1);
                memcpy(buf + rlen1, rb->p_o, rlen2);
            }
            rb->p_r = rb->p_o + rlen2;
        } else {
            if (buf) {
                memcpy(buf, rb->p_r, read_size);
            }
            rb->p_r = rb->p_r + read_size;
        }

        buf_len -= read_size;
        rb->fill_cnt -= read_size;
        total_read_size += read_size;
        buf += read_size;
        XR_OS_MutexUnlock(&rb->lock);
        if (buf_len == 0) {
            break;
        }
    }
read_err:
    XR_OS_SemaphoreRelease(&rb->can_write);

end:
    if ((ret_val == RB_FAIL) || (ret_val == RB_ABORT)) {
        total_read_size = ret_val;
    }

    return total_read_size > 0 ? total_read_size : ret_val;
}

int rb_write(ringbuf_handle_t rb, uint8_t *buf, int buf_len, uint32_t timeout)
{
    int c = 0;
    int write_size;
    int total_write_size = 0;
    int ret_val = 0;

    XR_OS_MutexLock(&rb_mutex, XR_OS_WAIT_FOREVER);
    if (destroy_flag == 1) {
        printf("BT(W_RB_INVALID)\n");
        XR_OS_MutexUnlock(&rb_mutex);
        return RB_FAIL;
    }

    if (rb == NULL || buf == NULL) {
        XR_OS_MutexUnlock(&rb_mutex);
        return RB_FAIL;
    }
    while (buf_len) {
        //take buffer lock
        if (XR_OS_MutexLock(&rb->lock, XR_OS_WAIT_FOREVER) != RB_OK) {
            ret_val = RB_TIMEOUT;
            goto write_err;
        }
        write_size = rb_get_remain_bytes(rb);
        //no space to write, release thread block to allow other to read data
        if (write_size == 0) {
            if (rb->abort_write) {
                c++;
                if (c % 10 == 0)
                    printf("BT(W_RB_FULL)\n");

                ret_val = RB_ABORT;
                XR_OS_MutexUnlock(&rb->lock);
                goto write_err;
            }
            XR_OS_MutexUnlock(&rb->lock);
            XR_OS_SemaphoreRelease(&rb->can_read);
            //wait till we have some empty space to write
            if (XR_OS_SemaphoreWait(&rb->can_write, timeout) != RB_OK) {
                printf("BT(W_RB_TIMEOUT)\n");
                ret_val = RB_TIMEOUT;
                goto end;
            }
            continue;
        }
        if (buf_len > write_size) {
            XR_OS_MutexUnlock(&rb->lock);
            break;
        }
        write_size = buf_len;
        if ((rb->p_w + write_size) > (rb->p_o + rb->size)) {
            int wlen1 = rb->p_o + rb->size - rb->p_w;
            int wlen2 = write_size - wlen1;
            memcpy(rb->p_w, buf, wlen1);
            memcpy(rb->p_o, buf + wlen1, wlen2);
            rb->p_w = rb->p_o + wlen2;
        } else {
            memcpy(rb->p_w, buf, write_size);
            rb->p_w = rb->p_w + write_size;
        }
        buf_len -= write_size;
        rb->fill_cnt += write_size;
        total_write_size += write_size;
        buf += write_size;
        XR_OS_MutexUnlock(&rb->lock);
        if (buf_len == 0) {
            break;
        }
    }

write_err:
    XR_OS_SemaphoreRelease(&rb->can_read);

end:
    XR_OS_MutexUnlock(&rb_mutex);

    if ((ret_val == RB_TIMEOUT) || (ret_val == RB_ABORT)) {
        return ret_val;
    }

    return total_write_size;
}

int rb_unblock_read(ringbuf_handle_t rb, uint8_t *buf, int buf_len)
{
    if (rb == NULL) {
        return BT_ERR_INVALID_ARG;
    }

    rb->abort_read = true;

    return rb_read(rb, buf, buf_len, 0);
}

int rb_unblock_write(ringbuf_handle_t rb, uint8_t *buf, int buf_len)
{
    if (rb == NULL) {
        return BT_ERR_INVALID_ARG;
    }

    rb->abort_write = true;

    return rb_write(rb, buf, buf_len, 0);
}