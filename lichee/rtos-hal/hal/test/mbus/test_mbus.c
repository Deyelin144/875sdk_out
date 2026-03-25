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
#include <sunxi_hal_mbus.h>


int cmd_mbus(int argc, char **argv)
{

	int cnt = 1;
	int ms_delay = 0;
	int windows_us;
	uint32_t cpu_value = 0, gpu_value = 0, ve_value = 0, disp_value = 0;
	uint32_t total_value = 0, di_value = 0, oth_value = 0, csi_value = 0;
	uint32_t tvd_value = 0, g2d_value = 0, iommu_value = 0, rv_value = 0;
	uint32_t dsp_value = 0, dma0_value = 0, dma1_value = 0, de_value = 0;
	uint32_t ce_value = 0, mahb_value = 0,rv_sys_value = 0;

	printf("============MBUS TEST===============\n");
	hal_mbus_pmu_enable();

	if (argc >= 2)
		cnt = atoi(argv[1]);

	if (argc >= 3)
		ms_delay = atoi(argv[2]);

	printf("the bus bandwidth occupancy status is :\n");
	while (cnt--) {
		hal_mbus_pmu_get_value(MBUS_PMU_CPU, &cpu_value);
		hal_mbus_pmu_get_value(MBUS_PMU_GPU, &gpu_value);
		hal_mbus_pmu_get_value(MBUS_PMU_RV_SYS, &rv_sys_value);
		hal_mbus_pmu_get_value(MBUS_PMU_VE, &ve_value);
		hal_mbus_pmu_get_value(MBUS_PMU_DISP, &disp_value);
		hal_mbus_pmu_get_value(MBUS_PMU_OTH, &oth_value);
		hal_mbus_pmu_get_value(MBUS_PMU_CE, &ce_value);
		hal_mbus_pmu_get_value(MBUS_PMU_DI, &di_value);
		hal_mbus_pmu_get_value(MBUS_PMU_DE, &de_value);
		hal_mbus_pmu_get_value(MBUS_PMU_CSI, &csi_value);
		hal_mbus_pmu_get_value(MBUS_PMU_TVD, &tvd_value);
		hal_mbus_pmu_get_value(MBUS_PMU_G2D, &g2d_value);
		hal_mbus_pmu_get_value(MBUS_PMU_IOMMU, &iommu_value);
		hal_mbus_pmu_get_value(MBUS_PMU_RV_SYS, &rv_value);
		hal_mbus_pmu_get_value(MBUS_PMU_DSP_SYS, &dsp_value);
		hal_mbus_pmu_get_value(MBUS_PMU_DMA0, &dma0_value);
		hal_mbus_pmu_get_value(MBUS_PMU_DMA1, &dma1_value);
		hal_mbus_pmu_get_value(MBUS_PMU_MAHB, &mahb_value);
		hal_mbus_pmu_get_value(MBUS_PMU_TOTAL, &total_value); //mbus calculate bw every window time, total is the max one
		hal_mbus_pmu_get_window(&windows_us);
		printf("window(us) maxbw(k) cpu      gpu      ve       disp     di       csi      tvd      g2d      iommu    rv       dsp      dma0     dma1     cd       de       mahb		others	rv_sys\n");
		printf("%-10d %-8d %-8d %-8d %-8d %-8d %-8d %-8d %-8d %-8d %-8d %-8d %-8d %-8d %-8d %-8d %-8d %-8d %-8d %-8d\n",\
				windows_us , total_value, cpu_value, gpu_value, ve_value, disp_value,\
				di_value, csi_value, tvd_value, g2d_value, iommu_value,\
				rv_value, dsp_value, dma0_value, dma1_value, ce_value,\
				de_value, mahb_value, oth_value,rv_sys_value);

		if (cnt && ms_delay)
			mdelay(ms_delay);
	}

	return 0;
}

FINSH_FUNCTION_EXPORT_CMD(cmd_mbus, mbus_test, Mbus hal APIs tests);

int cmd_mbus_enable(int argc, char **argv)
{
	hal_mbus_pmu_enable();

	return 0;
}

FINSH_FUNCTION_EXPORT_CMD(cmd_mbus_enable, mbus_enable, Mbus hal enable APIs tests);

