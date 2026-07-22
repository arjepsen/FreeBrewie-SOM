#ifndef FREEBREWIE_SCREEN_RECIPE_BUILDER_H
#define FREEBREWIE_SCREEN_RECIPE_BUILDER_H

/****************************************************************************************
 * @file Screen_recipe_builder.h
 * @brief Safe old-Brewie-inspired recipe builder scaffold.
 *
 * Responsibility: show the first non-persistent recipe data-entry shape.
 * Owns: Recipe-builder LVGL objects, selected-field state, and back/menu callbacks.
 * Must not own: recipe storage, keyboard/input persistence, brewing start logic, or MCU commands.
 ****************************************************************************************/

#include <stdint.h>

#include "Logic/Recipe_draft.h"
#include "UI_types.h"
#include "UI_text_editor.h"
#include "lvgl.h"

typedef struct
{
    ui_action_t action;
    uint32_t value;
    ui_action_handler_t handler;
    void *user_data;
} screen_recipe_builder_nav_context_t;

typedef struct
{
    struct screen_recipe_builder_t *builder;
} screen_recipe_builder_name_context_t;

/**
 * First recipe-builder screen state.
 *
 * The screen owns LVGL objects and a reusable text editor, but the actual recipe values
 * live in `recipe_draft_t` so future storage/web import work can reuse the model.
 */
typedef struct screen_recipe_builder_t
{
    lv_obj_t *screen;
    lv_obj_t *name_value_label;
    lv_obj_t *done_button;
    screen_recipe_builder_nav_context_t back_button_context;
    screen_recipe_builder_nav_context_t done_button_context;
    screen_recipe_builder_name_context_t name_context;
    recipe_draft_t *draft;
    ui_text_editor_t name_editor;
} screen_recipe_builder_t;

void screen_recipe_builder_init(screen_recipe_builder_t *builder,
                                recipe_draft_t *draft,
                                ui_action_handler_t action_handler,
                                void *user_data);
void screen_recipe_builder_show(screen_recipe_builder_t *builder);

#endif
