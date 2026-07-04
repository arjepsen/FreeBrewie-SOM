#ifndef FREEBREWIE_APP_TYPES_H
#define FREEBREWIE_APP_TYPES_H

/****************************************************************************************
 * @file App_types.h
 * @brief Small app-facing presentation types shared between logic and UI.
 *
 * Responsibility: define lightweight data crossing the Logic/UI boundary.
 * Owns: small presentation structs used by current screens.
 * Does not own: full machine state or workflow models.
 ****************************************************************************************/

#include <stdint.h>

typedef struct
{
    /*
     * This is the current diagnostic status-screen view model. It intentionally stores
     * readable strings because the screen is a bring-up/debug view.
     *
     * Production brewing screens should move toward raw values and screen-specific dirty
     * updates, so fixed labels and unchanged widgets are not redrawn just because one number
     * changes.
     */
    const char *display_text;
    const char *serial_text;
    const char *heartbeat_text;
    const char *last_rx_text;
    const char *link_text;
    const char *mcu_status_text;
    const char *pressure_text;
    const char *pump_text;
    const char *solenoid_text;
    const char *fault_text;
    uint32_t heartbeat_count;
} status_screen_view_model_t;

#endif
