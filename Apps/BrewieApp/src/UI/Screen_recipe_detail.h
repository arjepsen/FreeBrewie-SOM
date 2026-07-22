#ifndef FREEBREWIE_SCREEN_RECIPE_DETAIL_H
#define FREEBREWIE_SCREEN_RECIPE_DETAIL_H

/****************************************************************************************
 * @file Screen_recipe_detail.h
 * @brief Safe old-Brewie-inspired recipe detail screen.
 *
 * Responsibility: show one selected recipe and emit navigation requests.
 * Owns: Recipe-detail LVGL objects and safe navigation/action button presentation.
 * Must not own: recipe storage, recipe editing, brewing start logic, or MCU commands.
 ****************************************************************************************/

#include "Logic/Recipe_types.h"
#include "UI_types.h"
#include "lvgl.h"

typedef struct
{
    ui_action_t action;
    uint32_t value;
    ui_action_handler_t handler;
    void *user_data;
} screen_recipe_detail_button_context_t;

/**
 * Read-only selected-recipe overview state.
 *
 * The button contexts carry the selected recipe id into later screens. This screen displays
 * catalog data only; it does not edit or start brewing directly.
 */
typedef struct
{
    lv_obj_t *screen;
    lv_obj_t *name_label;
    lv_obj_t *style_label;
    recipe_id_t shown_recipe_id;  // Recipe currently shown, used to avoid unchanged label updates.
    screen_recipe_detail_button_context_t back_button_context;
    screen_recipe_detail_button_context_t menu_button_context;
    screen_recipe_detail_button_context_t details_context;
    screen_recipe_detail_button_context_t ingredients_context;
    screen_recipe_detail_button_context_t brewing_context;
    screen_recipe_detail_button_context_t fermentation_context;
    screen_recipe_detail_button_context_t brew_context;
} screen_recipe_detail_t;

void screen_recipe_detail_init(screen_recipe_detail_t *detail,
                               ui_action_handler_t action_handler,
                               void *user_data);
void screen_recipe_detail_show_recipe(screen_recipe_detail_t *detail, const recipe_catalog_entry_t *recipe);

#endif
