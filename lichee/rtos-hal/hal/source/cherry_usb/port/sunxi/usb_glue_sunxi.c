/*
 * Copyright (c) 2022, sakumisu
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <hal_gpio.h>
#include <hal_time.h>
#include <hal_interrupt.h>
#include <hal_cache.h>
#include <hal_cfg.h>
#include <script.h>
#include "usbh_core.h"
#include "usbh_hub.h"
#include "usb_hc_ehci.h"
#include "sunxi_hal_common.h"


#define SUNXI_USB_CTRL		0x800
#define SUNXI_USB_PHY_CTRL     0x810
#define HOST_MAXNUM		5

#define SUNXI_CCM_BASE                (0x4003C000)
#define SUNXI_USBOTG_BASE                 (0x40b42000)
#define SUNXI_EHCI0_BASE          0x40b43000
#define USBEHCI0_RST_BIT 31
#define USBEHCI0_GATIING_BIT 31
#define USBPHY0_RST_BIT 25

typedef unsigned int u32;
static u32 usb_vbase = SUNXI_EHCI0_BASE;
unsigned int usb_drv_vbus_gpio = -1;

struct sunxi_ccm_reg {
    u32 reserved_0x000[1];          /* 0x0000 */
    u32 bus_clk_gating_ctrl0;
    u32 bus_clk_gating_ctrl1;
    u32 dev_rst_ctrl0;
    u32 dev_rst_ctrl1;          /* 0x0010 */
    u32 cpu_dsp_rv_clk_gating_ctrl;
    u32 cpu_dsp_rv_rst_ctrl;
    u32 mbus_clk_gating_ctrl;
    u32 spi0_clk_ctrl;          /* 0x0020 */
    u32 spi1_clk_ctrl;
    u32 sdc_clk_ctrl;
    u32 ss_clk_ctrl;
    u32 csi_dclk_ctrl;          /* 0x0030 */
    u32 ledc_clk_ctrl;
    u32 irrx_clk_ctrl;
    u32 irtx_clk_ctrl;
    u32 systick_refclk_ctrl;        /* 0x0040 */
    u32 systick_calib_ctrl;
    u32 reserved_0x048[2];
    u32 csi_out_mclk_ctrl;          /* 0x0050 */
    u32 flashc_mclk_ctrl;
    u32 sqpi_psramc_clk_ctrl;
    u32 apb_spc_clk_ctrl;
    u32 usb_clk_ctrl;           /* 0x0060 */
    u32 riscv_clk_ctrl;
    u32 dsp_clk_ctrl;
    u32 hpsram_clk_ctrl;
    u32 lpsram_clk_ctrl;            /* 0x0070 */
    u32 g2d_clk_ctrl;
    u32 de_clk_ctrl;
    u32 lcd_clk_ctrl;
    u32 reserved_0x080[0x100-0x80];     /* 0x0080 */
    u32 reset_source_recode;        /* 0x0100 */
};

typedef struct _ehci_config
{
	u32 ehci_base;
	u32 bus_soft_reset_ofs;
	u32 bus_clk_gating_ofs;
	u32 phy_reset_ofs;
	u32 phy_slk_gatimg_ofs;
	u32 usb0_support;
	char name[32];
}ehci_config_t;


static ehci_config_t ehci_cfg[HOST_MAXNUM] = {
	{SUNXI_EHCI0_BASE, USBEHCI0_RST_BIT, USBEHCI0_GATIING_BIT, USBPHY0_RST_BIT, 0, 1, "USB-DRD"},
#ifdef SUNXI_EHCI1_BASE
	{SUNXI_EHCI1_BASE, USBEHCI1_RST_BIT, USBEHCI1_GATIING_BIT, USBPHY1_RST_BIT, USBPHY1_SCLK_GATING_BIT, 0, "USB1-Host"},
#endif
#ifdef SUNXI_EHCI2_BASE
	{SUNXI_EHCI2_BASE, USBEHCI2_RST_BIT, USBEHCI2_GATIING_BIT, USBPHY2_RST_BIT, USBPHY2_SCLK_GATING_BIT, 0, "USB2-Host"},
#endif
#ifdef SUNXI_EHCI3_BASE
	{SUNXI_EHCI3_BASE, USBEHCI3_RST_BIT, USBEHCI3_GATIING_BIT, USBPHY3_RST_BIT, USBPHY3_SCLK_GATING_BIT, 0, "USB3-Host"},
#endif
};

struct usbh_bus *usb_otg_hs_bus;

#ifndef USB_BASE
#define USB_BASE (0x40040000UL)
#endif

