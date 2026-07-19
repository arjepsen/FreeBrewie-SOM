#ifndef FREEBREWIE_SCREEN_RECIPES_H
#define FREEBREWIE_SCREEN_RECIPES_H

/****************************************************************************************
 * @file Screen_recipes.h
 * @brief Old-Brewie-inspired recipe chooser screen scaffold.
 *
 * Responsibility: show the safe first Recipes screen and emit navigation requests.
 * Owns: Recipes LVGL objects and button callback contexts.
 * Must not own: recipe storage, brewing start logic, or direct hardware actions.
 ****************************************************************************************/

#include "UI_types.h"
#include "Logic/Recipe_types.h"
#include "lvgl.h"

typedef struct
{
    /** Navigation action emitted when this button is clicked. */
    ui_action_t action;
    /** Optional action value, used as recipe_id for recipe detail navigation. */
    uint32_t value;
    /** Callback owned by the UI shell. */
    ui_action_handler_t handler;
    /** Opaque pointer passed back to the callback, normally the ui_t instance. */
    void *user_data;
} screen_recipes_button_context_t;

typedef struct
{
    /** Root LVGL screen object for Recipes. */
    lv_obj_t *screen;
    /** Event callback context for returning to Home. */
    screen_recipes_button_context_t back_button_context;
    /** Event callback context for opening the top-level menu. */
    screen_recipes_button_context_t menu_button_context;
    /** Event callback context for opening the safe recipe-builder scaffold. */
    screen_recipes_button_context_t create_button_context;
    /** Event callback contexts for recipe rows. */
    screen_recipes_button_context_t recipe_row_contexts[8];
} screen_recipes_t;

void screen_recipes_init(screen_recipes_t *recipes, ui_action_handler_t action_handler, void *user_data);

#endif
