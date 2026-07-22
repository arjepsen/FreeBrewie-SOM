#ifndef FREEBREWIE_RECIPE_DRAFT_H
#define FREEBREWIE_RECIPE_DRAFT_H

/****************************************************************************************
 * @file Recipe_draft.h
 * @brief In-memory editable recipe draft model.
 *
 * Responsibility: own the current unsaved recipe values while the user is building or
 * editing a recipe.
 * Owns: RAM-only draft fields, dirty flags, and completion/validation helpers.
 * Must not own: LVGL widgets, recipe files, import/export mapping, web transport, brewing
 * runtime state, or MCU commands.
 ****************************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include "Recipe_types.h"
#include "Style_catalog.h"

typedef struct
{
    char name[RECIPE_NAME_MAX_LENGTH];  // Bounded draft-owned name buffer; storage/catalog can copy from here later.
    bool has_name;
    recipe_style_t style;
    recipe_calculated_t calculated;
    uint8_t fermentable_count;
    recipe_fermentable_t fermentables[RECIPE_MAX_FERMENTABLES];
    uint8_t hop_count;
    recipe_hop_t hops[RECIPE_MAX_HOPS];
    recipe_brewing_t brewing;
    recipe_fermentation_t fermentation;
    bool dirty;
} recipe_draft_t;

/** Lightweight completeness result for UI guidance; not hardware safety validation. */
typedef struct
{
    bool can_brew;
    const char *status_text;
} recipe_draft_validation_t;

void recipe_draft_init(recipe_draft_t *draft);
void recipe_draft_reset(recipe_draft_t *draft);
void recipe_draft_set_name(recipe_draft_t *draft, const char *name);
void recipe_draft_set_style(recipe_draft_t *draft, const style_catalog_style_t *style);
void recipe_draft_set_batch_size_dl(recipe_draft_t *draft, uint16_t batch_size_dl);
void recipe_draft_set_mash_in_water_dl(recipe_draft_t *draft, uint16_t mash_in_water_dl);
void recipe_draft_set_mash_in_temperature_c(recipe_draft_t *draft, uint8_t mash_in_temperature_c);
void recipe_draft_set_sparge_water_dl(recipe_draft_t *draft, uint16_t sparge_water_dl);
void recipe_draft_set_sparge_temperature_c(recipe_draft_t *draft, uint8_t sparge_temperature_c);
void recipe_draft_set_sparge_time_min(recipe_draft_t *draft, uint16_t sparge_time_min);
void recipe_draft_set_boil_time_min(recipe_draft_t *draft, uint16_t boil_time_min);
void recipe_draft_set_delayed_hopping_min(recipe_draft_t *draft, uint16_t delayed_hopping_min);
void recipe_draft_set_cooling_target_c(recipe_draft_t *draft, uint8_t cooling_target_c);
void recipe_draft_set_fermentation_temperature_c(recipe_draft_t *draft,
                                                 uint8_t step_index,
                                                 uint8_t temperature_c);
void recipe_draft_set_fermentation_duration_days(recipe_draft_t *draft,
                                                 uint8_t step_index,
                                                 uint16_t duration_days);
void recipe_draft_apply_sample(recipe_draft_t *draft);
const char *recipe_draft_get_name(const recipe_draft_t *draft);
bool recipe_draft_has_name(const recipe_draft_t *draft);
bool recipe_draft_is_dirty(const recipe_draft_t *draft);
void recipe_draft_validate(const recipe_draft_t *draft, recipe_draft_validation_t *validation);

#endif
