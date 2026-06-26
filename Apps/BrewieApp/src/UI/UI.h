#ifndef FREEBREWIE_UI_H
#define FREEBREWIE_UI_H

#include "Screen_status.h"

typedef struct
{
    screen_status_t status;
} ui_t;

void ui_init(ui_t *ui);
void ui_update_status_screen(ui_t *ui, const status_screen_view_model_t *view_model);

#endif
