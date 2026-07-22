#include "Recipe_draft.h"

#include <stddef.h>
#include <string.h>

#define RECIPE_DRAFT_PLACEHOLDER_NAME "Tap to name"
#define RECIPE_DRAFT_PLACEHOLDER_TEXT "--"

static void recipe_draft_clear_details(recipe_draft_t *draft);
static void recipe_draft_clear_ingredients(recipe_draft_t *draft);
static void recipe_draft_clear_brewing(recipe_draft_t *draft);
static void recipe_draft_clear_fermentation(recipe_draft_t *draft);

/****************************************************************************************
 * @brief Clear detail fields to stable placeholders.
 *
 * Keeping placeholders in the draft model lets every UI surface show the same current
 * draft state without each screen inventing its own fallback text.
 ****************************************************************************************/
static void recipe_draft_clear_details(recipe_draft_t *draft)
{
    draft->style.style_name = RECIPE_DRAFT_PLACEHOLDER_TEXT;
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
 * @brief Clear the first RAM-only fermentation schedule.
 *
 * Fermentation is already stored as ordered steps because the future advanced editor should
 * eventually expose more explicit process steps. The friendly editor can still show these
 * as simple named rows.
 ****************************************************************************************/
static void recipe_draft_clear_fermentation(recipe_draft_t *draft)
{
    uint8_t index;

    draft->fermentation.step_count = 3U;
    draft->fermentation.steps[0].name = "Primary";
    draft->fermentation.steps[1].name = "Secondary";
    draft->fermentation.steps[2].name = "Conditioning";
    for (index = 0U; index < RECIPE_DRAFT_MAX_FERMENTATION_STEPS; ++index)
    {
        if (draft->fermentation.steps[index].name == NULL)
        {
            draft->fermentation.steps[index].name = RECIPE_DRAFT_PLACEHOLDER_TEXT;
        }
        draft->fermentation.steps[index].temperature_c = 0U;
        draft->fermentation.steps[index].duration_days = 0U;
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

    draft->name[0] = '\0';
    draft->has_name = false;
    recipe_draft_clear_details(draft);
    recipe_draft_clear_ingredients(draft);
    recipe_draft_clear_brewing(draft);
    recipe_draft_clear_fermentation(draft);
    draft->dirty = false;
}

/****************************************************************************************
 * @brief Store the current draft recipe name.
 *
 * The text is copied into bounded draft-owned storage. This keeps future storage,
 * BeerXML/BeerJSON import, Brewfather sync, and UI text entry from pointing at temporary
 * screen-owned buffers.
 ****************************************************************************************/
void recipe_draft_set_name(recipe_draft_t *draft, const char *name)
{
    size_t name_length;

    if (draft == NULL)
    {
        return;
    }

    if (name == NULL)
    {
        draft->name[0] = '\0';
        draft->has_name = false;
        draft->dirty = true;
        return;
    }

    name_length = strlen(name);
    if (name_length >= RECIPE_DRAFT_NAME_MAX_LENGTH)
    {
        name_length = RECIPE_DRAFT_NAME_MAX_LENGTH - 1U;
    }

    memcpy(draft->name, name, name_length);
    draft->name[name_length] = '\0';
    draft->has_name = (draft->name[0] != '\0');
    draft->dirty = true;
}

/****************************************************************************************
 * @brief Copy one catalog style into the RAM-only draft.
 *
 * The draft stores only the selected values. It does not own or search the style catalog,
 * which keeps style data loading separate from editable recipe state.
 ****************************************************************************************/
void recipe_draft_set_style(recipe_draft_t *draft, const style_catalog_style_t *style)
{
    if (draft == NULL || style == NULL)
    {
        return;
    }

    draft->style.style_name = style->style_name;
    draft->style.style_number = style->style_number;
    draft->style.style_category = style->style_category;
    draft->style.style_type = style->style_type;
    draft->dirty = true;
}

/****************************************************************************************
 * @brief Store the selected batch size as deciliters.
 *
 * Keeping this in deciliters avoids floating point in the model while still showing one
 * decimal place in the UI, for example 200 means 20.0 L.
 ****************************************************************************************/
void recipe_draft_set_batch_size_dl(recipe_draft_t *draft, uint16_t batch_size_dl)
{
    if (draft == NULL)
    {
        return;
    }

    draft->calculated.batch_size_dl = batch_size_dl;
    draft->dirty = true;
}

/****************************************************************************************
 * @brief Store the mash-in water amount as deciliters.
 *
 * This is an editable brewing-process field, not a command to fill water. The actual MCU
 * action path will come later through validation and brewing start/preflight logic.
 ****************************************************************************************/
void recipe_draft_set_mash_in_water_dl(recipe_draft_t *draft, uint16_t mash_in_water_dl)
{
    if (draft == NULL)
    {
        return;
    }

    draft->brewing.mash_in_water_dl = mash_in_water_dl;
    draft->dirty = true;
}

/****************************************************************************************
 * @brief Store the mash-in target temperature.
 ****************************************************************************************/
void recipe_draft_set_mash_in_temperature_c(recipe_draft_t *draft, uint8_t mash_in_temperature_c)
{
    if (draft == NULL)
    {
        return;
    }

    draft->brewing.mash_in_temperature_c = mash_in_temperature_c;
    draft->dirty = true;
}

/****************************************************************************************
 * @brief Store the sparge water amount as deciliters.
 ****************************************************************************************/
void recipe_draft_set_sparge_water_dl(recipe_draft_t *draft, uint16_t sparge_water_dl)
{
    if (draft == NULL)
    {
        return;
    }

    draft->brewing.sparge_water_dl = sparge_water_dl;
    draft->dirty = true;
}

/****************************************************************************************
 * @brief Store the sparge water temperature.
 ****************************************************************************************/
void recipe_draft_set_sparge_temperature_c(recipe_draft_t *draft, uint8_t sparge_temperature_c)
{
    if (draft == NULL)
    {
        return;
    }

    draft->brewing.sparge_temperature_c = sparge_temperature_c;
    draft->dirty = true;
}

/****************************************************************************************
 * @brief Store the sparge duration in minutes.
 ****************************************************************************************/
void recipe_draft_set_sparge_time_min(recipe_draft_t *draft, uint16_t sparge_time_min)
{
    if (draft == NULL)
    {
        return;
    }

    draft->brewing.sparge_time_min = sparge_time_min;
    draft->dirty = true;
}

/****************************************************************************************
 * @brief Store the boil duration in minutes.
 ****************************************************************************************/
void recipe_draft_set_boil_time_min(recipe_draft_t *draft, uint16_t boil_time_min)
{
    if (draft == NULL)
    {
        return;
    }

    draft->brewing.boil_time_min = boil_time_min;
    draft->dirty = true;
}

/****************************************************************************************
 * @brief Store the delayed hopping timing in minutes.
 ****************************************************************************************/
void recipe_draft_set_delayed_hopping_min(recipe_draft_t *draft, uint16_t delayed_hopping_min)
{
    if (draft == NULL)
    {
        return;
    }

    draft->brewing.delayed_hopping_min = delayed_hopping_min;
    draft->dirty = true;
}

/****************************************************************************************
 * @brief Store the cooling target temperature.
 ****************************************************************************************/
void recipe_draft_set_cooling_target_c(recipe_draft_t *draft, uint8_t cooling_target_c)
{
    if (draft == NULL)
    {
        return;
    }

    draft->brewing.cooling_target_c = cooling_target_c;
    draft->dirty = true;
}

/****************************************************************************************
 * @brief Store one fermentation step target temperature.
 ****************************************************************************************/
void recipe_draft_set_fermentation_temperature_c(recipe_draft_t *draft,
                                                 uint8_t step_index,
                                                 uint8_t temperature_c)
{
    if (draft == NULL || step_index >= draft->fermentation.step_count)
    {
        return;
    }

    draft->fermentation.steps[step_index].temperature_c = temperature_c;
    draft->dirty = true;
}

/****************************************************************************************
 * @brief Store one fermentation step duration in days.
 ****************************************************************************************/
void recipe_draft_set_fermentation_duration_days(recipe_draft_t *draft,
                                                 uint8_t step_index,
                                                 uint16_t duration_days)
{
    if (draft == NULL || step_index >= draft->fermentation.step_count)
    {
        return;
    }

    draft->fermentation.steps[step_index].duration_days = duration_days;
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

    recipe_draft_set_name(draft, "Demo Pale Ale");
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
    draft->fermentation.steps[0].temperature_c = 19U;
    draft->fermentation.steps[0].duration_days = 10U;
    draft->fermentation.steps[1].temperature_c = 20U;
    draft->fermentation.steps[1].duration_days = 4U;
    draft->fermentation.steps[2].temperature_c = 4U;
    draft->fermentation.steps[2].duration_days = 7U;
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

    if (!draft->has_name)
    {
        return RECIPE_DRAFT_PLACEHOLDER_NAME;
    }

    return draft->name;
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
