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
#include "UI_choice_dialog.h"
#include "UI_dialog.h"
#include "lvgl.h"

typedef enum
{
    SCREEN_RECIPE_BUILDER_FIELD_NAME = 0,
    SCREEN_RECIPE_BUILDER_FIELD_STYLE,
    SCREEN_RECIPE_BUILDER_FIELD_BATCH,
    SCREEN_RECIPE_BUILDER_FIELD_INGREDIENTS,
    SCREEN_RECIPE_BUILDER_FIELD_BREWING,
    SCREEN_RECIPE_BUILDER_FIELD_FERMENTATION,
    SCREEN_RECIPE_BUILDER_FIELD_COUNT
} screen_recipe_builder_field_id_t;

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
    /** Local field selected when this row is clicked. */
    screen_recipe_builder_field_id_t field_id;
    /** Screen instance updated by this row. */
    struct screen_recipe_builder_t *builder;
} screen_recipe_builder_field_context_t;

typedef struct
{
    /** Draft recipe name shown in the non-persistent builder. */
    const char *name;
    /** Draft style shown in the non-persistent builder. */
    const char *style;
    /** Draft batch summary shown in the non-persistent builder. */
    const char *batch;
    /** Draft ingredients summary shown in the non-persistent builder. */
    const char *ingredients;
    /** Draft brewing summary shown in the non-persistent builder. */
    const char *brewing;
    /** Draft fermentation summary shown in the non-persistent builder. */
    const char *fermentation;
} screen_recipe_builder_draft_t;

typedef struct screen_recipe_builder_t
{
    /** Root LVGL screen object for Recipe Builder. */
    lv_obj_t *screen;
    /** Selected field title label. */
    lv_obj_t *selected_title_label;
    /** Selected field detail label. */
    lv_obj_t *selected_body_label;
    /** Event callback context for returning to Recipes. */
    screen_recipe_builder_nav_context_t back_button_context;
    /** Event callback context for opening the top-level menu. */
    screen_recipe_builder_nav_context_t menu_button_context;
    /** Event callback contexts for local field rows. */
    screen_recipe_builder_field_context_t field_contexts[SCREEN_RECIPE_BUILDER_FIELD_COUNT];
    /** Row value labels showing current draft values. */
    lv_obj_t *field_value_labels[SCREEN_RECIPE_BUILDER_FIELD_COUNT];
    /** Local non-persistent draft values shown by this screen. */
    screen_recipe_builder_draft_t draft;
    /** Local-only field editor preview dialog. */
    ui_dialog_t editor_dialog;
    /** Local-only picker dialog used by fields with fixed choices. */
    ui_choice_dialog_t choice_dialog;
    /** Backing storage for the editor preview body text. */
    char editor_dialog_body[192];
    /** Currently selected local field. */
    screen_recipe_builder_field_id_t selected_field_id;
    /** Field affected by the editor dialog's local-only action button. */
    screen_recipe_builder_field_id_t editing_field_id;
} screen_recipe_builder_t;

void screen_recipe_builder_init(screen_recipe_builder_t *builder,
                                ui_action_handler_t action_handler,
                                void *user_data);

#endif
