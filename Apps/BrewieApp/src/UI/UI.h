#ifndef FREEBREWIE_UI_H
#define FREEBREWIE_UI_H

/****************************************************************************************
 * @file UI.h
 * @brief Top-level LVGL UI shell and navigation owner.
 *
 * Responsibility: initialize screens and apply UI navigation.
 * Owns: screen instances, menu/placeholder targets, and deferred screen changes.
 * Does not own: comms, protocol parsing, or machine safety decisions.
 ****************************************************************************************/

#include <stdbool.h>

#include "Screen_brew_setup.h"
#include "Screen_home.h"
#include "Screen_manual.h"
#include "Screen_menu.h"
#include "Screen_recipe_detail.h"
#include "Screen_recipe_section.h"
#include "Screen_recipes.h"
#include "Screen_settings.h"
#include "Screen_status.h"
#include "UI_types.h"

typedef struct ui_t ui_t;

struct ui_t
{
    /** Always-created Home screen. */
    screen_home_t home;
    /** Bring-up/status screen with comms and touch diagnostics. */
    screen_status_t status;
    /** Safe first recipe chooser scaffold. */
    screen_recipes_t recipes;
    /** Safe selected-recipe detail screen. */
    screen_recipe_detail_t recipe_detail;
    /** Safe selected-recipe section placeholder screen. */
    screen_recipe_section_t recipe_section;
    /** Safe brew setup scaffold, lazy-created on first use. */
    screen_brew_setup_t brew_setup;
    /** True after the Brew Setup scaffold has been created. */
    bool brew_setup_created;
    /** Full-screen navigation menu. */
    screen_menu_t menu;
    /** Safe Manual/Cleaning scaffold, lazy-created on first use. */
    screen_manual_t manual;
    /** True after the Manual/Cleaning scaffold has been created. */
    bool manual_created;
    /** Safe Settings scaffold, lazy-created on first use. */
    screen_settings_t settings;
    /** True after the Settings scaffold has been created. */
    bool settings_created;
    /** Screen currently loaded into LVGL. */
    ui_screen_id_t current_screen;
    /** Deferred navigation target requested from an LVGL event callback. */
    ui_screen_id_t pending_screen;
    /** Optional value for the pending navigation, currently recipe_id for recipe detail. */
    uint32_t pending_value;
    /** Optional section id for recipe-section navigation. */
    recipe_section_id_t pending_recipe_section;
    /** True when pending_screen should be applied during ui_update(). */
    bool has_pending_screen;
};

void ui_init(ui_t *ui);
void ui_update(ui_t *ui, const status_screen_view_model_t *view_model);

#endif
