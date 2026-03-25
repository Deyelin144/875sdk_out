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

#include <hal_interrupt.h>
#include "sunxi_amp.h"

static int test_send_to_dev(sunxi_amp_info *amp, sunxi_amp_msg *msg)
{
    xQueueSend(amp->recv_queue, msg, portMAX_DELAY);
    return 0;
}

static int test_send_to_queue(sunxi_amp_info *amp, sunxi_amp_msg *msg)
{
    BaseType_t ret;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (hal_interrupt_get_nest()) {
        ret = xQueueSendFromISR(amp->send_queue, msg, &xHigherPriorityTaskWoken);
        if (ret == pdPASS) {
            (void)xHigherPriorityTaskWoken;
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
    else
        ret = xQueueSend(amp->send_queue, msg, portMAX_DELAY);

    if (ret != pdPASS) {
        return -1;
    }
    return 0;
}

static int test_receive_from_dev(sunxi_amp_info *amp, sunxi_amp_msg *msg)
{
    xQueueSend(amp->recv_queue, msg, portMAX_DELAY);
    return 0;
}

sunxi_amp_msg_ops test_ops =
{
    .send_to_queue = test_send_to_queue,
    .send_to_dev = test_send_to_dev,
    .receive_from_dev = test_receive_from_dev,
};

