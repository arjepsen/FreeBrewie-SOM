#ifndef FREEBREWIE_DISPLAY_H
#define FREEBREWIE_DISPLAY_H

/****************************************************************************************
 * @file Display.h
 * @brief LVGL display and input backend wrapper.
 *
 * Responsibility: provide LVGL with display/input service.
 * Owns: display backend initialization, LVGL tick updates, and target/simulator display
 * glue.
 * Does not own: screen layout or app workflow decisions.
 ****************************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include "lvgl.h"

typedef struct
{
    /** True after LVGL and the selected display backend have initialized successfully. */
    bool ready;
    /** True when running in the desktop SDL simulator rather than on the SOM display. */
    bool simulator;
    /** Last monotonic time passed to LVGL, used to advance LVGL by real elapsed time. */
    uint64_t last_tick_ms;
} display_t;

bool display_init(display_t *display);
void display_update(display_t *display, uint64_t now_ms);

#endif
