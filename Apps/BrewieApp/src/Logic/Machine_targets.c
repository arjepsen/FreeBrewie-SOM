#include "Machine_targets.h"

#include <string.h>

static void machine_targets_apply_pump_masks(machine_targets_t *targets,
                                             uint32_t pump_on_mask,
                                             uint32_t pump_off_mask);

/****************************************************************************************
 * @brief Apply future pump on/off masks to the current target image.
 *
 * The present MCU protocol has two pump setpoint bytes. The process-plan scaffold uses
 * masks so later expert/web editing can name pump targets without changing the whole plan
 * shape. For now, bit 0 is mash pump and bit 1 is boil pump; "on" means full setpoint.
 ****************************************************************************************/
static void machine_targets_apply_pump_masks(machine_targets_t *targets,
                                             uint32_t pump_on_mask,
                                             uint32_t pump_off_mask)
{
    if ((pump_off_mask & 1U) != 0U)
    {
        targets->mash_pump_setpoint = 0U;
    }

    if ((pump_off_mask & 2U) != 0U)
    {
        targets->boil_pump_setpoint = 0U;
    }

    if ((pump_on_mask & 1U) != 0U)
    {
        targets->mash_pump_setpoint = 100U;
    }

    if ((pump_on_mask & 2U) != 0U)
    {
        targets->boil_pump_setpoint = 100U;
    }
}

/****************************************************************************************
 * @brief Initialize a target image to the known idle/default state.
 ****************************************************************************************/
void machine_targets_init(machine_targets_t *targets)
{
    machine_targets_clear(targets);
}

/****************************************************************************************
 * @brief Clear all requested targets.
 *
 * A cleared valve command byte means "no requested valve target" on the shared wire
 * protocol. Explicit open/close requests use the named MACHINE_TARGET_VALVE_* values.
 ****************************************************************************************/
void machine_targets_clear(machine_targets_t *targets)
{
    if (targets == NULL)
    {
        return;
    }

    memset(targets, 0, sizeof(*targets));
}

/****************************************************************************************
 * @brief Apply one process-plan step to the carried target image.
 *
 * Process-plan steps are state segments. A step only changes the fields it explicitly sets;
 * every other target carries forward from the previous step.
 ****************************************************************************************/
void machine_targets_apply_process_step(machine_targets_t *targets,
                                        const process_plan_step_t *step)
{
    const process_plan_target_changes_t *changes;

    if (targets == NULL || step == NULL || step->kind != PROCESS_PLAN_STEP_TARGET_SEGMENT)
    {
        return;
    }

    changes = &step->targets;

    if (changes->set_mash_temperature)
    {
        targets->mash_target_c = changes->mash_temperature_c;
    }

    if (changes->set_boil_temperature)
    {
        targets->boil_target_c = changes->boil_temperature_c;
    }

    if (changes->set_cooling_temperature)
    {
        targets->cooling_target_c = changes->cooling_temperature_c;
    }

    if (changes->set_heater_duty_limit)
    {
        targets->heater_duty_limit_percent = changes->heater_duty_limit_percent;
    }

    machine_targets_apply_pump_masks(targets, changes->pump_on_mask, changes->pump_off_mask);

    /*
     * Valve masks are intentionally not mapped into the snapshot yet. The protocol can now
     * represent open and close safely, but the process-plan-to-valve ownership rules still
     * need to be locked down before the SOM starts sending real valve targets.
     */
}

/****************************************************************************************
 * @brief Pack the current target image into today's 16-byte MCU CONTROL_SNAPSHOT payload.
 *
 * This function only builds the payload. Protocol framing, sequence numbers, serial send,
 * ACK/NACK handling, and safety permission checks belong in other modules.
 ****************************************************************************************/
bool machine_targets_encode_control_snapshot(const machine_targets_t *targets,
                                             uint8_t *payload,
                                             size_t payload_size)
{
    if (targets == NULL || payload == NULL ||
        payload_size < MACHINE_TARGET_CONTROL_SNAPSHOT_SIZE)
    {
        return false;
    }

    payload[0] = targets->mash_target_c;
    payload[1] = targets->boil_target_c;
    payload[2] = targets->mash_pump_setpoint;
    payload[3] = targets->boil_pump_setpoint;
    payload[4] = targets->solenoid_state_bits;
    memcpy(&payload[5], targets->valve_command, MACHINE_TARGET_VALVE_COUNT);

    return true;
}
