/*
 * Copyright (c) 2022, sakumisu
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <hal_gpio.h>
#include <hal_time.h>
#include <hal_interrupt.h>
#include <hal_cfg.h>
#include <script.h>
#include "hal_clk.h"
#include "hal_reset.h"
#include "usbd_core.h"
#include "usb_dc_sunxi.h"

#ifndef USBD_IRQHandler
#error "please define USBD_IRQHandler in usb_config.h"
#endif

#ifndef CONFIG_USBDEV_EP_NUM
#define CONFIG_USBDEV_EP_NUM 8
#endif

/* Driver state */
struct udc {
    volatile uint8_t dev_addr;
    volatile uint32_t fifo_size_offset;
    __attribute__((aligned(32))) struct usb_setup_packet setup;
    struct udc_ep_state in_ep[CONFIG_USBDEV_EP_NUM];  /*!< IN endpoint parameters*/
    struct udc_ep_state out_ep[CONFIG_USBDEV_EP_NUM]; /*!< OUT endpoint parameters */
} g_udc;

static volatile uint8_t usb_ep0_state = USB_EP0_STATE_SETUP;
volatile bool zlp_flag = 0;
static volatile UDC_REGISTER_T *sunxi_udc = (UDC_REGISTER_T *)SUNXI_USB_OTG_PBASE;
static volatile USBPHY_REGISTER_T *sunxi_udc_phy = (USBPHY_REGISTER_T *)(SUNXI_USB_OTG_PBASE + USB_PHY_BASE_OFFSET);

static void *usbc_select_fifo(uint32_t ep_index)
{
    uint32_t offset;

    offset = 0x0 + (ep_index << 2);

    return (void *)((char *)&sunxi_udc->fifo0 + offset);
}

/* get current active ep */
static uint8_t udc_get_active_ep(void)
{
    return USB_DRV_Reg8(&sunxi_udc->index);
}

/* set the active ep */
static void udc_set_active_ep(uint8_t ep_index)
{
    USB_DRV_WriteReg8(&sunxi_udc->index, ep_index);
}

static void udc_write_packet(uint8_t ep_idx, uint8_t *buffer, uint16_t len)
{
    uint32_t *buf32;
    uint8_t *buf8;
    uint32_t count32;
    uint32_t count8;
    int i;

    if ((uint32_t)(uintptr_t)buffer & 0x03) {
        buf8 = buffer;
        for (i = 0; i < len; i++) {
            USB_DRV_WriteReg8(usbc_select_fifo(ep_idx), *buf8++);
        }
    } else {
        count32 = len >> 2;
        count8 = len & 0x03;

        buf32 = (uint32_t *)buffer;

        while (count32--) {
            USB_DRV_WriteReg32(usbc_select_fifo(ep_idx), *buf32++);
        }

        buf8 = (uint8_t *)buf32;

        while (count8--) {
            USB_DRV_WriteReg8(usbc_select_fifo(ep_idx), *buf8++);
        }
    }
}

static void udc_read_packet(uint8_t ep_idx, uint8_t *buffer, uint16_t len)
{
    uint32_t *buf32;
    uint8_t *buf8;
    uint32_t count32;
    uint32_t count8;
    int i;

    if ((uint32_t)(uintptr_t)buffer & 0x03) {
        buf8 = buffer;
        for (i = 0; i < len; i++) {
            *buf8++ = USB_DRV_Reg8(usbc_select_fifo(ep_idx));
        }
    } else {
        count32 = len >> 2;
        count8 = len & 0x03;

        buf32 = (uint32_t *)buffer;

        while (count32--) {
            *buf32++ = USB_DRV_Reg32(usbc_select_fifo(ep_idx));
        }

        buf8 = (uint8_t *)buf32;

        while (count8--) {
            *buf8++ = USB_DRV_Reg8(usbc_select_fifo(ep_idx));
        }
    }
}

static uint32_t udc_get_fifo_size(uint16_t mps, uint16_t *used)
{
    uint32_t size;

    for (uint8_t i = USB_TXFIFOSZ_SIZE_8; i <= USB_TXFIFOSZ_SIZE_2048; i++) {
        size = (8 << i);
        if (mps <= size) {
            *used = size;
            return i;
        }
    }

    *used = 0;
    return USB_TXFIFOSZ_SIZE_8;
}

static void sunxi_usbd_set_drv_vbus(void)
{
    int ret = -1,vbus_det_io;

#ifdef CONFIG_DRIVER_SYSCONFIG
    user_gpio_set_t gpio_set;
    ret = hal_cfg_get_keyvalue("usbc0", KEY_USB_DRVVBUS_GPIO, (int32_t *)&gpio_set,
            sizeof(user_gpio_set_t) >> 2);
    if (ret == 0) {
        vbus_det_io = (gpio_set.port - 1) * 32 + gpio_set.port_num;
        hal_log_err("get %s, ret:%d\n", KEY_USB_DRVVBUS_GPIO, ret);
        hal_gpio_pinmux_set_function(vbus_det_io, GPIO_MUXSEL_IN);
    } else {
        vbus_det_io = -1;
        hal_log_err("not support det vbus\n");
    }
#else
    vbus_det_io = -1;
#endif
    USB_LOG_INFO("usbc0 drv_vbus_io:%d\n", vbus_det_io);
}

