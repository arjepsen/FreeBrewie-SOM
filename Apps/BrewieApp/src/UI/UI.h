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

#include "Screen_home.h"
#include "Screen_menu.h"
#include "Screen_recipe_detail.h"
#include "Screen_recipes.h"
#include "Screen_status.h"
#include "UI_types.h"

typedef struct ui_t ui_t;

typedef struct
{
    ui_action_t action;
    ui_t *ui;
} ui_button_context_t;

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
    /** Full-screen navigation menu. */
    screen_menu_t menu;
    /** Placeholder screens used until real workflows are implemented. */
    lv_obj_t *manual_screen;
    lv_obj_t *settings_screen;
    /** Screen currently loaded into LVGL. */
    ui_screen_id_t current_screen;
    /** Deferred navigation target requested from an LVGL event callback. */
    ui_screen_id_t pending_screen;
    /** Optional value for the pending navigation, currently recipe_id for recipe detail. */
    uint32_t pending_value;
    /** True when pending_screen should be applied during ui_update(). */
    bool has_pending_screen;
    /** Button callback contexts owned by this UI instance. */
    ui_button_context_t manual_back_context;
    ui_button_context_t settings_back_context;
};

void ui_init(ui_t *ui);
void ui_update(ui_t *ui, const status_screen_view_model_t *view_model);

#endif
