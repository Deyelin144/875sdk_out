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
#include <stdlib.h>
#include <stdio.h>
#include <tinatest.h>

#include <hal_cmd.h>
#include <cpufreq.h>

//extern int set_cpu_freq(uint32_t target_freq);
extern uint32_t get_available_cpu_freq_num(void);
extern int get_available_cpu_freq_info(uint32_t freq_index, uint32_t *cpu_freq, uint32_t *cpu_voltage);
extern int get_cpu_voltage(uint32_t *cpu_voltage);
extern int get_cpu_freq(uint32_t *cpu_freq);

int tt_cpu_freq(int argc, char **argv)
{
	int times = strtol(CONFIG_CPU_FREQ_LOOP_TIMES, NULL, 0);

	int ret = 0;
	uint32_t cpu_freq = 0, cpu_voltage = 0, cpu_freq_get = 0, cpu_voltage_get, freq_num, freq_index;

	printf("=============TEST FOR CPU FREQ===========\n");

	while(times) {
		times --;
		/*get available cpu freq info*/
		freq_num = get_available_cpu_freq_num();
		if (!freq_num) {
		printf("no available cpu frequency!\n");
		goto fail;
		}

		for (freq_index = 0; freq_index < freq_num; freq_index++) {
			ret = get_available_cpu_freq_info(freq_index, &cpu_freq, &cpu_voltage);
			if (ret) {
				printf("get_available_cpu_freq_info failed, freq_index: %d, ret: %d\n", freq_index, ret);
				goto fail;
			} else {
				printf("available cpu frequency %d: %uHz, %umV\n", freq_index, cpu_freq, cpu_voltage);
			}
			/*set cpu frequency */
		/*	ret = set_cpu_freq(cpu_freq);
			if (ret) {
				printf("set_cpu_freq failed, ret: %d\n", ret);
				goto fail;
			}
			printf("set_cpu_freq ok, frequency: %d\n", cpu_freq);*/

			/*get cpu frequency  */
	/*		ret = get_cpu_freq(&cpu_freq_get);
			if (ret) {
				printf("get_cpu_freq failed, ret: %d\n", ret);
				goto fail;
			}
			printf("get_cpu_freq ok, frequency: %d\n", cpu_freq_get);

			if (cpu_freq_get != cpu_freq) {
				printf("set cpu freq not equal to get cpu freq\n");
				goto fail;
			}
	*/

		}
		/*get cpu frequency  */
		ret = get_cpu_freq(&cpu_freq_get);
		if (ret) {
			printf("get_cpu_freq failed, ret: %d\n", ret);
			goto fail;
		}
		printf("get_cpu_freq ok, frequency: %d\n", cpu_freq_get);

		/*get cpu voltage */
		ret = get_cpu_voltage(&cpu_voltage_get);
		if (ret) {
			printf("get_cpu_voltage failed, ret: %d\n", ret);
			goto fail;
		}
		printf("get_cpu_voltage ok, voltage: %d\n", cpu_voltage_get);


		printf("======LOOP TESTED OK TIMES:%d======\n", strtol(CONFIG_CPU_FREQ_LOOP_TIMES, NULL, 0) - times);
		printf("======LOOP LEFT TIMES:%d======\n", times);

		//hal_mdelay(500);
	}

	printf("============TEST FOR CPU FREQ OK!============\n");

	return ret;
fail:
	printf("============TEST FOR CPU FREQ FAIL!============\n");
	return ret;
}
testcase_init(tt_cpu_freq, cpufreq, cpu_freq for tinatest);
