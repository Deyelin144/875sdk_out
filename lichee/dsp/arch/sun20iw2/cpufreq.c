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

#include "cpufreq.h"
#include <hal_clk.h>

typedef struct cpu_freq_setting
{
	const uint32_t freq;
	int div_hw_cnt;
	hal_clk_type_t parent_clk_type;
	hal_clk_id_t parent_clk_id;
	uint32_t voltage; //unit: mV
} cpu_freq_setting_t;

static cpu_freq_setting_t vf_table1[] =
{
#ifdef CONFIG_DSP_IS_POWERED_BY_INTERNAL_LDO
	/* The voltage of DSP should be set to 1.775V when powered externally, otherwise 1.225V */
	{ 400000000, 2, HAL_SUNXI_AON_CCU, CLK_CKPLL_HIFI5_SEL, 1225},
#else
	{ 400000000, 2, HAL_SUNXI_AON_CCU, CLK_CKPLL_HIFI5_SEL, 1175},
#endif
};

static cpu_freq_setting_t *cpu_freq_table = NULL;

uint32_t get_available_cpu_freq_num(void)
{
	return sizeof(vf_table1)/sizeof(vf_table1[0]);
}

int get_available_cpu_freq(uint32_t freq_index, uint32_t *cpu_freq)
{
	uint32_t tmp_voltage = 0;
	return get_available_cpu_freq_info(freq_index, cpu_freq, &tmp_voltage);
}

int get_available_cpu_freq_info(uint32_t freq_index, uint32_t *cpu_freq, uint32_t *cpu_voltage)
{
	if (freq_index < 0 || freq_index >= get_available_cpu_freq_num())
		return -1;

	if (!cpu_freq_table)
		return -2;

	*cpu_freq = cpu_freq_table[freq_index].freq;
	*cpu_voltage = cpu_freq_table[freq_index].voltage;
	return 0;
}

#define GPRCM_BASE_ADDR 0x40050000
#define DSP_LDO_CTRL_REG_OFFSET 0x4C
#define MAX_OUTPUT_VOLTAGE 1375
#define MIN_OUTPUT_VOLTAGE 600
#define VOLTAGE_STEP 25
#define MAX_LDO_VOL_FIELD_VALUE 0x1F
#define EFUSE_LDO_ADDR		(0x4004e624)
#define EFUSE_LDO_MASK		(0x1FU)
#define EFUSE_LDO_DSP_SHIFT	(17)
#define EFUSE_VF_ADDR		(0x4004e648)
#define EFUSE_VF_MASK		(0xFFU)
#define EFUSE_VF_SHIFT		(4)
#define LDO_CAL_BASE		(1125)

#define GET_BIT_VAL(reg, shift, vmask)  (((reg) >> (shift)) & (vmask))
static uint32_t GetDSPLDOCalibrationValue(void)
{
	return GET_BIT_VAL(*(volatile uint32_t *)(long)EFUSE_LDO_ADDR, EFUSE_LDO_DSP_SHIFT, EFUSE_LDO_MASK);
}

static int set_cpu_voltage(uint32_t vol)
{
	uint32_t reg_data, reg_addr, ldo_vol_field, ldo_cal;

	if ((vol < MIN_OUTPUT_VOLTAGE) || (vol > MAX_OUTPUT_VOLTAGE))
		return -1;

	ldo_cal = GetDSPLDOCalibrationValue();
	if (ldo_cal)
		ldo_vol_field = ldo_cal + ((int)vol - LDO_CAL_BASE) / VOLTAGE_STEP;
	else
		ldo_vol_field = ((vol - MIN_OUTPUT_VOLTAGE) + (VOLTAGE_STEP - 1))/VOLTAGE_STEP;

	if (ldo_vol_field > MAX_LDO_VOL_FIELD_VALUE)
	{
		return -2;
	}

	reg_addr = GPRCM_BASE_ADDR + DSP_LDO_CTRL_REG_OFFSET;
	reg_data = readl(reg_addr);
	reg_data &= ~(MAX_LDO_VOL_FIELD_VALUE << 4);
	reg_data |= (ldo_vol_field & MAX_LDO_VOL_FIELD_VALUE) << 4;
	writel(reg_data, reg_addr);

	return 0;
}

int get_cpu_voltage(uint32_t *cpu_voltage)
{
	uint32_t reg_data, reg_addr, ldo_vol_field, ldo_cal;

	reg_addr = GPRCM_BASE_ADDR + DSP_LDO_CTRL_REG_OFFSET;
	reg_data = readl(reg_addr);
	ldo_vol_field = (reg_data >> 4) & MAX_LDO_VOL_FIELD_VALUE;

	ldo_cal = GetDSPLDOCalibrationValue();
	if (ldo_cal)
		*cpu_voltage = LDO_CAL_BASE + VOLTAGE_STEP * ((int)ldo_vol_field - ldo_cal);
	else
		*cpu_voltage = MIN_OUTPUT_VOLTAGE + VOLTAGE_STEP * ldo_vol_field;

	return 0;
}

