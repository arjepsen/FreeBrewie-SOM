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
    /** Full-screen navigation menu. */
    lv_obj_t *menu_screen;
    /** Placeholder screens used until real workflows are implemented. */
    lv_obj_t *extras_screen;
    lv_obj_t *settings_screen;
    /** Screen currently loaded into LVGL. */
    ui_screen_id_t current_screen;
    /** Deferred navigation target requested from an LVGL event callback. */
    ui_screen_id_t pending_screen;
    /** True when pending_screen should be applied during ui_update(). */
    bool has_pending_screen;
    /** Button callback contexts owned by this UI instance. */
    ui_button_context_t extras_back_context;
    ui_button_context_t settings_back_context;
    ui_button_context_t menu_home_context;
    ui_button_context_t menu_status_context;
    ui_button_context_t menu_extras_context;
    ui_button_context_t menu_recipes_context;
    ui_button_context_t menu_settings_context;
};

void ui_init(ui_t *ui);
void ui_update(ui_t *ui, const status_screen_view_model_t *view_model);

#endif
