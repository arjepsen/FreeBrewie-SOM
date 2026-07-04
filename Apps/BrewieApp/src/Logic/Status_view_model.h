#ifndef FREEBREWIE_STATUS_VIEW_MODEL_H
#define FREEBREWIE_STATUS_VIEW_MODEL_H

#include <stdbool.h>
#include <stdint.h>

#include "App_types.h"
#include "Comms/Comms_types.h"

#define STATUS_VIEW_MODEL_LAST_RX_TEXT_SIZE   48U
#define STATUS_VIEW_MODEL_STATUS_TEXT_SIZE    64U
#define STATUS_VIEW_MODEL_PRESSURE_TEXT_SIZE  48U
#define STATUS_VIEW_MODEL_PUMP_TEXT_SIZE      64U
#define STATUS_VIEW_MODEL_SOLENOID_TEXT_SIZE  64U
#define STATUS_VIEW_MODEL_FAULT_TEXT_SIZE     64U

typedef struct
{
    /** UI-ready diagnostic/status text built from the latest comms snapshot. */
    status_screen_view_model_t values;
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
