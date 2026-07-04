#ifndef FREEBREWIE_COMMS_TYPES_H
#define FREEBREWIE_COMMS_TYPES_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    /** The Linux serial device could not be opened. */
    COMMS_LINK_DOWN = 0,
    /** The serial device is open, but the MCU has not sent a recent valid frame. */
    COMMS_LINK_WAITING,
    /** A valid MCU frame arrived recently, so the SOM considers the link alive. */
    COMMS_LINK_OK
} comms_link_state_t;

/**
 * Latest decoded STATUS_REPORT from the MCU.
 *
 * This is the SOM-side copy of the compact binary payload documented in the SOM/MCU
 * protocol docs. Values stay in simple machine units here: temperatures are tenths of a
 * degree C, pump setpoints are the MCU's byte-sized command values, and valve states are
 * the numeric valve positions reported by the firmware.
 */
typedef struct
{
    bool valid;
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
    uint8_t valve_state[11];
} comms_mcu_status_report_t;

/**
 * Latest decoded FAULT_REPORT from the MCU.
 *
 * The MCU owns the real safety decisions. The SOM stores the flags here so the UI can show
 * what the controller believes is wrong without trying to reinterpret hardware state.
 */
typedef struct
{
    bool valid;
    uint16_t active_fault_flags;
    uint16_t latched_fault_flags;
    uint8_t primary_reason;
} comms_mcu_fault_report_t;

/**
 * Public communications status shared with app logic and UI.
 *
 * The comms module updates this structure as frames arrive. Other layers read it as a
 * snapshot; they should not write protocol/link fields directly.
 */
typedef struct
{
    bool serial_ready;
    bool heartbeat_running;
    uint32_t heartbeat_count;
    uint64_t last_rx_ms;
    comms_link_state_t link_state;
    uint8_t last_rx_type;
    uint8_t last_rx_seq;
    uint8_t last_rx_len;
    comms_mcu_status_report_t mcu_status;
    comms_mcu_fault_report_t mcu_faults;
} comms_status_t;

#endif
