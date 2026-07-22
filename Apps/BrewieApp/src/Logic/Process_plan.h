#ifndef FREEBREWIE_PROCESS_PLAN_H
#define FREEBREWIE_PROCESS_PLAN_H

/****************************************************************************************
 * @file Process_plan.h
 * @brief Ordered SOM-side process intent derived from a selected recipe.
 *
 * Responsibility: convert friendly recipe fields into ordered brewing-process steps.
 * Owns: local process-step data used before hardware preflight and MCU target generation.
 * Must not own: LVGL widgets, recipe storage, protocol frames, or direct MCU commands.
 ****************************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include "Recipe_model.h"

#define PROCESS_PLAN_MAX_STEPS 32U

typedef enum
{
    PROCESS_PLAN_STEP_MASH_IN = 0,
    PROCESS_PLAN_STEP_MASH_REST,
    PROCESS_PLAN_STEP_SPARGE,
    PROCESS_PLAN_STEP_BOIL,
    PROCESS_PLAN_STEP_HOP_ADDITION,
    PROCESS_PLAN_STEP_COOL,
    PROCESS_PLAN_STEP_FERMENTATION,
    PROCESS_PLAN_STEP_COMPLETE
} process_plan_step_kind_t;

typedef struct
{
    /** Step type. This tells the future runner how to interpret the remaining fields. */
    process_plan_step_kind_t kind;
    /** Short stable display/debug label. */
    const char *label;
    /** Target temperature in degrees C, or zero when the step has no temperature target. */
    uint8_t target_temperature_c;
    /** Water amount in deciliters, or zero when the step does not add water. */
    uint16_t water_amount_dl;
    /** Step duration in minutes, or zero for an instant/action marker. */
    uint16_t duration_min;
    /** Time from the start of boil, used by hop additions. */
    uint16_t boil_elapsed_min;
    /** Original draft array index when the step came from a repeated draft section. */
    uint8_t source_index;
} process_plan_step_t;

typedef struct
{
    /** Number of active entries in steps[]. */
    uint8_t step_count;
    /** Ordered process intent. Fixed-size to keep runtime memory predictable. */
    process_plan_step_t steps[PROCESS_PLAN_MAX_STEPS];
    /** True when the selected recipe was complete enough to build this first plan. */
    bool ready_for_preflight;
    /** Short human-readable build result for UI/debug surfaces. */
    const char *status_text;
} process_plan_t;

void process_plan_init(process_plan_t *plan);
bool process_plan_build_from_recipe(const recipe_model_t *recipe, process_plan_t *plan);
bool process_plan_build_from_draft(const recipe_draft_t *draft, process_plan_t *plan);

#endif
