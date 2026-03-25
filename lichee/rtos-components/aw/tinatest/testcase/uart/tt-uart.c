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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tinatest.h>
#include <hal_timer.h>
#include <hal/hal_uart.h>

#define UART_PORT	CONFIG_UART_PORT
#define UART_BAUDRATE   CONFIG_UART_BAUDRATE

//tt uarttester
int tt_uarttest(int argc, char **argv)
{
	char tbuf[6] = {"hello"};
	uint8_t rbuf[10] = {0};
	uart_port_t port;
	uint32_t baudrate;
	_uart_config_t uart_config;
	int i;

	printf("==========Begin to test uart===========\n");
	printf("Note that:short circuit tx and rx!\n");
	port = strtol(UART_PORT, NULL, 0);
	baudrate = strtol(UART_BAUDRATE, NULL, 0);

	printf("test args is:\n");
	printf("uart port:uart%d\n", port);
	printf("uart baudrate:%d\n", baudrate);

	memset(rbuf, 0, 10 * sizeof(uint8_t));

	switch (baudrate) {
	case 4800:
		uart_config.baudrate = UART_BAUDRATE_4800;
		break;
	case 9600:
		uart_config.baudrate = UART_BAUDRATE_9600;
		break;
	case 115200:
		uart_config.baudrate = UART_BAUDRATE_115200;
		break;
	default:
		uart_config.baudrate = UART_BAUDRATE_115200;
		break;
	}

	uart_config.word_length = UART_WORD_LENGTH_8;
	uart_config.stop_bit = UART_STOP_BIT_1;
	uart_config.parity = UART_PARITY_NONE;

	hal_uart_init(port);
	hal_uart_control(port, 0, &uart_config);
	hal_uart_disable_flowcontrol(port);
	hal_uart_set_loopback(port, 1);

	/* send */
	hal_uart_send(port, tbuf, 5);

	printf("Sending:");
	for (i = 0; i < 5; i++)
		printf("%c", tbuf[i]);
	printf("\n");

	/* receive */
	hal_uart_receive_no_block(port, rbuf, 5, MS_TO_OSTICK(1000));

	printf("Receiving:");
	for (i = 0; i < 5; i++)
		printf("%c", rbuf[i]);
	printf("\n");

	/* verify data */
	for (i = 0; i < 5; i++) {
		if (tbuf[i] != rbuf[i])
			break;
	}
	if (i == 5)
		printf("==========Test uart%d success!========\n", port);
	else
		printf("!!Test uart%d failed!\n", port);
	hal_msleep(1000);
	hal_uart_deinit(port);

	return 0;
}
testcase_init(tt_uarttest, uarttester, uarttester for tinatest);
