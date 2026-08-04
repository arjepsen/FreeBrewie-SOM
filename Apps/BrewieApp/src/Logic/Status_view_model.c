#include "Status_view_model.h"

#include <stdio.h>
#include <string.h>

#include "Comms/Comms_link.h"

#define MCU_STATUS_BIT_PRESSURE_VALID  (1U << 2)
#define MCU_STATUS_BIT_HEARTBEAT_ALIVE (1U << 3)
#define MCU_SOLENOID_BIT_BREW_INLET    (1U << 0)
#define MCU_SOLENOID_BIT_COOLING_INLET (1U << 1)

static bool status_view_model_status_report_matches(const comms_mcu_status_report_t *left,
                                                    const comms_mcu_status_report_t *right);
static bool status_view_model_fault_report_matches(const comms_mcu_fault_report_t *left,
                                                   const comms_mcu_fault_report_t *right);
static void status_view_model_update_last_rx_text(status_view_model_t *model, const comms_status_t *comms_status);
static void status_view_model_update_mcu_status_text(status_view_model_t *model,
                                                     const comms_mcu_status_report_t *mcu_status);
static void status_view_model_update_fault_text(status_view_model_t *model,
                                                const comms_mcu_status_report_t *mcu_status,
                                                const comms_mcu_fault_report_t *mcu_faults);
static void status_view_model_update_machine_snapshot(status_view_model_t *model,
                                                       const comms_mcu_status_report_t *mcu_status);
static void status_view_model_format_control_snapshot(status_view_model_t *model,
                                                      const uint8_t *payload,
                                                      uint8_t payload_size);

/****************************************************************************************
 * @brief Compare two decoded MCU status reports field by field.
 *
 * Do not use memcmp() for C structs here. Struct padding bytes are not part of the protocol
 * data and may contain arbitrary values, so explicit field comparisons are safer and clearer.
 ****************************************************************************************/
static bool status_view_model_status_report_matches(const comms_mcu_status_report_t *left,
                                                    const comms_mcu_status_report_t *right)
{
    uint8_t valve_index;

    if (left == NULL || right == NULL)
    {
        return false;
    }

    if (left->valid != right->valid ||
        left->mash_target_c != right->mash_target_c ||
        left->boil_target_c != right->boil_target_c ||
        left->mash_temp_c_x10 != right->mash_temp_c_x10 ||
        left->boil_temp_c_x10 != right->boil_temp_c_x10 ||
        left->mash_pump_setpoint != right->mash_pump_setpoint ||
        left->boil_pump_setpoint != right->boil_pump_setpoint ||
        left->mash_pump_running != right->mash_pump_running ||
        left->boil_pump_running != right->boil_pump_running ||
        left->pressure_count != right->pressure_count ||
        left->solenoid_state_bits != right->solenoid_state_bits ||
        left->status_bits != right->status_bits ||
        left->fault_flags != right->fault_flags)
    {
        return false;
    }

    for (valve_index = 0U; valve_index < 11U; valve_index++)
    {
        if (left->valve_state[valve_index] != right->valve_state[valve_index])
        {
            return false;
        }
    }

    return true;
}

/****************************************************************************************
 * @brief Compare two decoded MCU fault reports field by field.
 ****************************************************************************************/
static bool status_view_model_fault_report_matches(const comms_mcu_fault_report_t *left,
                                                   const comms_mcu_fault_report_t *right)
{
    if (left == NULL || right == NULL)
    {
        return false;
    }

    return left->valid == right->valid &&
           left->active_fault_flags == right->active_fault_flags &&
           left->latched_fault_flags == right->latched_fault_flags &&
           left->primary_reason == right->primary_reason;
}

/****************************************************************************************
 * @brief Rebuild the small "last received frame" text.
 ****************************************************************************************/
static void status_view_model_update_last_rx_text(status_view_model_t *model, const comms_status_t *comms_status)
{
    if (comms_status->last_rx_ms == 0U)
    {
        model->values.last_rx_text = "none";
        return;
    }

    snprintf(model->last_rx_text,
             sizeof(model->last_rx_text),
             "type=%u seq=%u len=%u",
             (unsigned int)comms_status->last_rx_type,
             (unsigned int)comms_status->last_rx_seq,
             (unsigned int)comms_status->last_rx_len);
    model->values.last_rx_text = model->last_rx_text;
}

