/*
 * Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
 *
 * Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
 * the the people's Republic of China and other countries.
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
#include <stdarg.h>
#include "adb_log.h"

/*
Using printf in adb shell will recursion, and stack over
*/
#if ADB_SUPPORT_DEBUG
extern int uart_console_write(const void *buffer, size_t len, void *privata_data);
void uart_printf(const char *format, ...)
{
    char buffer[256];
    va_list args;

    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (len < 0) {
        return;
    } else if (len >= (int)sizeof(buffer)) {
        len = sizeof(buffer) - 1;
    }

    if (len > 0) {
        uart_console_write((uint8_t *)buffer, (uint16_t)len, NULL);
    }
}

void print_apacket(const char *label, struct adb_packet *p)
{
    char *x, *tag;
    unsigned int count;
    struct adb_msg *msg = &p->msg;

    switch (msg->command) {
    case A_SYNC:
        tag = "SYNC";
        break;
    case A_CNXN:
        tag = "CNXN";
        break;
    case A_OPEN:
        tag = "OPEN";
        break;
    case A_OKAY:
        tag = "OKAY";
        break;
    case A_CLSE:
        tag = "CLSE";
        break;
    case A_WRTE:
        tag = "WRTE";
        break;
    case A_AUTH:
        tag = "AUTH";
        break;
    default:
        tag = "????";
        uart_printf("command:0x%08x\n", msg->command);
        break;
    }
    uart_printf("%s: %s %08x %08x %04x ,check:0x%08x, magic:0x%08x\n", label, tag, msg->arg0,
                msg->arg1, msg->data_length, msg->data_crc32, msg->magic);
    /*return;*/
    count = msg->data_length;
    x = (char *)(p->payload);
    if (count > DUMPMAX) {
        count = DUMPMAX;
        tag = "\n";
    } else {
        tag = "\"\n";
    }
    while (count-- > 0) {
        if ((*x >= ' ') && (*x < 127))
            uart_printf("%c", *x);
        else
            uart_printf(".");
        x++;
    }
    uart_printf("%s", tag);
    return;
}
#endif