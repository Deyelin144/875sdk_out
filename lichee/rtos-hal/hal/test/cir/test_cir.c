/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the people's Republic of China and other countries.
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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <hal_log.h>
#include <hal_cmd.h>
#include "sunxi_hal_cir.h"

//#define IRRX_DEG
//#define IRRX_INFO

#ifdef IRRX_DEG
#define TEST_IRRX_DEG(fmt, arg...) hal_log_debug(fmt, ##arg)
#else
#define TEST_IRRX_DEG(fmt, arg...) do {}while(0)
#endif

#ifdef IRRX_INFO
#define TEST_IRRX_INFO(fmt, arg...) hal_log_info(fmt, ##arg)
#else
#define TEST_IRRX_INFO(fmt, arg...) do {}while(0)
#endif

uint8_t cir_rxbuf[1024];

typedef union {
    struct {
		uint8_t addr;
		uint8_t addr_rev;
		uint8_t cmd;
		uint8_t cmd_rev;
    } unit;
    uint32_t data;
} cirrx_data;

static struct cirrx_code_t {
	uint32_t high_time;
	uint32_t low_time;
	uint32_t high_time_lock;
	uint32_t low_time_lock;
} cirrx_code;

#define NEC_UNIT		   	560  /* ns. Logic data bit pulse length */
#define LEADCODE_CNT_1     	16
#define LEADCODE_CNT_2	   	8
#define REPEATCODE_CNT_1   	16
#define REPEATCODE_CNT_2   	4

#define IDLE_LEVEL			0
#define LEADCODE_LEVEL		1
#define DATA_LEVEL			2
#define STOP_LEVEL	    	3
#define REPEAT_LEVEL    	4

#define CIRRX_DATA_IS_HIGH(x)  ((x >> 7) & 0x01)
#define CIRRX_DATA_GET_TIME(x, port) ((x & 0x7f) * \
			sunxi_cir_get_sample_uint_ns(port) / 1000)

#define CIRRX_CHECK(x, y)	   (x & y)

#define  CHICK_CIR_LEADCODE(x,y)                \
	(abs(x / NEC_UNIT - LEADCODE_CNT_1) <= 2    \
	&& abs(y / NEC_UNIT - LEADCODE_CNT_2) <= 2) \
	|| (abs(x / NEC_UNIT - LEADCODE_CNT_2) <= 2 \
	&& abs(y / NEC_UNIT - LEADCODE_CNT_1) <= 2) \

#define  CHICK_CIR_REPEATCODE(x,y)                \
	(abs(x / NEC_UNIT - REPEATCODE_CNT_1) <= 2    \
	&& abs(y / NEC_UNIT - REPEATCODE_CNT_2) <= 2) \
	|| (abs(x / NEC_UNIT - REPEATCODE_CNT_2) <= 2 \
	&& abs(y / NEC_UNIT - REPEATCODE_CNT_1) <= 2) \

#define  CHICK_CIR_LOGICCODE_1(x,y) \
	(x / y >= 2) || (y / x >= 2)    \

#define  CHICK_CIR_LOGICCODE_0(x,y) \
	x / y <= 1						\

uint8_t check_en;
uint8_t cirrx_parse_level;

