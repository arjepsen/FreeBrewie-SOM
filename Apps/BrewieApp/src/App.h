#ifndef FREEBREWIE_APP_H
#define FREEBREWIE_APP_H

/****************************************************************************************
 * @file App.h
 * @brief Top-level BrewieApp coordinator.
 *
 * Responsibility: coordinate the main app lifecycle and update order.
 * Owns: app context, subsystem initialization order, and periodic app updates.
 * Does not own: protocol parsing, widget construction, or display driver details.
 ****************************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include "Comms/Comms.h"
#include "Logic/App_orchestrator.h"
#include "Platform/Platform.h"
#include "UI/UI.h"

typedef struct
{
    /** Linux/display/input resources owned by the process. */
    platform_t platform;
    /** Serial protocol state for the MCU link. */
    comms_t comms;
    /** Logic-side app state and future workflow routing. */
    app_orchestrator_t orchestrator;
    /** LVGL screen objects and navigation state. */
    ui_t ui;
    /** Last time the human-readable UI labels were refreshed. */
    uint64_t last_ui_update_ms;
    /** False when display setup failed, allowing comms-only bring-up. */
    bool display_enabled;
} app_t;

bool app_init(app_t *app);
void app_update(app_t *app);
void app_shutdown(app_t *app);

#endif
