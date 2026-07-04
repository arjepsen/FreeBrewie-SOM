#include "App_logic.h"

#include <stdio.h>
#include <string.h>

#include "Comms/Comms_link.h"

#define MCU_STATUS_BIT_PRESSURE_VALID  (1U << 2)
#define MCU_STATUS_BIT_HEARTBEAT_ALIVE (1U << 3)
#define MCU_SOLENOID_BIT_BREW_INLET    (1U << 0)
#define MCU_SOLENOID_BIT_COOLING_INLET (1U << 1)

static bool app_logic_status_report_matches(const comms_mcu_status_report_t *left,
                                            const comms_mcu_status_report_t *right);
static bool app_logic_fault_report_matches(const comms_mcu_fault_report_t *left,
                                           const comms_mcu_fault_report_t *right);
static void app_logic_update_last_rx_text(app_logic_t *logic, const comms_status_t *comms_status);
static void app_logic_update_mcu_status_text(app_logic_t *logic, const comms_mcu_status_report_t *mcu_status);
static void app_logic_update_fault_text(app_logic_t *logic,
                                        const comms_mcu_status_report_t *mcu_status,
                                        const comms_mcu_fault_report_t *mcu_faults);

/****************************************************************************************
 * @brief Compare two decoded MCU status reports field by field.
 *
 * Do not use memcmp() for C structs here. Struct padding bytes are not part of the protocol
 * data and may contain arbitrary values, so explicit field comparisons are safer and clearer.
 ****************************************************************************************/
