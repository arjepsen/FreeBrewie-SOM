#ifndef FREEBREWIE_TIME_BASE_H
#define FREEBREWIE_TIME_BASE_H

/****************************************************************************************
 * @file Time_base.h
 * @brief Monotonic millisecond clock for scheduling app work.
 *
 * Responsibility: provide monotonic milliseconds.
 * Owns: time source used for app-loop, heartbeat, and UI refresh cadence.
 * Does not own: LVGL tick policy or protocol behavior.
 ****************************************************************************************/

#include <stdint.h>

uint64_t time_base_now_ms();

#endif
