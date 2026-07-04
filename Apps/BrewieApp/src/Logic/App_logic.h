#ifndef FREEBREWIE_APP_LOGIC_H
#define FREEBREWIE_APP_LOGIC_H

#include <stdbool.h>
#include <stdint.h>

#include "App_types.h"
#include "Comms/Comms_types.h"

#define APP_LOGIC_LAST_RX_TEXT_SIZE     48U
#define APP_LOGIC_STATUS_TEXT_SIZE      64U
#define APP_LOGIC_PRESSURE_TEXT_SIZE    48U
#define APP_LOGIC_PUMP_TEXT_SIZE        64U
#define APP_LOGIC_SOLENOID_TEXT_SIZE    64U
#define APP_LOGIC_FAULT_TEXT_SIZE       64U

typedef struct
{
    /** UI-ready diagnostic/status text built from the latest comms snapshot. */
    status_screen_view_model_t status_screen;
    /*
     * Backing storage for formatted UI strings.
     *
     * status_screen_view_model_t stores const char pointers so screens can read simple
     * strings. Any string that is formatted at runtime must live somewhere stable after the
     * formatting function returns; these arrays are that stable storage.
     */
    char last_rx_text[APP_LOGIC_LAST_RX_TEXT_SIZE];
    char mcu_status_text[APP_LOGIC_STATUS_TEXT_SIZE];
    char pressure_text[APP_LOGIC_PRESSURE_TEXT_SIZE];
    char pump_text[APP_LOGIC_PUMP_TEXT_SIZE];
    char solenoid_text[APP_LOGIC_SOLENOID_TEXT_SIZE];
    char fault_text[APP_LOGIC_FAULT_TEXT_SIZE];
    /*
     * Last comms values used to build the view model.
     *
     * These caches let app_logic_update_status_view_model() skip snprintf() calls when the
     * underlying data is unchanged. That matters because most UI refreshes see identical
     * status text between one-second MCU reports.
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
} app_logic_t;

void app_logic_init(app_logic_t *logic);
void app_logic_set_serial_ready(app_logic_t *logic, bool serial_ready);
void app_logic_update_fast(app_logic_t *logic, const comms_status_t *comms_status, uint64_t now_ms);
void app_logic_update_status_view_model(app_logic_t *logic, const comms_status_t *comms_status);

#endif
