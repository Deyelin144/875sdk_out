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
#include <errno.h>
#include <tinatest.h>

#define tt_printf(fmt, arg...) printf("\e[0m" "[%s] " fmt "\e[0m\n", tt_name, ##arg)

int tt_demo(int argc, char **argv)
{
    int i, opt;
    char reply[10];
    int ret = -1;
	char tt_name[TT_NAME_LEN_MAX+1];
	int sleep_ms = 500;
	tname(tt_name);
	tt_printf("=======TINATEST FOR %s=========", tt_name);

    tt_printf("It's in demo for tinatest\n");
    tt_printf("Calling: ");
    for (i = 0; i < argc; i++)
	    tt_printf("%s ", argv[i]);
    tt_printf("\n");

	if( (2==argc) && argv && argv[1]){
		char *err = NULL;
		unsigned long tmp = strtoul(argv[1], &err, 0);
		if (*err == 0){
			sleep_ms = tmp;
			tt_printf("set sleep_ms:%d", sleep_ms);
		}
	}

    ttips("ttips test: print tips to user\n");

    ret = ttrue("true test: user select yes/no?\n");
    if (ret < 0) {
	    tt_printf("enter no\n");
	    return -1;
    }

    ret = task("task test: user enter string", reply, 10);
    if (ret < 0) {
	    tt_printf("task err\n");
	    return -1;
    }
    tt_printf("user entry reply %s\n", reply);

	usleep(sleep_ms*1000);
	tt_printf("======TINATEST FOR %s OK=======", tt_name);
    return 0;
err:
	tt_printf("=====TINATEST FOR %s FAIL======", tt_name);
	return -1;
}
testcase_init(tt_demo, demo, demo for tinatest);
