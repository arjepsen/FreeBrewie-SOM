#ifndef FREEBREWIE_SCREEN_HOME_H
#define FREEBREWIE_SCREEN_HOME_H

/****************************************************************************************
 * @file Screen_home.h
 * @brief Main user-facing Home screen.
 *
 * Responsibility: show the normal user landing screen.
 * Owns: Home LVGL objects, machine summary presentation, and menu entry.
 * Does not own: hardware control or protocol messages.
 ****************************************************************************************/

#include "Logic/Status_view_model.h"
#include "UI_types.h"
#include "lvgl.h"

typedef struct
{
    ui_action_t action;
    uint32_t value;
    ui_action_handler_t handler;
    void *user_data;
} screen_home_button_context_t;

/**
 * Always-live Home screen state.
 *
 * Home stays in memory because it is the normal landing screen. It keeps tiny cached text
 * state so the periodic UI update does not write unchanged labels every tick.
 */
typedef struct
{
    lv_obj_t *screen;
    lv_obj_t *mcu_value;
    /** Last link text shown on Home, used to avoid unchanged label updates. */
    const char *shown_link_text;
    char mcu_text[16];
    screen_home_button_context_t menu_button_context;
    screen_home_button_context_t brew_button_context;
} screen_home_t;

void screen_home_init(screen_home_t *home, ui_action_handler_t action_handler, void *user_data);
void screen_home_update(screen_home_t *home, const status_screen_view_model_t *view_model);

#endif
