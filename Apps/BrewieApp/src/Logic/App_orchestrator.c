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
    orchestrator->has_selected_recipe = false;
    orchestrator->has_process_plan = false;

    if (!recipe_catalog_build_model(recipe_id, &orchestrator->selected_recipe))
    {
        orchestrator->status_text = "Recipe incomplete";
        return false;
    }

    orchestrator->has_selected_recipe = true;
    orchestrator->selected_recipe_id = recipe_id;

    if (!process_plan_build_from_recipe(&orchestrator->selected_recipe,
                                        &orchestrator->process_plan))
    {
        orchestrator->status_text = orchestrator->process_plan.status_text;
        return false;
    }

    orchestrator->has_process_plan = true;
    orchestrator->status_text = "Process plan ready";
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
        orchestrator->status_text = "Process start failed";
        return false;
    }

    orchestrator->status_text = orchestrator->process_runner.status_text;
    return true;
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
