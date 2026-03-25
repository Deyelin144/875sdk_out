#ifndef _WRAPPER_H_
#define _WRAPPER_H_
#include "../gulitesf_config.h"

#include "../../mod_realize/realize_unit_log/realize_unit_log.h"

#include "amr_wrapper/amr_wrapper.h"
#include "battery_wrapper/battery_wrapper.h"
#include "scan_wrapper/scan_wrapper.h"
#include "device_wrapper/device_wrapper.h"
#include "encrypt_wrapper/encrypt_wrapper.h"
#include "fs_wrapper/fs_wrapper.h"
#include "heap_wrapper/heap_wrapper.h"
#include "input_wrapper/input_wrapper.h"
#include "mqtt_wrapper/mqtt_wrapper.h"
#include "os_wrapper/os_wrapper.h"
#include "player_wrapper/player_wrapper.h"
#include "record_wrapper/record_wrapper.h"
#include "speex_wrapper/speex_wrapper.h"
#include "websocket_wrapper/websocket_wrapper.h"
#include "wifi_wrapper/wifi_wrapper.h"
#include "database_wrapper/database_wrapper.h"
#include "diskinfo_wrapper/diskinfo_wrapper.h"
#include "video_player_wrapper/video_player_wrapper.h"
#include "transparent_wrapper/transparent_wrapper.h"
#include "flash_wrapper/flash_wrapper.h"
#ifdef CONFIG_CAMERA_SUPPORT
#include "camera_wrapper/camera_wrapper.h"
#endif
#include "pm_wrapper/pm_wrapper.h"
#ifdef CONFIG_USE_SERIAL_TRANS
#include "serial_trans_wrapper/serial_trans_wrapper.h"
#endif
#include "exception_wrapper/exception_wrapper.h"

typedef struct {
	os_wrapper_t os;
	heap_wrapper_t heap;
#if defined(CONFIG_AMR_SUPPORT) && !defined(CONFIG_AMR_USER_INTERNAL)
	amr_wrapper_t amr;
#endif
	battery_wrapper_t battery;
#ifdef CONFIG_FUNC_SCAN_READ
	scan_wrapper_t scan;
#endif
#if !defined(CONFIG_MBEDTLS_USER_INTERNAL)
	encrypt_wrapper_t encrypt;
#endif
	fs_wrapper_t fs;
	input_wrapper_t input;
	player_wrapper_t player;
	record_wrapper_t record;
#if defined(CONFIG_SPEEX_SUPPORT) && !defined(CONFIG_SPEEX_USER_INTERNAL)
	speex_wrapper_t speex;
#endif
#if defined(CONFIG_MQTT_SUPPORT) && !defined(CONFIG_MQTT_USER_INTERNAL)
	mqtt_wrapper_t mqtt;
#endif
#if defined(CONFIG_WEBSOCKET_SUPPORT) && !defined(CONFIG_WEBSOCKET_USER_INTERNAL)
	websocket_wrapper_t websocket;
#endif
	wifi_wrapper_t wifi;
	device_wrapper_t device;
	diskinfo_wrapper_t diskinfo;
	video_player_wrapper_t video_player;
	transparent_wrapper_t transparent;
	flash_wrapper_t flash;
#ifdef CONFIG_PM_SUPPORT
	pm_wrapper_t pm;
#endif
#if defined(CONFIG_DATABASE_SUPPORT)
#if !defined(CONFIG_SQLITE_USER_INTERNAL)
	db_wrapper_t database;
#endif
#endif

#ifdef CONFIG_CAMERA_SUPPORT
	camera_wrapper_t camera;
#endif
#if defined(CONFIG_USE_SERIAL_TRANS)
    serial_trans_wrapper_t serial_trans;
#endif
    exception_wrapper_t exception;
} wrapper_t;

int wrapper_init(wrapper_t *wrapper_cb);
wrapper_t *wrapper_get_handle(void);

#endif

