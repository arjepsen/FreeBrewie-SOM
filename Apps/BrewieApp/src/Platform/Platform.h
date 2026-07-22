#ifndef FREEBREWIE_PLATFORM_H
#define FREEBREWIE_PLATFORM_H

/****************************************************************************************
 * @file Platform.h
 * @brief Top-level platform resource owner.
 *
 * Responsibility: group target/simulator resources below app logic.
 * Owns: platform initialization and shutdown, currently display/input state.
 * Does not own: comms, UI screen layout, or machine logic.
 ****************************************************************************************/

#include <stdbool.h>

#include "Display.h"

typedef struct
{
    display_t display;  // LVGL display/input state and selected backend details.
} platform_t;

bool platform_init(platform_t *platform);
void platform_shutdown(platform_t *platform);

#endif
