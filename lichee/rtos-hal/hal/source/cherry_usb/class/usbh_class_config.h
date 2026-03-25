#ifndef USBH_CLASS_CONFIG_H
#define USBH_CLASS_CONFIG_H

#include "usb_cdc.h"

#ifdef CONFIG_CHERRY_USB_HOST_AUDIO
#include "usbh_audio.h"
extern const struct usbh_class_driver audio_ctrl_class_driver;
extern const struct usbh_class_driver audio_streaming_class_driver;
#endif
#ifdef CONFIG_CHERRY_USB_HOST_CDC_ACM
extern const struct usbh_class_driver cdc_acm_class_driver;
extern const struct usbh_class_driver cdc_data_class_driver;
#endif
#ifdef CONFIG_CHERRY_USB_HOST_CDC_ECM
extern const struct usbh_class_driver cdc_ecm_class_driver;
#endif
#ifdef CONFIG_CHERRY_USB_HOST_HI
extern const struct usbh_class_driver hid_class_driver;
#endif
#ifdef CONFIG_CHERRY_USB_HOST_MSC
extern const struct usbh_class_driver msc_class_driver;
#endif
#ifdef CONFIG_CHERRY_USB_HOST_VIDEO
extern const struct usbh_class_driver video_ctrl_class_driver;
extern const struct usbh_class_driver video_streaming_class_driver;
#endif
#ifdef CONFIG_CHERRY_USB_HOST_WIRELESS
extern static const struct usbh_class_driver rndis_class_driver;
#endif
#ifdef CONFIG_CHERRY_USB_HOST_HUB
#if CONFIG_USBHOST_MAX_EXTHUBS > 0
extern const struct usbh_class_driver hub_class_driver;
#endif
#endif
const struct usbh_class_info usbh_class_type_info[] = {
#ifdef CONFIG_CHERRY_USB_HOST_AUDIO
    {
        .match_flags = USB_CLASS_MATCH_INTF_CLASS | USB_CLASS_MATCH_INTF_SUBCLASS,
        .class = USB_DEVICE_CLASS_AUDIO,
        .subclass = AUDIO_SUBCLASS_AUDIOCONTROL,
        .protocol = 0x00,
        .vid = 0x00,
        .pid = 0x00,
        .class_driver = &audio_ctrl_class_driver
    },

    {
        .match_flags = USB_CLASS_MATCH_INTF_CLASS | USB_CLASS_MATCH_INTF_SUBCLASS,
        .class = USB_DEVICE_CLASS_AUDIO,
        .subclass = AUDIO_SUBCLASS_AUDIOSTREAMING,
        .protocol = 0x00,
        .vid = 0x00,
        .pid = 0x00,
        .class_driver = &audio_streaming_class_driver
    },
#endif

#ifdef CONFIG_CHERRY_USB_HOST_CDC_ACM
    /* USB Host ACM CLAS INFO */
    {
        .match_flags = USB_CLASS_MATCH_INTF_CLASS | USB_CLASS_MATCH_INTF_SUBCLASS | USB_CLASS_MATCH_INTF_PROTOCOL,
        .class = USB_DEVICE_CLASS_CDC,
        .subclass = CDC_ABSTRACT_CONTROL_MODEL,
        .protocol = CDC_COMMON_PROTOCOL_AT_COMMANDS,
        .vid = 0x00,
        .pid = 0x00,
        .class_driver = &cdc_acm_class_driver
    },
    /* EC800M-CN interface 02 cdc_data */
    {
        .match_flags = USB_CLASS_MATCH_INTF_CLASS,
        .class = USB_DEVICE_CLASS_CDC_DATA,
        .subclass = 0x00,
        .protocol = 0x00,
        .vid = 0x00,
        .pid = 0x00,
        .class_driver = &cdc_data_class_driver
    },
    /* EC800M-CN interface 03/04/05 DIAG/AT/MODEM */
    {
        .match_flags = USB_CLASS_MATCH_INTF_CLASS | USB_CLASS_MATCH_INTF_SUBCLASS | USB_CLASS_MATCH_INTF_PROTOCOL,
        .class = 0xff,
        .subclass = 0x00,
        .protocol = 0x00,
        .vid = 0x00,
        .pid = 0x00,
        .class_driver = &cdc_acm_class_driver
    },
#endif

#ifdef CONFIG_CHERRY_USB_HOST_CDC_ECM
    {
        .match_flags = USB_CLASS_MATCH_INTF_CLASS | USB_CLASS_MATCH_INTF_SUBCLASS | USB_CLASS_MATCH_INTF_PROTOCOL,
        .class = USB_DEVICE_CLASS_CDC,
        .subclass = CDC_ETHERNET_NETWORKING_CONTROL_MODEL,
        .protocol = CDC_COMMON_PROTOCOL_NONE,
        .vid = 0x00,
        .pid = 0x00,
        .class_driver = &cdc_ecm_class_driver
    },
    /* class for EC800M-CN */
    {
        .match_flags = USB_CLASS_MATCH_INTF_CLASS | USB_CLASS_MATCH_INTF_SUBCLASS | USB_CLASS_MATCH_INTF_PROTOCOL,
        .class = USB_DEVICE_CLASS_CDC,
        .subclass = 0x06,
        .protocol = 0x00,
        .vid = 0x2C7C,
        .pid = 0x6002,
        .class_driver = &cdc_ecm_class_driver
    },
    /* class for EC20-CN */
    {
        .match_flags = USB_CLASS_MATCH_INTF_CLASS | USB_CLASS_MATCH_INTF_SUBCLASS | USB_CLASS_MATCH_INTF_PROTOCOL,
        .class = USB_DEVICE_CLASS_VEND_SPECIFIC,
        .subclass = 0xff,
        .protocol = 0xff,
        .vid = 0x2C7C,
        .pid = 0x0125,
        .class_driver = &cdc_ecm_class_driver
    },
#endif

#ifdef CONFIG_CHERRY_USB_HOST_HI
    {
        .match_flags = USB_CLASS_MATCH_INTF_CLASS,
        .class = USB_DEVICE_CLASS_HID,
        .subclass = 0x00,
        .protocol = 0x00,
        .vid = 0x00,
        .pid = 0x00,
        .class_driver = &hid_class_driver
    },
#endif

#ifdef CONFIG_CHERRY_USB_HOST_MSC
    {
        .match_flags = USB_CLASS_MATCH_INTF_CLASS | USB_CLASS_MATCH_INTF_SUBCLASS | USB_CLASS_MATCH_INTF_PROTOCOL,
        .class = USB_DEVICE_CLASS_MASS_STORAGE,
        .subclass = MSC_SUBCLASS_SCSI,
        .protocol = MSC_PROTOCOL_BULK_ONLY,
        .vid = 0x00,
        .pid = 0x00,
        .class_driver = &msc_class_driver
    },
#endif

#ifdef CONFIG_CHERRY_USB_HOST_VIDEO
    {
        .match_flags = USB_CLASS_MATCH_INTF_CLASS | USB_CLASS_MATCH_INTF_SUBCLASS | USB_CLASS_MATCH_INTF_PROTOCOL,
        .class = USB_DEVICE_CLASS_VIDEO,
        .subclass = VIDEO_SC_VIDEOCONTROL,
        .protocol = VIDEO_PC_PROTOCOL_UNDEFINED,
        .vid = 0x00,
        .pid = 0x00,
        .class_driver = &video_ctrl_class_driver
    },

    {
        .match_flags = USB_CLASS_MATCH_INTF_CLASS | USB_CLASS_MATCH_INTF_SUBCLASS | USB_CLASS_MATCH_INTF_PROTOCOL,
        .class = USB_DEVICE_CLASS_VIDEO,
        .subclass = VIDEO_SC_VIDEOSTREAMING,
        .protocol = VIDEO_PC_PROTOCOL_UNDEFINED,
        .vid = 0x00,
        .pid = 0x00,
        .class_driver = &video_streaming_class_driver
    },
#endif

#ifdef CONFIG_CHERRY_USB_HOST_WIRELESS
    {
        .match_flags = USB_CLASS_MATCH_INTF_CLASS | USB_CLASS_MATCH_INTF_SUBCLASS | USB_CLASS_MATCH_INTF_PROTOCOL,
        .class = USB_DEVICE_CLASS_WIRELESS,
        .subclass = 0x01,
        .protocol = 0x03,
        .vid = 0x00,
        .pid = 0x00,
        .class_driver = &rndis_class_driver
    },
#endif

#ifdef CONFIG_CHERRY_USB_HOST_HUB
#if CONFIG_USBHOST_MAX_EXTHUBS > 0
    {
        .match_flags = USB_CLASS_MATCH_INTF_CLASS,
        .class = USB_DEVICE_CLASS_HUB,
        .subclass = 0,
        .protocol = 0,
        .vid = 0x00,
        .pid = 0x00,
        .class_driver = &hub_class_driver
    },
#endif
#endif
};


#endif