static int set_first_div_freq(uint32_t target_freq, hal_clk_t next_clk)
{
	int ret = 0;
	hal_clk_t clk_dpll3_hifi5_div;

	clk_dpll3_hifi5_div = hal_clock_get(HAL_SUNXI_AON_CCU, CLK_CK3_HIFI5);
	if (!clk_dpll3_hifi5_div)
	{
		ret = -1;
		goto err_get_clk_dpll3_div;
	}

	ret = hal_clk_set_rate(clk_dpll3_hifi5_div, target_freq);
	if (HAL_CLK_STATUS_OK != ret)
	{
		ret = -2;
		goto err_set_dpll3_div_freq;
	}

	ret = hal_clk_set_parent(next_clk, clk_dpll3_hifi5_div);
	if (HAL_CLK_STATUS_OK != ret)
	{
		ret = -3;
		goto err_set_first_mux_parent;
	}

err_set_first_mux_parent:
err_set_dpll3_div_freq:
	hal_clock_put(clk_dpll3_hifi5_div);

err_get_clk_dpll3_div:
	return ret;
}

int set_cpu_freq(uint32_t target_freq)
{
	int ret = 0;
	uint32_t i = 0, size = get_available_cpu_freq_num();
	int is_increase_freq = 0;
	uint32_t current_clk_freq = 0;
	cpu_freq_setting_t *freq_setting;
	hal_clk_t clk_hifi5_mux, clk_hifi5_div;
	hal_clk_t pclk;

	for (i = 0; i < size; i++)
	{
		if (cpu_freq_table[i].freq == target_freq)
			break;
	}

	if (i == size)
		return -1;

	freq_setting = &cpu_freq_table[i];

	if (freq_setting->freq == 0)
		return -2;

	ret = get_cpu_freq(&current_clk_freq);
	if (ret)
	{
		return -3;
	}

	if (current_clk_freq == target_freq)
	{
		return 0;
	}

	if (current_clk_freq < target_freq)
	{
		is_increase_freq = 1;
	}

	if (is_increase_freq)
	{
		ret = set_cpu_voltage(freq_setting->voltage);
		if (ret)
		{
			return -4;
		}
	}

	pclk = hal_clock_get(freq_setting->parent_clk_type, freq_setting->parent_clk_id);
	if (!pclk)
	{
		ret = -5;
		goto err_get_pclk;
	}

	if (freq_setting->div_hw_cnt > 1)
	{
		ret = set_first_div_freq(target_freq, pclk);
		if (ret < 0)
		{
			ret = -6;
			goto err_set_first_div_freq;
		}
	}

	clk_hifi5_mux = hal_clock_get(HAL_SUNXI_CCU, CLK_DSP_SEL);
	if (!clk_hifi5_mux)
	{
		ret = -7;
		goto err_get_hifi5_mux;
	}

	ret = hal_clk_set_parent(clk_hifi5_mux, pclk);
	if (ret)
	{
		ret = -8;
		goto err_set_parent;
	}

	clk_hifi5_div = hal_clock_get(HAL_SUNXI_CCU, CLK_DSP_DIV);
	if (!clk_hifi5_div)
	{
		ret = -9;
		goto err_get_clk_div;
	}

	ret = hal_clk_set_rate(clk_hifi5_div, target_freq);
	if (HAL_CLK_STATUS_OK != ret)
	{
		ret = -10;
		goto err_set_hifi5_div_freq;
	}

	if (!is_increase_freq)
	{
		ret = set_cpu_voltage(freq_setting->voltage);
		if (ret)
		{
			ret = -11;
		}
	}

err_set_hifi5_div_freq:
	hal_clock_put(clk_hifi5_div);

err_get_clk_div:

err_set_parent:
	hal_clock_put(clk_hifi5_mux);

err_get_hifi5_mux:

err_set_first_div_freq:
	hal_clock_put(pclk);

err_get_pclk:
	return ret;
}

int get_cpu_freq(uint32_t *cpu_freq)
{
	hal_clk_t clk = NULL;
	uint32_t clk_freq;

	clk = hal_clock_get(HAL_SUNXI_CCU, CLK_DSP_DIV);
	if (!clk)
	{
		return -1;
	}

	clk_freq = hal_clk_get_rate(clk);
	hal_clock_put(clk);

	if (clk_freq == 0)
	{
		return -2;
	}

	*cpu_freq = clk_freq;
	return 0;
}

int cpufreq_vf_init(void)
{
	cpu_freq_table = vf_table1;

	return 0;
}

