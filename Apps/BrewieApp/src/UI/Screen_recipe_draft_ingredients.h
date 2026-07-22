#ifndef FREEBREWIE_SCREEN_RECIPE_DRAFT_INGREDIENTS_H
#define FREEBREWIE_SCREEN_RECIPE_DRAFT_INGREDIENTS_H

/****************************************************************************************
 * @file Screen_recipe_draft_ingredients.h
 * @brief Local-only old-Brewie-style Ingredients screen for a draft recipe.
 *
 * Responsibility: show the read-only Ingredients section shape for an unsaved draft recipe.
 * Owns: draft Ingredients LVGL objects, Fermentables/Hops tabs, and navigation callbacks.
 * Must not own: recipe persistence, ingredient editing, unit conversion, or MCU commands.
 ****************************************************************************************/

#include <stdint.h>

#include "Logic/Recipe_draft.h"
#include "UI_types.h"
#include "lvgl.h"

typedef enum
{
    SCREEN_RECIPE_DRAFT_INGREDIENTS_TAB_FERMENTABLES = 0,
    SCREEN_RECIPE_DRAFT_INGREDIENTS_TAB_HOPS
} screen_recipe_draft_ingredients_tab_t;

typedef struct
{
    ui_action_t action;
    uint32_t value;
    ui_action_handler_t handler;
    void *user_data;
} screen_recipe_draft_ingredients_nav_context_t;

typedef struct
{
    screen_recipe_draft_ingredients_tab_t tab;
    struct screen_recipe_draft_ingredients_t *ingredients;
} screen_recipe_draft_ingredients_tab_context_t;

/**
 * Draft Ingredients screen state.
 *
 * The current screen is read-only but keeps the old Fermentables/Hops split. The tab
 * contexts are stable callback data for switching between the two local pages.
 */
typedef struct screen_recipe_draft_ingredients_t
{
    lv_obj_t *screen;
    lv_obj_t *name_label;
    lv_obj_t *fermentables_button;
    lv_obj_t *hops_button;
    lv_obj_t *fermentables_body;
    lv_obj_t *hops_body;
    /** Rebuilt from the draft model when the screen is shown. */
    lv_obj_t *fermentables_group;
    lv_obj_t *hops_group;
    screen_recipe_draft_ingredients_nav_context_t back_button_context;
    screen_recipe_draft_ingredients_tab_context_t tab_contexts[2];
    /** Last shown draft name, used to avoid unchanged label writes. */
    const char *shown_name;
    screen_recipe_draft_ingredients_tab_t active_tab;
} screen_recipe_draft_ingredients_t;

void screen_recipe_draft_ingredients_init(screen_recipe_draft_ingredients_t *ingredients,
                                          ui_action_handler_t action_handler,
                                          void *user_data);
void screen_recipe_draft_ingredients_show(screen_recipe_draft_ingredients_t *ingredients,
                                          const recipe_draft_t *draft);

#endif
