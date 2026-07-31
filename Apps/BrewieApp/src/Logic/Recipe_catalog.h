#ifndef FREEBREWIE_RECIPE_CATALOG_H
#define FREEBREWIE_RECIPE_CATALOG_H

/****************************************************************************************
 * @file Recipe_catalog.h
 * @brief Read-only recipe catalog interface.
 *
 * Responsibility: provide recipe data to UI/workflow layers without exposing storage or
 * LVGL details.
 * Owns: the current static recipe list until real recipe persistence is designed.
 * Must not own: screen widgets, brewing execution, or MCU commands.
 ****************************************************************************************/

#include <stdbool.h>
#include <stddef.h>

#include "Recipe_model.h"
#include "Recipe_types.h"

size_t recipe_catalog_count();
const recipe_catalog_entry_t *recipe_catalog_get_by_index(size_t index);
const recipe_catalog_entry_t *recipe_catalog_find_by_id(recipe_id_t recipe_id);
bool recipe_catalog_build_model(recipe_id_t recipe_id, recipe_model_t *recipe);

#endif
