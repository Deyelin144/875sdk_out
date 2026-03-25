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

#ifndef CONSOLE_H
#define CONSOLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <hal_thread.h>

#define SECTION(x)  __attribute__((section(x)))
#define RT_USED     __attribute__((used))

#define SH_MAX_CMD_ARGS 32//16
#define SH_MAX_CMD_LEN 512

typedef int (*syscall_func)(int argc, char **argv);

/* system call table */
struct finsh_syscall
{
    const char*     name;       /* the name of system call */
    const char*     desc;       /* description of system call */
    syscall_func func;      /* the function address of system call */
};
extern struct finsh_syscall *_syscall_table_begin, *_syscall_table_end;

#define FINSH_FUNCTION_EXPORT_CMD(name, cmd, desc)                      \
                char __fsym_##cmd##_name[] SECTION(".data") = #cmd;    \
                char __fsym_##cmd##_desc[] SECTION(".data") = #desc;   \
                RT_USED const struct finsh_syscall __fsym_##cmd SECTION("FSymTab")= \
                {                           \
                    __fsym_##cmd##_name,    \
                    __fsym_##cmd##_desc,    \
                    (syscall_func)&name     \
                };

#define FINSH_VAR_EXPORT(name, type, desc)                              \
                const char __vsym_##name##_name[] SECTION(".rodata") = #name;  \
                const char __vsym_##name##_desc[] SECTION(".rodata") = #desc;  \
                RT_USED const struct finsh_sysvar __vsym_##name SECTION("VSymTab")= \
                {                           \
                    __vsym_##name##_name,   \
                    __vsym_##name##_desc,   \
                    type,                   \
                    (void*)&name            \
                };

#define FINSH_FUNCTION_EXPORT_ALIAS(name, alias, desc)  \
        FINSH_FUNCTION_EXPORT_CMD(name, alias, desc)

extern struct finsh_syscall* finsh_syscall_lookup(const char* name);

extern void finsh_syscall_show(void);

/* create command-line console
 * @usStakSize: stack size of command-line task
 * @uxPriority: priority of command-line task
 * @console: the command line console
 * */
void vCommandConsoleStart( uint16_t usStackSize, portBASE_TYPE uxPriority, void * console );

#ifdef __cplusplus
}
#endif
#endif  /*CONSOLE_H*/
