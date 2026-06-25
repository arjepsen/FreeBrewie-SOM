#include "App_logic.h"

#include <stdio.h>
#include <string.h>

#include "Comms/Comms_link.h"

#define MCU_STATUS_BIT_PRESSURE_VALID  (1U << 2)
#define MCU_STATUS_BIT_HEARTBEAT_ALIVE (1U << 3)
#define MCU_SOLENOID_BIT_BREW_INLET    (1U << 0)
#define MCU_SOLENOID_BIT_COOLING_INLET (1U << 1)

void boot_screen_view_model_init(boot_screen_view_model_t *view_model)
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

void app_logic_init(app_logic_t *logic)
{
    if (logic == NULL)
    {
        return;
    }

    memset(logic, 0, sizeof(*logic));
    boot_screen_view_model_init(&logic->boot_screen);
}

void app_logic_set_serial_ready(app_logic_t *logic, bool serial_ready)
{
    if (logic == NULL)
    {
        return;
    }

    logic->boot_screen.serial_text = serial_ready ? "ready" : "not ready";
}

void app_logic_update(app_logic_t *logic, const comms_status_t *comms_status, uint64_t now_ms)
{
    static char heartbeat_count_text[32];
    static char last_rx_text[48];
    static char mcu_status_text[64];
    static char pressure_text[48];
    static char pump_text[64];
    static char solenoid_text[64];
    static char fault_text[64];
    const comms_mcu_status_report_t *mcu_status;
    const comms_mcu_fault_report_t *mcu_faults;

    (void)now_ms;

    if (logic == NULL || comms_status == NULL)
    {
        return;
    }

    logic->boot_screen.serial_text = comms_status->serial_ready ? "ready" : "not ready";
    logic->boot_screen.heartbeat_text = comms_status->heartbeat_running ? "running" : "stopped";
    logic->boot_screen.link_text = comms_link_state_name(comms_status->link_state);
    logic->boot_screen.heartbeat_count = comms_status->heartbeat_count;
    mcu_status = &comms_status->mcu_status;
    mcu_faults = &comms_status->mcu_faults;

    snprintf(heartbeat_count_text, sizeof(heartbeat_count_text), "%lu", (unsigned long)comms_status->heartbeat_count);
    (void)heartbeat_count_text;

    if (comms_status->last_rx_ms == 0U)
    {
        logic->boot_screen.last_rx_text = "none";
    }
    else
    {
        snprintf(last_rx_text,
                 sizeof(last_rx_text),
                 "type=%u seq=%u len=%u",
                 (unsigned int)comms_status->last_rx_type,
                 (unsigned int)comms_status->last_rx_seq,
                 (unsigned int)comms_status->last_rx_len);
        logic->boot_screen.last_rx_text = last_rx_text;
    }

    if (mcu_status->valid)
    {
        snprintf(mcu_status_text,
                 sizeof(mcu_status_text),
                 "status 0x%02X / target %uC %uC",
                 (unsigned int)mcu_status->status_bits,
                 (unsigned int)mcu_status->mash_target_c,
                 (unsigned int)mcu_status->boil_target_c);
        logic->boot_screen.mcu_status_text = mcu_status_text;

        if ((mcu_status->status_bits & MCU_STATUS_BIT_PRESSURE_VALID) != 0U)
        {
            snprintf(pressure_text,
                     sizeof(pressure_text),
                     "%u counts",
                     (unsigned int)mcu_status->pressure_count);
        }
        else
        {
            snprintf(pressure_text,
                     sizeof(pressure_text),
                     "not valid (%u)",
                     (unsigned int)mcu_status->pressure_count);
        }
        logic->boot_screen.pressure_text = pressure_text;

        snprintf(pump_text,
                 sizeof(pump_text),
                 "mash %u/%s  boil %u/%s",
                 (unsigned int)mcu_status->mash_pump_setpoint,
                 mcu_status->mash_pump_running ? "run" : "off",
                 (unsigned int)mcu_status->boil_pump_setpoint,
                 mcu_status->boil_pump_running ? "run" : "off");
        logic->boot_screen.pump_text = pump_text;

        snprintf(solenoid_text,
                 sizeof(solenoid_text),
                 "brew %s / cool %s / hb %s",
                 (mcu_status->solenoid_state_bits & MCU_SOLENOID_BIT_BREW_INLET) != 0U ? "on" : "off",
                 (mcu_status->solenoid_state_bits & MCU_SOLENOID_BIT_COOLING_INLET) != 0U ? "on" : "off",
                 (mcu_status->status_bits & MCU_STATUS_BIT_HEARTBEAT_ALIVE) != 0U ? "ok" : "lost");
        logic->boot_screen.solenoid_text = solenoid_text;

        if (mcu_status->fault_flags != 0U)
        {
            snprintf(fault_text,
                     sizeof(fault_text),
                     "active 0x%04X",
                     (unsigned int)mcu_status->fault_flags);
            logic->boot_screen.fault_text = fault_text;
        }
        else
        {
            logic->boot_screen.fault_text = "none";
        }
    }
    else
    {
        logic->boot_screen.mcu_status_text = "none";
        logic->boot_screen.pressure_text = "not valid";
        logic->boot_screen.pump_text = "mash off / boil off";
        logic->boot_screen.solenoid_text = "brew off / cool off";
        logic->boot_screen.fault_text = "none";
    }

    if (mcu_faults->valid)
    {
        snprintf(fault_text,
                 sizeof(fault_text),
                 "active 0x%04X latch 0x%04X reason %u",
                 (unsigned int)mcu_faults->active_fault_flags,
                 (unsigned int)mcu_faults->latched_fault_flags,
                 (unsigned int)mcu_faults->primary_reason);
        logic->boot_screen.fault_text = fault_text;
    }
}
