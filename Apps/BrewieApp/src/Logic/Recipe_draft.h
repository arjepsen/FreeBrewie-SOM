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

#include "Style_catalog.h"

#define RECIPE_DRAFT_NAME_MAX_LENGTH 40U
#define RECIPE_DRAFT_MAX_FERMENTABLES 8U
#define RECIPE_DRAFT_MAX_HOPS 8U
#define RECIPE_DRAFT_MAX_MASH_STEPS 6U
#define RECIPE_DRAFT_MAX_FERMENTATION_STEPS 6U

typedef struct
{
    /** Ingredient name. Points to stable string storage in this first scaffold. */
    const char *name;
    /** Addition amount in grams. */
    uint16_t amount_g;
} recipe_draft_fermentable_t;

typedef struct
{
    /** Hop name. Points to stable string storage in this first scaffold. */
    const char *name;
    /** Addition amount in grams. */
    uint16_t amount_g;
    /** Boil time in minutes before flameout/end of boil. */
    uint16_t boil_time_min;
} recipe_draft_hop_t;

typedef struct
{
    /** Target mash step temperature in degrees C. */
    uint8_t temperature_c;
    /** Step hold time in minutes. */
    uint16_t time_min;
} recipe_draft_mash_step_t;

typedef struct
{
    /** Mash-in water amount in deciliters. */
    uint16_t mash_in_water_dl;
    /** Mash-in target temperature in degrees C. */
    uint8_t mash_in_temperature_c;
    /** Number of active mash steps in mash_steps[]. */
    uint8_t mash_step_count;
    /** Ordered mash steps. */
    recipe_draft_mash_step_t mash_steps[RECIPE_DRAFT_MAX_MASH_STEPS];
    /** Sparge water amount in deciliters. */
    uint16_t sparge_water_dl;
    /** Sparge water temperature in degrees C. */
    uint8_t sparge_temperature_c;
    /** Sparge duration in minutes. */
    uint16_t sparge_time_min;
    /** Total boil time in minutes. */
    uint16_t boil_time_min;
    /** Delayed hopping time in minutes. */
    uint16_t delayed_hopping_min;
    /** Cooling target temperature in degrees C. */
    uint8_t cooling_target_c;
} recipe_draft_brewing_t;

typedef struct
{
    /** Step name shown in the normal recipe editor, for example Primary. */
    const char *name;
    /** Target fermentation temperature in degrees C. */
    uint8_t temperature_c;
    /** Step duration in days. */
    uint16_t duration_days;
} recipe_draft_fermentation_step_t;

typedef struct
{
    /** Number of active fermentation steps in steps[]. */
    uint8_t step_count;
    /** Ordered fermentation schedule. */
    recipe_draft_fermentation_step_t steps[RECIPE_DRAFT_MAX_FERMENTATION_STEPS];
} recipe_draft_fermentation_t;

typedef struct
{
    /** Display name of the selected beer style, or placeholder text when none is selected. */
    const char *style_name;
    /** BJCP or style-guide number text. Kept as text because external systems vary here. */
    const char *style_number;
    /** Style category text. */
    const char *style_category;
    /** Style type text, for example Ale or Lager. */
    const char *style_type;
} recipe_draft_style_t;

typedef struct
{
    /** Expected brewhouse efficiency in whole percent. */
    uint8_t efficiency_percent;
    /** Batch size in deciliters, so 200 means 20.0 L without floating point. */
    uint16_t batch_size_dl;
    /** Estimated alcohol in tenths of a percent, so 52 means 5.2%. */
    uint16_t estimated_abv_tenths;
    /** Estimated color in whole SRM for now. */
    uint16_t estimated_srm;
    /** Estimated bitterness in whole IBU. */
    uint16_t estimated_ibu;
    /** Estimated original gravity as gravity points, so 1050 means 1.050. */
    uint16_t estimated_og_points;
    /** Estimated final gravity as gravity points, so 1011 means 1.011. */
    uint16_t estimated_fg_points;
} recipe_draft_calculated_t;

typedef struct
{
    /** Current draft recipe name stored in bounded draft-owned RAM. */
    char name[RECIPE_DRAFT_NAME_MAX_LENGTH];
    /** True once the user has entered or selected a real name instead of the placeholder. */
    bool has_name;
    /** Beer style fields shown by the draft Details screen. */
    recipe_draft_style_t style;
    /** Calculated or placeholder values shown by the draft Details screen. */
    recipe_draft_calculated_t calculated;
    /** Number of active fermentable additions in fermentables[]. */
    uint8_t fermentable_count;
    /** RAM-only fermentable additions shown by the draft Ingredients screen. */
    recipe_draft_fermentable_t fermentables[RECIPE_DRAFT_MAX_FERMENTABLES];
    /** Number of active hop additions in hops[]. */
    uint8_t hop_count;
    /** RAM-only hop additions shown by the draft Ingredients screen. */
    recipe_draft_hop_t hops[RECIPE_DRAFT_MAX_HOPS];
    /** RAM-only brewing process values shown by the draft Brewing screen. */
    recipe_draft_brewing_t brewing;
    /** RAM-only fermentation schedule shown by the draft Fermentation screen. */
    recipe_draft_fermentation_t fermentation;
    /** True when draft data has changed since it was created or reset. */
    bool dirty;
} recipe_draft_t;

typedef struct
{
    /** True when the draft has enough data for a future preflight/brew-start path. */
    bool can_brew;
    /** Short human-readable reason used by the draft menu status line. */
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
