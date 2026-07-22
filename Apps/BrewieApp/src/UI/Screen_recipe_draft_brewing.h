#ifndef FREEBREWIE_SCREEN_RECIPE_DRAFT_BREWING_H
#define FREEBREWIE_SCREEN_RECIPE_DRAFT_BREWING_H

/****************************************************************************************
 * @file Screen_recipe_draft_brewing.h
 * @brief Local-only old-Brewie-style Brewing screen for a draft recipe.
 *
 * Responsibility: show and edit safe local Brewing values for an unsaved draft recipe.
 * Owns: draft Brewing LVGL objects, reusable number editor instance, and navigation callbacks.
 * Must not own: recipe persistence, brewing execution, validation, or MCU commands.
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
} screen_recipe_draft_brewing_nav_context_t;

typedef enum
{
    SCREEN_RECIPE_DRAFT_BREWING_EDIT_MASH_WATER = 0,
    SCREEN_RECIPE_DRAFT_BREWING_EDIT_MASH_TEMP,
    SCREEN_RECIPE_DRAFT_BREWING_EDIT_SPARGE_WATER,
    SCREEN_RECIPE_DRAFT_BREWING_EDIT_SPARGE_TEMP,
    SCREEN_RECIPE_DRAFT_BREWING_EDIT_SPARGE_TIME,
    SCREEN_RECIPE_DRAFT_BREWING_EDIT_BOIL_TIME,
    SCREEN_RECIPE_DRAFT_BREWING_EDIT_DELAYED_HOPS,
    SCREEN_RECIPE_DRAFT_BREWING_EDIT_COOLING_TARGET,
    SCREEN_RECIPE_DRAFT_BREWING_EDIT_COUNT
} screen_recipe_draft_brewing_edit_field_t;

typedef struct screen_recipe_draft_brewing_t screen_recipe_draft_brewing_t;

typedef struct
{
    screen_recipe_draft_brewing_t *brewing;
    screen_recipe_draft_brewing_edit_field_t field;
} screen_recipe_draft_brewing_edit_context_t;

/**
 * Draft Brewing editor state.
 *
 * Editable rows all share one number editor. The per-row contexts tell the callback which
 * draft field to read/write, keeping row creation data-driven and avoiding separate
 * callback functions for every value.
 */
typedef struct screen_recipe_draft_brewing_t
{
    lv_obj_t *screen;
    lv_obj_t *name_label;
    lv_obj_t *body;
    recipe_draft_t *draft;
    ui_number_editor_t number_editor;
    screen_recipe_draft_brewing_edit_context_t edit_contexts[SCREEN_RECIPE_DRAFT_BREWING_EDIT_COUNT];
    screen_recipe_draft_brewing_edit_field_t active_edit_field;
    screen_recipe_draft_brewing_nav_context_t back_button_context;
    /** Last shown draft name, used to avoid unchanged label writes. */
    const char *shown_name;
} screen_recipe_draft_brewing_t;

void screen_recipe_draft_brewing_init(screen_recipe_draft_brewing_t *brewing,
                                      recipe_draft_t *draft,
                                      ui_action_handler_t action_handler,
                                      void *user_data);
void screen_recipe_draft_brewing_show(screen_recipe_draft_brewing_t *brewing,
                                      const recipe_draft_t *draft);

#endif