static bool app_logic_status_report_matches(const comms_mcu_status_report_t *left,
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
static bool app_logic_fault_report_matches(const comms_mcu_fault_report_t *left,
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
static void app_logic_update_last_rx_text(app_logic_t *logic, const comms_status_t *comms_status)
{
    if (comms_status->last_rx_ms == 0U)
    {
        logic->status_screen.last_rx_text = "none";
        return;
    }

    snprintf(logic->last_rx_text,
             sizeof(logic->last_rx_text),
             "type=%u seq=%u len=%u",
             (unsigned int)comms_status->last_rx_type,
             (unsigned int)comms_status->last_rx_seq,
             (unsigned int)comms_status->last_rx_len);
    logic->status_screen.last_rx_text = logic->last_rx_text;
}

/****************************************************************************************
 * @brief Rebuild status, pressure, pump, and inlet text from the latest MCU status report.
 ****************************************************************************************/
static void app_logic_update_mcu_status_text(app_logic_t *logic, const comms_mcu_status_report_t *mcu_status)
{
    if (!mcu_status->valid)
    {
        logic->status_screen.mcu_status_text = "none";
        logic->status_screen.pressure_text = "not valid";
        logic->status_screen.pump_text = "mash off / boil off";
        logic->status_screen.solenoid_text = "brew off / cool off";
        return;
    }

    /*
     * The status screen is still a bring-up screen, so it favors compact engineering facts
     * over polished user text. These strings are rebuilt only when the MCU report changes.
     */
    snprintf(logic->mcu_status_text,
             sizeof(logic->mcu_status_text),
             "status 0x%02X / target %uC %uC",
             (unsigned int)mcu_status->status_bits,
             (unsigned int)mcu_status->mash_target_c,
             (unsigned int)mcu_status->boil_target_c);
    logic->status_screen.mcu_status_text = logic->mcu_status_text;

    if ((mcu_status->status_bits & MCU_STATUS_BIT_PRESSURE_VALID) != 0U)
    {
        snprintf(logic->pressure_text,
                 sizeof(logic->pressure_text),
                 "%u counts",
                 (unsigned int)mcu_status->pressure_count);
    }
    else
    {
        snprintf(logic->pressure_text,
                 sizeof(logic->pressure_text),
                 "not valid (%u)",
                 (unsigned int)mcu_status->pressure_count);
    }
    logic->status_screen.pressure_text = logic->pressure_text;

    snprintf(logic->pump_text,
             sizeof(logic->pump_text),
             "mash %u/%s  boil %u/%s",
             (unsigned int)mcu_status->mash_pump_setpoint,
             mcu_status->mash_pump_running ? "run" : "off",
             (unsigned int)mcu_status->boil_pump_setpoint,
             mcu_status->boil_pump_running ? "run" : "off");
    logic->status_screen.pump_text = logic->pump_text;

    snprintf(logic->solenoid_text,
             sizeof(logic->solenoid_text),
             "brew %s / cool %s / hb %s",
             (mcu_status->solenoid_state_bits & MCU_SOLENOID_BIT_BREW_INLET) != 0U ? "on" : "off",
             (mcu_status->solenoid_state_bits & MCU_SOLENOID_BIT_COOLING_INLET) != 0U ? "on" : "off",
             (mcu_status->status_bits & MCU_STATUS_BIT_HEARTBEAT_ALIVE) != 0U ? "ok" : "lost");
    logic->status_screen.solenoid_text = logic->solenoid_text;
}

/****************************************************************************************
 * @brief Rebuild the fault text from the best available MCU fault information.
 *
 * A detailed FAULT_REPORT takes priority. If that is not available, the compact fault flags
 * inside STATUS_REPORT are still useful enough for the bring-up screen.
 ****************************************************************************************/
static void app_logic_update_fault_text(app_logic_t *logic,
                                        const comms_mcu_status_report_t *mcu_status,
                                        const comms_mcu_fault_report_t *mcu_faults)
{
    if (mcu_faults->valid)
    {
        snprintf(logic->fault_text,
                 sizeof(logic->fault_text),
                 "active 0x%04X latch 0x%04X reason %u",
                 (unsigned int)mcu_faults->active_fault_flags,
                 (unsigned int)mcu_faults->latched_fault_flags,
                 (unsigned int)mcu_faults->primary_reason);
        logic->status_screen.fault_text = logic->fault_text;
        return;
    }

    if (mcu_status->valid && mcu_status->fault_flags != 0U)
    {
        snprintf(logic->fault_text,
                 sizeof(logic->fault_text),
                 "active 0x%04X",
                 (unsigned int)mcu_status->fault_flags);
        logic->status_screen.fault_text = logic->fault_text;
        return;
    }

    logic->status_screen.fault_text = "none";
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
}

/****************************************************************************************
 * @brief Initialize app-level logic state.
 ****************************************************************************************/
void app_logic_init(app_logic_t *logic)
{
    if (logic == NULL)
    {
        return;
    }

    memset(logic, 0, sizeof(*logic));
    status_screen_view_model_init(&logic->status_screen);
}

/****************************************************************************************
 * @brief Update the startup serial text after the comms layer tries to open the port.
 ****************************************************************************************/
void app_logic_set_serial_ready(app_logic_t *logic, bool serial_ready)
{
    if (logic == NULL)
    {
        return;
    }

    logic->status_screen.serial_text = serial_ready ? "ready" : "not ready";
}

/****************************************************************************************
 * @brief Run app logic that must stay cheap enough for the every-loop path.
 *
 * This is intentionally separate from UI text formatting. printf-style formatting is useful
 * for human-readable screens, but it is relatively expensive and should not become part of
 * future control/interlock logic that needs to run as often as possible.
 *
 * There is no real machine-control logic here yet. Keeping the function now gives future
 * brewing, cleaning, and safety-interlock work a clear place to live.
 ****************************************************************************************/
void app_logic_update_fast(app_logic_t *logic, const comms_status_t *comms_status, uint64_t now_ms)
{
    (void)logic;
    (void)comms_status;
    (void)now_ms;
}

/****************************************************************************************
 * @brief Convert raw communications state into UI-ready status text.
 *
 * This function is deliberately not called from the fastest part of the app loop. It uses
 * snprintf(), which is convenient and safe for building diagnostic text, but not something
 * we want to run hundreds of times per second on the A13 SOM.
 *
 * The output strings are stored inside app_logic_t. The view model then points at those
 * buffers, allowing LVGL screen code to read simple const char pointers without knowing
 * about protocol fields or formatting rules.
 ****************************************************************************************/
void app_logic_update_status_view_model(app_logic_t *logic, const comms_status_t *comms_status)
{
    const comms_mcu_status_report_t *mcu_status;
    const comms_mcu_fault_report_t *mcu_faults;
    bool last_rx_changed;
    bool mcu_status_changed;
    bool mcu_faults_changed;

    if (logic == NULL || comms_status == NULL)
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
    if (logic->cached_serial_ready != comms_status->serial_ready)
    {
        logic->status_screen.serial_text = comms_status->serial_ready ? "ready" : "not ready";
        logic->cached_serial_ready = comms_status->serial_ready;
    }

    if (logic->cached_heartbeat_running != comms_status->heartbeat_running)
    {
        logic->status_screen.heartbeat_text = comms_status->heartbeat_running ? "running" : "stopped";
        logic->cached_heartbeat_running = comms_status->heartbeat_running;
    }

    if (logic->cached_link_state != comms_status->link_state)
    {
        logic->status_screen.link_text = comms_link_state_name(comms_status->link_state);
        logic->cached_link_state = comms_status->link_state;
    }

    if (logic->cached_heartbeat_count != comms_status->heartbeat_count)
    {
        logic->status_screen.heartbeat_count = comms_status->heartbeat_count;
        logic->cached_heartbeat_count = comms_status->heartbeat_count;
    }

    last_rx_changed = logic->cached_last_rx_ms != comms_status->last_rx_ms ||
                      logic->cached_last_rx_type != comms_status->last_rx_type ||
                      logic->cached_last_rx_seq != comms_status->last_rx_seq ||
                      logic->cached_last_rx_len != comms_status->last_rx_len;
    if (last_rx_changed)
    {
        app_logic_update_last_rx_text(logic, comms_status);
        logic->cached_last_rx_ms = comms_status->last_rx_ms;
        logic->cached_last_rx_type = comms_status->last_rx_type;
        logic->cached_last_rx_seq = comms_status->last_rx_seq;
        logic->cached_last_rx_len = comms_status->last_rx_len;
    }

    mcu_status_changed = !app_logic_status_report_matches(&logic->cached_mcu_status, mcu_status);
    if (mcu_status_changed)
    {
        app_logic_update_mcu_status_text(logic, mcu_status);
        logic->cached_mcu_status = *mcu_status;
    }

    mcu_faults_changed = !app_logic_fault_report_matches(&logic->cached_mcu_faults, mcu_faults);
    if (mcu_status_changed || mcu_faults_changed)
    {
        app_logic_update_fault_text(logic, mcu_status, mcu_faults);
        logic->cached_mcu_faults = *mcu_faults;
    }
}
