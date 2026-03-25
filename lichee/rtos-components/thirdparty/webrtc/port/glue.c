#include <stdlib.h>
void *webrtc_malloc(size_t size)
{
	return malloc(size);
}

void *webrtc_realloc(void *ptr, size_t size)
{
	return realloc(ptr, size);
}

void *webrtc_calloc(size_t cnt, size_t size)
{
	return calloc(cnt, size);
}

void webrtc_free(void *ptr)
{
	free(ptr);
}

extern void qsort(void *base, size_t nitems, size_t size, int (*compar)(const void *, const void*));
void webrtc_qsort(void *base, size_t nitems, size_t size, int (*compar)(const void *, const void*))
{
	qsort(base,nitems,size,compar);
}

extern int abs(int x);
int webrtc_abs(int x)
{
	return abs(x);
}