u32 open_usbd_clock(struct platform_usb_config *sunxi_udc)
{
    hal_reset_type_t reset_type = HAL_SUNXI_RESET;
    hal_clk_type_t clk_type = HAL_SUNXI_CCU;
    hal_clk_status_t ret;
    struct reset_control *reset_phy, *reset_otg;
    hal_clk_t phy_clk, otg_clk;

    /*open otg clk*/
    reset_phy = hal_reset_control_get(reset_type, sunxi_udc->phy_rst);
    ret = hal_reset_control_deassert(reset_phy);
    if (ret) {
        hal_log_err("reset phy err!\n");
        return -1;
    }
    hal_reset_control_put(reset_phy);

    reset_otg = hal_reset_control_get(reset_type, sunxi_udc->usb_rst);
    ret = hal_reset_control_deassert(reset_otg);
    if (ret) {
        hal_log_err("reset otg err!\n");
        return -1;
    }
    hal_reset_control_put(reset_otg);

    /*open udc clk*/
    phy_clk = hal_clock_get(clk_type, sunxi_udc->phy_clk);
    ret = hal_clock_enable(phy_clk);
    if (ret) {
        hal_log_err("couldn't enable usb_phy_clk!\n");
        return -1;
    }

    otg_clk = hal_clock_get(clk_type, sunxi_udc->usb_clk);
    ret = hal_clock_enable(otg_clk);
    if (ret) {
        hal_log_err("couldn't enable otg_clk!\n");
        return -1;
    }

    return 0;
}

static void usbc_wakeup_clear_change_detect(void)
{
    USB_DRV_ClearBits32(&sunxi_udc_phy->iscr, USB_ISCR_VBUS_CHANGE_DETECT);
    USB_DRV_ClearBits32(&sunxi_udc_phy->iscr, USB_ISCR_ID_CHANGE_DETECT);
    USB_DRV_ClearBits32(&sunxi_udc_phy->iscr, USB_ISCR_DPDM_CHANGE_DETECT);
}

static void usbd_enable_dpdm_pullup(bool enable)
{
    if (enable) {
        USB_DRV_SetBits32(&sunxi_udc_phy->iscr, USB_ISCR_DPDM_PULLUP_EN);
    } else {
        USB_DRV_ClearBits32(&sunxi_udc_phy->iscr, USB_ISCR_DPDM_PULLUP_EN);
    }

    usbc_wakeup_clear_change_detect();

    USB_LOG_INFO("dp dm pull up %s\r\n", enable ? "enabled" : "disabled");
}

static void usbd_enable_id_pullup(bool enable)
{
    if (enable) {
        USB_DRV_SetBits32(&sunxi_udc_phy->iscr, USB_ISCR_ID_PULLUP_EN);
    } else {
        USB_DRV_ClearBits32(&sunxi_udc_phy->iscr, USB_ISCR_ID_PULLUP_EN);
    }

    usbc_wakeup_clear_change_detect();

    USB_LOG_INFO("id pull up %s\r\n", enable ? "enabled" : "disabled");
}

static void usbd_force_id(uint32_t id_type)
{
    USB_DRV_ClearBits32(&sunxi_udc_phy->iscr, USB_ISCR_FORCE_ID_MASK);
    USB_DRV_SetBits32(&sunxi_udc_phy->iscr, id_type);

    usbc_wakeup_clear_change_detect();

    USB_LOG_INFO("force id type: 0x%x\r\n", id_type);
}

static void usbd_select_bus(udc_io_type_t io_type, udc_ep_type_t ep_type, uint32_t ep_index)
{
    uint32_t reg_val;

    reg_val = USB_DRV_Reg8(&sunxi_udc->vend0);

    if (io_type == UDC_IO_TYPE_DMA) {
        if (ep_type == UDC_EP_TYPE_TX) {
            reg_val |= ((ep_index - 0x01) << 1) << USB_VEND0_DRQ_SEL; /* drq_sel */
            reg_val |= 0x1 << USB_VEND0_BUS_SEL; /* io_dma */
        } else {
            reg_val |= (((ep_index - 0x01) << 1) + 1) << USB_VEND0_DRQ_SEL; /* drq_sel */
            reg_val |= 0x1 << USB_VEND0_BUS_SEL; /* io_dma */
        }
    } else {
        reg_val &= 0x00; /* clear drq_sel, select pio */
    }

    /*
     * in SUN8IW5 SUN8IW6 and later ic, FIFO_BUS_SEL bit(bit24 of reg0x40
     * for host/device) is fixed to 1, the hw guarantee that it's ok for
     * cpu/inner_dma/outer_dma transfer.
     */
#if !defined(CONFIG_SOC_SUN20IW1) && !defined(CONFIG_SOC_SUN20IW2)
    reg_val |= 0x1 << USB_VEND0_BUS_SEL;
#endif
    USB_DRV_WriteReg8(&sunxi_udc->vend0, reg_val);
}

static void usbd_force_vbus_valid(uint32_t vbus_type)
{
    USB_DRV_ClearBits32(&sunxi_udc_phy->iscr, USB_ISCR_FORCE_VBUS_MASK);
    USB_DRV_SetBits32(&sunxi_udc_phy->iscr, vbus_type);

    usbc_wakeup_clear_change_detect();

    USB_LOG_INFO("force vbus valid type: 0x%x\r\n", vbus_type);
}

