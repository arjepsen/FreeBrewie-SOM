#ifndef FREEBREWIE_SCREEN_MENU_H
#define FREEBREWIE_SCREEN_MENU_H

/****************************************************************************************
 * @file Screen_menu.h
 * @brief Old-Brewie-inspired top-level navigation menu.
 *
 * Responsibility: show the main menu and emit UI navigation requests.
 * Owns: Menu LVGL objects and menu row callback contexts.
 * Must not own: workflow state, hardware actions, or MCU protocol messages.
 ****************************************************************************************/

#include "UI_types.h"
#include "lvgl.h"

typedef struct
{
    ui_action_t action;
    uint32_t value;
    ui_action_handler_t handler;
    void *user_data;
} screen_menu_button_context_t;

/** Full-screen menu state; one persistent context is kept for each row callback. */
typedef struct
{
    lv_obj_t *screen;
    screen_menu_button_context_t home_context;
    screen_menu_button_context_t recipes_context;
    screen_menu_button_context_t manual_context;
    screen_menu_button_context_t settings_context;
    screen_menu_button_context_t status_context;
} screen_menu_t;

void screen_menu_init(screen_menu_t *menu, ui_action_handler_t action_handler, void *user_data);

#endif
