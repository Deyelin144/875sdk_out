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
#include <stdlib.h>
#include "tinatest.h"
#include "interact.h"
#include "interact-actor.h"

struct list_head list_actor; // list head for all actors.
int cnt_actor;           // count for all actors.

#define INIT_LNODE() \
    struct actor *act = calloc(sizeof(struct actor), 1); \
    if (NULL == act) { \
        ERROR("malloc failed \n"); \
        return -1; \
    } \
    if (NULL == list_actor.prev) \
        INIT_LIST_HEAD(&list_actor); \
    list_add_tail(&act->lnode, &list_actor); \
    cnt_actor++; \
    DEBUG("register interact: %d\n", cnt_actor);

#define ADD_CMD(CMD) \
    act->func[cmd_ ## CMD] = (cmd_func)CMD;

int interact_register(
        f_ask ask,
        f_tips tips,
        f_istrue istrue)
{
    INIT_LNODE();

    ADD_CMD(ask);
    ADD_CMD(tips);
    ADD_CMD(istrue);

    return 0;
}

/***********************************************************
 * [below] Function to do command.
 **********************************************************/
static int interact_do_ask(struct acting *acting, struct actor *act)
{
    return ((f_ask)(act->func[acting->cmd]))(acting->testcase, acting->text, acting->reply, acting->len);
}

static int interact_do_istrue(struct acting *acting, struct actor *act)
{
    int ret = ((f_istrue)(act->func[acting->cmd]))(acting->testcase, acting->text);
    if (-1 == ret)
        return -1;
    acting->reply = (ret == (int)true ? STR_TRUE : STR_FALSE);
    return 0;
}

static int interact_do_tips(struct acting *acting, struct actor *act)
{
    return ((f_tips)(act->func[acting->cmd]))(acting->testcase, acting->text);
}

/***********************************************************
 * [above] Function to do command.
 **********************************************************/

#define interact_do(CMD, acting, need_resp) \
    if (need_resp != false) { \
        acting->need_respond = true; \
    } \
    if (!act->func[acting->cmd]) \
        return -1; \
    return interact_do_ ## CMD(acting, act);

int interact_actor_do(struct acting *acting, struct actor *act)
{
    switch (acting->cmd) {
    case cmd_ask:
	   interact_do(ask, acting, true);
	   break;
    case cmd_istrue:
	   interact_do(istrue, acting, true);
	   break;
    case cmd_tips:
	   interact_do(tips, acting, false);
	   break;
    default:
        return -1;
    }

    return 0;
}

int interact_actor(struct acting *acting)
{
    struct actor *act = NULL;

    list_for_each_entry(act, &list_actor, lnode) {
	    if (interact_actor_do(acting, act) < 0) {
		    ERROR("do acting->cmd err %d\n", acting->cmd);
		    return -1;
	    }
    }

    return 0;
}

void interact_actor_init(void)
{
	INIT_LIST_HEAD(&list_actor);
	cnt_actor = 0;
}