static void usbd_phy_set_ctl(bool set)
{
    /* NOTICE: 40nm platform is different */
#if defined(CONFIG_ARCH_SUN20IW2)
    /* USB BIAS enable, set 0 if usb disable to save power at sleep */
    USB_DRV_SetBits32(SUNXI_GPRCM_BASE + USB_BIAS_CTRL, USB_BIAS_CTRL_EN);
    /* TODO: 40nm platform's phyctrl register is the same as 28nm */
    if (set) {
        USB_DRV_ClearBits32(&sunxi_udc_phy->phyctrl28nm, USB_PHYCTL40NM_SIDDQ);
    } else {
        USB_DRV_SetBits32(&sunxi_udc_phy->phyctrl28nm, USB_PHYCTL40NM_SIDDQ);
    }
#else
    if (set) {
        USB_DRV_SetBits32(&sunxi_udc_phy->phyctrl28nm, USB_PHYCTL28NM_VBUSVLDEXT);
        USB_DRV_ClearBits32(&sunxi_udc_phy->phyctrl28nm, USB_PHYCTL28NM_SIDDQ);
    } else {
        USB_DRV_SetBits32(&sunxi_udc_phy->phyctrl28nm, USB_PHYCTL28NM_SIDDQ);
    }
#endif
    USB_LOG_INFO("%s:%d\n", __func__, __LINE__);
}

static void usbd_phy_otg_sel(bool otg_sel)
{
    if (otg_sel) {
        USB_DRV_SetBits32(&sunxi_udc_phy->physel, USB_PHYSEL_OTG_SEL);
    } else {
        USB_DRV_ClearBits32(&sunxi_udc_phy->physel, USB_PHYSEL_OTG_SEL);
    }
}

