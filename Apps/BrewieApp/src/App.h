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
#include "Logic/Brewing_process_view_model.h"
#include "Logic/Status_view_model.h"
#include "Platform/Platform.h"
#include "UI/UI.h"

typedef struct
{
    platform_t platform;  // Linux/display/input resources owned by the process.
    comms_t comms;  // Serial protocol state for the MCU link.
    status_view_model_t status_view_model;  // Diagnostic/status presentation model.
    brewing_process_view_model_t brewing_process_view_model;  // Active Brewing scaffold model.
    ui_t ui;  // LVGL screen objects and navigation state.
    uint64_t last_ui_update_ms;  // Last refresh of human-readable UI labels.
    bool display_enabled;  // False allows comms-only bring-up after display init failure.
} app_t;

bool app_init(app_t *app);
void app_update(app_t *app);
void app_shutdown(app_t *app);

#endif
