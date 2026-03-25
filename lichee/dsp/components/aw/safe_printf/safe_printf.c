/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the People's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY'S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS'SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY'S TECHNOLOGY.
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

#include <FreeRTOS.h>
#include <semphr.h>

#include <hal_hwspinlock.h>
#include <hal_interrupt.h>
#include <hal_thread.h>

#include <hal_uart.h>
#include <console.h>

#include <xstdio.h>

#ifdef CONFIG_COMPONENTS_OPENAMP_LOG_TRACE
void log_buff(int ch);
#endif

static void *custom_prout(void *fp, const char *buffer, size_t len)
{
	const unsigned char *cbuf = (const unsigned char *)buffer;
	int i, c;

	for (i = 0; i < len; i++) {
		c = *cbuf++;
#ifdef CONFIG_COMPONENTS_OPENAMP_LOG_TRACE
		log_buff(c);
#endif

		if (console_uart != UART_UNVALID) {
#if defined(CONFIG_DRIVERS_UART) && !defined(CONFIG_DISABLE_ALL_UART_LOG)
			if (c == '\n')
				hal_uart_put_char(console_uart, '\r');

			hal_uart_put_char(console_uart, c);
#endif
		}
	}
	return fp;
}

//The function '_Printf' is in libc, it's prototype is below:
//extern int _Printf(void *(*)(void *, const char *, size_t), void *, const char *, va_list, int);

static SemaphoreHandle_t s_printf_mutex;

int printf(const char *format, ...)
{
	va_list va;

	if (!hal_thread_is_in_critical_context())
	{
		if (!s_printf_mutex)
			s_printf_mutex = xSemaphoreCreateMutex();
		xSemaphoreTake(s_printf_mutex, portMAX_DELAY);
	}

	if (!hal_thread_is_in_critical_context())
		hal_hwspin_lock(SPINLOCK_CLI_UART_LOCK_BIT);

	int temp = 0;
	va_start(va, format);
	int ret = _Printf(custom_prout, stdout, format, va, (int)&temp);
	va_end(va);

	if (!hal_thread_is_in_critical_context())
		hal_hwspin_unlock(SPINLOCK_CLI_UART_LOCK_BIT);

	if (!hal_thread_is_in_critical_context())
		xSemaphoreGive(s_printf_mutex);
	return ret;
}