static int bit_offset(uint32_t mask)
{
    int offset = 0;

    for (offset = 0; offset < 32; offset++)
        if ((mask == 0) || (mask & (0x01 << offset)))
            break;
    return offset;
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

void usbd_phy_init(unsigned long phy_vbase, unsigned int usbc_no)
{
    /* 0xC - 0x1, enable calibration */
    usb_phy_tp_write(phy_vbase, 0xC, 0x1, 1);
    /* 0x20~0x21 - 0x0, adjust amplitude */
    usb_phy_tp_write(phy_vbase, 0x20, 0x0, 2);
    /* 0x22~0x24 - 0x1, adjust rate */
    usb_phy_tp_write(phy_vbase, 0x22, 0x1, 3);
    /* 0x3~0x4 - 0x0, adjust pll */
    usb_phy_tp_write(phy_vbase, 0x3, 0x0, 2);

    /* 0x1a~0x1f, ro, read calibration value */
    USB_LOG_INFO("calibration finish, val:0x%x, usbc_no:%d\n",
             usb_phy_tp_read(phy_vbase, 0x1a, 6), usbc_no);
}

static void usbd_disable(void)
{
    USB_LOG_INFO("udc disable\r\n");

    /* disable all interrupts */
    USB_DRV_WriteReg8(&sunxi_udc->intrusbe, 0);
    USB_DRV_WriteReg8(&sunxi_udc->intrtxe, 0);
    USB_DRV_WriteReg8(&sunxi_udc->intrrxe, 0);

    /* clear the interrupt registers */
    USB_DRV_WriteReg(&sunxi_udc->intrtx, 0xffff);
    USB_DRV_WriteReg(&sunxi_udc->intrrx, 0xffff);
    USB_DRV_WriteReg8(&sunxi_udc->intrusb, 0xff);

    /* clear soft connect */
    USB_DRV_ClearBits8(&sunxi_udc->power, USB_POWER_SOFTCONN);
}

static void usbd_enable(void)
{
    USB_LOG_INFO("udc enable\r\n");

    /* config usb transfer type, default: bulk transfer */
    USB_DRV_ClearBits8(&sunxi_udc->power, USB_POWER_ISOUPDATE);

    /* config usb gadget speed, default: high speed */
    USB_DRV_SetBits8(&sunxi_udc->power, USB_POWER_HSENAB);

    /* enable usb bus interrupt */
    USB_DRV_SetBits8(&sunxi_udc->intrusbe, USB_INTRUSB_SUSPEND
                        | USB_INTRUSB_RESUME
                        | USB_INTRUSB_RESET);

    /* enable ep0 interrupt */
    USB_DRV_SetBits(&sunxi_udc->intrtxe, USB_INTRE_EPEN << 0);

    /* set soft connect */
    USB_DRV_SetBits8(&sunxi_udc->power, USB_POWER_SOFTCONN);

    // krhino_spin_lock_init(&g_udc.lock);
}

void sunxi_start_udc(void)
{
    int ret;
    sunxi_usbd_set_drv_vbus();
    struct platform_usb_config *otg_config = &usb_otg_table;

    open_usbd_clock(otg_config);
    if (ret) {
        USB_LOG_ERR("open udc clk failed\n");
        return;
    }
    usbd_enable_dpdm_pullup(true);
    usbd_enable_id_pullup(true);
    usbd_force_id(USB_ISCR_FORCE_ID_HIGH);
    usbd_force_vbus_valid(USB_ISCR_FORCE_VBUS_HIGH);
    usbd_select_bus(UDC_IO_TYPE_PIO, UDC_EP_TYPE_EP0, 0);
    usbd_phy_set_ctl(true);
    usbd_phy_otg_sel(true);
    usbd_phy_init((unsigned long)sunxi_udc_phy, 0);
    usbd_disable();
    void USBD_IRQHandler(void);
        /* request irq */
    if (hal_request_irq(otg_config->irq, (hal_irq_handler_t)USBD_IRQHandler, "usb_udc", NULL) < 0) {
        USB_LOG_ERR("request irq error\n");
        return;
    }

    hal_enable_irq(otg_config->irq);
    usbd_enable();
}

void usb_dc_low_level_init(void)
{
    USB_LOG_INFO("%s->%d usb devices init\n", __func__, __LINE__);
    sunxi_start_udc();
}

int usb_dc_init(void)
{
    usb_dc_low_level_init();

    return 0;
}

int usb_dc_deinit(void)
{
    struct platform_usb_config *otg_config = &usb_otg_table;
    /*free irq*/
    hal_free_irq(otg_config->irq);
    /* udc bsp deinit */
    usbd_enable_dpdm_pullup(false);
    usbd_force_id(USB_ISCR_FORCE_ID_DISABLED);
    usbd_force_vbus_valid(USB_ISCR_FORCE_VBUS_DISABLED);

    memset(&g_udc, 0, sizeof(struct udc));
    return 0;
}

int usbd_set_address(const uint8_t addr)
{
    if (addr == 0) {
        USB_DRV_WriteReg8(&sunxi_udc->faddr, 0);
    }

    g_udc.dev_addr = addr;
    return 0;
}

uint8_t usbd_get_port_speed(const uint8_t port)
{
    uint8_t speed = USB_SPEED_UNKNOWN;

    if (USB_DRV_Reg8(&sunxi_udc->power) & USB_POWER_HSMODE) {
        speed = USB_SPEED_HIGH;
    } else if (USB_DRV_Reg8(&sunxi_udc->devctl) & USB_DEVCTL_FSDEV) {
        speed = USB_SPEED_FULL;
    } else if (USB_DRV_Reg8(&sunxi_udc->devctl) & USB_DEVCTL_LSDEV) {
        speed = USB_SPEED_LOW;
    }

    return speed;
}

int usbd_ep_open(const struct usb_endpoint_descriptor *ep)
{
    uint16_t used = 0;
    uint16_t fifo_size = 0;
    uint8_t ep_idx = USB_EP_GET_IDX(ep->bEndpointAddress);
    uint8_t old_ep_idx;
    uint32_t ui32Flags = 0;
    uint16_t ui32Register = 0;

    if (ep_idx == 0) {
        g_udc.out_ep[0].ep_mps = USB_CTRL_EP_MPS;
        g_udc.out_ep[0].ep_type = 0x00;
        g_udc.out_ep[0].ep_enable = true;
        g_udc.in_ep[0].ep_mps = USB_CTRL_EP_MPS;
        g_udc.in_ep[0].ep_type = 0x00;
        g_udc.in_ep[0].ep_enable = true;
        return 0;
    }

    if (ep_idx > (CONFIG_USBDEV_EP_NUM - 1)) {
        USB_LOG_ERR("Ep addr %02x overflow\r\n", ep->bEndpointAddress);
        return -1;
    }

    old_ep_idx = udc_get_active_ep();
    udc_set_active_ep(ep_idx);

    if (USB_EP_DIR_IS_OUT(ep->bEndpointAddress)) {
        g_udc.out_ep[ep_idx].ep_mps = USB_GET_MAXPACKETSIZE(ep->wMaxPacketSize);
        g_udc.out_ep[ep_idx].ep_type = USB_GET_ENDPOINT_TYPE(ep->bmAttributes);
        g_udc.out_ep[ep_idx].ep_enable = true;

        USB_DRV_WriteReg(&sunxi_udc->rxmap, USB_GET_MAXPACKETSIZE(ep->wMaxPacketSize));

        //
        // Allow auto clearing of RxPktRdy when packet of size max packet
        // has been unloaded from the FIFO.
        //
        if (ui32Flags & USB_EP_AUTO_CLEAR) {
            ui32Register = USB_RXCSRH1_AUTOCL;
        }
        //
        // Configure the DMA mode.
        //
        if (ui32Flags & USB_EP_DMA_MODE_1) {
            ui32Register |= USB_RXCSRH1_DMAEN | USB_RXCSRH1_DMAMOD;
        } else if (ui32Flags & USB_EP_DMA_MODE_0) {
            ui32Register |= USB_RXCSRH1_DMAEN;
        }
        //
        // If requested, disable NYET responses for high-speed bulk and
        // interrupt endpoints.
        //
        if (ui32Flags & USB_EP_DIS_NYET) {
            ui32Register |= USB_RXCSRH1_DISNYET;
        }

        //
        // Enable isochronous mode if requested.
        //
        if (USB_GET_ENDPOINT_TYPE(ep->bmAttributes) == 0x01) {
            ui32Register |= USB_RXCSRH1_ISO;
        }
        USB_DRV_WriteReg8(&sunxi_udc->rxcsrh, ui32Register);

        // Reset the Data toggle to zero.
        if (USB_DRV_Reg8(&sunxi_udc->rxcsrl) & USB_RXCSRL1_RXRDY ) {
            USB_DRV_WriteReg8(&sunxi_udc->rxcsrl, USB_RXCSRL1_CLRDT | USB_RXCSRL1_FLUSH);
        } else {
            USB_DRV_WriteReg8(&sunxi_udc->rxcsrl, USB_RXCSRL1_CLRDT);
        }

        fifo_size = udc_get_fifo_size(USB_GET_MAXPACKETSIZE(ep->wMaxPacketSize), &used);

        USB_DRV_WriteReg(&sunxi_udc->rxfifosz, (fifo_size & 0x0f));
        USB_DRV_WriteReg(&sunxi_udc->rxfifoadd, g_udc.fifo_size_offset >> 3);

        g_udc.fifo_size_offset += used;
    } else {
        g_udc.in_ep[ep_idx].ep_mps = USB_GET_MAXPACKETSIZE(ep->wMaxPacketSize);
        g_udc.in_ep[ep_idx].ep_type = USB_GET_ENDPOINT_TYPE(ep->bmAttributes);
        g_udc.in_ep[ep_idx].ep_enable = true;

        USB_DRV_WriteReg(&sunxi_udc->txmap, USB_GET_MAXPACKETSIZE(ep->wMaxPacketSize));

        //
        // Allow auto setting of TxPktRdy when max packet size has been loaded
        // into the FIFO.
        //
        if (ui32Flags & USB_EP_AUTO_SET) {
            ui32Register |= USB_TXCSRH1_AUTOSET;
        }

        //
        // Configure the DMA mode.
        //
        if (ui32Flags & USB_EP_DMA_MODE_1) {
            ui32Register |= USB_TXCSRH1_DMAEN | USB_TXCSRH1_DMAMOD;
        } else if (ui32Flags & USB_EP_DMA_MODE_0) {
            ui32Register |= USB_TXCSRH1_DMAEN;
        }

        //
        // Enable isochronous mode if requested.
        //
        if (USB_GET_ENDPOINT_TYPE(ep->bmAttributes) == 0x01) {
            ui32Register |= USB_TXCSRH1_ISO;
        }

        USB_DRV_WriteReg8(&sunxi_udc->txcsrh, ui32Register);

        // Reset the Data toggle to zero.
        if (USB_DRV_Reg8(&sunxi_udc->txcsrl) & USB_TXCSRL1_TXRDY) {
            USB_DRV_WriteReg8(&sunxi_udc->txcsrl, USB_TXCSRL1_CLRDT | USB_TXCSRL1_FLUSH);
        }
        else {
            USB_DRV_WriteReg8(&sunxi_udc->txcsrl, USB_TXCSRL1_CLRDT);
        }

        fifo_size = udc_get_fifo_size(USB_GET_MAXPACKETSIZE(ep->wMaxPacketSize), &used);

        USB_DRV_WriteReg(&sunxi_udc->txfifosz, fifo_size & 0x0f);
        USB_DRV_WriteReg(&sunxi_udc->txfifoadd, (g_udc.fifo_size_offset >> 3));

        g_udc.fifo_size_offset += used;
    }

    udc_set_active_ep(old_ep_idx);

    return 0;
}

int usbd_ep_close(const uint8_t ep)
{
    return 0;
}

int usbd_ep_set_stall(const uint8_t ep)
{
    uint8_t ep_idx = USB_EP_GET_IDX(ep);
    uint8_t old_ep_idx;

    old_ep_idx = udc_get_active_ep();
    udc_set_active_ep(ep_idx);

    if (USB_EP_DIR_IS_OUT(ep)) {
        if (ep_idx == 0x00) {
            usb_ep0_state = USB_EP0_STATE_STALL;
            USB_DRV_SetBits8(&sunxi_udc->txcsrl, USB_CSRL0_STALL | USB_CSRL0_RXRDYC);
        } else {
            USB_DRV_SetBits8(&sunxi_udc->rxcsrl, USB_RXCSRL1_STALL);
        }
    } else {
        if (ep_idx == 0x00) {
            usb_ep0_state = USB_EP0_STATE_STALL;
            USB_DRV_SetBits8(&sunxi_udc->txcsrl, USB_CSRL0_STALL | USB_CSRL0_RXRDYC);
        } else {
            USB_DRV_SetBits8(&sunxi_udc->txcsrl, USB_TXCSRL1_STALL);
        }
    }

    udc_set_active_ep(old_ep_idx);
    return 0;
}

int usbd_ep_clear_stall(const uint8_t ep)
{
    uint8_t ep_idx = USB_EP_GET_IDX(ep);
    uint8_t old_ep_idx;

    old_ep_idx = udc_get_active_ep();
    udc_set_active_ep(ep_idx);

    if (USB_EP_DIR_IS_OUT(ep)) {
        if (ep_idx == 0x00) {
            USB_DRV_ClearBits8(&sunxi_udc->txcsrl, USB_CSRL0_STALLED);
        } else {
            // Clear the stall on an OUT endpoint.
            USB_DRV_ClearBits8(&sunxi_udc->rxcsrl, USB_RXCSRL1_STALL | USB_RXCSRL1_STALLED);
            // Reset the data toggle.
            USB_DRV_SetBits8(&sunxi_udc->rxcsrl, USB_RXCSRL1_CLRDT);
        }
    } else {
        if (ep_idx == 0x00) {
            USB_DRV_ClearBits8(&sunxi_udc->txcsrl, USB_CSRL0_STALLED);
        } else {
            // Clear the stall on an IN endpoint.
            USB_DRV_ClearBits8(&sunxi_udc->txcsrl, USB_TXCSRL1_STALL | USB_TXCSRL1_STALLED);
            // Reset the data toggle.
            USB_DRV_SetBits8(&sunxi_udc->txcsrl, USB_TXCSRL1_CLRDT);
        }
    }

    udc_set_active_ep(old_ep_idx);
    return 0;
}

int usbd_ep_is_stalled(const uint8_t ep, uint8_t *stalled)
{
    return 0;
}

int usbd_ep_start_write_force(const uint8_t ep, const uint8_t *data, uint32_t data_len)
{
    uint8_t ep_idx = USB_EP_GET_IDX(ep);
    uint8_t old_ep_idx;

    if (!data && data_len) {
        return -1;
    }
    if (!g_udc.in_ep[ep_idx].ep_enable) {
        return -2;
    }

    old_ep_idx = udc_get_active_ep();
    udc_set_active_ep(ep_idx);

    // if (USB_DRV_Reg8(&sunxi_udc->txcsrl) & USB_TXCSRL1_TXRDY) {
    //     udc_set_active_ep(old_ep_idx);
    //     return -3;
    // }

    g_udc.in_ep[ep_idx].xfer_buf = (uint8_t *)data;
    g_udc.in_ep[ep_idx].xfer_len = data_len;
    g_udc.in_ep[ep_idx].actual_xfer_len = 0;

    if (data_len == 0) {
        if (ep_idx == 0x00) {
            if (g_udc.setup.wLength == 0) {
                usb_ep0_state = USB_EP0_STATE_IN_STATUS;
            } else {
                usb_ep0_state = USB_EP0_STATE_IN_ZLP;
            }
            USB_DRV_SetBits8(&sunxi_udc->txcsrl, USB_CSRL0_TXRDY | USB_CSRL0_DATAEND);
        } else {
            USB_DRV_WriteReg8(&sunxi_udc->txcsrl, USB_TXCSRL1_TXRDY);
            USB_DRV_SetBits(&sunxi_udc->intrtxe, 1 << ep_idx);
        }
        udc_set_active_ep(old_ep_idx);
        return 0;
    }
    data_len = MIN(data_len, g_udc.in_ep[ep_idx].ep_mps);

    udc_write_packet(ep_idx, (uint8_t *)data, data_len);
    USB_DRV_SetBits(&sunxi_udc->intrtxe, 1 << ep_idx);

    if (ep_idx == 0x00) {
        usb_ep0_state = USB_EP0_STATE_IN_DATA;
        if (data_len < g_udc.in_ep[ep_idx].ep_mps) {
            USB_DRV_WriteReg8(&sunxi_udc->txcsrl, USB_CSRL0_TXRDY | USB_CSRL0_DATAEND);
        } else {
            USB_DRV_WriteReg8(&sunxi_udc->txcsrl, USB_CSRL0_TXRDY);
        }
    } else {
        USB_DRV_WriteReg8(&sunxi_udc->txcsrl, USB_TXCSRL1_TXRDY);
    }

    udc_set_active_ep(old_ep_idx);
    return 0;
}

int usbd_ep_start_write(const uint8_t ep, const uint8_t *data, uint32_t data_len)
{
    uint8_t ep_idx = USB_EP_GET_IDX(ep);
    uint8_t old_ep_idx;

    if (!data && data_len) {
        return -1;
    }
    if (!g_udc.in_ep[ep_idx].ep_enable) {
        return -2;
    }

    old_ep_idx = udc_get_active_ep();
    udc_set_active_ep(ep_idx);

    if (USB_DRV_Reg8(&sunxi_udc->txcsrl) & USB_TXCSRL1_TXRDY) {
        udc_set_active_ep(old_ep_idx);
        return -3;
    }

    g_udc.in_ep[ep_idx].xfer_buf = (uint8_t *)data;
    g_udc.in_ep[ep_idx].xfer_len = data_len;
    g_udc.in_ep[ep_idx].actual_xfer_len = 0;

    if (data_len == 0) {
        if (ep_idx == 0x00) {
            if (g_udc.setup.wLength == 0) {
                usb_ep0_state = USB_EP0_STATE_IN_STATUS;
            } else {
                usb_ep0_state = USB_EP0_STATE_IN_ZLP;
            }
            USB_DRV_SetBits8(&sunxi_udc->txcsrl, USB_CSRL0_TXRDY | USB_CSRL0_DATAEND);
        } else {
            USB_DRV_WriteReg8(&sunxi_udc->txcsrl, USB_TXCSRL1_TXRDY);
            USB_DRV_SetBits(&sunxi_udc->intrtxe, 1 << ep_idx);
        }
        udc_set_active_ep(old_ep_idx);
        return 0;
    }
    data_len = MIN(data_len, g_udc.in_ep[ep_idx].ep_mps);

    udc_write_packet(ep_idx, (uint8_t *)data, data_len);
    USB_DRV_SetBits(&sunxi_udc->intrtxe, 1 << ep_idx);

    if (ep_idx == 0x00) {
        usb_ep0_state = USB_EP0_STATE_IN_DATA;
        if (data_len < g_udc.in_ep[ep_idx].ep_mps) {
            USB_DRV_WriteReg8(&sunxi_udc->txcsrl, USB_CSRL0_TXRDY | USB_CSRL0_DATAEND);
        } else {
            USB_DRV_WriteReg8(&sunxi_udc->txcsrl, USB_CSRL0_TXRDY);
        }
    } else {
        USB_DRV_WriteReg8(&sunxi_udc->txcsrl, USB_TXCSRL1_TXRDY);
    }

    udc_set_active_ep(old_ep_idx);
    return 0;
}

int usbd_ep_start_read(const uint8_t ep, uint8_t *data, uint32_t data_len)
{
    uint8_t ep_idx = USB_EP_GET_IDX(ep);
    uint8_t old_ep_idx;

    if (!data && data_len) {
        return -1;
    }
    if (!g_udc.out_ep[ep_idx].ep_enable) {
        return -2;
    }

    old_ep_idx = udc_get_active_ep();
    udc_set_active_ep(ep_idx);

    g_udc.out_ep[ep_idx].xfer_buf = data;
    g_udc.out_ep[ep_idx].xfer_len = data_len;
    g_udc.out_ep[ep_idx].actual_xfer_len = 0;

    if (data_len == 0) {
        if (ep_idx == 0) {
            usb_ep0_state = USB_EP0_STATE_SETUP;
        }
        udc_set_active_ep(old_ep_idx);
        return 0;
    }
    if (ep_idx == 0) {
        usb_ep0_state = USB_EP0_STATE_OUT_DATA;
    } else {
        USB_DRV_SetBits(&sunxi_udc->intrrxe, 1 << ep_idx);
    }
    udc_set_active_ep(old_ep_idx);
    return 0;
}

static void handle_ep0(void)
{
    uint8_t ep0_status = USB_DRV_Reg8(&sunxi_udc->txcsrl);
    uint16_t read_count;

    if (ep0_status & USB_CSRL0_STALLED) {
        USB_DRV_ClearBits8(&sunxi_udc->txcsrl, USB_CSRL0_STALLED);
        usb_ep0_state = USB_EP0_STATE_SETUP;
        return;
    }

    if (ep0_status & USB_CSRL0_SETEND) {
        USB_DRV_WriteReg8(&sunxi_udc->txcsrl, USB_CSRL0_SETENDC);
    }

    if (g_udc.dev_addr > 0) {
        USB_DRV_WriteReg8(&sunxi_udc->faddr, g_udc.dev_addr);
        g_udc.dev_addr = 0;
    }

    switch (usb_ep0_state) {
        case USB_EP0_STATE_SETUP:
            if (ep0_status & USB_CSRL0_RXRDY) {
                read_count = USB_DRV_Reg(&sunxi_udc->rxcount);

                if (read_count != 8) {
                    return;
                }

                udc_read_packet(0, (uint8_t *)&g_udc.setup, 8);
                if (g_udc.setup.wLength) {
                    USB_DRV_WriteReg8(&sunxi_udc->txcsrl, USB_CSRL0_RXRDYC);
                } else {
                    USB_DRV_WriteReg8(&sunxi_udc->txcsrl, USB_CSRL0_RXRDYC | USB_CSRL0_DATAEND);
                }

                usbd_event_ep0_setup_complete_handler((uint8_t *)&g_udc.setup);
            }
            break;

        case USB_EP0_STATE_IN_DATA:
            if (g_udc.in_ep[0].xfer_len > g_udc.in_ep[0].ep_mps) {
                g_udc.in_ep[0].actual_xfer_len += g_udc.in_ep[0].ep_mps;
                g_udc.in_ep[0].xfer_len -= g_udc.in_ep[0].ep_mps;
            } else {
                g_udc.in_ep[0].actual_xfer_len += g_udc.in_ep[0].xfer_len;
                g_udc.in_ep[0].xfer_len = 0;
            }

            usbd_event_ep_in_complete_handler(0x80, g_udc.in_ep[0].actual_xfer_len);

            break;
        case USB_EP0_STATE_OUT_DATA:
            if (ep0_status & USB_CSRL0_RXRDY) {
                read_count = USB_DRV_Reg(&sunxi_udc->rxcount);

                udc_read_packet(0, g_udc.out_ep[0].xfer_buf, read_count);
                g_udc.out_ep[0].xfer_buf += read_count;
                g_udc.out_ep[0].actual_xfer_len += read_count;

                if (read_count < g_udc.out_ep[0].ep_mps) {
                    usbd_event_ep_out_complete_handler(0x00, g_udc.out_ep[0].actual_xfer_len);
                    USB_DRV_WriteReg8(&sunxi_udc->txcsrl, USB_CSRL0_RXRDYC | USB_CSRL0_DATAEND);
                    usb_ep0_state = USB_EP0_STATE_IN_STATUS;
                } else {
                    USB_DRV_WriteReg8(&sunxi_udc->txcsrl, USB_CSRL0_RXRDYC);
                }
            }
            break;
        case USB_EP0_STATE_IN_STATUS:
        case USB_EP0_STATE_IN_ZLP:
            usb_ep0_state = USB_EP0_STATE_SETUP;
            usbd_event_ep_in_complete_handler(0x80, 0);
            break;
    }
}

void USBD_IRQHandler(void)
{
    uint32_t is;
    uint32_t txis;
    uint32_t rxis;
    uint8_t old_ep_idx;
    uint8_t ep_idx;
    uint16_t write_count, read_count;
    is = USB_DRV_Reg32(&sunxi_udc->intrusb);
    txis = USB_DRV_Reg(&sunxi_udc->intrtx);
    rxis = USB_DRV_Reg(&sunxi_udc->intrrx);

    USB_DRV_WriteReg32(&sunxi_udc->intrusb, is);

    old_ep_idx = udc_get_active_ep();

    /* Receive a reset signal from the USB bus */
    if (is & USB_IS_RESET) {
        memset(&g_udc, 0, sizeof(struct udc));
        g_udc.fifo_size_offset = USB_CTRL_EP_MPS;
        usbd_event_reset_handler();
        USB_DRV_WriteReg(&sunxi_udc->intrtxe, USB_TXIE_EP0);
        USB_DRV_WriteReg(&sunxi_udc->intrrxe, 0);

        for (uint8_t i = 1; i < CONFIG_USBDEV_EP_NUM; i++) {
            udc_set_active_ep(i);
            USB_DRV_WriteReg(&sunxi_udc->txfifosz, 0);
            USB_DRV_WriteReg(&sunxi_udc->txfifoadd, 0);
            USB_DRV_WriteReg(&sunxi_udc->rxfifosz, 0);
            USB_DRV_WriteReg(&sunxi_udc->rxfifoadd, 0);
        }
        usb_ep0_state = USB_EP0_STATE_SETUP;
    }

    if (is & USB_IS_SOF) {
    }

    if (is & USB_IS_RESUME) {
    }

    if (is & USB_IS_SUSPEND) {
    }
    txis &= USB_DRV_Reg(&sunxi_udc->intrtxe);
    /* Handle EP0 interrupt */
    if (txis & USB_TXIE_EP0) {
        USB_DRV_WriteReg(&sunxi_udc->intrtx, USB_TXIE_EP0);
        udc_set_active_ep(0);
        handle_ep0();
        txis &= ~USB_TXIE_EP0;
    }

    ep_idx = 1;
    while (txis) {
        if (txis & (1 << ep_idx)) {
            udc_set_active_ep(ep_idx);
            USB_DRV_WriteReg(&sunxi_udc->intrtx, (1 << ep_idx));
            if (USB_DRV_Reg8(&sunxi_udc->txcsrl) & USB_TXCSRL1_UNDRN) {
                USB_DRV_ClearBits8(&sunxi_udc->txcsrl, USB_TXCSRL1_UNDRN);
            }

            if (g_udc.in_ep[ep_idx].xfer_len > g_udc.in_ep[ep_idx].ep_mps) {
                g_udc.in_ep[ep_idx].xfer_buf += g_udc.in_ep[ep_idx].ep_mps;
                g_udc.in_ep[ep_idx].actual_xfer_len += g_udc.in_ep[ep_idx].ep_mps;
                g_udc.in_ep[ep_idx].xfer_len -= g_udc.in_ep[ep_idx].ep_mps;
            } else {
                g_udc.in_ep[ep_idx].xfer_buf += g_udc.in_ep[ep_idx].xfer_len;
                g_udc.in_ep[ep_idx].actual_xfer_len += g_udc.in_ep[ep_idx].xfer_len;
                g_udc.in_ep[ep_idx].xfer_len = 0;
            }

            if (g_udc.in_ep[ep_idx].xfer_len == 0) {
                USB_DRV_ClearBits(&sunxi_udc->intrtxe, (1 << ep_idx));
                usbd_event_ep_in_complete_handler(ep_idx | 0x80, g_udc.in_ep[ep_idx].actual_xfer_len);
            } else {
                write_count = MIN(g_udc.in_ep[ep_idx].xfer_len, g_udc.in_ep[ep_idx].ep_mps);

                udc_write_packet(ep_idx, g_udc.in_ep[ep_idx].xfer_buf, write_count);
                USB_DRV_WriteReg8(&sunxi_udc->txcsrl, USB_TXCSRL1_TXRDY);
            }

            txis &= ~(1 << ep_idx);
        }
        ep_idx++;
    }

    rxis &= USB_DRV_Reg(&sunxi_udc->intrrxe);
    ep_idx = 1;
    while (rxis) {
        if (rxis & (1 << ep_idx)) {
            udc_set_active_ep(ep_idx);
            USB_DRV_WriteReg(&sunxi_udc->intrrx, (1 << ep_idx));
            if (USB_DRV_Reg8(&sunxi_udc->rxcsrl) & USB_RXCSRL1_RXRDY) {
                read_count = USB_DRV_Reg(&sunxi_udc->rxcount);

                udc_read_packet(ep_idx, g_udc.out_ep[ep_idx].xfer_buf, read_count);
                USB_DRV_ClearBits8(&sunxi_udc->rxcsrl, USB_RXCSRL1_RXRDY);

                g_udc.out_ep[ep_idx].xfer_buf += read_count;
                g_udc.out_ep[ep_idx].actual_xfer_len += read_count;
                g_udc.out_ep[ep_idx].xfer_len -= read_count;

                if ((read_count < g_udc.out_ep[ep_idx].ep_mps) || (g_udc.out_ep[ep_idx].xfer_len == 0)) {
                    USB_DRV_ClearBits(&sunxi_udc->intrrxe, (1 << ep_idx));
                    usbd_event_ep_out_complete_handler(ep_idx, g_udc.out_ep[ep_idx].actual_xfer_len);
                } else {
                }
            }

            rxis &= ~(1 << ep_idx);
        }
        ep_idx++;
    }

    udc_set_active_ep(old_ep_idx);
}
