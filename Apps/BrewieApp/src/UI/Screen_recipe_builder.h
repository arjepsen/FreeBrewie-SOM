#ifndef FREEBREWIE_SCREEN_RECIPE_BUILDER_H
#define FREEBREWIE_SCREEN_RECIPE_BUILDER_H

/****************************************************************************************
 * @file Screen_recipe_builder.h
 * @brief Safe old-Brewie-inspired recipe builder scaffold.
 *
 * Responsibility: show the first non-persistent recipe data-entry shape.
 * Owns: Recipe-builder LVGL objects, local selected-field state, and back/menu callbacks.
 * Must not own: recipe storage, keyboard/input persistence, brewing start logic, or MCU commands.
 ****************************************************************************************/

#include <stdint.h>

#include "UI_types.h"
#include "UI_dialog.h"
#include "lvgl.h"

typedef struct
{
    /** Navigation action emitted when this button is clicked. */
    ui_action_t action;
    /** Optional action value, unused for the current Recipe Builder scaffold. */
    uint32_t value;
    /** Callback owned by the UI shell. */
    ui_action_handler_t handler;
    /** Opaque pointer passed back to the callback, normally the ui_t instance. */
    void *user_data;
} screen_recipe_builder_nav_context_t;

typedef struct
{
    /** Screen instance updated when the name row is clicked. */
    struct screen_recipe_builder_t *builder;
} screen_recipe_builder_name_context_t;

typedef struct
{
    /** Draft recipe name shown in the non-persistent builder. */
    const char *name;
} screen_recipe_builder_draft_t;

typedef struct screen_recipe_builder_t
{
    /** Root LVGL screen object for Recipe Builder. */
    lv_obj_t *screen;
    /** Label showing the current local draft name. */
    lv_obj_t *name_value_label;
    /** Event callback context for returning to Recipes. */
    screen_recipe_builder_nav_context_t back_button_context;
    /** Event callback context for the disabled future Done button. */
    screen_recipe_builder_nav_context_t done_button_context;
    /** Event callback context for the local draft-name row. */
    screen_recipe_builder_name_context_t name_context;
    /** Local non-persistent draft values shown by this screen. */
    screen_recipe_builder_draft_t draft;
    /** Local-only draft-name dialog. */
    ui_dialog_t editor_dialog;
    /** Backing storage for the editor preview body text. */
    char editor_dialog_body[192];
} screen_recipe_builder_t;

void screen_recipe_builder_init(screen_recipe_builder_t *builder,
                                ui_action_handler_t action_handler,
                                void *user_data);

#endif
