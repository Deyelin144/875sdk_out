#CherryUSB is a tiny, beautiful and portable USB host and device stack for embedded system with USB IP.

obj-$(CONFIG_CHERRY_USB_HOST_CLASS) += core/usbh_core.o
obj-$(CONFIG_CHERRY_USB_DEVICE_CLASS) += core/usbd_core.o

ifeq ($(CONFIG_CHERRY_USB_HOST_CLASS), y)

ifeq ($(CONFIG_CHERRY_USB_HOST_MSC), y)
	obj-y += class/msc/usbh_msc.o
endif

ifeq ($(CONFIG_CHERRY_USB_HOST_CDC_ECM), y)
	obj-y += class/cdc/usbh_cdc_ecm.o
endif

ifeq ($(CONFIG_CHERRY_USB_HOST_CDC_ACM), y)
	obj-y += class/cdc/usbh_cdc_acm.o
endif

ifeq ($(CONFIG_CHERRY_USB_HOST_AUDIO), y)
	obj-y += class/audio/usbh_audio.o
endif

ifeq ($(CONFIG_CHERRY_USB_HOST_DFU), y)
	obj-y += class/hid/usbh_dfu.o
endif

ifeq ($(CONFIG_CHERRY_USB_HOST_HID), y)
	obj-y +=  class/hid/usbh_hid.o
endif

ifeq ($(CONFIG_CHERRY_USB_HOST_HUB), y)
	obj-y +=  class/hub/usbh_hub.o
endif

ifeq ($(CONFIG_CHERRY_USB_HOST_VIDEO), y)
	obj-y += class/video/usbh_video.o
endif

ifeq ($(CONFIG_CHERRY_USB_HOST_WIRELESS), y)
	obj-y += class/wireless/usbh_rndis.o
endif

endif

ifeq ($(CONFIG_CHERRY_USB_DEVICE_CLASS), y)
	obj-y += port/sunxi/usb_dc_sunxi.o
ifeq ($(CONFIG_CHERRY_USB_DEVICE_MSC), y)
	obj-y += class/msc/usbd_msc.o
endif

ifeq ($(CONFIG_CHERRY_USB_DEVICE_CDC_ECM), y)
	obj-y += class/cdc/usbd_cdc_ecm.o
endif

ifeq ($(CONFIG_CHERRY_USB_DEVICE_ADB), y)
	obj-y += class/adb/usbd_adb.o
	obj-y += class/adb/adb_shell.o
	obj-y += class/adb/adb_sync.o
	obj-y += class/adb/adb_log.o
	# obj-y += class/adb/adb_rb.o
	# obj-y += class/adb/adb.o
	obj-y += class/adb/service.o
ifeq ($(CONFIG_CHERRY_ADB_SOCKET), y)
	obj-y += class/adb/adb_socket.o
endif
endif

ifeq ($(CONFIG_CHERRY_USB_DEVICE_CDC_ACM), y)
	obj-y += class/cdc/usbd_cdc.o
endif

ifeq ($(CONFIG_CHERRY_USB_DEVICE_AUDIO), y)
	obj-y += class/audio/usbd_audio.o
endif

ifeq ($(CONFIG_CHERRY_USB_DEVICE_DFU), y)
	obj-y += class/hid/usbd_dfu.o
endif

ifeq ($(CONFIG_CHERRY_USB_DEVICE_HID), y)
	obj-y +=  class/hid/usbd_hid.o
endif

ifeq ($(CONFIG_CHERRY_USB_DEVICE_VIDEO), y)
	obj-y += class/video/usbd_video.o
endif

ifeq ($(CONFIG_CHERRY_USB_DEVICE_WIRELESS), y)
	obj-y += class/wireless/usbd_rndis.o
endif

endif

#obj-y += third_party/FreeRTOS-10.4/croutine.o \
	third_party/FreeRTOS-10.4/event_groups.o \
	third_party/FreeRTOS-10.4/list.o \
	third_party/FreeRTOS-10.4/queue.o \
	third_party/FreeRTOS-10.4/stream_buffer.o \
	third_party/FreeRTOS-10.4/tasks.o \
	third_party/FreeRTOS-10.4/timers.o

obj-y += osal/usb_osal_freertos.o
obj-y += port/sunxi/usb_hc_ehci.o
obj-y += port/sunxi/usb_glue_sunxi.o

