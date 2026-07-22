#ifndef FREEBREWIE_SCREEN_STATUS_H
#define FREEBREWIE_SCREEN_STATUS_H

/****************************************************************************************
 * @file Screen_status.h
 * @brief Scrollable diagnostics/status screen.
 *
 * Responsibility: show live diagnostics in a scrollable status screen.
 * Owns: status LVGL rows, temporary touch proof rows, and dirty-checked label updates.
 * Does not own: formatting comms data or normal Home-screen UX.
 ****************************************************************************************/

#include <stdint.h>

#include "Logic/Status_view_model.h"
#include "UI_types.h"
#include "lvgl.h"

typedef struct
{
    lv_obj_t *screen;
    lv_obj_t *title_label;
    /** Value labels updated from status_screen_view_model_t. */
    lv_obj_t *display_value;
    lv_obj_t *serial_value;
    lv_obj_t *heartbeat_value;
    lv_obj_t *last_rx_value;
    lv_obj_t *link_value;
    lv_obj_t *hb_counter_value;
    lv_obj_t *mcu_status_value;
    lv_obj_t *pressure_value;
    lv_obj_t *pump_value;
    lv_obj_t *solenoid_value;
    lv_obj_t *fault_value;
    lv_obj_t *touch_value;
    lv_obj_t *button_value;
    /** Navigation callback used by the Home button. */
    ui_action_handler_t action_handler;
    void *action_user_data;
    /** Temporary counters used while proving touch input on the real panel. */
    uint32_t touch_event_count;
    uint32_t button_click_count;
    /** Last heartbeat count written to the visible label. */
    uint32_t shown_heartbeat_count;
    /** Stable text storage for the formatted heartbeat count. */
    char heartbeat_count_text[16];
} screen_status_t;

void screen_status_init(screen_status_t *status, ui_action_handler_t action_handler, void *user_data);
void screen_status_update(screen_status_t *status, const status_screen_view_model_t *view_model);

#endif
