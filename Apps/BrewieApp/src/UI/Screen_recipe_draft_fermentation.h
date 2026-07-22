#ifndef FREEBREWIE_SCREEN_RECIPE_DRAFT_FERMENTATION_H
#define FREEBREWIE_SCREEN_RECIPE_DRAFT_FERMENTATION_H

/****************************************************************************************
 * @file Screen_recipe_draft_fermentation.h
 * @brief Local-only old-Brewie-style Fermentation screen for a draft recipe.
 *
 * Responsibility: show and edit safe local Fermentation schedule values for an unsaved
 * draft recipe.
 * Owns: draft Fermentation LVGL objects, reusable number editor instance, and navigation
 * callbacks.
 * Must not own: recipe persistence, brewing execution, validation, storage, or MCU commands.
 ****************************************************************************************/

#include <stdint.h>

#include "Logic/Recipe_draft.h"
#include "UI_number_editor.h"
#include "UI_types.h"
#include "lvgl.h"

typedef struct
{
    ui_action_t action;
    uint32_t value;
    ui_action_handler_t handler;
    void *user_data;
} screen_recipe_draft_fermentation_nav_context_t;

typedef enum
{
    SCREEN_RECIPE_DRAFT_FERMENTATION_EDIT_TEMPERATURE = 0,
    SCREEN_RECIPE_DRAFT_FERMENTATION_EDIT_DURATION
} screen_recipe_draft_fermentation_edit_kind_t;

typedef struct screen_recipe_draft_fermentation_t screen_recipe_draft_fermentation_t;

typedef struct
{
    screen_recipe_draft_fermentation_t *fermentation;
    uint8_t step_index;
    screen_recipe_draft_fermentation_edit_kind_t edit_kind;
} screen_recipe_draft_fermentation_edit_context_t;

/**
 * Draft Fermentation editor state.
 *
 * Temperature and duration rows share one number editor. Separate fixed context arrays
 * keep callback data stable for every bounded fermentation step.
 */
typedef struct screen_recipe_draft_fermentation_t
{
    lv_obj_t *screen;
    lv_obj_t *name_label;
    lv_obj_t *body;
    recipe_draft_t *draft;
    ui_number_editor_t number_editor;
    screen_recipe_draft_fermentation_edit_context_t temperature_contexts[RECIPE_MAX_FERMENTATION_STEPS];
    screen_recipe_draft_fermentation_edit_context_t duration_contexts[RECIPE_MAX_FERMENTATION_STEPS];
    uint8_t active_step_index;
    screen_recipe_draft_fermentation_edit_kind_t active_edit_kind;
    screen_recipe_draft_fermentation_nav_context_t back_button_context;
    const char *shown_name;  // Last shown draft name, used to avoid unchanged label writes.
} screen_recipe_draft_fermentation_t;

void screen_recipe_draft_fermentation_init(screen_recipe_draft_fermentation_t *fermentation,
                                           recipe_draft_t *draft,
                                           ui_action_handler_t action_handler,
                                           void *user_data);
void screen_recipe_draft_fermentation_show(screen_recipe_draft_fermentation_t *fermentation,
                                           const recipe_draft_t *draft);

#endif
