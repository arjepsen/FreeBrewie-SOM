#include "Process_plan.h"

#include <string.h>

static bool process_plan_append(process_plan_t *plan, const process_plan_step_t *step);
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
 * @brief Add mash-in and mash-rest intent from the friendly draft fields.
 ****************************************************************************************/
static void process_plan_append_mash_steps(const recipe_model_t *recipe, process_plan_t *plan)
{
    uint8_t index;

    (void)process_plan_append(plan,
                              &(process_plan_step_t){
                                  .kind = PROCESS_PLAN_STEP_MASH_IN,
                                  .label = "Mash in",
                                  .target_temperature_c = recipe->brewing.mash_in_temperature_c,
                                  .water_amount_dl = recipe->brewing.mash_in_water_dl,
                                  .duration_min = 0U,
                                  .boil_elapsed_min = 0U,
                                  .source_index = 0U});

    for (index = 0U;
         index < recipe->brewing.mash_step_count && index < RECIPE_MAX_MASH_STEPS;
         ++index)
    {
        (void)process_plan_append(plan,
                                  &(process_plan_step_t){
                                      .kind = PROCESS_PLAN_STEP_MASH_REST,
                                      .label = "Mash rest",
                                      .target_temperature_c =
                                          recipe->brewing.mash_steps[index].temperature_c,
                                      .water_amount_dl = 0U,
                                      .duration_min = recipe->brewing.mash_steps[index].time_min,
                                      .boil_elapsed_min = 0U,
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
                                  .kind = PROCESS_PLAN_STEP_SPARGE,
                                  .label = "Sparge",
                                  .target_temperature_c = recipe->brewing.sparge_temperature_c,
                                  .water_amount_dl = recipe->brewing.sparge_water_dl,
                                  .duration_min = recipe->brewing.sparge_time_min,
                                  .boil_elapsed_min = 0U,
                                  .source_index = 0U});
}

/****************************************************************************************
 * @brief Add boil and hop-addition markers.
 *
 * Hop times in the draft are stored as "minutes before end of boil", matching normal recipe
 * notation. The process plan stores "minutes from start of boil" because that is easier for
 * a future runner to compare against elapsed boil time.
 ****************************************************************************************/
static void process_plan_append_boil_steps(const recipe_model_t *recipe, process_plan_t *plan)
{
    uint8_t index;

    (void)process_plan_append(plan,
                              &(process_plan_step_t){
                                  .kind = PROCESS_PLAN_STEP_BOIL,
                                  .label = "Boil",
                                  .target_temperature_c = 100U,
                                  .water_amount_dl = 0U,
                                  .duration_min = recipe->brewing.boil_time_min,
                                  .boil_elapsed_min = 0U,
                                  .source_index = 0U});

    for (index = 0U; index < recipe->hop_count && index < RECIPE_MAX_HOPS; ++index)
    {
        uint16_t boil_elapsed_min;

        boil_elapsed_min = 0U;
        if (recipe->hops[index].boil_time_min < recipe->brewing.boil_time_min)
        {
            boil_elapsed_min =
                (uint16_t)(recipe->brewing.boil_time_min - recipe->hops[index].boil_time_min);
        }

        (void)process_plan_append(plan,
                                  &(process_plan_step_t){
                                      .kind = PROCESS_PLAN_STEP_HOP_ADDITION,
                                      .label = recipe->hops[index].name,
                                      .target_temperature_c = 0U,
                                      .water_amount_dl = 0U,
                                      .duration_min = 0U,
                                      .boil_elapsed_min = boil_elapsed_min,
                                      .source_index = index});
    }
}

/****************************************************************************************
 * @brief Add cooling intent from the friendly draft cooling target.
 ****************************************************************************************/
static void process_plan_append_cooling_step(const recipe_model_t *recipe, process_plan_t *plan)
{
    (void)process_plan_append(plan,
                              &(process_plan_step_t){
                                  .kind = PROCESS_PLAN_STEP_COOL,
                                  .label = "Cool",
                                  .target_temperature_c = recipe->brewing.cooling_target_c,
                                  .water_amount_dl = 0U,
                                  .duration_min = 0U,
                                  .boil_elapsed_min = 0U,
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
        (void)process_plan_append(plan,
                                  &(process_plan_step_t){
                                      .kind = PROCESS_PLAN_STEP_FERMENTATION,
                                      .label = recipe->fermentation.steps[index].name,
                                      .target_temperature_c =
                                          recipe->fermentation.steps[index].temperature_c,
                                      .water_amount_dl = 0U,
                                      .duration_min =
                                          (uint16_t)(recipe->fermentation.steps[index].duration_days *
                                                     24U * 60U),
                                      .boil_elapsed_min = 0U,
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
    (void)process_plan_append(plan,
                              &(process_plan_step_t){
                                  .kind = PROCESS_PLAN_STEP_COMPLETE,
                                  .label = "Complete",
                                  .target_temperature_c = 0U,
                                  .water_amount_dl = 0U,
                                  .duration_min = 0U,
                                  .boil_elapsed_min = 0U,
                                  .source_index = 0U});

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