static int nec_ir_decode(cir_port_t port, uint8_t *buf_in, uint32_t count, cirrx_data *data_out)
{
	static uint8_t cursor;
	uint32_t time_cnt, time;
	int ret = 0;
	for (int i = 0; i < count; i++) {
		if (CIRRX_DATA_IS_HIGH(cir_rxbuf[i])) {
			if(check_en) {
				check_en = 2;
				cirrx_code.high_time += CIRRX_DATA_GET_TIME(cir_rxbuf[i], port);
			}
		} else {
			if (check_en == 2) {
				cirrx_code.low_time_lock = cirrx_code.low_time;
				cirrx_code.high_time_lock = cirrx_code.high_time;
				cirrx_code.low_time = 0;
				cirrx_code.high_time = 0;
				check_en = 0;
			}
			if (check_en == 0)
				check_en = 1;
			cirrx_code.low_time += CIRRX_DATA_GET_TIME(cir_rxbuf[i], port);
		}
		if (!cirrx_code.low_time_lock || !cirrx_code.high_time_lock)
			continue;
		TEST_IRRX_DEG("L:%dus - H:%dus\n", cirrx_code.low_time_lock, cirrx_code.high_time_lock);
		if (cirrx_parse_level == IDLE_LEVEL) {
			if (CHICK_CIR_LEADCODE(cirrx_code.low_time_lock,cirrx_code.high_time_lock)) {
				cirrx_parse_level = LEADCODE_LEVEL;
				goto conti;
			}
		}
		if (cirrx_parse_level == LEADCODE_LEVEL && cirrx_parse_level != DATA_LEVEL) {
			if (CHICK_CIR_LOGICCODE_0(cirrx_code.high_time_lock,cirrx_code.low_time_lock)) {
				data_out->data &= ~(1 << cursor);
			} else if (CHICK_CIR_LOGICCODE_1(cirrx_code.high_time_lock,cirrx_code.low_time_lock)) {
				data_out->data |= 1 << cursor;
			} else {
				TEST_IRRX_INFO("x-L:%dus - H:%dus %d\n", cirrx_code.low_time_lock, cirrx_code.high_time_lock, cirrx_code.high_time_lock / cirrx_code.low_time_lock);
				ret = -1;
				goto end;
			}
			cursor++;
			if (cursor >= 32) {
				cursor = 0;
				cirrx_parse_level = DATA_LEVEL;
				if (CIRRX_CHECK(data_out->unit.addr, data_out->unit.addr_rev)) {
					TEST_IRRX_INFO("addr(%08x-%08x) check failed !", data_out->unit.addr, data_out->unit.addr_rev);
					ret = -1;
					goto end;
				}
				if (CIRRX_CHECK(data_out->unit.cmd, data_out->unit.cmd_rev)) {
					TEST_IRRX_INFO("cmd(%08x-%08x) check failed !", data_out->unit.cmd, data_out->unit.cmd_rev);
					ret = -1;
					goto end;
				}
			}
		}
		if (cirrx_parse_level != REPEAT_LEVEL) {
			if (CHICK_CIR_REPEATCODE(cirrx_code.low_time_lock,cirrx_code.high_time_lock)) {
				cirrx_parse_level = REPEAT_LEVEL;
				ret = 1;
				goto conti;
			}
		}
conti:
		cirrx_code.low_time_lock = 0;
		cirrx_code.high_time_lock = 0;
	}
end:
	if (cirrx_parse_level != DATA_LEVEL && cirrx_parse_level != REPEAT_LEVEL)
		ret = -1;
	cirrx_parse_level = IDLE_LEVEL;
	return ret;
}

static cir_callback_t cir_irq_callback(cir_port_t port, uint32_t data_type, uint32_t data)
{
	static uint32_t count;
	cirrx_data irrx_data;
	int ret = 0;
	if (data_type == RA) {
		cir_rxbuf[count ++] = data;
		TEST_IRRX_DEG("rx:0x%02x; %s : %dms\n", data, CIRRX_DATA_IS_HIGH(data) ? "H" : "L", CIRRX_DATA_GET_TIME(data, port));
	}
	if (data_type == RPE) {
		TEST_IRRX_INFO("RPE(%d)\n", count);
		ret = nec_ir_decode(port, cir_rxbuf, count, &irrx_data);
		if (ret < 0) {
			printf("ir decode failed\n");
		} else {
			if (ret == 1)
				printf("repeat code\n");
			else
				printf("addr:%02x - addr_rev:%02x - cmd:%02x - cmd_rev:%02x\n", irrx_data.unit.addr, irrx_data.unit.addr_rev, irrx_data.unit.cmd, irrx_data.unit.cmd_rev);
		}
		count = 0;
	}
	if (data_type == ROI) {
		//printf("ROI(%d)\n", count);
	}
	return 0;
}

int cmd_test_cir(int argc, char **argv)
{
    cir_port_t port;
    int ret = -1;
    int timeout_sec = 15;
    TickType_t start_ticks, current_ticks;

    printf("Run ir test\n");

    if (argc < 2)
    {
	    hal_log_err("usage: hal_ir channel\n");
	    return -1;
    }

    port = strtol(argv[1], NULL, 0);
    ret = sunxi_cir_init(port);
    if (ret) {
        hal_log_err("cir init failed!\n");
        return -1;
    }

    sunxi_cir_callback_register(port, cir_irq_callback);
    start_ticks = xTaskGetTickCount();
    printf("start_ticks: %u\n", start_ticks);

    while (1) {
	current_ticks = xTaskGetTickCount();
        if ((current_ticks - start_ticks) * portTICK_PERIOD_MS
                                >= timeout_sec * 1000) {
		printf("current_ticks: %u\n", current_ticks);
                break;
        }
    }
    //sunxi_cir_deinit(port);

    return 0;
}

FINSH_FUNCTION_EXPORT_CMD(cmd_test_cir, hal_cir, ir hal APIs tests)
