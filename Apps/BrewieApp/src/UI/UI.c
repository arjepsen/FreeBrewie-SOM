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
    screen_boot_init(&ui->boot);
}

void ui_update_boot_screen(ui_t *ui, const boot_screen_view_model_t *view_model)
{
    if (ui == NULL)
    {
        return;
    }

    screen_boot_update(&ui->boot, view_model);
}