/****************************************************************************************
 * @brief Rebuild status, pressure, pump, and inlet text from the latest MCU status report.
 ****************************************************************************************/
static void status_view_model_update_mcu_status_text(status_view_model_t *model,
                                                     const comms_mcu_status_report_t *mcu_status)
{
    if (!mcu_status->valid)
    {
        model->values.mcu_status_text = "none";
        model->values.pressure_text = "not valid";
        model->values.pump_text = "mash off / boil off";
        model->values.solenoid_text = "brew off / cool off";
        return;
    }

    /*
     * The status screen is still a bring-up screen, so it favors compact engineering facts
     * over polished user text. These strings are rebuilt only when the MCU report changes.
     */
    snprintf(model->mcu_status_text,
             sizeof(model->mcu_status_text),
             "status 0x%02X / target %uC %uC",
             (unsigned int)mcu_status->status_bits,
             (unsigned int)mcu_status->mash_target_c,
             (unsigned int)mcu_status->boil_target_c);
    model->values.mcu_status_text = model->mcu_status_text;

    if ((mcu_status->status_bits & MCU_STATUS_BIT_PRESSURE_VALID) != 0U)
    {
        snprintf(model->pressure_text,
                 sizeof(model->pressure_text),
                 "%u counts",
                 (unsigned int)mcu_status->pressure_count);
    }
    else
    {
        snprintf(model->pressure_text,
                 sizeof(model->pressure_text),
                 "not valid (%u)",
                 (unsigned int)mcu_status->pressure_count);
    }
    model->values.pressure_text = model->pressure_text;

    snprintf(model->pump_text,
             sizeof(model->pump_text),
             "mash %u/%s  boil %u/%s",
             (unsigned int)mcu_status->mash_pump_setpoint,
             mcu_status->mash_pump_running ? "run" : "off",
             (unsigned int)mcu_status->boil_pump_setpoint,
             mcu_status->boil_pump_running ? "run" : "off");
    model->values.pump_text = model->pump_text;

    snprintf(model->solenoid_text,
             sizeof(model->solenoid_text),
             "brew %s / cool %s / hb %s",
             (mcu_status->solenoid_state_bits & MCU_SOLENOID_BIT_BREW_INLET) != 0U ? "on" : "off",
             (mcu_status->solenoid_state_bits & MCU_SOLENOID_BIT_COOLING_INLET) != 0U ? "on" : "off",
             (mcu_status->status_bits & MCU_STATUS_BIT_HEARTBEAT_ALIVE) != 0U ? "ok" : "lost");
    model->values.solenoid_text = model->solenoid_text;
}

/****************************************************************************************
 * @brief Copy raw MCU status facts into the product-screen machine snapshot.
 *
 * Diagnostic strings are useful on the Status screen, but the normal product screens need
 * simple numbers and flags. Keeping this copy here avoids text parsing in UI code and gives
 * future screens one stable read-only place to look for live machine facts.
 ****************************************************************************************/
static void status_view_model_update_machine_snapshot(status_view_model_t *model,
                                                       const comms_mcu_status_report_t *mcu_status)
{
    model->values.machine.mcu_status_valid = mcu_status->valid;
    model->values.machine.mash_target_c = mcu_status->mash_target_c;
    model->values.machine.boil_target_c = mcu_status->boil_target_c;
    model->values.machine.mash_temp_c_x10 = mcu_status->mash_temp_c_x10;
    model->values.machine.boil_temp_c_x10 = mcu_status->boil_temp_c_x10;
    model->values.machine.mash_pump_setpoint = mcu_status->mash_pump_setpoint;
    model->values.machine.boil_pump_setpoint = mcu_status->boil_pump_setpoint;
    model->values.machine.mash_pump_running = mcu_status->mash_pump_running;
    model->values.machine.boil_pump_running = mcu_status->boil_pump_running;
    model->values.machine.pressure_count = mcu_status->pressure_count;
    model->values.machine.solenoid_state_bits = mcu_status->solenoid_state_bits;
    model->values.machine.status_bits = mcu_status->status_bits;
    model->values.machine.fault_flags = mcu_status->fault_flags;
}

