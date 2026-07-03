#ifndef FREEBREWIE_SCREEN_HOME_H
#define FREEBREWIE_SCREEN_HOME_H

#include "UI_types.h"
#include "lvgl.h"

typedef struct
{
    ui_action_t action;
    ui_action_handler_t handler;
    void *user_data;
} screen_home_button_context_t;

typedef struct
{
    lv_obj_t *screen;
    lv_obj_t *mcu_value;
    lv_obj_t *mash_temp_value;
    lv_obj_t *boil_temp_value;
    screen_home_button_context_t status_button_context;
    screen_home_button_context_t manual_button_context;
    screen_home_button_context_t clean_button_context;
    screen_home_button_context_t settings_button_context;
} screen_home_t;

void screen_home_init(screen_home_t *home, ui_action_handler_t action_handler, void *user_data);
void screen_home_update(screen_home_t *home, const status_screen_view_model_t *view_model);

#endif
