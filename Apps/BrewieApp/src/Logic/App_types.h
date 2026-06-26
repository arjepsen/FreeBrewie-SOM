#ifndef FREEBREWIE_APP_TYPES_H
#define FREEBREWIE_APP_TYPES_H

#include <stdint.h>

typedef struct
{
    const char *display_text;
    const char *serial_text;
    const char *heartbeat_text;
    const char *last_rx_text;
    const char *link_text;
    const char *mcu_status_text;
    const char *pressure_text;
    const char *pump_text;
    const char *solenoid_text;
    const char *fault_text;
    uint32_t heartbeat_count;
} status_screen_view_model_t;

void status_screen_view_model_init(status_screen_view_model_t *view_model);

#endif
