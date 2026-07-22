#ifndef FREEBREWIE_SCREEN_BREW_SETUP_H
#define FREEBREWIE_SCREEN_BREW_SETUP_H

/****************************************************************************************
 * @file Screen_brew_setup.h
 * @brief Safe old-Brewie-inspired brew setup scaffold.
 *
 * Responsibility: show pre-brew parameter choices for a selected recipe.
 * Owns: Brew setup LVGL objects, local option state, and navigation callback contexts.
 * Must not own: brewing start permission, active brewing state, or MCU protocol commands.
 ****************************************************************************************/

#include <stdbool.h>

#include "Logic/Recipe_types.h"
#include "UI_types.h"
#include "lvgl.h"

typedef enum
{
    SCREEN_BREW_SETUP_OPTION_WATER_INLET = 0,
    SCREEN_BREW_SETUP_OPTION_COOLING,
    SCREEN_BREW_SETUP_OPTION_COUNT
} screen_brew_setup_option_id_t;

typedef struct
{
    ui_action_t action;
    uint32_t value;
    ui_action_handler_t handler;
    void *user_data;
} screen_brew_setup_nav_context_t;

typedef struct
{
    screen_brew_setup_option_id_t option_id;
    struct screen_brew_setup_t *setup;
} screen_brew_setup_option_context_t;

/**
 * Pre-brew setup screen state.
 *
 * The option toggles are currently local UI state only. They make the old Brewie flow
 * visible without pretending that UI widgets can authorize hardware behavior.
 */
typedef struct screen_brew_setup_t
{
    lv_obj_t *screen;
    lv_obj_t *recipe_label;
    lv_obj_t *option_check_fills[SCREEN_BREW_SETUP_OPTION_COUNT];
    screen_brew_setup_nav_context_t back_button_context;
    screen_brew_setup_nav_context_t checklist_button_context;
    screen_brew_setup_option_context_t option_contexts[SCREEN_BREW_SETUP_OPTION_COUNT];
    recipe_id_t shown_recipe_id;  // Recipe currently shown, used to avoid unchanged label updates.
    bool option_enabled[SCREEN_BREW_SETUP_OPTION_COUNT];  // Display-only until app-level safety routing exists.
} screen_brew_setup_t;

void screen_brew_setup_init(screen_brew_setup_t *setup,
                            ui_action_handler_t action_handler,
                            void *user_data);
void screen_brew_setup_show_recipe(screen_brew_setup_t *setup, const recipe_t *recipe);

#endif