int cmd_mbus_value(int argc, char **argv)
{

	int cnt = 1;
	int ms_delay = 0;
	int windows_us;
	uint32_t cpu_value = 0, gpu_value = 0, ve_value = 0, disp_value = 0;
	uint32_t total_value = 0, di_value = 0, oth_value = 0, csi_value = 0;
	uint32_t tvd_value = 0, g2d_value = 0, iommu_value = 0, rv_value = 0;
	uint32_t dsp_value = 0, dma0_value = 0, dma1_value = 0, de_value = 0;
	uint32_t ce_value = 0, mahb_value = 0,rv_sys_value = 0;

	if (argc >= 2)
		cnt = atoi(argv[1]);

	if (argc >= 3)
		ms_delay = atoi(argv[2]);

	printf("the bus bandwidth occupancy status is :\n");
	while (cnt--) {
		hal_mbus_pmu_get_value(MBUS_PMU_CPU, &cpu_value);
		hal_mbus_pmu_get_value(MBUS_PMU_GPU, &gpu_value);
		hal_mbus_pmu_get_value(MBUS_PMU_VE, &ve_value);
		hal_mbus_pmu_get_value(MBUS_PMU_DISP, &disp_value);
		hal_mbus_pmu_get_value(MBUS_PMU_OTH, &oth_value);
		hal_mbus_pmu_get_value(MBUS_PMU_CE, &ce_value);
		hal_mbus_pmu_get_value(MBUS_PMU_DI, &di_value);
		hal_mbus_pmu_get_value(MBUS_PMU_DE, &de_value);
		hal_mbus_pmu_get_value(MBUS_PMU_CSI, &csi_value);
		hal_mbus_pmu_get_value(MBUS_PMU_TVD, &tvd_value);
		hal_mbus_pmu_get_value(MBUS_PMU_G2D, &g2d_value);
		hal_mbus_pmu_get_value(MBUS_PMU_IOMMU, &iommu_value);
		hal_mbus_pmu_get_value(MBUS_PMU_RV_SYS, &rv_value);
		hal_mbus_pmu_get_value(MBUS_PMU_DSP_SYS, &dsp_value);
		hal_mbus_pmu_get_value(MBUS_PMU_DMA0, &dma0_value);
		hal_mbus_pmu_get_value(MBUS_PMU_DMA1, &dma1_value);
		hal_mbus_pmu_get_value(MBUS_PMU_MAHB, &mahb_value);
		hal_mbus_pmu_get_value(MBUS_PMU_TOTAL, &total_value); //mbus calculate bw every window time, total is the max one
		hal_mbus_pmu_get_window(&windows_us);
		printf("window(us) maxbw(k) cpu      gpu      ve       disp     di       csi      tvd      g2d      iommu    rv       dsp      dma0     dma1     cd       de       mahb		others \n");
		printf("%-10d %-8d %-8d %-8d %-8d %-8d %-8d %-8d %-8d %-8d %-8d   0x%08x  %-8d %-8d %-8d %-8d %-8d %-8d %-8d \n",\
				windows_us , total_value, cpu_value, gpu_value, ve_value, disp_value,\
				di_value, csi_value, tvd_value, g2d_value, iommu_value,\
				rv_value, dsp_value, dma0_value, dma1_value, ce_value,\
				de_value, mahb_value, oth_value);

		if (cnt && ms_delay)
			mdelay(ms_delay);
	}

	return 0;
}

FINSH_FUNCTION_EXPORT_CMD(cmd_mbus_value, mbus_test_value, Mbus hal value APIs tests);

#ifdef CONFIG_DRIVERS_MBUS_SET_LIMIT
int cmd_mbus_set_limit(int argc, char **argv)
{
	int cnt = 1;
	int num = atoi(argv[1]);
	int value = atoi(argv[2]);

	printf("set mbus limit value: %d\n", value);
	while (cnt--) {
		hal_mbus_set_limit_value(num, value);
	}
}

FINSH_FUNCTION_EXPORT_CMD(cmd_mbus_set_limit, mbus_set_limit_test, Mbus hal value APIs tests);
#endif
