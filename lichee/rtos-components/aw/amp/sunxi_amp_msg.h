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

#ifndef _SUNXI_AMP_MSG_H
#define _SUNXI_AMP_MSG_H

#define SUNXI_AMP_MAX_ARGS_NUM (8)

enum MSG_TYPE
{
    MSG_SERIAL_FUNC_CALL = 0,
    MSG_SERIAL_FUNC_RET,
    MSG_SERIAL_FREE_BUFFER,
    MSG_DIRECT_FUNC_CALL,
    MSG_DIRECT_FUNC_RET,
    MSG_TYPE_NUM,
};

enum FUNC_RETURN_TYPE
{
    RET_NULL = 0,
    RET_POINTER,
    RET_NUMBER_32,
    RET_NUMBER_64,
};

enum RPC_MSG_DIRECTION
{
    RPC_MSG_DIR_UNKNOWN = 0,
    RPC_MSG_DIR_CM33 = 1,
    RPC_MSG_DIR_RV,
    RPC_MSG_DIR_DSP,
};

typedef struct _sunxi_amp_msg_args_t
{
    uint32_t args_num: 8;
    uint32_t reserved;
    uint32_t args[SUNXI_AMP_MAX_ARGS_NUM];
} __attribute__((packed)) sunxi_amp_msg_args;

typedef struct _sunxi_amp_msg_t
{
    uint32_t rpcid: 16;
    uint32_t prio: 5;
    uint32_t type: 5;
    uint32_t src: 3;
    uint32_t dst: 3;
    uint32_t data;
    uint32_t flags;
} __attribute__((packed)) sunxi_amp_msg;

typedef struct _MsgBuf MsgBuf;
struct _MsgBuf
{
    int rpos;
    int wpos;
    int size;
    char *data;
    int status;
    int (*read)(MsgBuf *msgBuf, void *buffer, int len);
    int (*write)(MsgBuf *msgBuf, void *buffer, int len);
    int (*getStatus)(MsgBuf *msgBuf);
} __attribute__((packed));

typedef struct _MsgBufHead
{
    sunxi_amp_msg msg;
    int msgSize;
    int bufferSize;
    unsigned int crc16;
} __attribute__((packed)) MsgBufHeader;

typedef int (*sunxi_amp_service_func)(sunxi_amp_msg *msg, MsgBuf *msgbuf, MsgBuf **sendBuffer);

typedef struct _sunxi_amp_func_table
{
    uint32_t args_num: 8;
    uint32_t return_type: 8;
    sunxi_amp_service_func func;
} sunxi_amp_func_table;

MsgBuf *msg_buf_init(MsgBufHeader *header);
int msg_buf_deinit(MsgBuf *msgBuffer);

#endif
