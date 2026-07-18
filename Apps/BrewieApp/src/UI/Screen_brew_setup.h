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
    /** Navigation action emitted when the back/menu button is clicked. */
    ui_action_t action;
    /** Optional action value, currently the selected recipe ID. */
    uint32_t value;
    /** Callback owned by the UI shell. */
    ui_action_handler_t handler;
    /** Opaque pointer passed back to the callback, normally the ui_t instance. */
    void *user_data;
} screen_brew_setup_nav_context_t;

typedef struct
{
    /** Option toggled when this row is clicked. */
    screen_brew_setup_option_id_t option_id;
    /** Screen instance that receives the local toggle. */
    struct screen_brew_setup_t *setup;
} screen_brew_setup_option_context_t;

typedef struct screen_brew_setup_t
{
    /** Root LVGL screen object for Brew Setup. */
    lv_obj_t *screen;
    /** Selected recipe name shown near the top of the setup flow. */
    lv_obj_t *recipe_label;
    /** Local checkbox fill objects shown/hidden when option rows are clicked. */
    lv_obj_t *option_check_fills[SCREEN_BREW_SETUP_OPTION_COUNT];
    /** Event callback context for returning to the selected recipe detail. */
    screen_brew_setup_nav_context_t back_button_context;
    /** Event callback contexts for local option rows. */
    screen_brew_setup_option_context_t option_contexts[SCREEN_BREW_SETUP_OPTION_COUNT];
    /** Recipe currently shown, used to avoid unchanged label updates. */
    recipe_id_t shown_recipe_id;
    /** Local option state. These are display-only until app-level safety routing exists. */
    bool option_enabled[SCREEN_BREW_SETUP_OPTION_COUNT];
} screen_brew_setup_t;

void screen_brew_setup_init(screen_brew_setup_t *setup,
                            ui_action_handler_t action_handler,
                            void *user_data);
void screen_brew_setup_show_recipe(screen_brew_setup_t *setup, const recipe_t *recipe);

#endif
