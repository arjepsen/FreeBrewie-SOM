#ifndef FREEBREWIE_APP_LOGIC_H
#define FREEBREWIE_APP_LOGIC_H

#include <stdint.h>

#include "Comms/Comms_types.h"
#include "Status_view_model.h"

typedef struct
{
    /** Diagnostic/status screen model. This is presentation data, not machine control. */
    status_view_model_t status;
} app_logic_t;

void app_logic_init(app_logic_t *logic);
void app_logic_update_fast(app_logic_t *logic, const comms_status_t *comms_status, uint64_t now_ms);

#endif
