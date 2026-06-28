#include "App.h"

#include <string.h>
#include <unistd.h>

#include "Platform/Logging.h"
#include "Platform/Time_base.h"

#define APP_LOOP_SLEEP_US 10000
#define APP_UI_REFRESH_PERIOD_MS 250U

bool app_init(app_t *app)
{
    if (app == NULL)
    {
        return false;
    }

    memset(app, 0, sizeof(*app));

    if (!platform_init(&app->platform))
    {
        return false;
    }

    app->display_enabled = display_init(&app->platform.display);
    if (app->display_enabled)
    {
        ui_init(&app->ui);
    }

    app_logic_init(&app->logic);

    if (!comms_init(&app->comms, "/dev/ttyS1", 115200))
    {
        log_error("app_init: comms init failed");
        return false;
    }

    app_logic_set_serial_ready(&app->logic, comms_is_serial_ready(&app->comms));
    return true;
}

void app_update(app_t *app)
{
    uint64_t now_ms;

    if (app == NULL)
    {
        return;
    }

    now_ms = time_base_now_ms();

    comms_update(&app->comms, now_ms);
    app_logic_update(&app->logic, comms_get_status(&app->comms), now_ms);

    if (app->display_enabled)
    {
        /*
         * The status screen is useful during bring-up, but rewriting every label on every
         * loop makes LVGL redraw far more often than a human can read. Keep comms running
         * every loop, but refresh this diagnostic UI at a modest rate so the SOM stays
         * responsive and has CPU left for the real control work.
         */
        if ((now_ms - app->last_ui_update_ms) >= APP_UI_REFRESH_PERIOD_MS)
        {
            ui_update_status_screen(&app->ui, &app->logic.status_screen);
            app->last_ui_update_ms = now_ms;
        }

        display_update(&app->platform.display, now_ms);
    }
    else
    {
        usleep(APP_LOOP_SLEEP_US);
    }
}

void app_shutdown(app_t *app)
{
    if (app == NULL)
    {
        return;
    }

    comms_shutdown(&app->comms);
    platform_shutdown(&app->platform);
}
