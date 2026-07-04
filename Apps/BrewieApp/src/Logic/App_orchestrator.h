#ifndef FREEBREWIE_APP_ORCHESTRATOR_H
#define FREEBREWIE_APP_ORCHESTRATOR_H

/****************************************************************************************
 * @file App_orchestrator.h
 * @brief Logic-side application state orchestrator.
 *
 * Responsibility: Keep app state coherent by routing MCU facts and user requests through
 * the right logic modules.
 * Owns: High-level app state, allowed-action routing, and future workflow coordination.
 * Does not own: Widgets, serial transport, or protocol parsing.
 ****************************************************************************************/

#include <stdint.h>

#include "Comms/Comms_types.h"
#include "Status_view_model.h"

typedef struct
{
    /** Diagnostic/status screen model. This is presentation data, not machine control. */
    status_view_model_t status;
} app_orchestrator_t;

void app_orchestrator_init(app_orchestrator_t *orchestrator);
void app_orchestrator_update_fast(app_orchestrator_t *orchestrator,
                                  const comms_status_t *comms_status,
                                  uint64_t now_ms);

#endif
