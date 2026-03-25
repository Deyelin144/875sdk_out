/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the People's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY¡¯S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS¡¯SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY¡¯S TECHNOLOGY.
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

#include "cdxFifo.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

static mread_callback mv_mread_callback = 0;
MemHandle *mv_mopen(uint32_t size)
{
    MemHandle *mp = malloc(sizeof(MemHandle));
    if (mp == NULL) {
        return NULL;
    }

    unsigned char *mbuff = malloc(size);
    if (mbuff == NULL) {
        free(mp);
        return NULL;
    }
    memset(mbuff, 0, sizeof(MemHandle));

    mp->addr = mbuff;
    mp->mem_capacity = size;
    mp->mem_len = 0;
    mp->p = 0;
    mp->cb = NULL;
    return mp;
}

void mv_mclose(MemHandle *mp)
{
    if (mp == NULL) {
        return;
    }
    free(mp->addr);
    free(mp);
}

void mv_mread_callback_set(MemHandle *mp, mread_callback callback)
{
    mp->cb = callback;
}

void mv_mread_callback_unset(MemHandle *mp)
{
    mp->cb = NULL;
}

uint32_t mv_mread(void *_buffer, uint32_t size, uint32_t count, void *mp)
{
    CircleBufferContext *cbc = (CircleBufferContext *)mp;

    uint8_t *buffer = (uint8_t *)_buffer;
    uint32_t required_bytes = size * count;
    uint32_t read_bytes;
    uint32_t remain_bytes;
    uint32_t wp = cbc->wp;

    if (required_bytes == 0)
        return 0;

    if (wp >= cbc->rp) {
        remain_bytes = wp - cbc->rp;

        if (required_bytes > remain_bytes) {
            read_bytes = remain_bytes;
            memcpy(buffer, &cbc->address[cbc->rp], read_bytes);
            cbc->rp += read_bytes;

            if (mv_mread_callback) {
                read_bytes += mv_mread_callback(((uint8_t *)buffer) + read_bytes,
                                                required_bytes - read_bytes);
            }
        } else {
            read_bytes = required_bytes;
            memcpy(buffer, &cbc->address[cbc->rp], read_bytes);
            cbc->rp += read_bytes;
        }
    } else {
        remain_bytes = cbc->capacity - cbc->rp;

        if (required_bytes > remain_bytes) {
            read_bytes = remain_bytes;
            memcpy(buffer, &cbc->address[cbc->rp], read_bytes);

            if (required_bytes - read_bytes > wp) {
                memcpy(buffer + read_bytes, &cbc->address[0], wp);
                cbc->rp = wp;
                read_bytes += wp;

                if (mv_mread_callback) {
                    read_bytes += mv_mread_callback(((uint8_t *)buffer) + read_bytes,
                                                    required_bytes - read_bytes);
                }
            } else {
                memcpy(buffer + read_bytes, &cbc->address[0], required_bytes - read_bytes);
                cbc->rp = required_bytes - read_bytes;
                read_bytes = required_bytes;
            }
        } else {
            read_bytes = required_bytes;
            memcpy(buffer, &cbc->address[cbc->rp], read_bytes);
            cbc->rp += read_bytes;
        }
    }

    return read_bytes;
}

#define RWP_SAFE_INTERVAL (1)
uint32_t mv_mwrite(void *_buffer, uint32_t size, uint32_t count, void *mp)
{
    CircleBufferContext *cbc = (CircleBufferContext *)mp;

    uint8_t *buffer = (uint8_t *)_buffer;
    uint32_t remain_bytes;
    uint32_t write_bytes = size * count;
    uint32_t rp = cbc->rp;

    if (cbc->wp >= rp) {
        remain_bytes = cbc->capacity - cbc->wp + rp;

        if (remain_bytes >= write_bytes + RWP_SAFE_INTERVAL) {
            remain_bytes = cbc->capacity - cbc->wp;

            if (remain_bytes >= write_bytes) {
                memcpy(&cbc->address[cbc->wp], buffer, write_bytes);
                cbc->wp += write_bytes;
            } else {
                memcpy(&cbc->address[cbc->wp], buffer, remain_bytes);
                cbc->wp = write_bytes - remain_bytes;
                memcpy(&cbc->address[0], &buffer[remain_bytes], cbc->wp);
            }
        } else {
            return 0;
        }
    } else {
        remain_bytes = rp - cbc->wp;

        if (remain_bytes >= write_bytes + RWP_SAFE_INTERVAL) {
            memcpy(&cbc->address[cbc->wp], buffer, write_bytes);
            cbc->wp += write_bytes;
        } else {
            return 0;
        }
    }

    if (cbc->wp >= cbc->capacity && cbc->rp) {
        cbc->wp = 0;
    }

    return write_bytes;
}

int32_t mv_meom(void *mp)
{
    return 0; //((MemHandle*)mp)->p>=((MemHandle*)mp)->mem_len ? 1 : 0;
}

int32_t mv_msize(void *mp)
{
    CircleBufferContext *cbc = (CircleBufferContext *)mp;
    return cbc->wp >= cbc->rp ? cbc->wp - cbc->rp : cbc->capacity - cbc->rp + cbc->wp;
}

int32_t mv_mremain(void *mp)
{
    int32_t remain = ((CircleBufferContext *)mp)->capacity - mv_msize(mp);
    return remain > RWP_SAFE_INTERVAL ? remain - RWP_SAFE_INTERVAL : 0;
}

void mv_mZero(void *mp)
{
    CircleBufferContext *cbc = (CircleBufferContext *)mp;
    cbc->rp = 0;
    cbc->wp = 0;
}
/******************************************** end of file ********************************************/
