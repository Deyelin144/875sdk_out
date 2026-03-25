
#ifndef __MD5_ADAPTER_H__
#define __MD5_ADAPTER_H__

#include "mbedtls/md5.h"


#define MD5_LEN 16

void *md5_adapter_init(void);
int md5_adapter_update(void *hdl, char *data, int in_len);
void md5_adapter_finish(void **hdl, unsigned char *out);
int md5_adapter_get_file_md5(char *file_path, unsigned char *out);

#endif // __MD5_ADAPTER_H__