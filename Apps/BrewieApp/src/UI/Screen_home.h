#ifndef FREEBREWIE_SCREEN_HOME_H
#define FREEBREWIE_SCREEN_HOME_H

#include "UI_types.h"
#include "lvgl.h"

typedef struct
{
    /** Navigation action emitted when this button is clicked. */
    ui_action_t action;
    /** Callback owned by the UI shell. */
    ui_action_handler_t handler;
    /** Opaque pointer passed back to the callback, normally the ui_t instance. */
    void *user_data;
} screen_home_button_context_t;

typedef struct
{
    /** Root LVGL screen object for Home. */
    lv_obj_t *screen;
    /** Small label showing current MCU link state. */
    lv_obj_t *mcu_value;
    /** Last link text shown on Home, used to avoid unchanged label updates. */
    const char *shown_link_text;
    /** Backing storage for the small "MCU ok/waiting/down" Home label. */
    char mcu_text[16];
    /** Event callback context for the hamburger/menu button. */
    screen_home_button_context_t menu_button_context;
} screen_home_t;

void screen_home_init(screen_home_t *home, ui_action_handler_t action_handler, void *user_data);
void screen_home_update(screen_home_t *home, const status_screen_view_model_t *view_model);

#endif
