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

#include "UI_types.h"
#include "lvgl.h"

typedef enum
{
    SCREEN_RECIPE_DRAFT_INGREDIENTS_TAB_FERMENTABLES = 0,
    SCREEN_RECIPE_DRAFT_INGREDIENTS_TAB_HOPS
} screen_recipe_draft_ingredients_tab_t;

typedef struct
{
    /** Navigation action emitted when this button is clicked. */
    ui_action_t action;
    /** Optional action value, unused for the local draft ingredients screen. */
    uint32_t value;
    /** Callback owned by the UI shell. */
    ui_action_handler_t handler;
    /** Opaque pointer passed back to the callback, normally the ui_t instance. */
    void *user_data;
} screen_recipe_draft_ingredients_nav_context_t;

typedef struct
{
    /** Tab selected when this button is clicked. */
    screen_recipe_draft_ingredients_tab_t tab;
    /** Screen instance updated by the tab button. */
    struct screen_recipe_draft_ingredients_t *ingredients;
} screen_recipe_draft_ingredients_tab_context_t;

typedef struct screen_recipe_draft_ingredients_t
{
    /** Root LVGL screen object for the draft Ingredients view. */
    lv_obj_t *screen;
    /** Draft recipe name shown as the header subcaption. */
    lv_obj_t *name_label;
    /** Fermentables tab button. */
    lv_obj_t *fermentables_button;
    /** Hops tab button. */
    lv_obj_t *hops_button;
    /** Fermentables tab body. */
    lv_obj_t *fermentables_body;
    /** Hops tab body. */
    lv_obj_t *hops_body;
    /** Event callback context for returning to the draft recipe menu. */
    screen_recipe_draft_ingredients_nav_context_t back_button_context;
    /** Event callback contexts for local tab switching. */
    screen_recipe_draft_ingredients_tab_context_t tab_contexts[2];
    /** Last shown draft name, used to avoid unchanged label writes. */
    const char *shown_name;
    /** Currently selected local tab. */
    screen_recipe_draft_ingredients_tab_t active_tab;
} screen_recipe_draft_ingredients_t;

void screen_recipe_draft_ingredients_init(screen_recipe_draft_ingredients_t *ingredients,
                                          ui_action_handler_t action_handler,
                                          void *user_data);
void screen_recipe_draft_ingredients_show(screen_recipe_draft_ingredients_t *ingredients,
                                          const char *draft_name);

#endif
