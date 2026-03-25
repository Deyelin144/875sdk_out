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
#include <stdint.h>
#include <hal_cmd.h>
#include <hal_timer.h>
#include <sunxi_hal_ahb.h>

/* this ahb can only detect up to 3 huster at the same time */
int cmd_ahb(int argc, char **argv)
{

	int cnt = 100;
	/* 0 is curent real, 1 is sum from register, 2 is last time sum from register */
	unsigned cpu0_value[3] = {0}, cpu1_value[3] = {0}, riscv_value[3] = {0}, dsp_value[3] = {0};
	unsigned ce_value[3] = {0}, dma0_value[3] = {0}, dma1_value[3] = {0}, csi_value[3] = {0};
	unsigned st0_value[3] = {0}, st1_value[3] = {0};
	unsigned total_read = 0, total_write =  0, total[3] = {0};

	printf("============AHB TEST===============\n");
	hal_ahb_huster_enable();

	hal_msleep(1);
	printf("the bus unit is : Byte\n");
	printf("the bus bandwidth occupancy status is :\n");
	while (cnt--) {
		hal_ahb_huster_get_value(AHB_CPU0, &cpu0_value[1]);
		if (cpu0_value[1] >= cpu0_value[2]) {
			cpu0_value[0] = cpu0_value[1] - cpu0_value[2];
			cpu0_value[2] = cpu0_value[1];
		} else {
			cpu0_value[0] = cpu0_value[1];
			cpu0_value[2] = cpu0_value[1];
		}
		hal_ahb_huster_get_value(AHB_DMA0, &dma0_value[1]);
		if (dma0_value[1] >= dma0_value[2]) {
			dma0_value[0] = dma0_value[1] - dma0_value[2];
			dma0_value[2] = dma0_value[1];
		} else {
			dma0_value[0] = dma0_value[1];
			dma0_value[2] = dma0_value[1];
		}
		hal_ahb_huster_get_value(AHB_DSP, &dsp_value[1]);
		if (dsp_value[1] >= dsp_value[2]) {
			dsp_value[0] = dsp_value[1] - dsp_value[2];
			dsp_value[2] = dsp_value[1];
		} else {
			dsp_value[0] = dsp_value[1];
			dsp_value[2] = dsp_value[1];
		}
/*
		hal_ahb_huster_get_value(AHB_CPU1, &cpu1_value);
		hal_ahb_huster_get_value(AHB_RISCV, &riscv_value);
		hal_ahb_huster_get_value(AHB_DSP, &dsp_value);
		hal_ahb_huster_get_value(AHB_CE, &ce_value);
		hal_ahb_huster_get_value(AHB_DMA0, &dma0_value);
		hal_ahb_huster_get_value(AHB_DMA1, &dma1_value);
		hal_ahb_huster_get_value(AHB_CSI, &csi_value);
		hal_ahb_huster_get_value(AHB_ST0, &st0_value);
		hal_ahb_huster_get_value(AHB_ST1, &st1_value);
*/
		total_read = ahb_get_sum_total_read();
		total_write = ahb_get_sum_total_write();
		total[1] = total_read + total_write;
		if (total[1] > total[2]) {
			total[0] = total[1] - total[2];
			total[2] = total[1];
		} else {
			total[0] = total[1];
			total[2] = total[1];
		}
		printf("total           cpu0      cpu1      riscv      dsp     ce      dma0      dma1      csi      st0      st1\n");
		printf("%d        %d        %d        %d         %d      %d      %d        %d        %d       %d       %d\n",\
				total[0], cpu0_value[0], cpu1_value[0], riscv_value[0], dsp_value[0], ce_value[0],\
				dma0_value[0], dma1_value[0], csi_value[0], st0_value[0], st1_value[0]);
	}
	hal_ahb_disable_id_chan(AHB_CPU0);
	hal_ahb_disable_id_chan(AHB_DMA0);
	hal_ahb_disable_id_chan(AHB_DSP);

	return 0;
}

FINSH_FUNCTION_EXPORT_CMD(cmd_ahb, ahb_test, AHB hal APIs tests);
