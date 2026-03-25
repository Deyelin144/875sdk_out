// SPDX-License-Identifier: GPL-2.0+
/*
 * sunxi SPI driver for uboot.
 *
 * Copyright (C) 2018
 * 2013.5.7  Mintow <duanmintao@allwinnertech.com>
 * 2018.11.7 wangwei <wangwei@allwinnertech.com>
 */

#include <common.h>
#include <malloc.h>
#include <memalign.h>
#include <spi.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#ifdef CONFIG_SUNXI_DMA
#include <asm/arch/dma.h>
#endif
#include <asm/arch/gpio.h>
#include <sunxi_board.h>
#include "sunxi_dbi.h"
#include <sys_config.h>
#include <fdt_support.h>
#include <linux/mtd/spi-nor.h>

#ifdef CONFIG_SPI_USE_DMA
static sunxi_dma_set *spi_tx_dma;
static sunxi_dma_set *spi_rx_dma;
static uint dbi_tx_dma_hd;
static uint dbi_rx_dma_hd;
#endif

#define	SUNXI_SPI_MAX_TIMEOUT	1000000
#define	SUNXI_SPI_PORT_OFFSET	0x1000
#define SUNXI_SPI_DEFAULT_CLK  (40000000)

/* For debug */
#define SPI_DEBUG 0

#if SPI_DEBUG
#define SPI_EXIT()		printf("%s()%d - %s\n", __func__, __LINE__, "Exit")
#define SPI_ENTER()		printf("%s()%d - %s\n", __func__, __LINE__, "Enter ...")
#define DBI_DBG(fmt, arg...)	printf("%s()%d - "fmt, __func__, __LINE__, ##arg)
#define SPI_INF(fmt, arg...)	printf("%s()%d - "fmt, __func__, __LINE__, ##arg)

#else
#define SPI_EXIT()		pr_debug("%s()%d - %s\n", __func__, __LINE__, "Exit")
#define SPI_ENTER()		pr_debug("%s()%d - %s\n", __func__, __LINE__, "Enter ...")
#define DBI_DBG(fmt, arg...)
#define SPI_INF(fmt, arg...)	pr_debug("%s()%d - "fmt, __func__, __LINE__, ##arg)
#endif

#define DBI_ERR(fmt, arg...)	printf("%s()%d - "fmt, __func__, __LINE__, ##arg)
#define SUNXI_SPI_OK   0
#define SUNXI_SPI_FAIL -1

static hal_dbi_config_t g_dbi_cfg;
void __iomem *dbi_base_addr[2] = {
    (void __iomem *)(unsigned long)SUNXI_SPI0_BASE,
    (void __iomem *)(unsigned long)SUNXI_SPI1_BASE
};

// static int sunxi_get_dbic_clk(int bus);
/* soft reset spi controller */
static void dbi_soft_reset(uint32_t port)
{
    void __iomem *base_addr = dbi_base_addr[port];
    uint32_t reg_val = readl(base_addr + SPI_GC_REG);

    reg_val |= SPI_GC_SRST;
    writel(reg_val, base_addr + SPI_GC_REG);
}

/* reset fifo */
static void dbi_reset_fifo(uint32_t port)
{
    void __iomem *base_addr = dbi_base_addr[port];
    uint32_t reg_val = readl(base_addr + SPI_FIFO_CTL_REG);

    reg_val |= (SPI_FIFO_CTL_RX_RST|SPI_FIFO_CTL_TX_RST);
    /* Set the trigger level of RxFIFO/TxFIFO. */
    reg_val &= ~(SPI_FIFO_CTL_RX_LEVEL|SPI_FIFO_CTL_TX_LEVEL);
    reg_val |= (0x20<<16) | 0x20;
    writel(reg_val, base_addr + SPI_FIFO_CTL_REG);
}

static int sunxi_dbi_gpio_init(void)
{
    sunxi_gpio_set_cfgpin(12, 6);
    sunxi_gpio_set_cfgpin(13, 6);
    sunxi_gpio_set_cfgpin(14, 3);
    sunxi_gpio_set_cfgpin(15, 3);
    return 0;
}

