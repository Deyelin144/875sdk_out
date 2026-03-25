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

#ifndef __SUNXI_INPUT_H
#define __SUNXI_INPUT_H

#include <hal_osal.h>
#include "semphr.h"
#include "aw_list.h"
#include "input_event.h"

/*********************************bit define******************************************/
#define INPUT_BITS_PER_BYTE 8
#define INPUT_DIV_ROUND_UP(n,d)		(((n) + (d) - 1) / (d))
#define INPUT_BITS_TO_LONGS(nr)		INPUT_DIV_ROUND_UP(nr, INPUT_BITS_PER_BYTE * sizeof(long))


static inline void input_set_bit(int nr,  volatile void * addr)
{
	((int *) addr)[nr >> 5] |= (1UL << (nr & 31));
}

static inline int input_test_bit(int nr, const volatile void * addr)
{
	return (1UL & (((const int *) addr)[nr >> 5] >> (nr & 31))) != 0UL;
}


/******************************input event define***************************************/
#define EVENT_BUFFER_SIZE 64U

struct sunxi_input_event {
	unsigned int type;
	unsigned int code;
	unsigned int value;
};

struct sunxi_input_dev {
	const char *name;

	unsigned long evbit[INPUT_BITS_TO_LONGS(INPUT_EVENT_CNT)];
	unsigned long keybit[INPUT_BITS_TO_LONGS(INPUT_KEY_CNT)];
	unsigned long relbit[INPUT_BITS_TO_LONGS(INPUT_RELA_CNT)];
	unsigned long absbit[INPUT_BITS_TO_LONGS(INPUT_ABS_CNT)];
	unsigned long mscbit[INPUT_BITS_TO_LONGS(INPUT_MISC_CNT)];

	struct list_head        h_list;
	struct list_head        node;

};

//one task that read input dev, corresponds one sunxi_evdev
struct sunxi_evdev {
	int fd;
	const char *name;
	hal_sem_t sem;

	struct sunxi_input_event buffer[EVENT_BUFFER_SIZE];
	unsigned int head;
	unsigned int tail;
	unsigned int packet_head;

	struct list_head        h_list;
	struct list_head        node;

};

void input_set_capability(struct sunxi_input_dev *dev, unsigned int type, unsigned int code);
struct sunxi_input_dev *sunxi_input_allocate_device(void);
int sunxi_input_register_device(struct sunxi_input_dev *dev);
void sunxi_input_event(struct sunxi_input_dev *dev,
			unsigned int type, unsigned int code, unsigned int value);
int sunxi_input_read(int fd, void *buffer, unsigned int size);
int sunxi_input_readb(int fd, void *buffer, unsigned int size);
int sunxi_input_open(const char *name);
int sunxi_input_init(void);
static inline void input_report_key(struct sunxi_input_dev *dev, unsigned int code, int value)
{
	sunxi_input_event(dev, INPUT_EVENT_KEY, code, !!value);
}

static inline void input_report_rel(struct sunxi_input_dev *dev, unsigned int code, int value)
{
	sunxi_input_event(dev, INPUT_EVENT_REL, code, value);
}

static inline void input_report_abs(struct sunxi_input_dev *dev, unsigned int code, int value)
{
	sunxi_input_event(dev, INPUT_EVENT_ABS, code, value);
}
static inline void input_report_misc(struct sunxi_input_dev *dev, unsigned int code, int value)
{
	sunxi_input_event(dev, INPUT_EVENT_MSC, code, value);
}
static inline void input_sync(struct sunxi_input_dev *dev)
{
	sunxi_input_event(dev, INPUT_EVENT_SYN, INPUT_SYNC_REPORT, 0);
}

#endif//__SUNXI_INPUT_H
