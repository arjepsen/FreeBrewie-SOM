#include "Recipe_draft.h"

#include <stddef.h>

#define RECIPE_DRAFT_PLACEHOLDER_NAME "Tap to name"
#define RECIPE_DRAFT_PLACEHOLDER_TEXT "--"
#define RECIPE_DRAFT_NO_STYLE_TEXT "No selected style"

static const char *recipe_draft_clean_name(const char *name);
static void recipe_draft_clear_details(recipe_draft_t *draft);
static void recipe_draft_clear_ingredients(recipe_draft_t *draft);
static void recipe_draft_clear_brewing(recipe_draft_t *draft);

/****************************************************************************************
 * @brief Return either a usable name pointer or the draft placeholder.
 *
 * This first draft model stores stable string pointers only. A real keyboard/storage pass
 * will replace this with fixed character buffers or another carefully bounded string
 * strategy, but the UI should already talk to this logic model instead of owning recipe
 * values itself.
 ****************************************************************************************/
static const char *recipe_draft_clean_name(const char *name)
{
    if (name == NULL || name[0] == '\0')
    {
        return RECIPE_DRAFT_PLACEHOLDER_NAME;
    }

    return name;
}

/****************************************************************************************
 * @brief Clear detail fields to stable placeholders.
 *
 * Keeping placeholders in the draft model lets every UI surface show the same current
 * draft state without each screen inventing its own fallback text.
 ****************************************************************************************/
static void recipe_draft_clear_details(recipe_draft_t *draft)
{
    draft->style.style_name = RECIPE_DRAFT_NO_STYLE_TEXT;
    draft->style.style_number = RECIPE_DRAFT_PLACEHOLDER_TEXT;
    draft->style.style_category = RECIPE_DRAFT_PLACEHOLDER_TEXT;
    draft->style.style_type = RECIPE_DRAFT_PLACEHOLDER_TEXT;
    draft->calculated.efficiency_percent = 0U;
    draft->calculated.batch_size_dl = 0U;
    draft->calculated.estimated_abv_tenths = 0U;
    draft->calculated.estimated_srm = 0U;
    draft->calculated.estimated_ibu = 0U;
    draft->calculated.estimated_og_points = 0U;
    draft->calculated.estimated_fg_points = 0U;
}

/****************************************************************************************
 * @brief Clear ingredient arrays without freeing anything.
 *
 * The first draft model uses fixed arrays and stable string pointers. Later storage/import
 * code can fill these arrays from files or web/API data without changing the UI contract.
 ****************************************************************************************/
static void recipe_draft_clear_ingredients(recipe_draft_t *draft)
{
    uint8_t index;

    draft->fermentable_count = 0U;
    draft->hop_count = 0U;
    for (index = 0U; index < RECIPE_DRAFT_MAX_FERMENTABLES; ++index)
    {
        draft->fermentables[index].name = RECIPE_DRAFT_PLACEHOLDER_TEXT;
        draft->fermentables[index].amount_g = 0U;
    }

    for (index = 0U; index < RECIPE_DRAFT_MAX_HOPS; ++index)
    {
        draft->hops[index].name = RECIPE_DRAFT_PLACEHOLDER_TEXT;
        draft->hops[index].amount_g = 0U;
        draft->hops[index].boil_time_min = 0U;
    }
}

/****************************************************************************************
 * @brief Clear brewing-process fields to zero placeholders.
 *
 * Zero means "not set yet" in this draft scaffold. UI screens format zero as "--" so the
 * model can stay numeric and easy to map to BeerXML/BeerJSON/Brewfather concepts later.
 ****************************************************************************************/
static void recipe_draft_clear_brewing(recipe_draft_t *draft)
{
    uint8_t index;

    draft->brewing.mash_in_water_dl = 0U;
    draft->brewing.mash_in_temperature_c = 0U;
    draft->brewing.mash_step_count = 0U;
    draft->brewing.sparge_water_dl = 0U;
    draft->brewing.sparge_temperature_c = 0U;
    draft->brewing.sparge_time_min = 0U;
    draft->brewing.boil_time_min = 0U;
    draft->brewing.delayed_hopping_min = 0U;
    draft->brewing.cooling_target_c = 0U;

    for (index = 0U; index < RECIPE_DRAFT_MAX_MASH_STEPS; ++index)
    {
        draft->brewing.mash_steps[index].temperature_c = 0U;
        draft->brewing.mash_steps[index].time_min = 0U;
    }
}

/****************************************************************************************
 * @brief Initialize a RAM-only recipe draft.
 ****************************************************************************************/
void recipe_draft_init(recipe_draft_t *draft)
{
    recipe_draft_reset(draft);
}

