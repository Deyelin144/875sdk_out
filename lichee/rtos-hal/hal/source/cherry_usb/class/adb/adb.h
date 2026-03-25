#ifndef __ADB_H
#define __ADB_H

#include "usbd_adb.h"
enum {
	ADB_SERVICE_TYPE_UNKNOWN = 0,
	ADB_SERVICE_TYPE_SHELL,
	ADB_SERVICE_TYPE_SYNC,
	ADB_SERVICE_TYPE_SOCKET,
};
adb_packet *get_apacket(void);
void put_apacket(adb_packet *p);

#endif