uint8_t usbh_get_port_speed(const uint8_t port)
{
	u32 ehci_vbase = ehci_cfg[0].ehci_base;
	u32 reg_val = 0;

	reg_val = hal_readl(ehci_vbase + 0x54);

	hal_log_debug("getting speed: 0x%x\n", reg_val);

	if ((reg_val & EHCI_PORTSC_LSTATUS_MASK) == EHCI_PORTSC_LSTATUS_KSTATE)
		return USB_SPEED_LOW;

	if (reg_val & EHCI_PORTSC_PE)
		return USB_SPEED_HIGH;
	else
		return USB_SPEED_FULL;

	return USB_SPEED_UNKNOWN;
}


/*Port:port+num<fun><pull><drv><data>*/
ulong config_usb_pin(int index)
{

	//usb_id_gpio 			= port:PB04<0><0><default><default>
	hal_gpio_pinmux_set_function(GPIO_PB4, 0);

	//usb_det_vbus_gpio       = port:PA24<0><0><default><default>
	hal_gpio_pinmux_set_function(GPIO_PA24, 0);

	//usb_drv_vbus_gpio		= port:PA29<1><0><default><default>
	hal_gpio_pinmux_set_function(GPIO_PA29, 1);
	/* reserved is pull down */
	hal_gpio_set_pull(GPIO_PA29, 0);

	return 0;
}

__WEAK u32 axp_usb_vbus_output(int is_on)
{
	return 0;
}

#define KEY_USB_DRVVBUS_TYPE            "usb_drv_vbus_type"
#define KEY_USB_DRVVBUS_GPIO            "usb_drv_vbus_gpio"

enum usb_drv_vbus_type {
	USB_DRV_VBUS_TYPE_NULL = 0,
	USB_DRV_VBUS_TYPE_GIPO,
	USB_DRV_VBUS_TYPE_AXP,
};

void sunxi_set_drv_vbus(int is_on)
{
	int pin_type = 0, drv_vbus_gpio_set = 0;
	enum usb_drv_vbus_type drv_vbus_type;
	user_gpio_set_t pin_value = {0};
	int ret = -1;

	ret = hal_cfg_get_keyvalue("usbc0", KEY_USB_DRVVBUS_TYPE, (int32_t *)&pin_type, 1);
	if (ret) {
		hal_log_err("get %s fail!", KEY_USB_DRVVBUS_TYPE);
		drv_vbus_type = USB_DRV_VBUS_TYPE_NULL;
		drv_vbus_gpio_set = -1;
	} else {
		drv_vbus_type = pin_type;
		ret = hal_cfg_get_keyvalue("usbc0", KEY_USB_DRVVBUS_GPIO, (int32_t *)&pin_value,
				(sizeof(user_gpio_set_t) + 3) / sizeof(int));
		if (ret) {
			hal_log_err("get %s fail!", KEY_USB_DRVVBUS_GPIO);
			drv_vbus_gpio_set = -1;
		} else {
			drv_vbus_gpio_set = (pin_value.port - 1) * PINS_PER_BANK + pin_value.port_num;
			hal_gpio_set_direction(drv_vbus_gpio_set, GPIO_DIRECTION_OUTPUT);
		}
	}
	printf("usbc0 get drv_type:%d drv_vbus_io:%d\n", drv_vbus_type, drv_vbus_gpio_set);

	if (drv_vbus_type == USB_DRV_VBUS_TYPE_GIPO) {
		if (drv_vbus_gpio_set != -1)
			hal_gpio_set_data(drv_vbus_gpio_set, is_on);
	} else if (drv_vbus_type == USB_DRV_VBUS_TYPE_AXP) {
		axp_usb_vbus_output(is_on);
	}
}

#if defined(CONFIG_ARCH_SUN20IW2)
#define SUNXI_GPRCM_BASE                (0x40050000)
#define USB_BIAS_CTRL                   (0x0064)
#define USB_BIAS_CTRL_EN                (0x0001)

static void sunxi_usb_enable_bias(void)
{
	int reg_value = 0;

	reg_value = hal_readl(SUNXI_GPRCM_BASE + USB_BIAS_CTRL);
	reg_value |= USB_BIAS_CTRL_EN;
	hal_writel(reg_value, SUNXI_GPRCM_BASE + USB_BIAS_CTRL);
}
#endif

#define USB_PHY_OFFSET_CTRL             0x10
#define USB_PHY_OFFSET_TUNE             0x18
#define USB_PHY_OFFSET_STATUS           0x24

/*phy read/write vc-bus contrl register*/
#define USB_PHY_CTRL_VC_CLK             (0x1 << 0)      //bit0
#define USB_PHY_CTRL_VC_EN              (0x1 << 1)      //bit1
#define USB_PHY_CTRL_VC_DI              (0x1 << 7)      //bit7
#define USB_PHY_CTRL_VC_ADDR            (0xff << 8)     //bit8-15
#define USB_PHY_STATUS_VC_DO            (0x1 << 0)      //bit0

