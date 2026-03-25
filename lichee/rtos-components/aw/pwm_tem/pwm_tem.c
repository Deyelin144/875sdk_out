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
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sunxi_hal_pwm.h>
#include <hal_cmd.h>

#include "thermal_external.h"
#include "pid.h"


#define PWM_PERIOD_NS  (128 * 1000)
#define PWM_SAFETY_GAP (500)

static int pwm_channel_ = 0;
static int target_temp_ = 80;

int cycle_to_duty(int cycle)
{
	if (cycle > 3072)
		cycle = 3072;
    else if (cycle < 0)
		cycle = 0;

    /* nanoseconds */
	const float plus = 41.67f;
	float active_ns = cycle * plus;

	int duty = floor(active_ns);

	if (duty > (PWM_PERIOD_NS - PWM_SAFETY_GAP))
		duty = PWM_PERIOD_NS - PWM_SAFETY_GAP;

	return duty;
}

int cmd_pwm_tem(int argc, char** argv)
{
	struct pwm_config *config;
	int duty = 0;
	int cycle = 0;
	int i = 0;
	int id_buf[1];
	int temperature[1];

	if (argc < 3) {
		printf("Please input correct parameter!\n");
		return -1;
    }

	pwm_channel_ = strtol(argv[1], NULL, 0);
	target_temp_ = strtol(argv[2], NULL, 0);
	config = (struct pwm_config *)malloc(sizeof(struct pwm_config));
	config->duty_ns   = duty;
	config->period_ns = PWM_PERIOD_NS;
	config->polarity  = PWM_POLARITY_NORMAL;

	hal_pwm_init();
	InitPID();

	while (1) {
		thermal_external_get_temp(id_buf, temperature, 1);//thermal_temperature_get();
		temperature[0] = temperature[0] / 1000;
		//printf("The current temperature is %d\n", temperature[0]);

		if (temperature[0] < target_temp_) {
			pid.SetTemperature    = target_temp_;
			pid.ActualTemperature = temperature[0];
			cycle = LocPIDCalc();
			//printf("cycle is %d\n", cycle);
			duty  = cycle_to_duty(cycle);
        }
        else {
			duty = 0;
			pid.SumError = 0;
		}

		//printf("duty is %d\n",duty);
		i = i + 1;
		config->duty_ns = duty;
		hal_pwm_control(pwm_channel_, config);

		usleep(1000 * 1000);
		if (i == 10) {
			printf("pwm_tem is running! duty = %d \n", duty);
			i = 0;
		}
	}
	return 0;
}

FINSH_FUNCTION_EXPORT_CMD(cmd_pwm_tem, hal_pwm_tem, pwm tem)
