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

#include "pid.h"

PID pid;


void InitPID(void)
{
	pid.SetTemperature = 125;       //target temperature
	pid.SumError = 0;
	pid.Kp = 700;                  	//proportional constant

	pid.Ki = 0.600;                	//integral constant
	pid.Kd = 0.00;                 	//derivative constant

	pid.NowError = 0;               //current error
	pid.LastError = 0;              //last error

	pid.Out0 = 0;                   //output correction value
	pid.Out = 0;                    //final output value
}


/*******************************************************************************
* function : LocPIDCalc
* description : PID control calculation
* input : None
* output : Results of PID
* output = kp*et + ki*etSum + kd*det+ out0;
*******************************************************************************/

int LocPIDCalc(void)
{
	static int out1, out2, out3;

	pid.NowError = pid.SetTemperature - pid.ActualTemperature;
	pid.SumError += pid.NowError;

	out1 = pid.Kp * pid.NowError;                                    //proportional
	out2 = pid.Kp * pid.Ki * pid.SumError;                           //integral
	out3 = pid.Kp * pid.Kd * (pid.NowError - pid.LastError);         //derivative
//			  pid.Out;                                              //correction

	pid.LastError = pid.NowError;

	return out1 + out2 + out3 + pid.Out;
}

void SetPID_Kp(int value)
{
	pid.Kp = (float)value/1000;
}

void SetPID_Ki(int value)
{
	pid.Ki = (float)value/1000;
}

void SetPID_Kd(int value)
{
	pid.Kd = (float)value/1000;
}

void SetPID_Temperature(int value)
{
	pid.SetTemperature = value;
}