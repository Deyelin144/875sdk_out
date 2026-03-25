ifeq ($(CONFIG_ARCH_ARM_CORTEX_A7),y)
MULTIMEDIA_LIBS_DIR := components/aw/multimedia/lib/lib_a7
SPEEX_LIBS_DIR := components/thirdparty/speex/libs/lib_a7
else ifeq ($(CONFIG_ARCH_ARM_CORTEX_M33),y)
MULTIMEDIA_LIBS_DIR := components/aw/multimedia/lib/lib_m33
SPEEX_LIBS_DIR := components/thirdparty/speex/libs/lib_m33
else ifeq ($(CONFIG_ARCH_RISCV),y)
MULTIMEDIA_LIBS_DIR := components/aw/multimedia/lib/lib_riscv
SPEEX_LIBS_DIR := components/thirdparty/speex/libs/lib_riscv
else ifeq ($(CONFIG_ARCH_DSP),y)
MULTIMEDIA_LIBS_DIR := components/aw/multimedia/lib_dsp
SPEEX_LIBS_DIR := components/thirdparty/speex/libs/lib_dsp
endif
LDFLAGS-$(CONFIG_RTOPUS_TEST) += -L$(MULTIMEDIA_LIBS_DIR) -law_opuscodec
ifeq ($(CONFIG_LIB_RECORDER),y)
  LDFLAGS += -L$(MULTIMEDIA_LIBS_DIR)
  ifeq ($(CONFIG_LIB_MULTIMEDIA_CROP),y)
    LDFLAGS += -lxrecorder_x
  else
    LDFLAGS += -lxrecorder -lrecord
  endif

  LDFLAGS += -lstream -lawrecorder -lmuxer -law_amrenc -larecoder -lcdx_base

  ifeq ($(CONFIG_ARCH_SUN20IW2),y)
    LDFLAGS += -law_mp3enc -law_aacenc
  endif
endif
#-law_oggdec
#LDFLAGS-$(CONFIG_RTPLAYER_TEST) += -L$(MULTIMEDIA_LIBS_DIR) -lrtplayer -lxplayer -lcdx_base -lplayback -lparser \
#			-lstream -ladecoder -law_aacdec -law_mp3dec -law_wavdec -law_opuscodec
ifeq ($(CONFIG_LIB_MULTIMEDIA),y)
  LDFLAGS += -L$(MULTIMEDIA_LIBS_DIR)
  ifeq ($(CONFIG_LIB_MULTIMEDIA_CROP),y)
    LDFLAGS += -lxplayer_x -lplayback_x
  else
    LDFLAGS += -lrtplayer -lxplayer -lplayback
  endif

  ifeq ($(CONFIG_MP4_TEST),y)
    LDFLAGS += -lparser_v
  else
    LDFLAGS += -lparser
  endif

  LDFLAGS += -lstream -ladecoder -law_aacdec -law_mp3dec -law_opuscodec  -law_wavdec -ladecoder -law_flacdec -law_amrenc -law_amrdec -law_oggdec -lcdx_base
endif
#LDFLAGS-$(CONFIG_RTPLAYER_TEST) += -L$(MULTIMEDIA_LIBS_DIR) -lrtosplayer -law_aacdec -law_mp3dec -law_opuscodec  -law_wavdec -lrtosplayer

LDFLAGS-$(CONFIG_COMPONENT_SPEEX) += -L$(SPEEX_LIBS_DIR) -law_speex

GUROBOT_LIBS_DIR := projects/xr875s1h10/solution_c906/src/lib

ifeq ($(CONFIG_COMPONENTS_USB_GADGET_ADBD),y)
       LDFLAGS += -Lcomponents/common/aw/usb/gadget/adbd/ -law-adbd
endif

