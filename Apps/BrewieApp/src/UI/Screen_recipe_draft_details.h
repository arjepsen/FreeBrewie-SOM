#ifndef FREEBREWIE_SCREEN_RECIPE_DRAFT_DETAILS_H
#define FREEBREWIE_SCREEN_RECIPE_DRAFT_DETAILS_H

/****************************************************************************************
 * @file Screen_recipe_draft_details.h
 * @brief Local-only old-Brewie-style details screen for a draft recipe.
 *
 * Responsibility: show the read-only Details section shape for an unsaved draft recipe.
 * Owns: draft Details LVGL objects and navigation callbacks.
 * Must not own: recipe persistence, style database selection, validation, or MCU commands.
 ****************************************************************************************/

#include <stdint.h>

#include "Logic/Recipe_draft.h"
#include "UI_types.h"
#include "lvgl.h"

typedef struct
{
    /** Navigation action emitted when this button is clicked. */
    ui_action_t action;
    /** Optional action value, unused for the local draft details screen. */
    uint32_t value;
    /** Callback owned by the UI shell. */
    ui_action_handler_t handler;
    /** Opaque pointer passed back to the callback, normally the ui_t instance. */
    void *user_data;
} screen_recipe_draft_details_nav_context_t;

typedef struct screen_recipe_draft_details_t
{
    /** Root LVGL screen object for the draft Details view. */
    lv_obj_t *screen;
    /** Draft recipe name shown as the header subcaption. */
    lv_obj_t *name_label;
    /** Style value labels, updated from the draft model before the screen is shown. */
    lv_obj_t *style_name_label;
    lv_obj_t *style_number_label;
    lv_obj_t *style_category_label;
    lv_obj_t *style_type_label;
    /** Calculated value labels, updated from the draft model before the screen is shown. */
    lv_obj_t *efficiency_label;
    lv_obj_t *batch_size_label;
    lv_obj_t *abv_label;
    lv_obj_t *srm_label;
    lv_obj_t *ibu_label;
    lv_obj_t *og_label;
    lv_obj_t *fg_label;
    /** Event callback context for returning to the draft recipe menu. */
    screen_recipe_draft_details_nav_context_t back_button_context;
    /** Event callback context for the disabled future Modify button. */
    screen_recipe_draft_details_nav_context_t modify_button_context;
    /** Last shown draft name, used to avoid unchanged label writes. */
    const char *shown_name;
} screen_recipe_draft_details_t;

void screen_recipe_draft_details_init(screen_recipe_draft_details_t *details,
                                      ui_action_handler_t action_handler,
                                      void *user_data);
void screen_recipe_draft_details_show(screen_recipe_draft_details_t *details,
                                      const recipe_draft_t *draft);

#endif
