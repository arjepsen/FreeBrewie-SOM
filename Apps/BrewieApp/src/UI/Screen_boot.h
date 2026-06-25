#ifndef FREEBREWIE_SCREEN_BOOT_H
#define FREEBREWIE_SCREEN_BOOT_H

#include <stdint.h>

#include "Logic/App_types.h"
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
    lv_obj_t *mcu_status_value;
    lv_obj_t *pressure_value;
    lv_obj_t *pump_value;
    lv_obj_t *solenoid_value;
    lv_obj_t *fault_value;
} screen_boot_t;

void screen_boot_init(screen_boot_t *boot);
void screen_boot_update(screen_boot_t *boot, const boot_screen_view_model_t *view_model);

#endif