ifeq ($(CONFIG_DRIVERS_XRADIO),y)
  ifeq ($(CONFIG_DRIVER_R128), y)
	LDFLAGS += -Ldrivers/drv/wireless/xradio/lib/sun20iw2
  else
	LDFLAGS += -Ldrivers/drv/wireless/xradio/lib
  endif

  ifeq ($(CONFIG_DRIVER_R128), y)
    ifeq ($(CONFIG_WLAN_STA)_$(CONFIG_WLAN_AP), y_y)
      ifeq ($(CONFIG_WLAN_STA_WPS), y)
        LIB_WPA += -lxrwifi_wpas_wps_hostapd
      else
        LIB_WPA += -lxrwifi_wpas_hostapd
      endif
    else
      ifeq ($(CONFIG_WLAN_STA), y)
        ifeq ($(CONFIG_WLAN_STA_WPS), y)
          LIB_WPA += -lxrwifi_wpas_wps
        else
          LIB_WPA += -lxrwifi_wpas
        endif
      endif
      ifeq ($(CONFIG_WLAN_AP), y)
        LIB_WPA += -lxrwifi_hostapd
      endif
    endif
  endif

  ifeq ($(CONFIG_ETF), y)
    ifeq ($(CONFIG_DRIVER_R128), y)
      LDFLAGS += -lxrwifi_etf
    else
      LDFLAGS += -lxretf
    endif
  else
    ifeq ($(CONFIG_DRIVER_XR819), y)
      LDFLAGS += -lxr819
    else ifeq ($(CONFIG_DRIVER_XR829)_$(CONFIG_XR829_40M_FW), y_y)
      LDFLAGS += -lxr829_40M
    else ifeq ($(CONFIG_DRIVER_R128), y)
$(info $(CONFIG_WLAN_STA)_$(CONFIG_WLAN_AP)_$(CONFIG_WLAN_MONITOR) )
      ifeq ($(CONFIG_WLAN_STA)_$(CONFIG_WLAN_AP)_$(CONFIG_WLAN_MONITOR), y_y_y)
        LDFLAGS += -Ldrivers/drv/wireless/xradio/lib/sun20iw2/01_sta_ap_monitor
      else ifeq ($(CONFIG_WLAN_STA)_$(CONFIG_WLAN_AP)_$(CONFIG_WLAN_MONITOR), y_y_)
        LDFLAGS += -Ldrivers/drv/wireless/xradio/lib/sun20iw2/02_sta_ap
      else ifeq ($(CONFIG_WLAN_STA)_$(CONFIG_WLAN_AP)_$(CONFIG_WLAN_MONITOR), y__y)
        LDFLAGS += -Ldrivers/drv/wireless/xradio/lib/sun20iw2/03_sta_monitor
      else ifeq ($(CONFIG_WLAN_STA)_$(CONFIG_WLAN_AP)_$(CONFIG_WLAN_MONITOR), y__)
        LDFLAGS += -Ldrivers/drv/wireless/xradio/lib/sun20iw2/04_sta
      else ifeq ($(CONFIG_WLAN_STA)_$(CONFIG_WLAN_AP)_$(CONFIG_WLAN_MONITOR), _y_)
        LDFLAGS += -Ldrivers/drv/wireless/xradio/lib/sun20iw2/05_ap
      else ifeq ($(CONFIG_WLAN_STA)_$(CONFIG_WLAN_AP)_$(CONFIG_WLAN_MONITOR), __y)
        LDFLAGS += -Ldrivers/drv/wireless/xradio/lib/sun20iw2/06_monitor
      endif
      ifeq ($(CONFIG_ARCH_ARM_ARMV8M), y)
        ifeq ($(CONFIG_COMPONENTS_AMP), y)
          LDFLAGS += -lxrwifi_wlan_m33
        else
          LDFLAGS += -lxrwifi_wlan
        endif
        LDFLAGS += -lxrwifi_mac $(LIB_WPA)
        LDFLAGS += -lxrwifi_wireless_phy
      endif
      ifeq ($(CONFIG_COMPONENTS_AMP)_$(CONFIG_ARCH_RISCV_RV64), y_y)
        LDFLAGS += -lxrwifi_wlan_rv
      endif
    endif
  endif
endif
