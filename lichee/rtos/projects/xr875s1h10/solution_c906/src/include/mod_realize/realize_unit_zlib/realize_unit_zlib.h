#ifndef __REALIZE_UNIT_ZLIB_H__
#define __REALIZE_UNIT_ZLIB_H__

#ifdef CONFIG_ZLIB_SUPPORT

int realize_unit_zlib_gunzip(const char *src, int src_len, char **dest, int *dest_len);

#endif

#endif