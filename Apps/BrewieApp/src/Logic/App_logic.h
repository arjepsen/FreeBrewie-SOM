#ifndef FREEBREWIE_APP_LOGIC_H
#define FREEBREWIE_APP_LOGIC_H

#include <stdbool.h>
#include <stdint.h>

#include "App_types.h"
#include "Comms/Comms_types.h"

typedef struct
{
    boot_screen_view_model_t boot_screen;
} app_logic_t;

void app_logic_init(app_logic_t *logic);
void app_logic_set_serial_ready(app_logic_t *logic, bool serial_ready);
void app_logic_update(app_logic_t *logic, const comms_status_t *comms_status, uint64_t now_ms);

#endif