static int bit_offset(uint32_t mask)
{
	int offset = 0;

	for (offset = 0; offset < 32; offset++)
		if ((mask == 0) || (mask & (0x01 << offset)))
			break;
	return offset;
}

static int usb_phy_tp_write(unsigned long phy_vbase, int addr, int data, int len)
{
	int j = 0;
	int temp = 0;
	int dtmp = data;

	/*VC_EN enable*/
	temp = hal_readb(phy_vbase + USB_PHY_OFFSET_CTRL);
	temp |= USB_PHY_CTRL_VC_EN;
	hal_writeb(temp, phy_vbase + USB_PHY_OFFSET_CTRL);

	for (j = 0; j < len; j++) {
		/*ensure VC_CLK low*/
		temp = hal_readb(phy_vbase + USB_PHY_OFFSET_CTRL);
		temp &= ~USB_PHY_CTRL_VC_CLK;
		hal_writeb(temp, phy_vbase + USB_PHY_OFFSET_CTRL);

		/*set write address*/
		temp = hal_readl(phy_vbase + USB_PHY_OFFSET_CTRL);
		temp &= ~USB_PHY_CTRL_VC_ADDR; //clear
		temp |= ((addr + j) << bit_offset(USB_PHY_CTRL_VC_ADDR));  // write
		hal_writel(temp, phy_vbase + USB_PHY_OFFSET_CTRL);

		/*write data to VC_DI*/
		temp = hal_readb(phy_vbase + USB_PHY_OFFSET_CTRL);
		temp &= ~USB_PHY_CTRL_VC_DI; //clear
		temp |= ((dtmp & 0x01) << bit_offset(USB_PHY_CTRL_VC_DI));  // write
		hal_writeb(temp, phy_vbase + USB_PHY_OFFSET_CTRL);

		/*set VC_CLK high*/
		temp |= USB_PHY_CTRL_VC_CLK;
		hal_writeb(temp, phy_vbase + USB_PHY_OFFSET_CTRL);

		/*right move one bit*/
		dtmp >>= 1;
	}

	/*set VC_CLK low*/
	temp = hal_readb(phy_vbase + USB_PHY_OFFSET_CTRL);
	temp &= ~USB_PHY_CTRL_VC_CLK;
	hal_writeb(temp, phy_vbase + USB_PHY_OFFSET_CTRL);

	/*VC_EN disable*/
	temp = hal_readb(phy_vbase + USB_PHY_OFFSET_CTRL);
	temp &= ~USB_PHY_CTRL_VC_EN;
	hal_writeb(temp, phy_vbase + USB_PHY_OFFSET_CTRL);

	return 0;
}

static int usb_phy_tp_read(unsigned long phy_vbase, int addr, int len)
{
	int temp = 0, i = 0, j = 0, ret = 0 ;

	/*VC_EN enable*/
	temp = hal_readb(phy_vbase + USB_PHY_OFFSET_CTRL);
	temp |= USB_PHY_CTRL_VC_EN;
	hal_writeb(temp, phy_vbase + USB_PHY_OFFSET_CTRL);

	for (j = len; j > 0; j--) {
		/*set write address*/
		temp = hal_readl(phy_vbase + USB_PHY_OFFSET_CTRL);
		temp &= ~USB_PHY_CTRL_VC_ADDR;//clear
		temp |= ((addr + j - 1) << bit_offset(USB_PHY_CTRL_VC_ADDR));  // write
		hal_writel(temp, phy_vbase + USB_PHY_OFFSET_CTRL);

		/*delsy 1us*/
		hal_udelay(1);

		/*read data from VC_DO*/
		temp = hal_readb(phy_vbase + USB_PHY_OFFSET_STATUS);
		ret <<= 1;
		ret |= temp & USB_PHY_STATUS_VC_DO;
	}

	/*VC_EN disable*/
	temp = hal_readb(phy_vbase + USB_PHY_OFFSET_CTRL);
	temp &= ~USB_PHY_CTRL_VC_EN;
	hal_writeb(temp, phy_vbase + USB_PHY_OFFSET_CTRL);

	return ret;
}

