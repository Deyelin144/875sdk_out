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

#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "tinatest.h"
#include "socket_main.h"
#include "init_entry.h"

#define PACKET_HEADLEN  20

static inline int fill_head_new(char *buf, int payloadsize, int externsize, int type)
{
	struct head_packet_t *head = (struct head_packet_t *)buf;
    head->magic = MAGIC;
    head->externsize = externsize;
    head->payloadsize = payloadsize;
    head->type = 1;
    head->version = 0x00000001;

    return 0;
}

pthread_mutex_t msg_id_mutex = FREERTOS_POSIX_MUTEX_INITIALIZER;
static inline int getMsgid(void)
{
	static int id_num =100 ;
	int msgid;

	pthread_mutex_lock(&msg_id_mutex);
	msgid = id_num;
	id_num++;
	pthread_mutex_unlock(&msg_id_mutex);

	return msgid;
}

int fill_msg_start(char *buf, int limit, const char *testname){
	if(!testname || !buf || (limit<=HEAD_SIZE))
		return -1;
    int msgid = getMsgid();
	int len = fill_packet_start(buf + HEAD_SIZE, limit - HEAD_SIZE, testname, msgid);
	fill_head_new(buf, len, 0, 1);
    return msgid;
}

int fill_msg_end(char *buf, int limit, const char *testname, int result, const char *mark){
	if(!testname || !buf || (limit<=HEAD_SIZE))
		return -1;
    int msgid = getMsgid();
	int len = fill_packet_end(buf + HEAD_SIZE, limit - HEAD_SIZE, testname, msgid, result, mark);
	fill_head_new(buf, len, 0, 1);
    return msgid;
}

int fill_msg_finish(char *buf, int limit, int result, const char *mark){
	if(!buf || (limit<=HEAD_SIZE))
		return -1;
    int msgid = getMsgid();
	int len = fill_packet_finish(buf + HEAD_SIZE, limit - HEAD_SIZE, msgid, result, mark);
	fill_head_new(buf, len, 0, 1);
    return msgid;
}

int fill_msg_edit(char *buf, int limit, const char *testname, const char *tip, const char *editvalue, const char *mark, int timeout){
	if(!testname || !buf || (limit<=HEAD_SIZE))
		return -1;
    int msgid = getMsgid();
	int len = fill_packet_edit(buf + HEAD_SIZE, limit - HEAD_SIZE, testname, msgid, tip, editvalue, mark, timeout);
	fill_head_new(buf, len, 0, 1);
    return msgid;
}

int fill_msg_select(char *buf, int limit, const char *testname, const char *tip, const char *mark, int timeout){
	if(!testname || !buf || (limit<=HEAD_SIZE))
		return -1;
    int msgid = getMsgid();
	int len = fill_packet_select(buf + HEAD_SIZE, limit - HEAD_SIZE, testname, msgid, tip, mark, timeout);
	fill_head_new(buf, len, 0, 1);
    return msgid;
}

int fill_msg_tip(char *buf, int limit, const char *testname, const char *tip){
	if(!testname || !buf || (limit<=HEAD_SIZE))
		return -1;
    int msgid = getMsgid();
	int len = fill_packet_tip(buf + HEAD_SIZE, limit - HEAD_SIZE, testname, msgid, tip);
	fill_head_new(buf, len, 0, 1);
    return msgid;
}


int getReturn_fix(char *buf)
{
    int ret;
    char szVal[1024];

    GetXmlNode(buf, "response", "result", szVal);

    if (strcasecmp(szVal, "ok") != 0)
    {
	    DEBUG("ttrue result : fail\n");
        return 0;
    }
    else
    {
	    DEBUG("ttrue result : ok\n");
        return 1;
    }
}


int getValue_fix(char* buf, const char* szKey, char*szValue, int *nLen)
{
    int ret;
    char value[512];
    char node[32];

    memset(value,0x0,sizeof(value));
    strcpy(node, szKey);
    ret = GetXmlNode(buf, "response", node, value);
    if(ret == -1)
    {
        ERROR("get value fialure\n");
        return ret;
    }

    strcpy(szValue, value);
    DEBUG("szValue : %s\n",szValue);

    *nLen = strlen(szValue);

    return 1;
}

