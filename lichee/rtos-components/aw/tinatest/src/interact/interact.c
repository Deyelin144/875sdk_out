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
#include <stdlib.h>
#include <string.h>

#include "tinatest.h"
#include "aw_types.h"
#include "interact-actor.h"

/********************************************************
 * [above]: library for c api
 * [below]: c/c++ api for testcase
 *******************************************************/

int ttips(const char *tips){
	int len;
	int ret;
	char name[TT_NAME_LEN_MAX+1];
	struct acting *acting = malloc(sizeof(*acting));
	if(!acting)
		return -1;

	len = min(strlen(tips), MAX_TEXT);
	acting->cmd = cmd_tips;
	tname(name);
	acting->testcase = name;//current_tt->name;
	strncpy(acting->text, tips, len);
	acting->text[len] = '\0';

	ret = interact_actor(acting);
exit:
	free(acting);
	return ret;
}

int task(const char *ask, char *reply, int length){
	int len;
	int ret;
	char name[TT_NAME_LEN_MAX+1];
	struct acting *acting = malloc(sizeof(*acting));
	if(!acting)
		return -1;

	len = min(strlen(ask), MAX_TEXT);
	acting->cmd = cmd_ask;
	tname(name);
	acting->testcase = name;//current_tt->name;
	acting->reply = reply;
	acting->len = length;
	strncpy(acting->text, ask, len);
	acting->text[len] = '\0';

	ret = interact_actor(acting);
exit:
	free(acting);
	return ret;
};

int ttrue(const char *tips){
	int len;
	int ret;
	char name[TT_NAME_LEN_MAX+1];
	struct acting *acting = malloc(sizeof(*acting));
	if(!acting)
		return -1;

	len = min(strlen(tips), MAX_TEXT);
	acting->cmd = cmd_istrue;
	tname(name);
	acting->testcase = name;//current_tt->name;
	strncpy(acting->text, tips, len);
	acting->text[len] = '\0';

	if (interact_actor(acting) < 0) {
		ERROR("interact_actor err\n");
		ret = -1;
		goto exit;
	}

	if (!strncmp(acting->reply, STR_TRUE, sizeof(STR_TRUE)))
		ret = 0;
	else
		ret = -1;
exit:
	free(acting);
	return ret;
};

struct interact_data_t{
	void *Reserved;
};

struct interact_data_t *interact_init(void){
	interact_actor_init();
	return (struct interact_data_t *)0x10;
}

int interact_exit(struct interact_data_t *data){
	
	return 0;
}

