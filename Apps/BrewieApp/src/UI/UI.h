#ifndef FREEBREWIE_UI_H
#define FREEBREWIE_UI_H

#include "Screen_boot.h"

typedef struct
{
    screen_boot_t boot;
} ui_t;

void ui_init(ui_t *ui);
void ui_update_boot_screen(ui_t *ui, const boot_screen_view_model_t *view_model);

#endif
