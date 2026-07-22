#ifndef FREEBREWIE_RECIPE_MODEL_H
#define FREEBREWIE_RECIPE_MODEL_H

/****************************************************************************************
 * @file Recipe_model.h
 * @brief First native brewable recipe domain model.
 *
 * Responsibility: hold a normal recipe after draft editing and before process planning.
 * Owns: compact recipe identity and raw recipe section values.
 * Must not own: LVGL widgets, draft-editing state, recipe storage files, process timing,
 * hardware safety checks, or MCU commands.
 ****************************************************************************************/

#include <stdbool.h>

#include "Recipe_draft.h"
#include "Recipe_types.h"

typedef struct
{
    /** Stable recipe id once catalog/storage exists. Zero means unsaved/not assigned yet. */
    recipe_id_t id;
    /** Recipe name copied into recipe-owned bounded storage. */
    char name[RECIPE_DRAFT_NAME_MAX_LENGTH];
    /** Style values selected for this recipe. */
    recipe_draft_style_t style;
    /** Calculated values or placeholders carried with the recipe. */
    recipe_draft_calculated_t calculated;
    /** Number of active fermentable additions in fermentables[]. */
    uint8_t fermentable_count;
    /** Fermentable additions. */
    recipe_draft_fermentable_t fermentables[RECIPE_DRAFT_MAX_FERMENTABLES];
    /** Number of active hop additions in hops[]. */
    uint8_t hop_count;
    /** Hop additions. */
    recipe_draft_hop_t hops[RECIPE_DRAFT_MAX_HOPS];
    /** Brewing-process values used by the normal recipe editor. */
    recipe_draft_brewing_t brewing;
    /** Fermentation schedule. */
    recipe_draft_fermentation_t fermentation;
} recipe_model_t;

void recipe_model_init(recipe_model_t *recipe);
bool recipe_model_from_draft(const recipe_draft_t *draft, recipe_model_t *recipe);
bool recipe_model_is_complete_for_process_plan(const recipe_model_t *recipe);

#endif
