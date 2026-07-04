#define _POSIX_C_SOURCE 200809L

#include "Time_base.h"

#include <time.h>

/****************************************************************************************
 * @brief Read the monotonic clock in milliseconds.
 *
 * The seconds part is widened before multiplying because it can grow for the whole system
 * uptime. The nanoseconds part is intentionally divided before widening: tv_nsec is always
 * below one billion, so a small integer divide is enough and the ARM target avoids pulling
 * in the slower 64-bit division helper for this hot timing helper.
 *
 * @return Monotonic milliseconds, or zero if the OS clock call fails.
 ****************************************************************************************/
uint64_t time_base_now_ms()
{
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
    {
        return 0U;
    }

    return ((uint64_t)ts.tv_sec * 1000ULL) + (uint64_t)(ts.tv_nsec / 1000000L);
}
