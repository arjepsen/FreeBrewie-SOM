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
    process_plan_step_kind_t kind;  // Tells the future runner how to interpret the step.
    const char *label;  // Stable display/debug label.
    uint8_t target_temperature_c;  // Degrees C, or zero when not temperature-controlled.
    uint16_t water_amount_dl;  // Deciliters, or zero when no water is added.
    uint16_t duration_min;  // Minutes, or zero for an instant/action marker.
    uint16_t boil_elapsed_min;  // Minutes from boil start, used by hop additions.
    uint8_t source_index;  // Source array index for repeated recipe sections.
} process_plan_step_t;

typedef struct
{
    uint8_t step_count;  // Active entries in steps[].
    process_plan_step_t steps[PROCESS_PLAN_MAX_STEPS];  // Fixed-size for predictable memory.
    bool ready_for_preflight;  // Recipe was complete enough to build this plan.
    const char *status_text;  // Short build result for UI/debug surfaces.
} process_plan_t;

void process_plan_init(process_plan_t *plan);
bool process_plan_build_from_recipe(const recipe_model_t *recipe, process_plan_t *plan);
bool process_plan_build_from_draft(const recipe_draft_t *draft, process_plan_t *plan);

#endif
