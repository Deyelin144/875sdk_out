#include <time.h>
#include "sys_time.h"

uint32_t sys_time_now(void)
{
    struct timespec ts;
    #ifdef CLOCK_MONOTONIC
    clock_gettime(CLOCK_MONOTONIC, &ts);
    #else
    clock_gettime(CLOCK_REALTIME, &ts);
    #endif
    return (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
}
