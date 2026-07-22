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
    /** Navigation action emitted when this button is clicked. */
    ui_action_t action;
    /** Optional action value, unused for the local draft Brewing screen. */
    uint32_t value;
    /** Callback owned by the UI shell. */
    ui_action_handler_t handler;
    /** Opaque pointer passed back to the callback, normally the ui_t instance. */
    void *user_data;
} screen_recipe_draft_brewing_nav_context_t;

typedef struct screen_recipe_draft_brewing_t
{
    /** Root LVGL screen object for the draft Brewing view. */
    lv_obj_t *screen;
    /** Draft recipe name shown as the header subcaption. */
    lv_obj_t *name_label;
    /** Scrollable body containing rebuilt Brewing rows. */
    lv_obj_t *body;
    /** Draft model edited by this screen's local-only numeric editor. */
    recipe_draft_t *draft;
    /** Reusable number editor, first used for Mash water. */
    ui_number_editor_t mash_water_editor;
    /** Event callback context for returning to the draft recipe menu. */
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
