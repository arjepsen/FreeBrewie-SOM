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
    char name[RECIPE_NAME_MAX_LENGTH];
    recipe_style_t style;
    recipe_calculated_t calculated;
    uint8_t fermentable_count;
    recipe_fermentable_t fermentables[RECIPE_MAX_FERMENTABLES];
    uint8_t hop_count;
    recipe_hop_t hops[RECIPE_MAX_HOPS];
    recipe_brewing_t brewing;
    recipe_fermentation_t fermentation;
} recipe_model_t;

void recipe_model_init(recipe_model_t *recipe);
bool recipe_model_from_draft(const recipe_draft_t *draft, recipe_model_t *recipe);
bool recipe_model_is_complete_for_process_plan(const recipe_model_t *recipe);

#endif
