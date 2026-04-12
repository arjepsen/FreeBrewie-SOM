#ifndef FREEBREWIE_UI_H
#define FREEBREWIE_UI_H

#include <stdint.h>

#include "lvgl.h"

typedef struct
{
    lv_obj_t *screen;
    lv_obj_t *title_label;
    lv_obj_t *display_value;
    lv_obj_t *serial_value;
    lv_obj_t *heartbeat_value;
    lv_obj_t *last_rx_value;
    lv_obj_t *link_value;
    lv_obj_t *hb_counter_value;
} ui_state_t;

void ui_init(ui_state_t *ui);
void ui_set_status(ui_state_t *ui, const char *key, const char *value);
void ui_set_counter(ui_state_t *ui, const char *key, uint32_t value);

#endif
