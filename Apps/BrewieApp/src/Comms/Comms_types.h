#ifndef FREEBREWIE_COMMS_TYPES_H
#define FREEBREWIE_COMMS_TYPES_H

/****************************************************************************************
 * @file Comms_types.h
 * @brief Shared data types produced by the communications layer.
 *
 * Responsibility: define plain communication snapshots shared upward.
 * Owns: MCU report structs, link-state enum, and comms status struct.
 * Does not own: LVGL objects, Linux file descriptors, or workflow decisions.
 ****************************************************************************************/

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    COMMS_LINK_DOWN = 0,  // Serial device could not be opened.
    COMMS_LINK_WAITING,  // Serial is open, but no recent valid MCU frame arrived.
    COMMS_LINK_OK  // A valid MCU frame arrived recently.
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

/** Latest ACK/NACK returned by the MCU for a SOM command frame. */
typedef struct
{
    bool valid;
    bool accepted;
    uint8_t response_seq;
    uint8_t referenced_type;
    uint8_t referenced_seq;
    uint8_t nack_reason;
} comms_mcu_command_response_t;

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
    comms_mcu_command_response_t command_response;
} comms_status_t;

#endif