static void sunxi_usb_phy_init(unsigned long phy_vbase, unsigned int usbc_no)
{
	/* 0xC - 0x1, enable calibration */
	usb_phy_tp_write(phy_vbase, 0xc, 0x1, 1);
	/* 0x20~0x21 - 0x0, adjust amplitude */
	usb_phy_tp_write(phy_vbase, 0x20, 0x0, 2);
	/* 0x22~0x24 - 0x1, adjust rate */
	usb_phy_tp_write(phy_vbase, 0x22, 0x1, 3);
	/* 0x3~0x4 - 0x0, adjust pll */
	usb_phy_tp_write(phy_vbase, 0x3, 0x0, 2);

	/* 0x1a~0x1f, ro, read calibration value */
	printf("calibration finish, val:0x%x, usbc_no:%d\n",
			usb_phy_tp_read(phy_vbase, 0x1a, 6), usbc_no);
}

u32 open_usb_clock(int index)
{
	u32 reg_value = 0;
	u32 ehci_vbase = ehci_cfg[index].ehci_base;

	struct sunxi_ccm_reg *const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	/* Bus reset for ehci */
	reg_value = hal_readl(&ccm->dev_rst_ctrl0);
	reg_value |= (1 << ehci_cfg[index].bus_soft_reset_ofs);
	hal_writel(reg_value, &ccm->dev_rst_ctrl0);

	/* Bus gating for ehci */
	reg_value = hal_readl(&ccm->bus_clk_gating_ctrl0);
	reg_value |= (1 << ehci_cfg[index].bus_clk_gating_ofs);
	hal_writel(reg_value, &ccm->bus_clk_gating_ctrl0);

	/* open clk for usb phy */
	reg_value = hal_readl(&ccm->dev_rst_ctrl0);
	reg_value |= (1 << ehci_cfg[index].phy_reset_ofs);
	hal_writel(reg_value, &ccm->dev_rst_ctrl0);

	hal_writel(0x1, 0x4003c060);

	printf("config usb clk ok\n");

#if defined(CONFIG_ARCH_SUN20IW2)
	sunxi_usb_enable_bias();
#endif
	sunxi_usb_phy_init(ehci_vbase + SUNXI_USB_CTRL, index);

	return 0;
}

void cherryusb_passby(int index, u32 enable)
{
	unsigned long reg_value = 0;
	u32 ehci_vbase = ehci_cfg[index].ehci_base;

	if(ehci_cfg[index].usb0_support)
	{
		/* the default mode of usb0 is OTG,so change it here. */
		reg_value = hal_readl(SUNXI_USBOTG_BASE + 0x420);
		reg_value &= ~(0x01);
		hal_writel(reg_value, (SUNXI_USBOTG_BASE + 0x420));
	}

	reg_value = hal_readl(ehci_vbase + SUNXI_USB_PHY_CTRL);
	reg_value &= ~(0x01 << 1);
	hal_writel(reg_value, (ehci_vbase + SUNXI_USB_PHY_CTRL));

	reg_value = hal_readl(ehci_vbase + SUNXI_USB_CTRL);
	if (enable) {
		reg_value |= (1 << 10);		/* AHB Master interface INCR8 enable */
		reg_value |= (1 << 9);     	/* AHB Master interface burst type INCR4 enable */
		reg_value |= (1 << 8);     	/* AHB Master interface INCRX align enable */
		reg_value |= (1 << 0);     	/* ULPI bypass enable */
	} else if(!enable) {
		reg_value &= ~(1 << 10);	/* AHB Master interface INCR8 disable */
		reg_value &= ~(1 << 9);     /* AHB Master interface burst type INCR4 disable */
		reg_value &= ~(1 << 8);     /* AHB Master interface INCRX align disable */
		reg_value &= ~(1 << 0);     /* ULPI bypass disable */
	}
	hal_writel(reg_value, (ehci_vbase + SUNXI_USB_CTRL));

	return;
}

int sunxi_start_ehci(int index)
{
	u32 ehci_vbase = ehci_cfg[index].ehci_base;
	u32 reg_value = 0;

	sunxi_set_drv_vbus(1);

	extern void USBH_IRQHandler(void);
	if (hal_request_irq(79, (hal_irq_handler_t)USBH_IRQHandler, "usb_ehci", NULL) < 0) {
		printf("request irq failed\n");
		return -1;
	}
	hal_enable_irq(79);

	open_usb_clock(index);
	cherryusb_passby(index, 1);

	return 0;
}

void usb_hc_low_level_init(void)
{
	printf("%s->%d usb host init\n", __func__, __LINE__);
	sunxi_start_ehci(0);
}

void usb_ehci_dcache_clean(unsigned long addr, unsigned long size)
{
	hal_dcache_clean(addr, size);
}

void usb_ehci_dcache_invalidate(unsigned long addr, unsigned long size)
{
	hal_dcache_invalidate(addr, size);
}

void usb_ehci_dcache_clean_invalidate(unsigned long addr, unsigned long size)
{
	hal_dcache_clean_invalidate(addr, size);
}
