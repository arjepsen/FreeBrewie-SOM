#ifndef FREEBREWIE_APP_ORCHESTRATOR_H
#define FREEBREWIE_APP_ORCHESTRATOR_H

/****************************************************************************************
 * @file App_orchestrator.h
 * @brief Logic-side application state orchestrator.
 *
 * Responsibility: Keep app state coherent by routing MCU facts and user requests through
 * the right logic modules.
 * Owns: high-level app state, allowed-action routing, and workflow coordination.
 * Must not own: Widgets, serial transport, protocol parsing, or low-level hardware control.
 ****************************************************************************************/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "Machine_targets.h"
#include "Process_plan.h"
#include "Process_runner.h"
#include "Recipe_model.h"
#include "Recipe_types.h"

typedef enum
{
    APP_ORCHESTRATOR_STATE_IDLE = 0,
    APP_ORCHESTRATOR_STATE_RECIPE_PREPARED,
    APP_ORCHESTRATOR_STATE_PREFLIGHT,
    APP_ORCHESTRATOR_STATE_RUNNING,
    APP_ORCHESTRATOR_STATE_COMPLETE,
    APP_ORCHESTRATOR_STATE_ERROR
} app_orchestrator_state_t;

typedef struct
{
    uint8_t payload[MACHINE_TARGET_CONTROL_SNAPSHOT_SIZE];
    size_t payload_size;
    bool valid;
} app_control_snapshot_preview_t;

typedef struct
{
    recipe_model_t selected_recipe;
    process_plan_t process_plan;
    process_runner_t process_runner;
    app_control_snapshot_preview_t control_snapshot_preview;
    recipe_id_t selected_recipe_id;
    app_orchestrator_state_t state;
    const char *status_text;  // Short app-level workflow status for debug/presentation.
    bool has_selected_recipe;
    bool has_process_plan;
} app_orchestrator_t;

void app_orchestrator_init(app_orchestrator_t *orchestrator);
bool app_orchestrator_prepare_recipe(app_orchestrator_t *orchestrator, recipe_id_t recipe_id);
bool app_orchestrator_enter_preflight(app_orchestrator_t *orchestrator, recipe_id_t recipe_id);
bool app_orchestrator_start_prepared_process(app_orchestrator_t *orchestrator,
                                             recipe_id_t recipe_id);
app_orchestrator_state_t app_orchestrator_get_state(const app_orchestrator_t *orchestrator);
const char *app_orchestrator_get_status_text(const app_orchestrator_t *orchestrator);
const process_runner_t *app_orchestrator_get_process_runner(const app_orchestrator_t *orchestrator);
const app_control_snapshot_preview_t *
app_orchestrator_get_control_snapshot_preview(const app_orchestrator_t *orchestrator);

#endif
