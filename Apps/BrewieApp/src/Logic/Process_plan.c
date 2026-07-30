#include "Process_plan.h"

#include <stdbool.h>
#include <string.h>

static bool process_plan_append(process_plan_t *plan, const process_plan_step_t *step);
static void process_plan_append_complete_step(process_plan_t *plan);
static void process_plan_append_mash_steps(const recipe_model_t *recipe, process_plan_t *plan);
static void process_plan_append_sparge_step(const recipe_model_t *recipe, process_plan_t *plan);
static void process_plan_append_boil_steps(const recipe_model_t *recipe, process_plan_t *plan);
static void process_plan_append_cooling_step(const recipe_model_t *recipe, process_plan_t *plan);
static void process_plan_append_fermentation_steps(const recipe_model_t *recipe,
                                                   process_plan_t *plan);

/****************************************************************************************
 * @brief Append one step if fixed plan storage still has room.
 *
 * The plan uses a fixed array instead of heap allocation. That keeps memory predictable on
 * the SOM and makes the later active brewing runner easier to reason about.
 ****************************************************************************************/
static bool process_plan_append(process_plan_t *plan, const process_plan_step_t *step)
{
    if (plan == NULL || step == NULL || plan->step_count >= PROCESS_PLAN_MAX_STEPS)
    {
        return false;
    }

    plan->steps[plan->step_count] = *step;
    plan->step_count++;
    return true;
}

/****************************************************************************************
 * @brief Add the explicit end marker used by later runners and debug views.
 ****************************************************************************************/
static void process_plan_append_complete_step(process_plan_t *plan)
{
    (void)process_plan_append(plan,
                              &(process_plan_step_t){
                                  .kind = PROCESS_PLAN_STEP_COMPLETE,
                                  .label = "Complete",
                                  .targets = {0},
                                  .exit = {.conditions = PROCESS_PLAN_EXIT_IMMEDIATE},
                                  .source_index = 0U});
}

/****************************************************************************************
 * @brief Add mash-in and mash-rest target segments from the friendly recipe fields.
 *
 * The mash-in segment demonstrates the process-plan direction: it asks for a fill volume
 * and a temperature, then exits when both are satisfied. The runtime/control layer later
 * decides exactly how to open valves, settle sensors, and avoid overshoot.
 ****************************************************************************************/
static void process_plan_append_mash_steps(const recipe_model_t *recipe, process_plan_t *plan)
{
    uint8_t index;

    (void)process_plan_append(plan,
                              &(process_plan_step_t){
                                  .kind = PROCESS_PLAN_STEP_TARGET_SEGMENT,
                                  .label = "Mash in",
                                  .targets =
                                      {
                                          .set_mash_fill_volume = true,
                                          .mash_fill_volume_dl = recipe->brewing.mash_in_water_dl,
                                          .set_mash_temperature = true,
                                          .mash_temperature_c =
                                              recipe->brewing.mash_in_temperature_c,
                                      },
                                  .exit =
                                      {
                                          .conditions = PROCESS_PLAN_EXIT_MASH_VOLUME |
                                                        PROCESS_PLAN_EXIT_MASH_TEMPERATURE,
                                          .mash_volume_dl = recipe->brewing.mash_in_water_dl,
                                          .mash_temperature_c =
                                              recipe->brewing.mash_in_temperature_c,
                                      },
                                  .source_index = 0U});

    for (index = 0U;
         index < recipe->brewing.mash_step_count && index < RECIPE_MAX_MASH_STEPS;
         ++index)
    {
        (void)process_plan_append(plan,
                                  &(process_plan_step_t){
                                      .kind = PROCESS_PLAN_STEP_TARGET_SEGMENT,
                                      .label = "Mash rest",
                                      .targets =
                                          {
                                              .set_mash_temperature = true,
                                              .mash_temperature_c =
                                                  recipe->brewing.mash_steps[index].temperature_c,
                                          },
                                      .exit =
                                          {
                                              .conditions = PROCESS_PLAN_EXIT_DURATION,
                                              .duration_min =
                                                  recipe->brewing.mash_steps[index].time_min,
                                          },
                                      .source_index = index});
    }
}

/****************************************************************************************
 * @brief Add sparge intent when the friendly draft includes sparge values.
 ****************************************************************************************/
