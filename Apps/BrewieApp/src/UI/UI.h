#ifndef FREEBREWIE_UI_H
#define FREEBREWIE_UI_H

#include <stdbool.h>

#include "Screen_home.h"
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
    /** Full-screen navigation menu. */
    lv_obj_t *menu_screen;
    /** Placeholder screens used until real workflows are implemented. */
    lv_obj_t *manual_screen;
    lv_obj_t *clean_screen;
    lv_obj_t *settings_screen;
    /** Screen currently loaded into LVGL. */
    ui_screen_id_t current_screen;
    /** Deferred navigation target requested from an LVGL event callback. */
    ui_screen_id_t pending_screen;
    /** True when pending_screen should be applied during ui_update(). */
    bool has_pending_screen;
    /** Button callback contexts owned by this UI instance. */
    ui_button_context_t manual_back_context;
    ui_button_context_t clean_back_context;
    ui_button_context_t settings_back_context;
    ui_button_context_t menu_home_context;
    ui_button_context_t menu_status_context;
    ui_button_context_t menu_clean_context;
    ui_button_context_t menu_manual_context;
    ui_button_context_t menu_settings_context;
};

void ui_init(ui_t *ui);
void ui_update(ui_t *ui, const status_screen_view_model_t *view_model);

#endif