/****************************************************************************************
 * @brief Reset the draft to the first recipe-builder state.
 ****************************************************************************************/
void recipe_draft_reset(recipe_draft_t *draft)
{
    if (draft == NULL)
    {
        return;
    }

    draft->name = RECIPE_DRAFT_PLACEHOLDER_NAME;
    draft->has_name = false;
    recipe_draft_clear_details(draft);
    recipe_draft_clear_ingredients(draft);
    recipe_draft_clear_brewing(draft);
    draft->dirty = false;
}

/****************************************************************************************
 * @brief Store the current draft recipe name.
 *
 * The current UI only offers stable built-in sample strings, so this function stores the
 * pointer directly. When real text entry arrives, this module should become the place that
 * copies into bounded draft-owned storage.
 ****************************************************************************************/
void recipe_draft_set_name(recipe_draft_t *draft, const char *name)
{
    if (draft == NULL)
    {
        return;
    }

    draft->name = recipe_draft_clean_name(name);
    draft->has_name = (draft->name != RECIPE_DRAFT_PLACEHOLDER_NAME);
    draft->dirty = true;
}

/****************************************************************************************
 * @brief Fill the RAM-only draft with a small sample recipe profile.
 *
 * This is temporary UI scaffolding. It gives the draft Details screen real model-owned
 * values to render before keyboard input, style selection, calculation, and storage exist.
 ****************************************************************************************/
void recipe_draft_apply_sample(recipe_draft_t *draft)
{
    if (draft == NULL)
    {
        return;
    }

    draft->name = "Demo Pale Ale";
    draft->has_name = true;
    draft->style.style_name = "American Pale Ale";
    draft->style.style_number = "18B";
    draft->style.style_category = "Pale American Ale";
    draft->style.style_type = "Ale";
    draft->calculated.efficiency_percent = 70U;
    draft->calculated.batch_size_dl = 200U;
    draft->calculated.estimated_abv_tenths = 52U;
    draft->calculated.estimated_srm = 7U;
    draft->calculated.estimated_ibu = 38U;
    draft->calculated.estimated_og_points = 1050U;
    draft->calculated.estimated_fg_points = 1011U;
    draft->fermentable_count = 3U;
    draft->fermentables[0].name = "Pale malt";
    draft->fermentables[0].amount_g = 4200U;
    draft->fermentables[1].name = "Munich malt";
    draft->fermentables[1].amount_g = 450U;
    draft->fermentables[2].name = "Crystal malt";
    draft->fermentables[2].amount_g = 250U;
    draft->hop_count = 3U;
    draft->hops[0].name = "Cascade";
    draft->hops[0].amount_g = 22U;
    draft->hops[0].boil_time_min = 60U;
    draft->hops[1].name = "Centennial";
    draft->hops[1].amount_g = 18U;
    draft->hops[1].boil_time_min = 15U;
    draft->hops[2].name = "Citra";
    draft->hops[2].amount_g = 25U;
    draft->hops[2].boil_time_min = 5U;
    draft->brewing.mash_in_water_dl = 150U;
    draft->brewing.mash_in_temperature_c = 67U;
    draft->brewing.mash_step_count = 2U;
    draft->brewing.mash_steps[0].temperature_c = 66U;
    draft->brewing.mash_steps[0].time_min = 60U;
    draft->brewing.mash_steps[1].temperature_c = 72U;
    draft->brewing.mash_steps[1].time_min = 10U;
    draft->brewing.sparge_water_dl = 120U;
    draft->brewing.sparge_temperature_c = 78U;
    draft->brewing.sparge_time_min = 20U;
    draft->brewing.boil_time_min = 60U;
    draft->brewing.delayed_hopping_min = 10U;
    draft->brewing.cooling_target_c = 20U;
    draft->dirty = true;
}

/****************************************************************************************
 * @brief Return the current visible draft recipe name.
 ****************************************************************************************/
const char *recipe_draft_get_name(const recipe_draft_t *draft)
{
    if (draft == NULL)
    {
        return "";
    }

    return recipe_draft_clean_name(draft->name);
}

/****************************************************************************************
 * @brief Return true once the draft has a real recipe name.
 ****************************************************************************************/
bool recipe_draft_has_name(const recipe_draft_t *draft)
{
    if (draft == NULL)
    {
        return false;
    }

    return draft->has_name;
}

/****************************************************************************************
 * @brief Return true when the user has changed draft data since reset.
 ****************************************************************************************/
bool recipe_draft_is_dirty(const recipe_draft_t *draft)
{
    if (draft == NULL)
    {
        return false;
    }

    return draft->dirty;
}
