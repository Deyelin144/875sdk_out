#ifndef __MAC_H
#define __MAC_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MAC   "04:21:54:fc:25:dc"
#define UUID  "f8f1436b-8490-4e56-bc3b-3f4738ffc5f4"

int get_dev_mac(char *mac, int mac_len);
char *get_uuid(void);

#endif /* __MAC_H */
