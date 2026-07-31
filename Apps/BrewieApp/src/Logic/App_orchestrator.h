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

#include "Process_plan.h"
#include "Process_runner.h"
#include "Recipe_model.h"
#include "Recipe_types.h"

typedef struct
{
    recipe_model_t selected_recipe;
    process_plan_t process_plan;
    process_runner_t process_runner;
    recipe_id_t selected_recipe_id;
    const char *status_text;  // Short app-level workflow status for debug/presentation.
    bool has_selected_recipe;
    bool has_process_plan;
} app_orchestrator_t;

void app_orchestrator_init(app_orchestrator_t *orchestrator);
bool app_orchestrator_prepare_recipe(app_orchestrator_t *orchestrator, recipe_id_t recipe_id);
bool app_orchestrator_start_prepared_process(app_orchestrator_t *orchestrator,
                                             recipe_id_t recipe_id);
const process_runner_t *app_orchestrator_get_process_runner(const app_orchestrator_t *orchestrator);

#endif
