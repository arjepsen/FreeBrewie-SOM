#ifndef FREEBREWIE_STATUS_VIEW_MODEL_H
#define FREEBREWIE_STATUS_VIEW_MODEL_H

/****************************************************************************************
 * @file Status_view_model.h
 * @brief Diagnostic status-screen presentation model.
 *
 * Responsibility: prepare diagnostics text for the status screen.
 * Owns: formatted status strings, backing storage, and comms-change caches.
 * Does not own: LVGL widgets or machine-control decisions.
 ****************************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include "Comms/Comms_types.h"

#define STATUS_VIEW_MODEL_LAST_RX_TEXT_SIZE   48U
#define STATUS_VIEW_MODEL_STATUS_TEXT_SIZE    64U
#define STATUS_VIEW_MODEL_PRESSURE_TEXT_SIZE  48U
#define STATUS_VIEW_MODEL_PUMP_TEXT_SIZE      64U
#define STATUS_VIEW_MODEL_SOLENOID_TEXT_SIZE  64U
#define STATUS_VIEW_MODEL_FAULT_TEXT_SIZE     64U

typedef struct
{
    /*
     * Small raw machine snapshot shared with product-shaped screens.
     *
     * The dense status screen still uses formatted diagnostic strings, but normal screens
     * should read compact values like these and update only the widgets that changed.
     */
    bool mcu_status_valid;
    uint8_t mash_target_c;
    uint8_t boil_target_c;
    int16_t mash_temp_c_x10;
    int16_t boil_temp_c_x10;
    uint8_t mash_pump_setpoint;
    uint8_t boil_pump_setpoint;
    bool mash_pump_running;
    bool boil_pump_running;
    uint16_t pressure_count;
    uint8_t solenoid_state_bits;
    uint8_t status_bits;
    uint16_t fault_flags;
} status_machine_snapshot_t;

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
    status_machine_snapshot_t machine;
} status_screen_view_model_t;

typedef struct
{
    status_screen_view_model_t values;  // UI-ready diagnostic/status text built from the latest comms snapshot.
    /*
     * Backing storage for formatted UI strings.
     *
     * status_screen_view_model_t stores const char pointers so screens can read simple
     * strings. Any string that is formatted at runtime must live somewhere stable after the
     * formatting function returns; these arrays are that stable storage.
     */
    char last_rx_text[STATUS_VIEW_MODEL_LAST_RX_TEXT_SIZE];
    char mcu_status_text[STATUS_VIEW_MODEL_STATUS_TEXT_SIZE];
    char pressure_text[STATUS_VIEW_MODEL_PRESSURE_TEXT_SIZE];
    char pump_text[STATUS_VIEW_MODEL_PUMP_TEXT_SIZE];
    char solenoid_text[STATUS_VIEW_MODEL_SOLENOID_TEXT_SIZE];
    char fault_text[STATUS_VIEW_MODEL_FAULT_TEXT_SIZE];
    /*
     * Last comms values used to build the view model.
     *
     * These caches let status_view_model_update() skip snprintf() calls when the underlying
     * data is unchanged. That matters because most UI refreshes see identical status text
     * between one-second MCU reports.
     */
    bool cached_serial_ready;
    bool cached_heartbeat_running;
    uint32_t cached_heartbeat_count;
    uint64_t cached_last_rx_ms;
    comms_link_state_t cached_link_state;
    uint8_t cached_last_rx_type;
    uint8_t cached_last_rx_seq;
    uint8_t cached_last_rx_len;
    comms_mcu_status_report_t cached_mcu_status;
    comms_mcu_fault_report_t cached_mcu_faults;
} status_view_model_t;

void status_screen_view_model_init(status_screen_view_model_t *view_model);
void status_view_model_init(status_view_model_t *model);
void status_view_model_set_serial_ready(status_view_model_t *model, bool serial_ready);
void status_view_model_update(status_view_model_t *model, const comms_status_t *comms_status);

#endif
