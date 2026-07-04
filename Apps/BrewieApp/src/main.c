#include <signal.h>
#include <stdbool.h>

#include "App.h"

static volatile sig_atomic_t keep_running = 1;

/****************************************************************************************
 * @brief Ask the main loop to stop after systemd or the terminal sends a signal.
 *
 * Signal handlers must stay tiny. The real shutdown work happens after app_update() returns
 * to the main loop and sees keep_running become false.
 ****************************************************************************************/
static void signal_handler(int signum)
{
    (void)signum;
    keep_running = 0;
}

int main()
{
    app_t app;

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /*
     * The service process is intentionally a simple loop. display_update() and comms_update()
     * contain the short sleeps/polls that keep CPU use under control while still reacting to
     * serial traffic and touch input.
     */
    if (!app_init(&app))
    {
        return 1;
    }

    while (keep_running)
    {
        app_update(&app);
    }

    app_shutdown(&app);
    return 0;
}
