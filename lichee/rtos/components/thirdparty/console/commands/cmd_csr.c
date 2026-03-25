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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <csr.h>
#include <console.h>

int cmd_csr_read(int argc, char **argv)
{
    unsigned long csr_value = 0;
    if (argc < 2)
    {
        printf("Usage: csr_read csr\n");
        return -1;
    }

    if (!strcmp(argv[1], "mstatus"))
    {
        csr_value = read_csr(CSR_MSTATUS);
    }
    else if (!strcmp(argv[1], "mepc"))
    {
        csr_value = read_csr(CSR_MEPC);
    }
    else if (!strcmp(argv[1], "mtvec"))
    {
        csr_value = read_csr(CSR_MTVEC);
    }
    else if (!strcmp(argv[1], "mcause"))
    {
        csr_value = read_csr(CSR_MCAUSE);
    }
    else if (!strcmp(argv[1], "mie"))
    {
        csr_value = read_csr(CSR_MIE);
    }
    else if (!strcmp(argv[1], "mip"))
    {
        csr_value = read_csr(CSR_MIP);
    }
    else if (!strcmp(argv[1], "mtval"))
    {
        csr_value = read_csr(CSR_MTVAL);
    }
    else if (!strcmp(argv[1], "mscratch"))
    {
        csr_value = read_csr(CSR_MSCRATCH);
    }
    else if (!strcmp(argv[1], "mhcr"))
    {
        csr_value = read_csr(CSR_MHCR);
    }
	else if (!strcmp(argv[1], "sstatus"))
    {
        csr_value = read_csr(CSR_SSTATUS);
    }
    else if (!strcmp(argv[1], "sepc"))
    {
        csr_value = read_csr(CSR_SEPC);
    }
    else if (!strcmp(argv[1], "stvec"))
    {
        csr_value = read_csr(CSR_STVEC);
    }
    else if (!strcmp(argv[1], "scause"))
    {
        csr_value = read_csr(CSR_SCAUSE);
    }
    else if (!strcmp(argv[1], "sie"))
    {
        csr_value = read_csr(CSR_SIE);
    }
    else if (!strcmp(argv[1], "sip"))
    {
        csr_value = read_csr(CSR_SIP);
    }
    else if (!strcmp(argv[1], "stval"))
    {
        csr_value = read_csr(CSR_STVAL);
    }
    else if (!strcmp(argv[1], "sscratch"))
    {
        csr_value = read_csr(CSR_SSCRATCH);
    }
    else
    {
        printf("can not support the csr\n");
        return -1;
    }

    printf("%s:0x%lx\n", argv[1], csr_value);
    return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_csr_read, csr_read, read csr);

int cmd_csr_write(int argc, char **argv)
{
    unsigned long csr_value = 0;
    char *end = NULL;

    if (argc < 3)
    {
        printf("Usage: csr_write csr value\n");
        return -1;
    }

    csr_value = strtoul(argv[2], &end, 0);

    if (!strcmp(argv[1], "mstatus"))
    {
        write_csr(CSR_MSTATUS, csr_value);
    }
    else if (!strcmp(argv[1], "mepc"))
    {
        write_csr(CSR_MEPC, csr_value);
    }
    else if (!strcmp(argv[1], "mtvec"))
    {
        write_csr(CSR_MTVEC, csr_value);
    }
    else if (!strcmp(argv[1], "mcause"))
    {
        write_csr(CSR_MCAUSE, csr_value);
    }
    else if (!strcmp(argv[1], "mie"))
    {
        write_csr(CSR_MIE, csr_value);
    }
    else if (!strcmp(argv[1], "mip"))
    {
        write_csr(CSR_MIP, csr_value);
    }
    else if (!strcmp(argv[1], "mtval"))
    {
        write_csr(CSR_MTVAL, csr_value);
    }
    else if (!strcmp(argv[1], "mscratch"))
    {
        write_csr(CSR_MSCRATCH, csr_value);
    }
    else if (!strcmp(argv[1], "sstatus"))
    {
        write_csr(CSR_SSTATUS, csr_value);
    }
    else if (!strcmp(argv[1], "sepc"))
    {
        write_csr(CSR_SEPC, csr_value);
    }
    else if (!strcmp(argv[1], "stvec"))
    {
        write_csr(CSR_STVEC, csr_value);
    }
    else if (!strcmp(argv[1], "scause"))
    {
        write_csr(CSR_SCAUSE, csr_value);
    }
    else if (!strcmp(argv[1], "sie"))
    {
        write_csr(CSR_SIE, csr_value);
    }
    else if (!strcmp(argv[1], "sip"))
    {
        write_csr(CSR_SIP, csr_value);
    }
    else if (!strcmp(argv[1], "stval"))
    {
        write_csr(CSR_STVAL, csr_value);
    }
    else if (!strcmp(argv[1], "sscratch"))
    {
        write_csr(CSR_SSCRATCH, csr_value);
    }
    else
    {
        printf("can not support the csr %s\n", argv[1]);
        return -1;
    }

    return 0;
}

FINSH_FUNCTION_EXPORT_CMD(cmd_csr_write, csr_write, write csr);