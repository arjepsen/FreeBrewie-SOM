#include "App_logic.h"

#include <string.h>

/****************************************************************************************
 * @brief Initialize app-level logic state.
 *
 * App_logic is the owner of future machine/workflow decisions. It does not format LVGL
 * labels directly; diagnostic status text lives in Status_view_model.
 ****************************************************************************************/
void app_logic_init(app_logic_t *logic)
{
    if (logic == NULL)
    {
        return;
    }

    memset(logic, 0, sizeof(*logic));
    status_view_model_init(&logic->status);
}

/****************************************************************************************
 * @brief Run app logic that must stay cheap enough for the every-loop path.
 *
 * There is no real machine-control logic here yet. Keeping this function gives future
 * brewing, cleaning, and safety-interlock work a clear place to live without mixing it with
 * UI formatting code.
 ****************************************************************************************/
void app_logic_update_fast(app_logic_t *logic, const comms_status_t *comms_status, uint64_t now_ms)
{
    (void)logic;
    (void)comms_status;
    (void)now_ms;
}
