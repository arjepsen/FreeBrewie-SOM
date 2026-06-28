#ifndef FREEBREWIE_APP_H
#define FREEBREWIE_APP_H

#include <stdbool.h>
#include <stdint.h>

#include "Comms/Comms.h"
#include "Logic/App_logic.h"
#include "Platform/Platform.h"
#include "UI/UI.h"

typedef struct
{
    platform_t platform;
    comms_t comms;
    app_logic_t logic;
    ui_t ui;
    uint64_t last_ui_update_ms;
    bool display_enabled;
} app_t;

bool app_init(app_t *app);
void app_update(app_t *app);
void app_shutdown(app_t *app);

#endif