static void process_plan_append_sparge_step(const recipe_model_t *recipe, process_plan_t *plan)
{
    if (recipe->brewing.sparge_water_dl == 0U &&
        recipe->brewing.sparge_temperature_c == 0U &&
        recipe->brewing.sparge_time_min == 0U)
    {
        return;
    }

    (void)process_plan_append(plan,
                              &(process_plan_step_t){
                                  .kind = PROCESS_PLAN_STEP_TARGET_SEGMENT,
                                  .label = "Sparge",
                                  .targets =
                                      {
                                          .set_mash_fill_volume = true,
                                          .mash_fill_volume_dl = recipe->brewing.sparge_water_dl,
                                          .set_mash_temperature = true,
                                          .mash_temperature_c =
                                              recipe->brewing.sparge_temperature_c,
                                      },
                                  .exit =
                                      {
                                          .conditions = PROCESS_PLAN_EXIT_DURATION,
                                          .duration_min = recipe->brewing.sparge_time_min,
                                      },
                                  .source_index = 0U});
}

/****************************************************************************************
 * @brief Add boil target segments and hop-addition prompts.
 *
 * Hop times in the draft are stored as "minutes before end of boil", matching normal recipe
 * notation. The generated process segments turn those into "wait this long while keeping the
 * boil target active, then prompt for this addition."
 ****************************************************************************************/
static void process_plan_append_boil_steps(const recipe_model_t *recipe, process_plan_t *plan)
{
    uint8_t index;
    uint8_t emitted_count;
    bool emitted[RECIPE_MAX_HOPS];
    uint16_t previous_elapsed_min;

    (void)process_plan_append(plan,
                              &(process_plan_step_t){
                                  .kind = PROCESS_PLAN_STEP_TARGET_SEGMENT,
                                  .label = "Boil heat",
                                  .targets =
                                      {
                                          .set_boil_temperature = true,
                                          .boil_temperature_c = 100U,
                                      },
                                  .exit =
                                      {
                                          .conditions = PROCESS_PLAN_EXIT_BOIL_TEMPERATURE,
                                          .boil_temperature_c = 100U,
                                      },
                                  .source_index = 0U});

    memset(emitted, 0, sizeof(emitted));
    previous_elapsed_min = 0U;
    emitted_count = 0U;

    while (emitted_count < recipe->hop_count && emitted_count < RECIPE_MAX_HOPS)
    {
        uint8_t next_index;
        uint16_t next_elapsed_min;
        bool found_next;

        next_index = 0U;
        next_elapsed_min = recipe->brewing.boil_time_min;
        found_next = false;

        for (index = 0U; index < recipe->hop_count && index < RECIPE_MAX_HOPS; ++index)
        {
            uint16_t boil_elapsed_min;

            if (emitted[index])
            {
                continue;
            }

            boil_elapsed_min = 0U;
            if (recipe->hops[index].boil_time_min < recipe->brewing.boil_time_min)
            {
                boil_elapsed_min =
                    (uint16_t)(recipe->brewing.boil_time_min - recipe->hops[index].boil_time_min);
            }

            if (!found_next || boil_elapsed_min < next_elapsed_min)
            {
                next_index = index;
                next_elapsed_min = boil_elapsed_min;
                found_next = true;
            }
        }

        if (!found_next)
        {
            break;
        }

        if (next_elapsed_min > previous_elapsed_min)
        {
            (void)process_plan_append(plan,
                                      &(process_plan_step_t){
                                          .kind = PROCESS_PLAN_STEP_TARGET_SEGMENT,
                                          .label = "Boil wait",
                                          .targets = {0},
                                          .exit =
                                              {
                                                  .conditions = PROCESS_PLAN_EXIT_DURATION,
                                                  .duration_min =
                                                      (uint16_t)(next_elapsed_min -
                                                                 previous_elapsed_min),
                                              },
                                          .source_index = 0U});
        }

        (void)process_plan_append(plan,
                                  &(process_plan_step_t){
                                      .kind = PROCESS_PLAN_STEP_PROMPT,
                                      .label = recipe->hops[next_index].name,
                                      .targets = {0},
                                      .exit = {.conditions = PROCESS_PLAN_EXIT_USER_CONFIRM},
                                      .source_index = next_index});

        emitted[next_index] = true;
        emitted_count++;
        previous_elapsed_min = next_elapsed_min;
    }

    if (recipe->brewing.boil_time_min > previous_elapsed_min)
    {
        (void)process_plan_append(plan,
                                  &(process_plan_step_t){
                                      .kind = PROCESS_PLAN_STEP_TARGET_SEGMENT,
                                      .label = "Boil finish",
                                      .targets = {0},
                                      .exit =
                                          {
                                              .conditions = PROCESS_PLAN_EXIT_DURATION,
                                              .duration_min =
                                                  (uint16_t)(recipe->brewing.boil_time_min -
                                                             previous_elapsed_min),
                                          },
                                      .source_index = 0U});
    }
}