/****************************************************************************************
 * @brief Rebuild the fault text from the best available MCU fault information.
 *
 * A detailed FAULT_REPORT takes priority. If that is not available, the compact fault flags
 * inside STATUS_REPORT are still useful enough for the bring-up screen.
 ****************************************************************************************/
static void status_view_model_update_fault_text(status_view_model_t *model,
                                                const comms_mcu_status_report_t *mcu_status,
                                                const comms_mcu_fault_report_t *mcu_faults)
{
    if (mcu_faults->valid)
    {
        snprintf(model->fault_text,
                 sizeof(model->fault_text),
                 "active 0x%04X latch 0x%04X reason %u",
                 (unsigned int)mcu_faults->active_fault_flags,
                 (unsigned int)mcu_faults->latched_fault_flags,
                 (unsigned int)mcu_faults->primary_reason);
        model->values.fault_text = model->fault_text;
        return;
    }

    if (mcu_status->valid && mcu_status->fault_flags != 0U)
    {
        snprintf(model->fault_text,
                 sizeof(model->fault_text),
                 "active 0x%04X",
                 (unsigned int)mcu_status->fault_flags);
        model->values.fault_text = model->fault_text;
        return;
    }

    model->values.fault_text = "none";
}

/****************************************************************************************
 * @brief Format the first local-only CONTROL_SNAPSHOT preview bytes.
 *
 * The Status screen only needs enough detail to prove the process-runner-to-snapshot bridge
 * is producing stable bytes. Full protocol inspection belongs in a future debug tool or log
 * view, not in a tiny portrait status row.
 ****************************************************************************************/
static void status_view_model_format_control_snapshot(status_view_model_t *model,
                                                      const uint8_t *payload,
                                                      uint8_t payload_size)
{
    if (payload == NULL || payload_size < 5U)
    {
        model->values.control_snapshot_text = "not ready";
        return;
    }

    snprintf(model->control_snapshot_text,
             sizeof(model->control_snapshot_text),
             "len %u: %02X %02X %02X %02X %02X...",
             (unsigned int)payload_size,
             (unsigned int)payload[0],
             (unsigned int)payload[1],
             (unsigned int)payload[2],
             (unsigned int)payload[3],
             (unsigned int)payload[4]);
    model->values.control_snapshot_text = model->control_snapshot_text;
}

/****************************************************************************************
 * @brief Fill the status view model with safe startup text.
 *
 * This gives every label something meaningful to show before the first MCU report arrives.
 ****************************************************************************************/
void status_screen_view_model_init(status_screen_view_model_t *view_model)
{
    if (view_model == NULL)
    {
        return;
    }

    memset(view_model, 0, sizeof(*view_model));
    view_model->display_text = "bypassed";
    view_model->serial_text = "not ready";
    view_model->heartbeat_text = "stopped";
    view_model->last_rx_text = "none";
    view_model->link_text = "down";
    view_model->mcu_status_text = "none";
    view_model->pressure_text = "not valid";
    view_model->pump_text = "mash off / boil off";
    view_model->solenoid_text = "brew off / cool off";
    view_model->fault_text = "none";
    view_model->control_snapshot_text = "not ready";
}

/****************************************************************************************
 * @brief Initialize diagnostic status-screen text and cache state.
 ****************************************************************************************/
void status_view_model_init(status_view_model_t *model)
{
    if (model == NULL)
    {
        return;
    }

    memset(model, 0, sizeof(*model));
    status_screen_view_model_init(&model->values);
}

/****************************************************************************************
 * @brief Update the startup serial text after the comms layer tries to open the port.
 ****************************************************************************************/
void status_view_model_set_serial_ready(status_view_model_t *model, bool serial_ready)
{
    if (model == NULL)
    {
        return;
    }

    model->values.serial_text = serial_ready ? "ready" : "not ready";
}

/****************************************************************************************
 * @brief Convert raw communications state into UI-ready diagnostic status text.
 *
 * This function is deliberately not called from the fastest part of the app loop. It uses
 * snprintf(), which is convenient and safe for building diagnostic text, but not something
 * we want to run hundreds of times per second on the A13 SOM.
 ****************************************************************************************/
