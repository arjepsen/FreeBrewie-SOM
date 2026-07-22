#include "Recipe_model.h"

#include <string.h>

static bool recipe_model_has_selected_style(const recipe_model_t *recipe);
static bool recipe_model_has_complete_brewing_basics(const recipe_model_t *recipe);
static bool recipe_model_has_complete_fermentation_basics(const recipe_model_t *recipe);

/****************************************************************************************
 * @brief Return true when a committed recipe has a real selected beer style.
 ****************************************************************************************/
static bool recipe_model_has_selected_style(const recipe_model_t *recipe)
{
    return recipe->style.style_name != NULL && strcmp(recipe->style.style_name, "--") != 0;
}

/****************************************************************************************
 * @brief Return true when the first normal brewing fields can form process steps.
 ****************************************************************************************/
static bool recipe_model_has_complete_brewing_basics(const recipe_model_t *recipe)
{
    return recipe->brewing.mash_in_water_dl > 0U &&
           recipe->brewing.mash_in_temperature_c > 0U &&
           recipe->brewing.mash_step_count > 0U &&
           recipe->brewing.boil_time_min > 0U &&
           recipe->brewing.cooling_target_c > 0U;
}

/****************************************************************************************
 * @brief Return true when all active fermentation steps have temperature and duration.
 ****************************************************************************************/
static bool recipe_model_has_complete_fermentation_basics(const recipe_model_t *recipe)
{
    uint8_t index;

    if (recipe->fermentation.step_count == 0U)
    {
        return false;
    }

    for (index = 0U;
         index < recipe->fermentation.step_count && index < RECIPE_MAX_FERMENTATION_STEPS;
         ++index)
    {
        if (recipe->fermentation.steps[index].temperature_c == 0U ||
            recipe->fermentation.steps[index].duration_days == 0U)
        {
            return false;
        }
    }

    return true;
}

/****************************************************************************************
 * @brief Reset a recipe model to empty.
 ****************************************************************************************/
void recipe_model_init(recipe_model_t *recipe)
{
    if (recipe == NULL)
    {
        return;
    }

    memset(recipe, 0, sizeof(*recipe));
}

/****************************************************************************************
 * @brief Commit a complete draft into the first normal recipe model.
 *
 * This creates the boundary we want long-term: editing happens in `Recipe_draft`, while
 * brewing starts from a normal recipe object chosen from the recipe/catalog flow.
 ****************************************************************************************/
bool recipe_model_from_draft(const recipe_draft_t *draft, recipe_model_t *recipe)
{
    recipe_draft_validation_t validation;

    if (draft == NULL || recipe == NULL)
    {
        return false;
    }

    recipe_model_init(recipe);
    recipe_draft_validate(draft, &validation);
    if (!validation.can_brew)
    {
        return false;
    }

    memcpy(recipe->name, draft->name, sizeof(recipe->name));
    recipe->style = draft->style;
    recipe->calculated = draft->calculated;
    recipe->fermentable_count = draft->fermentable_count;
    memcpy(recipe->fermentables, draft->fermentables, sizeof(recipe->fermentables));
    recipe->hop_count = draft->hop_count;
    memcpy(recipe->hops, draft->hops, sizeof(recipe->hops));
    recipe->brewing = draft->brewing;
    recipe->fermentation = draft->fermentation;
    return true;
}

/****************************************************************************************
 * @brief Return true when this recipe can be turned into the first process plan.
 *
 * This still is not hardware preflight. It only checks recipe-data completeness at the
 * recipe boundary, after draft editing and before process planning.
 ****************************************************************************************/
bool recipe_model_is_complete_for_process_plan(const recipe_model_t *recipe)
{
    if (recipe == NULL || recipe->name[0] == '\0')
    {
        return false;
    }

    return recipe_model_has_selected_style(recipe) &&
           recipe->calculated.batch_size_dl > 0U &&
           recipe->fermentable_count > 0U &&
           recipe->hop_count > 0U &&
           recipe_model_has_complete_brewing_basics(recipe) &&
           recipe_model_has_complete_fermentation_basics(recipe);
}
