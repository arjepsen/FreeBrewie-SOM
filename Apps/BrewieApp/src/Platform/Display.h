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
    bool ready;  // LVGL and selected backend initialized successfully.
    bool simulator;  // Running in the desktop SDL simulator.
    uint64_t last_tick_ms;  // Last monotonic time passed to LVGL.
} display_t;

bool display_init(display_t *display);
void display_update(display_t *display, uint64_t now_ms);

#endif
