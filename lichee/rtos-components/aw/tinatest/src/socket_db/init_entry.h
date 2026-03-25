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

#ifndef __INIT_ENTRY_H
#define __INIT_ENTRY_H

#include "xml_packet.h"

#define PASS 1
#define FAILED 0


#define MAGIC 0x54545450
#define VERSION 0x00000001

//20 byte head info
struct head_packet_t{
    unsigned int magic;
    unsigned int version;
    unsigned int type;
    unsigned int payloadsize;
    unsigned int externsize;
};
#define HEAD_SIZE (sizeof(struct head_packet_t))

//head file for init_entry.c
int init_entry();

int fill_msg_start(char *buf, int limit, const char *testname);
int fill_msg_end(char *buf, int limit, const char *testname, int result, const char *mark);
int fill_msg_finish(char *buf, int limit, int result, const char *mark);
int fill_msg_edit(char *buf, int limit, const char *testname, const char *tip, const char *editvalue, const char *mark, int timeout);
int fill_msg_select(char *buf, int limit, const char *testname, const char *tip, const char *mark, int timeout);
int fill_msg_tip(char *buf, int limit, const char *testname, const char *tip);

/* fix bug for testcase */
int sendCMDOperator_fix(const char *testname, const char *plugin, const char * datatype, char * data, char *buf);

int getPacketType_fix(char *buf, char *type);
int getValue_fix(char *buf, const char* szKey, char*szValue, int *nLen);
int getReturn_fix(char *buf);

#endif
