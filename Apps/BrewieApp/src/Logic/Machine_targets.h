#ifndef FREEBREWIE_MACHINE_TARGETS_H
#define FREEBREWIE_MACHINE_TARGETS_H

/****************************************************************************************
 * @file Machine_targets.h
 * @brief Requested machine-target model.
 *
 * Responsibility: hold the current SOM-side target image before it becomes an MCU control
 * snapshot.
 * Owns: compact target values that carry forward between process-plan steps.
 * Must not own: recipe editing, active workflow decisions, serial transport, or hardware
 * fault handling.
 ****************************************************************************************/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "Process_plan.h"

#define MACHINE_TARGET_CONTROL_SNAPSHOT_SIZE 16U
#define MACHINE_TARGET_VALVE_COUNT 11U

typedef enum
{
    MACHINE_TARGET_VALVE_NO_COMMAND = 0,
    MACHINE_TARGET_VALVE_OPEN = 1,
    MACHINE_TARGET_VALVE_CLOSE = 2,
    MACHINE_TARGET_VALVE_CLOSE_HARD = 3,
    MACHINE_TARGET_VALVE_SPARGE_OPEN = 4,
    MACHINE_TARGET_VALVE_SPARGE_CLOSE = 5
} machine_target_valve_position_t;

typedef enum
{
    MACHINE_TARGET_SOLENOID_BREW_INLET = 1U << 0U,
    MACHINE_TARGET_SOLENOID_COOLING_INLET = 1U << 1U
} machine_target_solenoid_bit_t;

typedef struct
{
    uint8_t mash_target_c;
    uint8_t boil_target_c;
    uint8_t mash_pump_setpoint;
    uint8_t boil_pump_setpoint;
    uint8_t solenoid_state_bits;
    uint8_t valve_command[MACHINE_TARGET_VALVE_COUNT];
    uint8_t cooling_target_c;          // SOM-only for now; the current MCU snapshot has no byte for this.
    uint8_t heater_duty_limit_percent; // SOM-only until the shared protocol grows this field.
} machine_targets_t;

void machine_targets_init(machine_targets_t *targets);
void machine_targets_clear(machine_targets_t *targets);
void machine_targets_apply_process_step(machine_targets_t *targets,
                                        const process_plan_step_t *step);
bool machine_targets_encode_control_snapshot(const machine_targets_t *targets,
                                             uint8_t *payload,
                                             size_t payload_size);

#endif
