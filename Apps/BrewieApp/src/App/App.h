#ifndef FREEBREWIE_APP_H
#define FREEBREWIE_APP_H

#include <stdbool.h>
#include <stdint.h>

#include "UI/UI.h"
#include "Protocol.h"
#include "Serial.h"

typedef struct
{
    serial_port_t serial;
    protocol_rx_state_t protocol_rx;
    protocol_sender_t protocol_sender;
    ui_state_t ui;
    uint64_t last_heartbeat_ms;
    uint64_t last_ui_refresh_ms;
    uint64_t last_rx_ms;
    uint32_t heartbeat_count;
    bool serial_ok;
    bool display_ok;
    bool heartbeat_running;
} app_t;

bool app_init(app_t *app);
void app_update(app_t *app);
void app_shutdown(app_t *app);

#endif