/****************************************************************************************
 * @brief Add cooling target from the friendly recipe cooling target.
 ****************************************************************************************/
static void process_plan_append_cooling_step(const recipe_model_t *recipe, process_plan_t *plan)
{
    (void)process_plan_append(plan,
                              &(process_plan_step_t){
                                  .kind = PROCESS_PLAN_STEP_TARGET_SEGMENT,
                                  .label = "Cool",
                                  .targets =
                                      {
                                          .set_cooling_temperature = true,
                                          .cooling_temperature_c = recipe->brewing.cooling_target_c,
                                      },
                                  .exit =
                                      {
                                          .conditions = PROCESS_PLAN_EXIT_COOLING_TEMPERATURE,
                                          .cooling_temperature_c = recipe->brewing.cooling_target_c,
                                      },
                                  .source_index = 0U});
}

/****************************************************************************************
 * @brief Add fermentation notes to the plan without implying MCU control.
 *
 * Brewie does not control fermentation hardware in the current design. These steps still
 * belong in the process plan because they are part of the recipe process and future web/UI
 * views should be able to show the complete intended schedule.
 ****************************************************************************************/
static void process_plan_append_fermentation_steps(const recipe_model_t *recipe,
                                                   process_plan_t *plan)
{
    uint8_t index;

    for (index = 0U;
         index < recipe->fermentation.step_count && index < RECIPE_MAX_FERMENTATION_STEPS;
         ++index)
    {
        uint16_t duration_min;

        duration_min = (uint16_t)(recipe->fermentation.steps[index].duration_days * 24U * 60U);
        (void)process_plan_append(plan,
                                  &(process_plan_step_t){
                                      .kind = PROCESS_PLAN_STEP_TARGET_SEGMENT,
                                      .label = recipe->fermentation.steps[index].name,
                                      .targets = {0},
                                      .exit =
                                          {
                                              .conditions = PROCESS_PLAN_EXIT_DURATION,
                                              .duration_min = duration_min,
                                          },
                                      .source_index = index});
    }
}

/****************************************************************************************
 * @brief Reset a process plan to empty.
 ****************************************************************************************/
void process_plan_init(process_plan_t *plan)
{
    if (plan == NULL)
    {
        return;
    }

    memset(plan, 0, sizeof(*plan));
    plan->status_text = "No process plan";
}

/****************************************************************************************
 * @brief Build the first ordered process plan from a normal selected recipe.
 *
 * This is a planning boundary, not a brew-start boundary. A successful result means the
 * draft can be expressed as ordered SOM-side intent. It does not mean the machine is ready,
 * safe, filled, closed, or allowed to heat.
 ****************************************************************************************/
bool process_plan_build_from_recipe(const recipe_model_t *recipe, process_plan_t *plan)
{
    if (plan == NULL)
    {
        return false;
    }

    process_plan_init(plan);
    if (!recipe_model_is_complete_for_process_plan(recipe))
    {
        plan->status_text = "Recipe incomplete";
        return false;
    }

    process_plan_append_mash_steps(recipe, plan);
    process_plan_append_sparge_step(recipe, plan);
    process_plan_append_boil_steps(recipe, plan);
    process_plan_append_cooling_step(recipe, plan);
    process_plan_append_fermentation_steps(recipe, plan);
    process_plan_append_complete_step(plan);

    plan->ready_for_preflight = true;
    plan->status_text = "Process plan ready";
    return true;
}

/****************************************************************************************
 * @brief Temporary adapter from draft editing to process planning.
 *
 * Long-term, brewing should start from a selected recipe in the catalog/list flow. This
 * adapter exists only while the recipe editor has no real storage/commit path yet.
 ****************************************************************************************/
bool process_plan_build_from_draft(const recipe_draft_t *draft, process_plan_t *plan)
{
    recipe_model_t recipe;

    if (!recipe_model_from_draft(draft, &recipe))
    {
        if (plan != NULL)
        {
            process_plan_init(plan);
            plan->status_text = "Draft incomplete";
        }
        return false;
    }

    return process_plan_build_from_recipe(&recipe, plan);
}