ifeq ($(CONFIG_DRIVERS_CHERRY_USB_PORT), y)
obj-y += port/ch32/usb_ch58x_dc_usbfs.o \
	port/ch32/usb_dc_usbfs.o \
	port/ch32/usb_dc_usbhs.o \
	port/ch32/usb_hc_usbfs.o \
	port/ch32/usb_hc_usbhs.o \
	port/dwc2/usb_dc_dwc2.o \
	port/dwc2/usb_glue_at.o \
	port/dwc2/usb_glue_gd.o \
	port/dwc2/usb_glue_st.o \
	port/dwc2/usb_hc_dwc2.o \
	port/ehci/usb_glue_bouffalo.o \
	port/ehci/usb_glue_hpm.o \
	port/ehci/usb_hc_ehci.o \
	port/fsdev/usb_dc_fsdev.o \
	port/hpm/usb_dc_hpm.o \
	port/musb/usb_dc_musb.o \
	port/musb/usb_hc_musb.o \
	port/nrf5x/usb_dc_nrf5x.o \
	port/nuvoton/usb_dc_usbfs.o \
	port/pusb2/usb_dc_pusb2.o \
	port/pusb2/usb_hc_pusb2.o \
	port/rp2040/usb_dc_rp2040.o \
	port/template/usb_dc.o \
	port/template/usb_hc.o \
	port/xhci/usb_hc_xhci.o \
	port/xhci/xhci.o \
	port/xhci/xhci_dbg.o
endif
ifeq ($(CONFIG_CHERRY_USB_DEMO), y)
obj-y += demo/usb_host.o

#obj-y += demo/audio_v1_mic_multichan_template.o \
	demo/audio_v1_mic_speaker_multichan_template.o \
	demo/audio_v2_mic_multichan_template.o \
	demo/audio_v2_mic_speaker_multichan_template.o \
	demo/audio_v2_speaker_multichan_template.o \
	demo/cdc_acm_hid_msc_template.o \
	demo/cdc_acm_msc_template.o \
	demo/cdc_acm_multi_template.o \
	demo/cdc_acm_template.o \
	demo/cdc_ecm_template.o \
	demo/cdc_rndis_template.o \
	demo/dfu_with_st_tool_template.o \
	demo/hid_custom_inout_template.o \
	demo/hid_keyboard_template.o \
	demo/hid_mouse_template.o \
	demo/midi_template.o \
	demo/msc_ram_template.o \
	demo/msc_storage_template.o \
	demo/video_static_mjpeg_template.o \
	demo/webusb_template.o \
	demo/winusb1.0_template.o \
	demo/dfu_st_cubemx_main.o \
	demo/printer_template.o \
	demo/mtp_template.o
endif

ifeq ($(CONFIG_CHERRY_USB_ADB_DEMO), y)
obj-y += demo/adb/usbd_adb_template.o
endif

ifeq ($(CONFIG_CHERRY_USB_MSC_DEMO), y)
obj-y += demo/msc_sdcard_template.o
endif

ifeq ($(CONFIG_CHERRY_USB_UAC_DEMO), y)
obj-y += demo/audio_v1_mic_speaker_multichan_template.o
endif

ifeq ($(CONFIG_CHERRY_USB_HID_DEMO), y)
obj-y += demo/hid_mouse_template.o
endif

CFLAGS += -I drivers/rtos-hal/hal/source/gpio/
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/ec800m
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/class
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/class/adb
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/class/audio
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/class/cdc
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/class/dfu
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/class/hid
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/class/hub
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/class/midi
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/class/msc
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/class/template
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/class/video
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/class/wireless
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/common/
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/core/
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/osal/
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/port/bouffalolab
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/port/ch32
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/port/dwc2
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/port/ehci
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/port/fsdev
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/port/hpm
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/port/musb
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/port/nrf5x
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/port/nuvoton
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/port/pusb2
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/port/rp2040
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/port/template
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/port/xhci
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/third_party
CFLAGS += -I drivers/rtos-hal/hal/source/cherry_usb/demo
CFLAGS += -I kernel/FreeRTOS/Source

CFLAGS += -I components/common/thirdparty/network/lwip-2.1.2/src/include
CFLAGS += -I components/common/aw/ringbuffer/include
CFLAGS += -I components/common/aw/xradio/include/
CFLAGS += -I components/thirdparty/elmfat/src/
CFLAGS += -I components/common/aw/AudioSystem/include

CFLAGS += -Werror
