#include "App_logic.h"

#include <stdio.h>
#include <string.h>

#include "Comms/Comms_link.h"

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

    (void)now_ms;

    if (logic == NULL || comms_status == NULL)
    {
        return;
    }

    logic->boot_screen.serial_text = comms_status->serial_ready ? "ready" : "not ready";
    logic->boot_screen.heartbeat_text = comms_status->heartbeat_running ? "running" : "stopped";
    logic->boot_screen.link_text = comms_link_state_name(comms_status->link_state);
    logic->boot_screen.heartbeat_count = comms_status->heartbeat_count;

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
}
