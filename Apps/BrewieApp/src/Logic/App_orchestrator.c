#include "App_orchestrator.h"

#include "Recipe_catalog.h"

static void app_orchestrator_clear_control_snapshot_preview(app_orchestrator_t *orchestrator);
static bool app_orchestrator_refresh_control_snapshot_preview(app_orchestrator_t *orchestrator);

/****************************************************************************************
 * @brief Mark the current would-be MCU snapshot as unavailable.
 ****************************************************************************************/
static void app_orchestrator_clear_control_snapshot_preview(app_orchestrator_t *orchestrator)
{
    if (orchestrator == NULL)
    {
        return;
    }

    orchestrator->control_snapshot_preview.payload_size = 0U;
    orchestrator->control_snapshot_preview.valid = false;
}

/****************************************************************************************
 * @brief Build a local-only CONTROL_SNAPSHOT preview from the current runner targets.
 *
 * This deliberately stops at payload bytes. The comms layer is still the only place that
 * may frame and transmit protocol messages, and later safety checks must happen before
 * this preview is allowed to become real serial traffic.
 ****************************************************************************************/
static bool app_orchestrator_refresh_control_snapshot_preview(app_orchestrator_t *orchestrator)
{
    const machine_targets_t *targets;

    if (orchestrator == NULL)
    {
        return false;
    }

    app_orchestrator_clear_control_snapshot_preview(orchestrator);
    targets = process_runner_current_targets(&orchestrator->process_runner);
    if (!process_runner_is_active(&orchestrator->process_runner) || targets == NULL)
    {
        return false;
    }

    if (!machine_targets_encode_control_snapshot(
            targets,
            orchestrator->control_snapshot_preview.payload,
            sizeof(orchestrator->control_snapshot_preview.payload)))
    {
        return false;
    }

    orchestrator->control_snapshot_preview.payload_size = MACHINE_TARGET_CONTROL_SNAPSHOT_SIZE;
    orchestrator->control_snapshot_preview.valid = true;
    return true;
}

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
    app_orchestrator_clear_control_snapshot_preview(orchestrator);
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
    app_orchestrator_clear_control_snapshot_preview(orchestrator);
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
        app_orchestrator_clear_control_snapshot_preview(orchestrator);
        orchestrator->status_text = "Process start failed";
        return false;
    }

    if (!app_orchestrator_refresh_control_snapshot_preview(orchestrator))
    {
        orchestrator->state = APP_ORCHESTRATOR_STATE_ERROR;
        orchestrator->status_text = "Snapshot preview failed";
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

/****************************************************************************************
 * @brief Return the current local-only CONTROL_SNAPSHOT payload preview.
 ****************************************************************************************/
const app_control_snapshot_preview_t *
app_orchestrator_get_control_snapshot_preview(const app_orchestrator_t *orchestrator)
{
    if (orchestrator == NULL)
    {
        return NULL;
    }

    return &orchestrator->control_snapshot_preview;
}
