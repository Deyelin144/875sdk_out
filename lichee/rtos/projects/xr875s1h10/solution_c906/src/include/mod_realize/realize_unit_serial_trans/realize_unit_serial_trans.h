
#ifndef __REALIZE_UNIT_SERIAL_TRANS_H__
#define __REALIZE_UNIT_SERIAL_TRANS_H__

#include "../../platform/gulitesf_config.h"

#ifdef CONFIG_USE_SERIAL_TRANS

#ifdef CONFIG_SERIAL_TRANS_USE_INTERNAL
#define XYZMODEM_PACKET_SIZE (4 * 1024)
#endif

int realize_unit_serial_trans_recv(char *dir_name);
int realize_unit_serial_trans_send(char *path);
void realize_unit_serial_trans_abort(unsigned char abort);
#endif

#endif