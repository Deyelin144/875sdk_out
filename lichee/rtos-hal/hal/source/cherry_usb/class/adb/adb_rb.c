/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the people's Republic of China and other countries.
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
#include <stdbool.h>
#include <FreeRTOS.h>
#include "adb_shell.h"
#include "adb_rb.h"
#include "misc.h"

struct adb_rb {
	unsigned char *buffer;
	unsigned int length;
	volatile unsigned int start,end,isfull;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	adb_ev *ev;
	/* lock */
};

adb_rb *adb_ringbuffer_init(int size)
{
	adb_rb *rb = NULL;

	if (size <= 0)
		return NULL;
	rb = adb_calloc(1, sizeof(adb_rb));
	if (!rb)
		USB_LOG_ERR("no memory\n");
	rb->buffer = adb_malloc(size);
	if (!rb->buffer)
		USB_LOG_ERR("no memory\n");
	rb->length = size;
	rb->start = 0;
	rb->end = 0;
	pthread_mutex_init(&rb->mutex, NULL);
	pthread_cond_init(&rb->cond, NULL);
	rb->ev = adb_event_init();
	return rb;
}

void adb_ringbuffer_release(adb_rb *rb)
{
	if (!rb)
		return;
	if (rb->buffer)
		adb_free(rb->buffer);
	pthread_mutex_destroy(&rb->mutex);
	pthread_cond_destroy(&rb->cond);
	adb_event_release(rb->ev);
	adb_free(rb);
	return;
}

int adb_ringbuffer_get(adb_rb *rb, void *buf, int size, int timeout)
{
	int len, cross = 0;

	if (!rb)
		return -1;
	if (rb->isfull) {
		len = rb->length;
		goto cal_actual_size;
	}
	if (rb->end - rb->start == 0 && timeout >= 0) {
		adb_event_get(rb->ev, ADB_EV_WRITE, timeout);
	}
	len = rb->end - rb->start;
	if (len == 0) {
		return 0;
	} else if (len < 0) {
		len += rb->length;
	} else  if (len > rb->length) {
		USB_LOG_ERR("len=%d, error\n", len);
	}
cal_actual_size:
	len = len > size ? size : len;
	if (rb->start + len >= rb->length)
		cross = 1;
	if (cross != 0) {
		int first = rb->length - rb->start;
		memcpy(buf, rb->buffer + rb->start, first);
		memcpy(buf + first, rb->buffer, len - first);
		rb->start = len - first;
	} else {
		memcpy(buf, rb->buffer + rb->start, len);
		rb->start += len;
	}
	if (rb->isfull && len != 0)
		rb->isfull = 0;
	return len;
}

int adb_ringbuffer_put(adb_rb *rb, const void *buf, int size)
{
	int len, cross = 0;

	if (!rb)
		return -1;
	if (rb->isfull) {
		adb_event_set(rb->ev, ADB_EV_WRITE);
		return 0;
	}
	if (rb->start > rb->end)
		len = rb->start - rb->end;
	else
		len = rb->length - (rb->end - rb->start);
	len = len > size ? size : len;
	if (rb->end + len > rb->length)
		cross = 1;
	if (cross != 0) {
		int first = rb->length - rb->end;
		memcpy(rb->buffer + rb->end, buf, first);
		memcpy(rb->buffer, buf + first, len - first);
		rb->end = len - first;
	} else {
		memcpy(rb->buffer + rb->end, buf, len);
		rb->end += len;
		rb->end %= rb->length;
	}
	if (rb->end == rb->start && len != 0) {
		rb->isfull = 1;
	}
	if (rb->isfull || rb->start != rb->end)
		adb_event_set(rb->ev, ADB_EV_WRITE);
	return len;
}

struct adb_ev {
	EventGroupHandle_t handle;
};

adb_ev *adb_event_init(void)
{
	adb_ev *ev = adb_calloc(1, sizeof(adb_ev));
	if (!ev)
		USB_LOG_ERR("no memory\n");
	ev->handle = xEventGroupCreate();
	if (!ev->handle)
		USB_LOG_ERR("Event Group Create failed\n");
	return ev;
}

void adb_event_release(adb_ev *ev)
{
	vEventGroupDelete(ev->handle);
	adb_free(ev);
}

int adb_event_set(adb_ev *ev, int bits)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	xEventGroupSetBitsFromISR(ev->handle, bits, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	return 0;
}

int adb_event_get(adb_ev *ev, int bits, int ms)
{
	EventBits_t uxBits;
	TickType_t xTicksToWait = ms / portTICK_PERIOD_MS;
	if (!ms)
		xTicksToWait = portMAX_DELAY;
	else if (ms < 0)
		xTicksToWait = 0;

	do {
		uxBits = xEventGroupWaitBits(ev->handle, bits, pdTRUE, pdFALSE, xTicksToWait);
		if (!(uxBits & bits)) {
			if (ms > 0)
				return -1;
			else if (ms < 0)
				return 0;
			continue;
		}
		break;
	} while (!ms);

	return uxBits;
}