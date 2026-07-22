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
    /** Navigation action emitted when this button is clicked. */
    ui_action_t action;
    /** Optional action value, unused for the local draft Fermentation screen. */
    uint32_t value;
    /** Callback owned by the UI shell. */
    ui_action_handler_t handler;
    /** Opaque pointer passed back to the callback, normally the ui_t instance. */
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
    /** Screen instance that owns the reusable number editor. */
    screen_recipe_draft_fermentation_t *fermentation;
    /** Fermentation step index inside Recipe_draft. */
    uint8_t step_index;
    /** Whether this row edits target temperature or duration. */
    screen_recipe_draft_fermentation_edit_kind_t edit_kind;
} screen_recipe_draft_fermentation_edit_context_t;

typedef struct screen_recipe_draft_fermentation_t
{
    /** Root LVGL screen object for the draft Fermentation view. */
    lv_obj_t *screen;
    /** Draft recipe name shown as the header subcaption. */
    lv_obj_t *name_label;
    /** Scrollable body containing rebuilt Fermentation rows. */
    lv_obj_t *body;
    /** Draft model edited by this screen's local-only numeric editor. */
    recipe_draft_t *draft;
    /** Reusable number editor shared by every editable Fermentation value row. */
    ui_number_editor_t number_editor;
    /** Stable callback contexts for temperature rows. */
    screen_recipe_draft_fermentation_edit_context_t temperature_contexts[RECIPE_DRAFT_MAX_FERMENTATION_STEPS];
    /** Stable callback contexts for duration rows. */
    screen_recipe_draft_fermentation_edit_context_t duration_contexts[RECIPE_DRAFT_MAX_FERMENTATION_STEPS];
    /** Step currently being edited by number_editor. */
    uint8_t active_step_index;
    /** Field currently being edited by number_editor. */
    screen_recipe_draft_fermentation_edit_kind_t active_edit_kind;
    /** Event callback context for returning to the draft recipe menu. */
    screen_recipe_draft_fermentation_nav_context_t back_button_context;
    /** Last shown draft name, used to avoid unchanged label writes. */
    const char *shown_name;
} screen_recipe_draft_fermentation_t;

void screen_recipe_draft_fermentation_init(screen_recipe_draft_fermentation_t *fermentation,
                                           recipe_draft_t *draft,
                                           ui_action_handler_t action_handler,
                                           void *user_data);
void screen_recipe_draft_fermentation_show(screen_recipe_draft_fermentation_t *fermentation,
                                           const recipe_draft_t *draft);

#endif
