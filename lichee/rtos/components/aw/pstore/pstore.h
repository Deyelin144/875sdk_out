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

#ifndef PSTORE_H
#define PSTORE_H

#ifndef _PSTORE_H_
#define _PSTORE_H_

struct pstore_header
{
    int valid;
    int payload_len;
    uint64_t index;
    uint64_t panic_time;
    struct timeval real_time;
    uint32_t crc;
};

/*
 * pstore_init
 *
 * init pstore storage addr info.
 *
 * NOTE: it will be called in the init program and the user should
 *        not call it again.
 *
 * return: if success return 0, otherwise negative.
 */
int pstore_init(uint32_t addr, uint32_t size);

/*
 * pstore_printf
 *
 * put data into pstore_buf, if buf is full, it will auto call pstore_flush.
 *
 * return: if success return 0, otherwise negative.
 */
int pstore_printf(const char *fmt, ...);

/*
 * pstore_flush
 *
 * flush current pstore_buf data to zone and move idx to next zone.
 *
 * return: if success return 0, otherwise negative.
 */
int pstore_flush(void);

/*
 * pstore_erase_all
 *
 * erase all pstore data and reset idx.
 *
 * return: if success return 0, otherwise negative.
 */
int pstore_erase_all(void);

/*
 * iterate callback
 *
 * @priv: user private pointer
 * @str: output string data
 * @is_data: if str is head info it will be set to false, otherwise true.
 *
 * return: if you want to terminate iterate, can return a negative num.
 */
typedef int (*iter_fn)(const char *str, int len, bool is_data, void *priv);

/*
 * pstore_iterate_data
 *
 * iterate data.
 * @priv: user private pointer
 * @func: will terminate if iter_fn retu negative.
 *
 * return: if success return 0, -EPERM if terminated by iter_fn.
 */
int pstore_iterate_data(iter_fn func, void *priv);

/*
 * pstore_get_date_len
 *
 * return: if success return the bytes, otherwise negative.
 */
int pstore_get_date_len(void);

/*
 * pstore_read_data
 *
 * read pstore data.
 * @buf: output buffer
 * @len: read len
 *
 * return: if success return the bytes read, otherwise negative.
 */
int pstore_read_data(void *buf, int len);

#endif

#endif  /*PSTORE_H*/
