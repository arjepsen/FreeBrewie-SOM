#include "UI.h"

#include <string.h>

#include "UI_theme.h"

void ui_init(ui_t *ui)
{
    if (ui == NULL)
    {
        return;
    }

    memset(ui, 0, sizeof(*ui));
    ui_theme_init();
    screen_status_init(&ui->status);
}

void ui_update_status_screen(ui_t *ui, const status_screen_view_model_t *view_model)
{
    if (ui == NULL)
    {
        return;
    }

    screen_status_update(&ui->status, view_model);
}
