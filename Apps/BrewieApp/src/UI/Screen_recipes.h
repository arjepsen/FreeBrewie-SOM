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
    ui_action_t action;
    uint32_t value;  // Recipe row callbacks use this as recipe_id; simple navigation leaves it zero.
    ui_action_handler_t handler;
    void *user_data;
} screen_recipes_button_context_t;

/**
 * Recipe chooser state.
 *
 * The row contexts are fixed-size for now because `Recipe_catalog` is still a small static
 * scaffold. Real storage can replace the list source without letting this screen own files.
 */
typedef struct
{
    lv_obj_t *screen;
    screen_recipes_button_context_t back_button_context;
    screen_recipes_button_context_t menu_button_context;
    screen_recipes_button_context_t create_button_context;
    screen_recipes_button_context_t recipe_row_contexts[8];
} screen_recipes_t;

void screen_recipes_init(screen_recipes_t *recipes, ui_action_handler_t action_handler, void *user_data);

#endif
