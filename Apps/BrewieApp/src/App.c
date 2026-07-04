#include "App.h"

#include <string.h>
#include <unistd.h>

#include "Platform/Logging.h"
#include "Platform/Time_base.h"

#define APP_LOOP_SLEEP_US 10000
#define APP_UI_REFRESH_PERIOD_MS 250U

/****************************************************************************************
 * @brief Bring the SOM application online.
 *
 * Initialization is ordered from lowest level to highest level:
 * platform/display first, then UI objects, app logic, and finally the MCU serial link.
 * The app currently treats a missing MCU link as fatal because most useful behavior depends
 * on exchanging heartbeats and status reports with the controller.
 ****************************************************************************************/
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

/****************************************************************************************
 * @brief Run one pass through the main application loop.
 *
 * This function is called forever from main(). Comms and fast app logic are serviced every
 * pass, while printf-style UI text formatting is refreshed at a slower human-readable rate.
 * LVGL still runs every pass through display_update(), which lets touch input and animations
 * remain responsive.
 ****************************************************************************************/
void app_update(app_t *app)
{
    uint64_t now_ms;

    if (app == NULL)
    {
        return;
    }

    now_ms = time_base_now_ms();

    comms_update(&app->comms, now_ms);
    app_logic_update_fast(&app->logic, comms_get_status(&app->comms), now_ms);

    if (app->display_enabled)
    {
        /*
         * The status screen is useful during bring-up, but formatting strings and rewriting
         * labels every loop makes the SOM do work faster than a human can read. Keep comms
         * and fast app logic running every loop; refresh the diagnostic view model and UI at
         * a modest rate.
         */
        if ((now_ms - app->last_ui_update_ms) >= APP_UI_REFRESH_PERIOD_MS)
        {
            app_logic_update_status_view_model(&app->logic, comms_get_status(&app->comms));
            ui_update(&app->ui, &app->logic.status_screen);
            app->last_ui_update_ms = now_ms;
        }

        display_update(&app->platform.display, now_ms);
    }
    else
    {
        usleep(APP_LOOP_SLEEP_US);
    }
}

/****************************************************************************************
 * @brief Release app-owned resources before the process exits.
 ****************************************************************************************/
void app_shutdown(app_t *app)
{
    if (app == NULL)
    {
        return;
    }

    comms_shutdown(&app->comms);
    platform_shutdown(&app->platform);
}
