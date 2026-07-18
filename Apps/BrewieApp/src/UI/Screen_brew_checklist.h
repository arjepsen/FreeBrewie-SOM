#ifndef FREEBREWIE_SCREEN_BREW_CHECKLIST_H
#define FREEBREWIE_SCREEN_BREW_CHECKLIST_H

/****************************************************************************************
 * @file Screen_brew_checklist.h
 * @brief Safe old-Brewie-inspired brewing checklist scaffold.
 *
 * Responsibility: show pre-brew checklist items for a selected recipe.
 * Owns: Checklist LVGL objects, local checklist state, and navigation callback contexts.
 * Must not own: checklist validation authority, active brewing state, or MCU commands.
 ****************************************************************************************/

#include <stdbool.h>

#include "Logic/Recipe_types.h"
#include "UI_types.h"
#include "lvgl.h"

typedef enum
{
    SCREEN_BREW_CHECKLIST_ITEM_RECIPE = 0,
    SCREEN_BREW_CHECKLIST_ITEM_WATER,
    SCREEN_BREW_CHECKLIST_ITEM_COOLING,
    SCREEN_BREW_CHECKLIST_ITEM_DRAIN,
    SCREEN_BREW_CHECKLIST_ITEM_COUNT
} screen_brew_checklist_item_id_t;

typedef struct
{
    /** Navigation action emitted when the back button is clicked. */
    ui_action_t action;
    /** Optional action value, currently the selected recipe ID. */
    uint32_t value;
    /** Callback owned by the UI shell. */
    ui_action_handler_t handler;
    /** Opaque pointer passed back to the callback, normally the ui_t instance. */
    void *user_data;
} screen_brew_checklist_nav_context_t;

typedef struct
{
    /** Checklist item toggled when this row is clicked. */
    screen_brew_checklist_item_id_t item_id;
    /** Screen instance that receives the local toggle. */
    struct screen_brew_checklist_t *checklist;
} screen_brew_checklist_item_context_t;

typedef struct screen_brew_checklist_t
{
    /** Root LVGL screen object for the brewing checklist. */
    lv_obj_t *screen;
    /** Selected recipe name shown near the top of the checklist. */
    lv_obj_t *recipe_label;
    /** Local checkbox fill objects shown/hidden when checklist rows are clicked. */
    lv_obj_t *item_check_fills[SCREEN_BREW_CHECKLIST_ITEM_COUNT];
    /** Event callback context for returning to Brew Setup. */
    screen_brew_checklist_nav_context_t back_button_context;
    /** Event callback contexts for local checklist rows. */
    screen_brew_checklist_item_context_t item_contexts[SCREEN_BREW_CHECKLIST_ITEM_COUNT];
    /** Recipe currently shown, used to avoid unchanged label updates. */
    recipe_id_t shown_recipe_id;
    /** Local checklist state. These are display-only until app-level validation exists. */
    bool item_checked[SCREEN_BREW_CHECKLIST_ITEM_COUNT];
} screen_brew_checklist_t;

void screen_brew_checklist_init(screen_brew_checklist_t *checklist,
                                ui_action_handler_t action_handler,
                                void *user_data);
void screen_brew_checklist_show_recipe(screen_brew_checklist_t *checklist, const recipe_t *recipe);

#endif
