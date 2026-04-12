#define _POSIX_C_SOURCE 200809L

#include "Time.h"

#include <time.h>

uint64_t time_now_ms()
{
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
    {
        return 0U;
    }

    return ((uint64_t)ts.tv_sec * 1000ULL) + ((uint64_t)ts.tv_nsec / 1000000ULL);
}