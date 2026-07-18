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
    /** Navigation action emitted when this menu row is clicked. */
    ui_action_t action;
    /** Optional action value; unused for simple menu navigation. */
    uint32_t value;
    /** Callback owned by the UI shell. */
    ui_action_handler_t handler;
    /** Opaque pointer passed back to the callback, normally the ui_t instance. */
    void *user_data;
} screen_menu_button_context_t;

typedef struct
{
    /** Root LVGL screen object for the full-screen menu. */
    lv_obj_t *screen;
    /** Event callback context for each menu row. */
    screen_menu_button_context_t home_context;
    screen_menu_button_context_t recipes_context;
    screen_menu_button_context_t manual_context;
    screen_menu_button_context_t settings_context;
    screen_menu_button_context_t status_context;
} screen_menu_t;

void screen_menu_init(screen_menu_t *menu, ui_action_handler_t action_handler, void *user_data);

#endif
