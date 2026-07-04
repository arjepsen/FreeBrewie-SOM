#include "App_orchestrator.h"

#include <string.h>

/****************************************************************************************
 * @brief Initialize app-level logic state.
 *
 * App_orchestrator will eventually tie together MCU facts, user requests, faults, and
 * workflow modules. It does not format LVGL labels directly; diagnostic status text lives
 * in Status_view_model.
 ****************************************************************************************/
void app_orchestrator_init(app_orchestrator_t *orchestrator)
{
    if (orchestrator == NULL)
    {
        return;
    }

    memset(orchestrator, 0, sizeof(*orchestrator));
    status_view_model_init(&orchestrator->status);
}

/****************************************************************************************
 * @brief Run app logic that must stay cheap enough for the every-loop path.
 *
 * There is no real machine-control logic here yet. Keeping this function gives future
 * brewing, cleaning, and safety-interlock work a clear place to live without mixing it with
 * UI formatting code.
 ****************************************************************************************/
void app_orchestrator_update_fast(app_orchestrator_t *orchestrator,
                                  const comms_status_t *comms_status,
                                  uint64_t now_ms)
{
    (void)orchestrator;
    (void)comms_status;
    (void)now_ms;
}
