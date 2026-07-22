#ifndef FREEBREWIE_SCREEN_RECIPE_DRAFT_DETAILS_H
#define FREEBREWIE_SCREEN_RECIPE_DRAFT_DETAILS_H

/****************************************************************************************
 * @file Screen_recipe_draft_details.h
 * @brief Local-only old-Brewie-style details screen for a draft recipe.
 *
 * Responsibility: show and edit draft Details values that are safe to change locally.
 * Owns: draft Details LVGL objects, the local style picker overlay, and navigation callbacks.
 * Must not own: recipe persistence, full style database/import mapping, validation, or MCU commands.
 ****************************************************************************************/

#include <stdint.h>

#include "Logic/Recipe_draft.h"
#include "Logic/Style_catalog.h"
#include "UI_number_editor.h"
#include "UI_types.h"
#include "lvgl.h"

typedef struct
{
    ui_action_t action;
    uint32_t value;
    ui_action_handler_t handler;
    void *user_data;
} screen_recipe_draft_details_nav_context_t;

typedef struct screen_recipe_draft_details_t screen_recipe_draft_details_t;

typedef struct
{
    screen_recipe_draft_details_t *details;
    /** Index into Style_catalog's fixed-size style cache. */
    uint8_t option_index;
} screen_recipe_draft_details_style_context_t;

/**
 * Draft Details editor state.
 *
 * The screen shows draft-owned values and owns only presentation objects, the local style
 * picker, and the reusable batch-size editor. The style picker uses fixed option contexts
 * because the current style catalog is a bounded in-memory list.
 */
typedef struct screen_recipe_draft_details_t
{
    lv_obj_t *screen;
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
    lv_obj_t *style_row;
    lv_obj_t *batch_size_row;
    lv_obj_t *style_picker_overlay;
    ui_number_editor_t batch_size_editor;
    recipe_draft_t *draft;
    screen_recipe_draft_details_style_context_t style_option_contexts[STYLE_CATALOG_MAX_STYLES];
    screen_recipe_draft_details_nav_context_t back_button_context;
    /** Last shown draft name, used to avoid unchanged label writes. */
    const char *shown_name;
} screen_recipe_draft_details_t;

void screen_recipe_draft_details_init(screen_recipe_draft_details_t *details,
                                      recipe_draft_t *draft,
                                      ui_action_handler_t action_handler,
                                      void *user_data);
void screen_recipe_draft_details_show(screen_recipe_draft_details_t *details,
                                      const recipe_draft_t *draft);

#endif