static int sunxi_dbi_clk_init(uint32_t bus, uint32_t mod_clk)
{
#ifdef CONFIG_MACH_SUN20IW2
    //FIXME
    struct sunxi_ccm_reg *const ccm =
        (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

    unsigned long mclk_base = (unsigned long)&ccm->spi0_clk_ctrl + bus*0x4;
    uint32_t source_clk = 0;
    uint32_t rval;
    uint32_t m, n, div, factor_m;

    source_clk = clock_get_pll6() * 1000000;
    SPI_INF("source_clk: %d Hz, mod_clk: %d Hz\n", source_clk, mod_clk);

    div = (source_clk + mod_clk - 1) / mod_clk;
    div = div == 0 ? 1 : div;
    if (div > 128) {
        m = 1;
        n = 0;
        return -1;
    } else if (div > 64) {
        n = 3;
        m = div >> 3;
    } else if (div > 32) {
        n = 2;
        m = div >> 2;
    } else if (div > 16) {
        n = 1;
        m = div >> 1;
    } else {
        n = 0;
        m = div;
    }

    factor_m = m - 1;
    rval = (1U << 31) | (0x1 << 24) | (n << 16) | factor_m;

    writel(rval, (volatile void __iomem *)mclk_base);

    clrbits_le32(&ccm->dev_rst_ctrl0,
             1 << (SPI_RESET_SHIFT + bus));
    udelay(2);

    setbits_le32(&ccm->dev_rst_ctrl0,
             1 << (SPI_RESET_SHIFT + bus));

    setbits_le32(&ccm->bus_clk_gating_ctrl0,
             1 << (SPI_GATING_SHIFT + bus));

    SPI_INF("src: %d Hz, spic:%d Hz,  n=%d, m=%d\n", source_clk, source_clk/ (1 << n) / m, n, m);
    return 0;
#else
    struct sunxi_ccm_reg *const ccm =
        (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
    unsigned long mclk_base = (unsigned long)&ccm->spi0_clk_cfg + bus*0x4;
    uint32_t source_clk = 0;
    uint32_t rval;
    uint32_t m, n, div, factor_m;

    /* SCLK = src/M/N */
    /* N: 00:1 01:2 10:4 11:8 */
    /* M: factor_m + 1 */
#ifdef FPGA_PLATFORM
    n = 0;
    m = 1;
    factor_m = m - 1;
    rval = (1U << 31);
    source_clk = 24000000;
#else
    source_clk = clock_get_pll6() * 1000000;
    SPI_INF("source_clk: %d Hz, mod_clk: %d Hz\n", source_clk, mod_clk);

    div = (source_clk + mod_clk - 1) / mod_clk;
    div = div == 0 ? 1 : div;
    if (div > 128) {
        m = 1;
        n = 0;
        return -1;
    } else if (div > 64) {
        n = 3;
        m = div >> 3;
    } else if (div > 32) {
        n = 2;
        m = div >> 2;
    } else if (div > 16) {
        n = 1;
        m = div >> 1;
    } else {
        n = 0;
        m = div;
    }

    factor_m = m - 1;
    rval = (1U << 31) | (0x1 << 24) | (n << 8) | factor_m;

#endif
    writel(rval, (volatile void __iomem *)mclk_base);
    /* spi reset */
    setbits_le32(&ccm->spi_gate_reset, (0<<RESET_SHIFT));
    setbits_le32(&ccm->spi_gate_reset, (1<<RESET_SHIFT));

    /* spi gating */
    setbits_le32(&ccm->spi_gate_reset, (1<<GATING_SHIFT));
    /*sclk_freq = source_clk / (1 << n) / m*/

    SPI_INF("src: %d Hz, spic:%d Hz,  n=%d, m=%d\n",source_clk, source_clk/ (1 << n) / m, n, m);
#endif
    return 0;
}

// static int sunxi_get_dbic_clk(int bus)
// {
//     struct sunxi_ccm_reg *const ccm =
//         (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
//     unsigned long mclk_base = (unsigned long)&ccm->spi0_clk_ctrl + bus*0x4;
//     uint32_t reg_val = 0;
//     uint32_t src = 0, clk = 0, sclk_freq = 0;
//     uint32_t n, m;

//     reg_val = readl((volatile void __iomem *)mclk_base);
//     src = (reg_val >> 24)&0x7;
//     n = (reg_val >> 8)&0x3;
//     m = ((reg_val >> 0)&0xf) + 1;

//     switch(src) {
//         case 0:
//             clk = 24000000;
//             break;
//         case 1:
//             clk = clock_get_pll6() * 1000000;
//             break;
//         default:
//             clk = 0;
//             break;
//     }
//     sclk_freq = clk / (1 << n) / m;
//     SPI_INF("sclk_freq= %d Hz,reg_val: %x , n=%d, m=%d\n", sclk_freq, reg_val, n, m);
//     return sclk_freq;
// }


// static int sunxi_spi_clk_exit(void)
// {
// #ifdef CONFIG_MACH_SUN20IW2
//     struct sunxi_ccm_reg *const ccm =
//         (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

//     clrbits_le32(&ccm->bus_clk_gating_ctrl0,
//              1 << (SPI_GATING_SHIFT + 0));

//     clrbits_le32(&ccm->dev_rst_ctrl0,
//              1 << (SPI_RESET_SHIFT + 0));
// #else
//     struct sunxi_ccm_reg *const ccm =
//         (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
//     /* spi gating */
//     clrbits_le32(&ccm->spi_gate_reset, 1<<GATING_SHIFT);

//     /* spi reset */
//     clrbits_le32(&ccm->spi_gate_reset, 1<<RESET_SHIFT);

// #endif
//     return 0;
// }

int dbi_claim_bus(uint32_t port)
{
    dbi_soft_reset(port);

    dbi_reset_fifo(port);

    return 0;
}

// void dbi_release_bus(struct spi_slave *slave)
// {
//     struct sunxi_spi_slave *sspi = to_sunxi_slave(slave);

//     SPI_ENTER();
//     /* disable the spi controller */
//     spi_disable_bus((void __iomem *)(unsigned long)sspi->base_addr);

// }

static void spi_sample_delay(uint32_t sdm, uint32_t sdc, uint32_t port)
{
    void __iomem *base_addr = dbi_base_addr[port];
    uint32_t reg_val = readl(base_addr + SPI_TC_REG);
    uint32_t org_val = reg_val;

    if (sdm) {
        reg_val |= SPI_TC_SDM;
    } else {
        reg_val &= ~SPI_TC_SDM;
    }

    if (sdc) {
        reg_val |= SPI_TC_SDC;
    } else {
        reg_val &= ~SPI_TC_SDC;
    }

    if (reg_val != org_val) {
        writel(reg_val, base_addr + SPI_TC_REG);
    }
}

dbi_master_status_t hal_dbi_hw_config(uint32_t port, hal_dbi_config_t *dbi_config)
{
    if (dbi_config->clock_frequency > SPI_MAX_FREQUENCY) {
        DBI_ERR("[dbi%d] invalid parameter! max_frequency is 100MHZ\n", port);
    } else {
        DBI_DBG("[dbi%d] clock_frequency = %ldHZ\n", port, clock_frequency);
    }

    if (dbi_config->clock_frequency >= SPI_HIGH_FREQUENCY) {
        spi_sample_delay(0, 1, port);
    } else if (dbi_config->clock_frequency <= SPI_LOW_FREQUENCY) {
        spi_sample_delay(1, 0, port);
    } else {
        spi_sample_delay(0, 0, port);
    }

    dbi_soft_reset(port);
    /* reset fifo */
    dbi_reset_fifo(port);

    return DBI_MASTER_OK;
}

int dbi_irq_enable(uint32_t port, enum dbi_mode_type mode_type)
{
    void __iomem *base_addr = dbi_base_addr[port];
    uint32_t reg_val = readl(base_addr + DBI_INT_REG);

    switch (mode_type) {
    case DBI_MODE_TYPE_RX:
        reg_val |= DBI_RD_DONE_EN;
        break;
    case DBI_MODE_TYPE_TX:
        reg_val |= DBI_FIFO_EMPTY_EN;
        break;
    default :
         return -1;
    }
    writel(reg_val, base_addr + DBI_INT_REG);

    return 0;
}

int dbi_irq_disable(uint32_t port, enum dbi_mode_type mode_type)
{
    void __iomem *base_addr = dbi_base_addr[port];
    uint32_t reg_val = readl(base_addr + DBI_INT_REG);

    switch (mode_type) {
        case DBI_MODE_TYPE_RX:
            reg_val &= ~DBI_RD_DONE_EN;
            break;
        case DBI_MODE_TYPE_TX:
            reg_val &= ~DBI_FIFO_EMPTY_EN;
            break;
        default : 
            return -1;
    }
    writel(reg_val, base_addr + DBI_INT_REG);

    return 0;
}

static void dbi_enable_irq(uint32_t bitmap, uint32_t port)
{
    void __iomem *base_addr = dbi_base_addr[port];
    uint32_t reg_val = readl(base_addr + DBI_INT_REG);
    reg_val |= bitmap;
    writel(reg_val, base_addr + DBI_INT_REG);
}

static void dbi_enable(uint32_t port)
{
    void __iomem *base_addr = dbi_base_addr[port];
    u32 reg_val;
    reg_val = readl(base_addr + SPI_GC_REG);
    reg_val |= SPI_CR_DBI_EN;
    writel(reg_val, base_addr + SPI_GC_REG);
}

static int set_dcx(uint32_t port, u32 dcx)
{
    void __iomem *base_addr = dbi_base_addr[port];
    u32 reg_val = readl(base_addr + DBI_CTL_REG1);
    reg_val &= ~(1 << DBI_DCX_DATA_OFFSET);
    if (dcx) {
        reg_val |= (1 << DBI_DCX_DATA_OFFSET);
    }
    writel(reg_val, base_addr + DBI_CTL_REG1);
    return 0;
}

static int config_dbi_data_src_sel(uint32_t port, u32 arg)
{
    void __iomem *base_addr = dbi_base_addr[port];
    u32 reg_val = readl(base_addr + DBI_CTL_REG1);
    reg_val &= ~(1 << DBI_DATA_SOURCE_OFFSET);
    if (arg) {
        reg_val |= (1 << DBI_DATA_SOURCE_OFFSET);
    }

    writel(reg_val, base_addr + DBI_CTL_REG1);
    return 0;
}

static int config_dbi_enable_mode(uint32_t port, u32 mode)
{
    void __iomem *base_addr = dbi_base_addr[port];
    u32 reg_val = readl(base_addr + DBI_CTL_REG1);
    reg_val &= ~(0x01 << DBI_CR_SOFT_TRIGGER);
    reg_val |= (((mode) & 0x01) << DBI_CR_SOFT_TRIGGER);

    writel(reg_val, base_addr + DBI_CTL_REG1);
    return 0;
}

static int config_dbi_output_clk_mode(uint32_t port, u32 mode)
{
    void __iomem *base_addr = dbi_base_addr[port];
    u32 reg_val = readl(base_addr + DBI_CTL_REG1);
    reg_val &= ~(1 << DBI_CLOCK_MODE_OFFSET);
    reg_val |= ((mode & 1) << DBI_CLOCK_MODE_OFFSET);

    writel(reg_val, base_addr + DBI_CTL_REG1);
    return 0;
}

static int set_dbi_en_mode(uint32_t port, u32 mode)
{
    void __iomem *base_addr = dbi_base_addr[port];
    u32 reg_val = readl(base_addr + DBI_CTL_REG1);
    reg_val &= ~(0x03 << DBI_EN_MODE_SEL_OFFSET);
    reg_val |= (((mode) & 0x03) << DBI_EN_MODE_SEL_OFFSET);

    writel(reg_val, base_addr + DBI_CTL_REG1);
    return true;
}

static int set_chip_select_control(uint32_t port)
{
    void __iomem *base_addr = dbi_base_addr[port];
    u32 reg_val = 0;

    reg_val |= SPI_TC_SDM;
    reg_val |= SPI_TC_DHB;
    reg_val |= SPI_TC_SS_OWNER;
    reg_val |= SPI_TC_SPOL;
    writel(reg_val, base_addr + SPI_TC_REG);
    return true;
}

static void set_master(uint32_t port)
{
    void __iomem *base_addr = dbi_base_addr[port];
    u32 reg_val = 0;

    reg_val = readl(base_addr + SPI_GC_REG);
    reg_val |= (0x1 << 1);
    writel(reg_val, base_addr + SPI_GC_REG);
}

static void set_master_counter(uint32_t port, u32 tx_len)
{
    void __iomem *base_addr = dbi_base_addr[port];
    writel(tx_len, base_addr + SPI_BURST_CNT_REG);
    writel(tx_len, base_addr + SPI_TRANSMIT_CNT_REG);
    writel(tx_len, base_addr + SPI_BCC_REG);
}

static dbi_master_status_t hal_dbi_register_config(uint32_t port, hal_dbi_config_t *dbi_config)
{
    void __iomem *base_addr = dbi_base_addr[port];
    uint32_t reg_val;

    /* Set SDBI working on dbi mode */
    reg_val = readl(base_addr + SPI_GC_REG);
    reg_val |= SPI_CR_SELECT_DBI_MODE;
    writel(reg_val, base_addr + SPI_GC_REG);

    /* 1.config dbi DBI_CTRL_REG0 */
    reg_val = 0;

    if (dbi_config->cmd_typ) {
        reg_val |= DBI_CR_READ;
        // dbi->mode_type = DBI_MODE_TYPE_RX;
    } else {
        reg_val &= ~DBI_CR_READ;
        // dbi->mode_type = DBI_MODE_TYPE_TX;
    }

    /* output data sequence */
    if(dbi_config->output_data_sequence) {
        reg_val |= DBI_CR_LSB_FIRST;
    } else {
        reg_val &= ~DBI_CR_LSB_FIRST;
    }

    dbi_irq_disable(port, DBI_MODE_TYPE_TX);
    /* transmit video */
    if(dbi_config->transmit_mode) {
        int line_size = dbi_config->h_size;
        reg_val |= DBI_CR_TRANSMIT_MODE;

        if (line_size < BULK_DATA_BOUNDARY)
            dbi_enable_irq(DBI_LINE_DONE_EN, port);  /* enable line int */

        switch(dbi_config->dbi_en_mode) {
        case DBI_MODE_ALLWAYS_ON:
            dbi_enable(port);
            dbi_enable_irq(DBI_FRAM_DONE_EN, port);
            break;
        case DBI_MODE_SOFTWARE_TRIGGER:
            dbi_enable_irq(DBI_TE_EN, port);  /* enable te int */
            break;
        case DBI_MODE_TIMER_TRIGGER:
            dbi_enable_irq(DBI_TIMER_EN, port);  /* enable timer int */

            reg_val = readl(base_addr + DBI_TIMER_REG);  /* dbi tmier value, clock source is SCLK */
            reg_val &= ~(0x7fffffff << 0);
            reg_val |= (0x38A40 << 0);  /* timer_value = SCLK/frame_rate - one_pixel_clock_cycle * one_frame_pixel_value */
            reg_val &= ~(0x1u << 31);
            reg_val |= (0x1u << 31);
            writel(reg_val, base_addr + DBI_TIMER_REG);
            break;
        case DBI_MODE_TE_TRIGGER:
            dbi_enable_irq(DBI_TE_EN, port);/* enable te int */
            break;
        }
    } else {
        reg_val &= ~DBI_CR_TRANSMIT_MODE;
        dbi_irq_enable(port, DBI_MODE_TYPE_TX);
    }

    /* output data format */
    reg_val &= ~(DBI_CR_FORMAT_MASK);
    if(dbi_config->dbi_output_data_format == DBI_RGB111) {
        reg_val &= ~(0x7 << DBI_CR_FORMAT);
    } else {
        reg_val |= ((dbi_config->dbi_output_data_format) << DBI_CR_FORMAT);
    }

    /* dbi interface select */
    reg_val &= ~(DBI_CR_INTERFACE_MASK);
    if (dbi_config->dbi_interface == L3I1) {
        reg_val &= ~((0x7) << DBI_CR_INTERFACE);
    } else {
        reg_val |= ((dbi_config->dbi_interface) << DBI_CR_INTERFACE);
    }
    /* source format */
    reg_val &= ~(DBI_CR_SRC_FORMAT_MASK);
    if (dbi_config->dbi_source_format == DBI_SRC_RGB) {
        reg_val &= ~((0xf) << DBI_CR_SRC_FORMAT);
    } else {
        reg_val |= ((dbi_config->dbi_source_format) << DBI_CR_SRC_FORMAT);
    }
    /* RGB bit order */
    if (dbi_config->rgb_bit_order == 1) {
        reg_val |= ((0x1) << 2);
    } else {
        reg_val &= ~((0x1) << 2);
    }

    if (dbi_config->epos) {
        reg_val |= ((0x1) << 1);
    } else {
        reg_val &= ~((0x1) << 1);
    }
    /* RGB16 or RGB32 */
    if(dbi_config->rgb_src_typ) {
        reg_val |= DBI_CR_VI_SRC_RGB16;
    } else {
        reg_val &= ~DBI_CR_VI_SRC_RGB16;
    }
    writel(reg_val, base_addr + DBI_CTL_REG0);

    /* 2. config dbi DBI_CTRL_REG1 */
    set_dcx(port, dbi_config->dcx_data);

    config_dbi_data_src_sel(port, dbi_config->rgb16_pix0_post);

    /* DBI enable mode select:0 always on mode;1software trgger mode;2 timer trgger mode; 3 te trigger mode */
    config_dbi_enable_mode(port, dbi_config->dbi_en_mode);

    /* config dbi clock mode */
    config_dbi_output_clk_mode(port, dbi_config->dbi_output_clk_mode);

    /* 3. config dbi DBI_CTRL_REG2 */
    reg_val = 0;
    if(dbi_config->ahb_ready_bypass) {
        reg_val |= DBI_CR2_HRDY_BYP;
    } else {
        reg_val &= ~DBI_CR2_HRDY_BYP;
    }
    if (dbi_config->dbi_interface == D2LI) {
        reg_val |= DBI_CR2_DCX_PIN;
        reg_val &= ~DBI_CR2_SDI_PIN;
    } else {
        reg_val &= ~DBI_CR2_DCX_PIN;
        reg_val |= DBI_CR2_SDI_PIN;
    }

    if(dbi_config->te_en) {
        // Only TE and non-TE dbi mode are supported.
        set_dbi_en_mode(port, DBI_MODE_TE_TRIGGER);

        /*te enable*/
        reg_val |= 0x1;
        if (dbi_config->te_edge_select == DBI_TE_FALLING_EDGE)
            reg_val |= (0x1 << 1); //1 TE falling edge
        else
            reg_val &= ~(0x1 << 1); //0 TE rising edge
    } else
        reg_val &= ~(0x3 << 0); // te disable

    writel(reg_val, base_addr + DBI_CTL_REG2);

    reg_val = 0;
    reg_val=(dbi_config->h_size & 0x7ff) | ((dbi_config->v_size & 0x7ff) << 16);
    writel(reg_val, base_addr + DBI_SIZE_REG);

    // default config
    config_dbi_data_src_sel(port, 0);

    return DBI_MASTER_OK;

}

dbi_master_status_t hal_set_dbi_config(uint32_t port, hal_dbi_config_t *dbi_config)
{
    if (hal_dbi_hw_config(port, dbi_config))
    {
        DBI_ERR("[dbi%d] hw config error\n", port);
        return DBI_MASTER_ERROR;
    }

    if(hal_dbi_register_config(port, dbi_config))
    {
        DBI_ERR("[dbi%d] init register error\n", port);
        return DBI_MASTER_ERROR;
    }
    return DBI_MASTER_OK;
}

static u32 spi_query_txfifo(void __iomem *base_addr)
{
    u32 reg_val = (SPI_FIFO_STA_TX_CNT & readl(base_addr + SPI_FIFO_STA_REG));

    reg_val >>= SPI_TXCNT_BIT_POS;
    return reg_val;
}
// static void sunxi_dbi_show_all_regs(void)
// {
// 	unsigned int reg_val[4] = {0};
// 	int reg_offset = 0;

// 	printf("================= sunxi dbi all regs ================\n");
// 	for (reg_offset = 0; reg_offset < 0x128; reg_offset += 0x10) {
// 		reg_val[0] = readl(0x4000F000 + reg_offset + 0x0);
// 		reg_val[1] = readl(0x4000F000 + reg_offset + 0x4);
// 		reg_val[2] = readl(0x4000F000 + reg_offset + 0x8);
// 		reg_val[3] = readl(0x4000F000 + reg_offset + 0xc);
// 		printf("0x%02x-0x%02x: 0x%08x 0x%08x 0x%08x 0x%08x\n",
// 			reg_offset, reg_offset+0xc,
// 			reg_val[0], reg_val[1], reg_val[2], reg_val[3]);
// 	}
// 	printf("=====================================================\n");
// }
static int dbi_cpu_write(uint32_t port, const unsigned char *buf, unsigned int len)
{
    // sunxi_dbi_show_all_regs();
    void __iomem *base_addr = dbi_base_addr[port];
    unsigned char time;
    unsigned int tx_len = len;	/* number of bytes receieved */
    unsigned char *tx_buf = (unsigned char *)buf;
    unsigned int poll_time = 0x7ffffff;

    for (; tx_len > 0; --tx_len) {
        writeb(*tx_buf++, base_addr + SPI_TXDATA_REG);
        while (spi_query_txfifo(base_addr) >= MAX_FIFU)
            for (time = 2; 0 < time; --time)
                ;
    }

    while (spi_query_txfifo(base_addr) && (--poll_time > 0))
        ;
    if (poll_time <= 0) {
        DBI_ERR("cpu transfer data time out!\n");
        return -1;
    }

    return 0;
}

static int dbi_cpu_write_short(uint32_t port, const unsigned char *buf, unsigned int len)
{
    // sunxi_dbi_show_all_regs();
    void __iomem *base_addr = dbi_base_addr[port];
    unsigned char time;
    unsigned int tx_len = (len / 4);	/* number of bytes receieved */
    unsigned int *tx_buf = (unsigned int *)buf;
    unsigned int poll_time = 0x7ffffff;

    for (; tx_len > 0; --tx_len) {
        writel(*tx_buf++, base_addr + SPI_TXDATA_REG);
        while (spi_query_txfifo(base_addr) >= 32)
            for (time = 2; 0 < time; --time)
                ;
    }

    while (spi_query_txfifo(base_addr) && (--poll_time > 0))
        ;
    if (poll_time <= 0) {
        DBI_ERR("cpu transfer data time out!\n");
        return -1;
    }

    return 0;
}

int hal_dbi_send(uint32_t port, uint8_t *buf, uint32_t len)
{
    dbi_reset_fifo(port);

    set_chip_select_control(port);
    set_master(port);

    set_master_counter(port, len);

    dbi_enable(port);
    return dbi_cpu_write(port, buf, len);
}

int hal_dbi_send_short(uint32_t port, uint8_t *buf, uint32_t len)
{
    dbi_reset_fifo(port);

    set_chip_select_control(port);
    set_master(port);

    set_master_counter(port, len);

    dbi_enable(port);
    return dbi_cpu_write_short(port, buf, len);
}

int disp_lcd_cmd_write_by_dbi(uint32_t port, uint8_t *buf, uint32_t size)
{
    g_dbi_cfg.cmd_typ = DBI_WRITE;
    g_dbi_cfg.output_data_sequence = 0;
    g_dbi_cfg.transmit_mode = 0;
    g_dbi_cfg.dcx_data = 0;
    g_dbi_cfg.dbi_output_data_format = DBI_RGB111;
    hal_set_dbi_config(port, &g_dbi_cfg);

    return hal_dbi_send(port, buf, size);
}

int disp_lcd_para_write_by_dbi(uint32_t port, uint8_t *buf, uint32_t size)
{
    g_dbi_cfg.cmd_typ = DBI_WRITE;
    g_dbi_cfg.output_data_sequence = 0;
    g_dbi_cfg.transmit_mode = 0;
    g_dbi_cfg.dcx_data = 1;
    g_dbi_cfg.dbi_output_data_format = DBI_RGB111;
    hal_set_dbi_config(port, &g_dbi_cfg);

    return hal_dbi_send(port, buf, size);
}

int disp_lcd_dbi_pixel_cfg(uint32_t port, uint8_t *buf, uint32_t size)
{
    g_dbi_cfg.cmd_typ = DBI_WRITE;//write command
    g_dbi_cfg.output_data_sequence = 0;//MSB first
    g_dbi_cfg.transmit_mode = 1;//video
    g_dbi_cfg.dcx_data = 1;
    g_dbi_cfg.dbi_output_data_format = DBI_RGB565;
    g_dbi_cfg.te_en = 0;
    hal_set_dbi_config(port, &g_dbi_cfg);

    return hal_dbi_send_short(port, buf, size);
}

int dbi_init(int x, int y)
{
    if (sunxi_dbi_clk_init(1, 48000000))
        printf("dbi clk init error\n");

    memset(&g_dbi_cfg, 0, sizeof(hal_dbi_config_t));
    g_dbi_cfg.v_size = x;
    g_dbi_cfg.h_size = y;
    g_dbi_cfg.dbi_output_clk_mode = 1;
    g_dbi_cfg.dbi_interface = L4I1;
    g_dbi_cfg.clock_frequency = 96000000;
    g_dbi_cfg.dbi_output_data_format = DBI_RGB565;
    g_dbi_cfg.rgb_src_typ = 1;
    g_dbi_cfg.ahb_ready_bypass = 1;
    g_dbi_cfg.dbi_en_mode = DBI_MODE_ALLWAYS_ON;// alway on

    sunxi_dbi_gpio_init();

    dbi_claim_bus(1);

    return 0;
}
