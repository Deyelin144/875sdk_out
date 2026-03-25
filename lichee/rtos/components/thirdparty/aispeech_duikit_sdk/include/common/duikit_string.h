#ifndef DUIKIT_STRING_H
#define DUIKIT_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "duikit_common.h"

#define duikit_strdup(ptr) ({ \
    const char *_s = (ptr); \
    char *_d = NULL; \
    if (_s) { \
        _d = duikit_malloc(strlen(_s) + 1); \
        if (_d) { \
            strcpy(_d, _s); \
        } \
    } \
    _d; \
})

#ifdef __cplusplus
}
#endif

#endif
