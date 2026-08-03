#include "App_orchestrator.h"

#include "Recipe_catalog.h"

/****************************************************************************************
 * @brief Reset the app workflow coordinator to idle.
 ****************************************************************************************/
void app_orchestrator_init(app_orchestrator_t *orchestrator)
{
    if (orchestrator == NULL)
    {
        return;
    }

    recipe_model_init(&orchestrator->selected_recipe);
    process_plan_init(&orchestrator->process_plan);
    process_runner_init(&orchestrator->process_runner);
    orchestrator->selected_recipe_id = 0U;
    orchestrator->state = APP_ORCHESTRATOR_STATE_IDLE;
    orchestrator->status_text = "No recipe prepared";
    orchestrator->has_selected_recipe = false;
    orchestrator->has_process_plan = false;
}

/****************************************************************************************
 * @brief Prepare a selected catalog recipe for preflight.
 *
 * This is the first real app-level routing boundary: selected recipe data is converted into
 * a normal recipe model and then into a process plan. It still does not start brewing and
 * still does not send any MCU control snapshots.
 ****************************************************************************************/
bool app_orchestrator_prepare_recipe(app_orchestrator_t *orchestrator, recipe_id_t recipe_id)
{
    if (orchestrator == NULL)
    {
        return false;
    }

    if (orchestrator->has_process_plan && orchestrator->selected_recipe_id == recipe_id)
    {
        orchestrator->status_text = "Process plan ready";
        return true;
    }

    recipe_model_init(&orchestrator->selected_recipe);
    process_plan_init(&orchestrator->process_plan);
    process_runner_init(&orchestrator->process_runner);
    orchestrator->selected_recipe_id = 0U;
    orchestrator->state = APP_ORCHESTRATOR_STATE_IDLE;
    orchestrator->has_selected_recipe = false;
    orchestrator->has_process_plan = false;

    if (!recipe_catalog_build_model(recipe_id, &orchestrator->selected_recipe))
    {
        orchestrator->state = APP_ORCHESTRATOR_STATE_ERROR;
        orchestrator->status_text = "Recipe incomplete";
        return false;
    }

    orchestrator->has_selected_recipe = true;
    orchestrator->selected_recipe_id = recipe_id;

    if (!process_plan_build_from_recipe(&orchestrator->selected_recipe,
                                        &orchestrator->process_plan))
    {
        orchestrator->state = APP_ORCHESTRATOR_STATE_ERROR;
        orchestrator->status_text = orchestrator->process_plan.status_text;
        return false;
    }

    orchestrator->has_process_plan = true;
    orchestrator->state = APP_ORCHESTRATOR_STATE_RECIPE_PREPARED;
    orchestrator->status_text = "Process plan ready";
    return true;
}

/****************************************************************************************
 * @brief Prepare a recipe and mark it as being reviewed in preflight.
 *
 * The checklist/preflight screen is the first user-visible safety gate. It means a plan
 * exists and the user is reviewing setup requirements, but no process step is running yet.
 ****************************************************************************************/
bool app_orchestrator_enter_preflight(app_orchestrator_t *orchestrator, recipe_id_t recipe_id)
{
    if (!app_orchestrator_prepare_recipe(orchestrator, recipe_id))
    {
        return false;
    }

    orchestrator->state = APP_ORCHESTRATOR_STATE_PREFLIGHT;
    orchestrator->status_text = "Preflight checklist";
    return true;
}

/****************************************************************************************
 * @brief Start the passive process runner for a prepared recipe.
 *
 * This function intentionally stops before hardware authority. Later, this is where fault
 * state, startup state, machine state, and preflight permissions should be checked before
 * any target image is allowed to become serial protocol traffic.
 ****************************************************************************************/
bool app_orchestrator_start_prepared_process(app_orchestrator_t *orchestrator,
                                             recipe_id_t recipe_id)
{
    if (orchestrator == NULL)
    {
        return false;
    }

    if (!app_orchestrator_prepare_recipe(orchestrator, recipe_id))
    {
        return false;
    }

    if (!process_runner_start(&orchestrator->process_runner, &orchestrator->process_plan))
    {
        orchestrator->state = APP_ORCHESTRATOR_STATE_ERROR;
        orchestrator->status_text = "Process start failed";
        return false;
    }

    orchestrator->state = APP_ORCHESTRATOR_STATE_RUNNING;
    orchestrator->status_text = orchestrator->process_runner.status_text;
    return true;
}

/****************************************************************************************
 * @brief Return the current high-level workflow state.
 ****************************************************************************************/
app_orchestrator_state_t app_orchestrator_get_state(const app_orchestrator_t *orchestrator)
{
    if (orchestrator == NULL)
    {
        return APP_ORCHESTRATOR_STATE_ERROR;
    }

    return orchestrator->state;
}

/****************************************************************************************
 * @brief Return short workflow text for status/debug presentation.
 ****************************************************************************************/
const char *app_orchestrator_get_status_text(const app_orchestrator_t *orchestrator)
{
    if (orchestrator == NULL || orchestrator->status_text == NULL)
    {
        return "Workflow unavailable";
    }

    return orchestrator->status_text;
}

/****************************************************************************************
 * @brief Return the passive process runner for read-only presentation.
 ****************************************************************************************/
const process_runner_t *app_orchestrator_get_process_runner(const app_orchestrator_t *orchestrator)
{
    if (orchestrator == NULL)
    {
        return NULL;
    }

    return &orchestrator->process_runner;
}
