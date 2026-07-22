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
    ui_action_t action;
    uint32_t value;
    ui_action_handler_t handler;
    void *user_data;
} screen_brew_checklist_nav_context_t;

typedef struct
{
    screen_brew_checklist_item_id_t item_id;
    struct screen_brew_checklist_t *checklist;
} screen_brew_checklist_item_context_t;

/**
 * Local checklist screen state.
 *
 * This currently mirrors the old pre-brew checklist visually. The checks are not safety
 * authority; later app logic must decide whether brewing may actually start.
 */
typedef struct screen_brew_checklist_t
{
    lv_obj_t *screen;
    lv_obj_t *recipe_label;
    lv_obj_t *item_check_fills[SCREEN_BREW_CHECKLIST_ITEM_COUNT];
    screen_brew_checklist_nav_context_t back_button_context;
    screen_brew_checklist_nav_context_t start_button_context;
    screen_brew_checklist_item_context_t item_contexts[SCREEN_BREW_CHECKLIST_ITEM_COUNT];
    recipe_id_t shown_recipe_id;  // Recipe currently shown, used to avoid unchanged label updates.
    bool item_checked[SCREEN_BREW_CHECKLIST_ITEM_COUNT];  // Display-only until app-level validation exists.
} screen_brew_checklist_t;

void screen_brew_checklist_init(screen_brew_checklist_t *checklist,
                                ui_action_handler_t action_handler,
                                void *user_data);
void screen_brew_checklist_show_recipe(screen_brew_checklist_t *checklist, const recipe_t *recipe);

#endif
