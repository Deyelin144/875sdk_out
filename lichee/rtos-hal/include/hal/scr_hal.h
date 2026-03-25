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
#include <interrupt.h>
#include <string.h>
#include "scr_config.h"

#ifndef _SCR_HAL_H_
#define _SCR_HAL_H_

//#ifdef CONFIG_SCR_TEST

typedef struct
{
	volatile uint32_t wptr;
	volatile uint32_t rptr;
	#define  SCR_BUFFER_SIZE_MASK		0xff
	volatile uint8_t buffer[SCR_BUFFER_SIZE_MASK+1];
}scr_buffer, *pscr_buffer;

typedef struct {
	uint32_t reg_base;
	uint32_t irq_no;
	volatile uint32_t irq_accsta;
	volatile uint32_t irq_cursta;
	volatile uint32_t irq_flag;
	//control and status register config
	uint32_t csr_config;
	//interrupt enable bit map
	uint32_t inten_bm;
	//txfifo threshold
	uint32_t txfifo_thh;
	//rxfifo threahold
	uint32_t rxfifo_thh;
	//tx repeat
	uint32_t tx_repeat;
	//rx repeat
	uint32_t rx_repeat;
	//scclk divisor
	uint32_t scclk_div;
	//baud divisor
	uint32_t baud_div;
	//activation/deactivation time, in scclk cycles
	uint32_t act_time;
	//reset time, in scclk cycles
	uint32_t rst_time;
	//ATR limit time, in scclk cycles
	uint32_t atr_time;
	//gaurd time, in ETUs
	uint32_t guard_time;
	//character limit time, in ETUs
	uint32_t chlimit_time;

	uint32_t debounce_time;

	scr_buffer rxbuf;
	scr_buffer txbuf;

	volatile uint32_t detected;
	volatile uint32_t activated;
	#define SCR_ATR_RESP_INVALID  	        0
	#define SCR_ATR_RESP_FAIL				1
	#define SCR_ATR_RESP_OK					2
	volatile uint32_t atr_resp;

	uint32_t chto_flag;

}scr_struct, *pscr_struct;


#define SCR_FSM_MAX_RECORD		1024
typedef struct {
	uint32_t count;
	uint32_t old;
	uint32_t record[SCR_FSM_MAX_RECORD];
}scr_fsm_record, *pscr_fsm_record;


typedef struct{
	uint8_t TS;

	uint8_t TK[15];
	uint8_t TK_NUM;

	uint32_t T;		//Protocol
	uint32_t FMAX; //in MHz
	uint32_t F;
	uint32_t D;
	uint32_t I;   //Max Cunrrent for Program, in mA
	uint32_t P;   //Program Voltage
	uint32_t N;   //Extra Guard Time, in ETUs
}scatr_struct, *pscatr_struct;


typedef struct {
	uint8_t ppss;
	uint8_t pps0;
	uint8_t pps1;
	uint8_t pps2;
	uint8_t pps3;
	uint8_t pck;
} upps_struct, *ppps_struct;


//SCR Test Stage (State Machine)
typedef enum {sts_wait_connect=0,
	          sts_wait_act,
	          sts_wait_atr,
			  sts_warm_reset,
			  sts_wait_atr_again,
			  sts_start_pps,
			  sts_wait_pps_resp,
			  sts_send_cmd,
			  sts_start_deact,
			  sts_wait_deact,
			  sts_wait_disconnect,
			  sts_idle,
			} scr_test_stage;


#endif //_SCR_HAL_H_

