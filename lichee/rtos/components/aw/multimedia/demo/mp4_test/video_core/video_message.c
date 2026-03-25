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

#include "video_message.h"
#include "video_fsm.h"
#include "video_debug/vd_log.h"

static hal_queue_t q_video = NULL;
static hal_sem_t sem_video = NULL;

int video_command_message_create(void)
{
    if (q_video != NULL) {
        vlog_error("video message has been created!");
        return -1;
    }

    q_video = xQueueCreate(5, sizeof(video_message));
    if (q_video == NULL) {
        vlog_error("video message create fail");
        return -1;
    }

    sem_video = hal_sem_create(0);
    if (sem_video == NULL) {
        vlog_error("video message create fail");
        hal_queue_delete(q_video);
        return -1;
    }

    return 0;
}

void video_command_message_destory(void)
{
    if (NULL != q_video) {
        vQueueDelete(q_video);
        q_video = NULL;
    }

    if (NULL != sem_video) {
        hal_sem_delete(sem_video);
        sem_video = NULL;
    }
}

int video_command_message_reset(void)
{
    if (q_video == NULL) {
        vlog_error("no msg can use!");
        return -1;
    }

    BaseType_t ret = xQueueReset(q_video);
    if (ret != pdPASS) {
        return -1;
    }

    int sem_value = 0;
    hal_sem_getvalue(sem_video, &sem_value);
    for (int i = 0; i < sem_value; i++) {
        hal_sem_post(sem_video);
    }
    return 0;
}

int video_command_message_send(video_message *msg, unsigned block)
{
    if (q_video == NULL) {
        vlog_error("no msg can use!");
        return -1;
    }

    BaseType_t ret = 0;
    if (block == 1) {
        msg->sem = sem_video;
    } else {
        msg->sem = NULL;
    }

    ret = xQueueSend(q_video, msg, DEFAULT_VIDEO_MESSAGE_TIMEOUT);
    if (ret != pdPASS) {
        return -1;
    }

    if (block == 1) {
        hal_sem_wait(sem_video);
    }
    return 0;
}

int video_command_message_send_front(video_message *msg, unsigned block)
{
    if (q_video == NULL) {
        vlog_error("no msg can use!");
        return -1;
    }

    BaseType_t ret = 0;
    if (block == 1) {
        msg->sem = sem_video;
    } else {
        msg->sem = NULL;
    }

    ret = xQueueSendToFront(q_video, msg, 0);
    if (ret != pdPASS) {
        return -1;
    }

    if (block == 1) {
        hal_sem_wait(sem_video);
    }
    return 0;
}

void video_command_message_recv(video_message *msg)
{
    xQueueReceive(q_video, msg, -1);
}