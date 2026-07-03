#ifndef FREEBREWIE_UI_H
#define FREEBREWIE_UI_H

#include "Screen_home.h"
#include "Screen_status.h"
#include "UI_types.h"

typedef struct
{
    screen_home_t home;
    screen_status_t status;
    lv_obj_t *manual_screen;
    lv_obj_t *clean_screen;
    lv_obj_t *settings_screen;
    ui_screen_id_t current_screen;
} ui_t;

void ui_init(ui_t *ui);
void ui_update(ui_t *ui, const status_screen_view_model_t *view_model);

#endif
