#include "Brewing_process_view_model.h"

#include <string.h>

/****************************************************************************************
 * @brief Fill the brewing-process view model with safe startup presentation.
 ****************************************************************************************/
void brewing_process_view_model_init(brewing_process_view_model_t *view_model)
{
    if (view_model == NULL)
    {
        return;
    }

    memset(view_model, 0, sizeof(*view_model));
    view_model->current_stage = BREWING_PROCESS_STAGE_WATER;
    view_model->state_text = "Waiting for MCU";
    view_model->detail_text = "Live values unavailable";
    view_model->progress_percent = 0U;
    view_model->pause_enabled = false;
    view_model->stop_enabled = false;
}

/****************************************************************************************
 * @brief Update read-only Active Brewing presentation from current app-facing facts.
 *
 * This is intentionally conservative. Until a real brewing workflow exists, the model only
 * shows that the UI has live MCU data and keeps process controls disabled.
 ****************************************************************************************/
void brewing_process_view_model_update(brewing_process_view_model_t *view_model,
                                       const status_screen_view_model_t *status_view_model,
                                       app_orchestrator_state_t orchestrator_state,
                                       const char *orchestrator_status_text,
                                       const process_runner_t *process_runner)
{
    if (view_model == NULL || status_view_model == NULL)
    {
        return;
    }

    view_model->current_stage = BREWING_PROCESS_STAGE_WATER;
    view_model->progress_percent = 0U;
    view_model->pause_enabled = false;
    view_model->stop_enabled = false;

    if (orchestrator_state == APP_ORCHESTRATOR_STATE_PREFLIGHT)
    {
        view_model->state_text = "Preflight";
        view_model->detail_text = "Checklist ready";
    }
    else if (process_runner != NULL && process_runner_is_active(process_runner))
    {
        const process_plan_step_t *step;

        step = process_runner_current_step(process_runner);
        view_model->state_text = (orchestrator_status_text != NULL) ?
                                     orchestrator_status_text :
                                     "Process active";
        view_model->detail_text = (step != NULL && step->label != NULL) ? step->label : "Waiting";
    }
    else if (orchestrator_state == APP_ORCHESTRATOR_STATE_RECIPE_PREPARED)
    {
        view_model->state_text = "Recipe prepared";
        view_model->detail_text = "Ready for checklist";
    }
    else if (orchestrator_state == APP_ORCHESTRATOR_STATE_ERROR)
    {
        view_model->state_text = "Workflow error";
        view_model->detail_text = (orchestrator_status_text != NULL) ?
                                      orchestrator_status_text :
                                      "Recipe or process invalid";
    }
    else if (status_view_model->machine.mcu_status_valid)
    {
        view_model->state_text = "MCU live";
        view_model->detail_text = "Ready for process state";
    }
    else
    {
        view_model->state_text = "Waiting for MCU";
        view_model->detail_text = "Live values unavailable";
    }
}