void status_view_model_update(status_view_model_t *model, const comms_status_t *comms_status)
{
    const comms_mcu_status_report_t *mcu_status;
    const comms_mcu_fault_report_t *mcu_faults;
    bool last_rx_changed;
    bool mcu_status_changed;
    bool mcu_faults_changed;

    if (model == NULL || comms_status == NULL)
    {
        return;
    }

    mcu_status = &comms_status->mcu_status;
    mcu_faults = &comms_status->mcu_faults;

    /*
     * Cheap direct fields are still updated only when they change. The saving is small, but
     * it keeps the view model honest: unchanged input should not cause needless downstream
     * churn in LVGL labels.
     */
    if (model->cached_serial_ready != comms_status->serial_ready)
    {
        model->values.serial_text = comms_status->serial_ready ? "ready" : "not ready";
        model->cached_serial_ready = comms_status->serial_ready;
    }

    if (model->cached_heartbeat_running != comms_status->heartbeat_running)
    {
        model->values.heartbeat_text = comms_status->heartbeat_running ? "running" : "stopped";
        model->cached_heartbeat_running = comms_status->heartbeat_running;
    }

    if (model->cached_link_state != comms_status->link_state)
    {
        model->values.link_text = comms_link_state_name(comms_status->link_state);
        model->cached_link_state = comms_status->link_state;
    }

    if (model->cached_heartbeat_count != comms_status->heartbeat_count)
    {
        model->values.heartbeat_count = comms_status->heartbeat_count;
        model->cached_heartbeat_count = comms_status->heartbeat_count;
    }

    last_rx_changed = model->cached_last_rx_ms != comms_status->last_rx_ms ||
                      model->cached_last_rx_type != comms_status->last_rx_type ||
                      model->cached_last_rx_seq != comms_status->last_rx_seq ||
                      model->cached_last_rx_len != comms_status->last_rx_len;
    if (last_rx_changed)
    {
        status_view_model_update_last_rx_text(model, comms_status);
        model->cached_last_rx_ms = comms_status->last_rx_ms;
        model->cached_last_rx_type = comms_status->last_rx_type;
        model->cached_last_rx_seq = comms_status->last_rx_seq;
        model->cached_last_rx_len = comms_status->last_rx_len;
    }

    mcu_status_changed = !status_view_model_status_report_matches(&model->cached_mcu_status, mcu_status);
    if (mcu_status_changed)
    {
        status_view_model_update_mcu_status_text(model, mcu_status);
        status_view_model_update_machine_snapshot(model, mcu_status);
        model->cached_mcu_status = *mcu_status;
    }

    mcu_faults_changed = !status_view_model_fault_report_matches(&model->cached_mcu_faults, mcu_faults);
    if (mcu_status_changed || mcu_faults_changed)
    {
        status_view_model_update_fault_text(model, mcu_status, mcu_faults);
        model->cached_mcu_faults = *mcu_faults;
    }
}

/****************************************************************************************
 * @brief Update the diagnostic text for the local-only CONTROL_SNAPSHOT preview.
 *
 * This is intentionally separate from comms status updates because the preview is app logic,
 * not data received from the MCU. The payload is never transmitted here.
 ****************************************************************************************/
void status_view_model_update_control_snapshot(status_view_model_t *model,
                                               const uint8_t *payload,
                                               uint8_t payload_size,
                                               bool valid)
{
    if (model == NULL)
    {
        return;
    }

    if (!valid)
    {
        if (model->cached_control_snapshot_valid || model->values.control_snapshot_text == NULL)
        {
            model->values.control_snapshot_text = "not ready";
            model->cached_control_snapshot_valid = false;
            model->cached_control_snapshot_size = 0U;
        }
        return;
    }

    if (payload == NULL || payload_size == 0U ||
        payload_size > sizeof(model->cached_control_snapshot_payload))
    {
        model->values.control_snapshot_text = "invalid preview";
        model->cached_control_snapshot_valid = false;
        model->cached_control_snapshot_size = 0U;
        return;
    }

    if (model->cached_control_snapshot_valid &&
        model->cached_control_snapshot_size == payload_size &&
        memcmp(model->cached_control_snapshot_payload, payload, payload_size) == 0)
    {
        return;
    }

    memcpy(model->cached_control_snapshot_payload, payload, payload_size);
    model->cached_control_snapshot_size = payload_size;
    model->cached_control_snapshot_valid = true;
    status_view_model_format_control_snapshot(model, payload, payload_size);
}
