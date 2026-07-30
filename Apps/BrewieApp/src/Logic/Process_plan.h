#ifndef FREEBREWIE_PROCESS_PLAN_H
#define FREEBREWIE_PROCESS_PLAN_H

/****************************************************************************************
 * @file Process_plan.h
 * @brief Ordered SOM-side executable process intent.
 *
 * Responsibility: hold the common process instruction shape produced by basic, advanced,
 * and future expert recipe editors.
 * Owns: local process-step data used before runtime execution and MCU target generation.
 * Must not own: LVGL widgets, recipe storage, protocol frames, or direct MCU commands.
 ****************************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include "Recipe_model.h"

#define PROCESS_PLAN_MAX_STEPS 64U

typedef enum
{
    PROCESS_PLAN_STEP_TARGET_SEGMENT = 0,
    PROCESS_PLAN_STEP_PROMPT,
    PROCESS_PLAN_STEP_COMPLETE
} process_plan_step_kind_t;

typedef enum
{
    PROCESS_PLAN_EXIT_IMMEDIATE = 0U,
    PROCESS_PLAN_EXIT_DURATION = 1U << 0U,
    PROCESS_PLAN_EXIT_MASH_TEMPERATURE = 1U << 1U,
    PROCESS_PLAN_EXIT_BOIL_TEMPERATURE = 1U << 2U,
    PROCESS_PLAN_EXIT_MASH_VOLUME = 1U << 3U,
    PROCESS_PLAN_EXIT_COOLING_TEMPERATURE = 1U << 4U,
    PROCESS_PLAN_EXIT_USER_CONFIRM = 1U << 5U
} process_plan_exit_condition_t;

typedef struct
{
    bool set_mash_fill_volume;
    uint16_t mash_fill_volume_dl;
    bool set_mash_temperature;
    uint8_t mash_temperature_c;
    bool set_boil_temperature;
    uint8_t boil_temperature_c;
    bool set_cooling_temperature;
    uint8_t cooling_temperature_c;
    bool set_heater_duty_limit;
    uint8_t heater_duty_limit_percent;
    uint32_t valve_open_mask;  // Future expert/web field: valves requested open by this step.
    uint32_t valve_close_mask;  // Future expert/web field: valves requested closed by this step.
    uint32_t pump_on_mask;  // Future expert/web field: pumps requested on by this step.
    uint32_t pump_off_mask;  // Future expert/web field: pumps requested off by this step.
} process_plan_target_changes_t;

typedef struct
{
    uint16_t conditions;  // process_plan_exit_condition_t flags; all active conditions must complete.
    uint16_t duration_min;
    uint16_t mash_volume_dl;
    uint8_t mash_temperature_c;
    uint8_t boil_temperature_c;
    uint8_t cooling_temperature_c;
} process_plan_exit_t;

typedef struct
{
    process_plan_step_kind_t kind;
    const char *label;  // Stable display/debug label.
    process_plan_target_changes_t targets;
    process_plan_exit_t exit;
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
