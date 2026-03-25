#ifndef __WEBRTC_STDLIB_H__
#define __WEBRTC_STDLIB_H__

#include <stddef.h>

void *webrtc_malloc(size_t size);
void *webrtc_realloc(void *ptr, size_t size);
void *webrtc_calloc(size_t cnt, size_t size);
void webrtc_free(void *ptr);
void webrtc_qsort(void *base, size_t nitems, size_t size, int (*compar)(const void *, const void *));

#define malloc(s)          webrtc_malloc(s)
#define calloc(n, s)       webrtc_calloc(n, s)
#define free(p)            webrtc_free(p)
#define realloc(p, s)      webrtc_realloc(p, s)
#define qsort(b, n, s, cr) webrtc_qsort(b, n, s, cr)
#define abs(x)             ((x) >= 0 ? (x) : -(x))
#endif
