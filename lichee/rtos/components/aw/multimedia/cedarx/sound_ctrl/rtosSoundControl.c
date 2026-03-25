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

//#define LOG_TAG "tsoundcontrol"
//#include "tlog.h"
#include "FreeRTOS_POSIX/pthread.h"
#include <types.h>
#include <FreeRTOS.h>
#include "stdlib.h"
#include "rtosSound.h"
#include "aw_list.h"

#define RENDERNAME_SIZE 15

struct CdxRenderNodeS {
    struct list_head node;
    const SoundControlOpsT *ops;
    void *(*CardOpen)(int /*mode*/);
    char num;
    char name[RENDERNAME_SIZE];
};

struct CdxrRenderListS {
    struct list_head list;
    char size;
};

struct CdxrRenderListS renderList;

int CedarxRenderListInit(void)
{
    INIT_LIST_HEAD(&renderList.list);
    renderList.size = 0;
    return 0;
}

int CedarxRenderRegister(void *probe, const SoundControlOpsT *ops, const char *name)
{
    struct CdxRenderNodeS *renderNode;

    renderNode = malloc(sizeof(struct CdxRenderNodeS));
    renderNode->CardOpen = probe;
    renderNode->ops = ops;
    renderNode->num = renderList.size;
    strncpy(renderNode->name, name, RENDERNAME_SIZE);

    list_add(&renderNode->node, &renderList.list);
    renderList.size++;
    return 0;
}

#define BLOCK_MODE     0
#define NON_BLOCK_MODE 1

SoundCtrl *RTSoundDeviceCreate(int card)
{
    RtCtrlContext *s = NULL;
    struct list_head *pos = NULL;
    struct CdxRenderNodeS *iter = NULL;
    // release by card destory
    s = malloc(sizeof(RtCtrlContext));
    logd("RTSoundDeviceCreate(%d)\n", card);
    if (s == NULL) {
        loge("malloc RtCtrlContext fail.\n");
        return NULL;
    }
    memset(s, 0, sizeof(RtCtrlContext));

    list_for_each (pos, &renderList.list) {
        iter = list_entry(pos, struct CdxRenderNodeS, node);
        if (iter->num == card) {
            break;
        }
    }

    if (iter == NULL) {
        loge("render card not register %d.\n", card);
        free(s);
        return NULL;
    }

    s->handle = iter->CardOpen(BLOCK_MODE);
    if (s->handle == NULL) {
        loge("open sound device fail\n");
        free(s);
        s = NULL;
        return NULL;
    }

    s->num = card;
    s->base.ops = iter->ops;

    return (SoundCtrl *)&s->base;
}

char GetRTSoundNum(const char *name)
{
    struct list_head *pos = NULL;
    struct CdxRenderNodeS *iter = NULL;
    char card_num = -1;

    list_for_each (pos, &renderList.list) {
        iter = list_entry(pos, struct CdxRenderNodeS, node);
        if (strcmp(iter->name, name) == 0) {
            card_num = iter->num;
            break;
        }
    }

    return card_num;
}

int RTSoundCtrl(char card_num, int cmd, void *para)
{
    struct list_head *pos = NULL;
    struct CdxRenderNodeS *iter = NULL;
    int found = 0;

    list_for_each (pos, &renderList.list) {
        iter = list_entry(pos, struct CdxRenderNodeS, node);
        if (iter->num == card_num) {
            found = 1;
            break;
        }
    }

    if (found == 1) {
        return iter->ops->control(NULL, cmd, para);
    }

    return -1;
}