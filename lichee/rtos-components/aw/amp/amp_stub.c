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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <reent.h>

#include "sunxi_amp.h"
#include <console.h>

#include <hal_cache.h>

unsigned long wait_ret_value(sunxi_amp_wait *wait)
{
    xSemaphoreTake(wait->signal, portMAX_DELAY);
    return wait->msg.data;
}

unsigned long wait_ret_value_msgbuf(sunxi_amp_wait *wait)
{
    xSemaphoreTake(wait->signal, portMAX_DELAY);
    MsgBuf *msgbuf = msg_buf_init((MsgBufHeader *)(unsigned long)wait->msg.data);
    MsgBufHeader *header = (MsgBufHeader *)msgbuf->data;
    hal_dcache_invalidate((unsigned long)header, sizeof(*header));
    hal_dcache_invalidate((unsigned long)header, header->bufferSize);
    dump_amp_msg(&header->msg);
    return (unsigned long)msgbuf;
}

int remote_serial_func_call_free_buffer(sunxi_amp_msg *msg, MsgBuf *msgBuffer)
{
    sunxi_amp_msg send_msg;

    memset(&send_msg, 0, sizeof(send_msg));

    send_msg.type = MSG_SERIAL_FREE_BUFFER;
    send_msg.rpcid = 0;
    send_msg.src = msg->dst;
    send_msg.dst = msg->src;
    send_msg.data = (uint32_t)(unsigned long)msgBuffer->data;
    hal_amp_msg_send(&send_msg);
    return 0;
}
/*
 * func_stub:
 */
unsigned long func_stub_test(uint32_t id, MsgBuf *msgBuffer, int haveRet, int stub_args_num, void *stub_args[])
{
    sunxi_amp_msg msg;
    sunxi_amp_msg_args args;
    sunxi_amp_wait wait;
    sunxi_amp_info *amp_info;
    MsgBufHeader *header;
    long ret = 0;
    int i;

    amp_info = get_amp_info();

    memset(&msg, 0, sizeof(msg));
    memset(&args, 0, sizeof(args));
    memset(&wait, 0, sizeof(wait));

    msg.type = MSG_SERIAL_FUNC_CALL;
    msg.rpcid = (id & 0xffff);
    msg.src = (id >> 24) & 0xf;
    msg.dst = (id >> 28) & 0xf;
    msg.data = (uint32_t)(unsigned long)msgBuffer->data;
    msg.flags = (uint32_t)(unsigned long)xTaskGetCurrentTaskHandle();

    args.args_num = stub_args_num;

    for (i = 0; i < stub_args_num; i++)
    {
        args.args[i] = 0;
    }
    header = (MsgBufHeader *)(msgBuffer->data);
    memcpy(&header->msg, &msg, sizeof(sunxi_amp_msg));
    header->msgSize = msgBuffer->wpos + sizeof(MsgBufHeader);
    header->bufferSize = msgBuffer->size;
    hal_dcache_clean((unsigned long)msgBuffer->data, msgBuffer->size);

    if (haveRet)
    {
        wait.flags = msg.flags;
        wait.task = xTaskGetCurrentTaskHandle();
        wait.signal = xSemaphoreCreateCounting(0xffffffffU, 0);
        if (wait.signal == NULL)
        {
            amp_err("create signal failed\n");
            return -1;
        }
        INIT_LIST_HEAD(&wait.i_list);
        taskENTER_CRITICAL();
        list_add(&wait.i_list, &amp_info->wait.i_list);
        taskEXIT_CRITICAL();
        hal_amp_msg_send(&msg);
        ret = wait_ret_value_msgbuf(&wait);
        vSemaphoreDelete(wait.signal);
    }
    else
    {
        hal_amp_msg_send(&msg);
    }
    return ret;
}

/*
 * func_stub:
 */
unsigned long func_stub(uint32_t id, int haveRet, int stub_args_num, void *stub_args[])
{
    sunxi_amp_msg msg;
    sunxi_amp_msg_args __attribute__((aligned(64))) args;
    sunxi_amp_wait wait;
    sunxi_amp_info *amp_info;
    int ret = 0;
    int i;

    amp_info = get_amp_info();

    memset(&msg, 0, sizeof(msg));
    memset(&args, 0, sizeof(args));
    memset(&wait, 0, sizeof(wait));

    msg.type = MSG_DIRECT_FUNC_CALL;
    msg.rpcid = (id & 0xffff);
    msg.src = (id >> 26) & 0xf;
    msg.dst = (id >> 29) & 0xf;
    msg.prio = uxTaskPriorityGet(NULL);
    msg.data = (uint32_t)(unsigned long)&args;
    msg.flags = (uint32_t)(unsigned long)xTaskGetCurrentTaskHandle();

    args.args_num = stub_args_num;

    for (i = 0; i < stub_args_num; i++)
    {
        args.args[i] = (uint32_t)(unsigned long)stub_args[i];
    }
    hal_dcache_clean((unsigned long)&args, sizeof(args));

    if (haveRet)
    {
        wait.flags = msg.flags;
        wait.task = xTaskGetCurrentTaskHandle();
        wait.signal = xSemaphoreCreateCounting(0xffffffffU, 0);
        if (wait.signal == NULL)
        {
            amp_err("create signal failed\n");
            return -1;
        }
        INIT_LIST_HEAD(&wait.i_list);
        taskENTER_CRITICAL();
        list_add(&wait.i_list, &amp_info->wait.i_list);
        taskEXIT_CRITICAL();
        hal_amp_msg_send(&msg);
        ret = wait_ret_value(&wait);
        vSemaphoreDelete(wait.signal);
    }
    else
    {
        hal_amp_msg_send(&msg);
    }
    return ret;
}

