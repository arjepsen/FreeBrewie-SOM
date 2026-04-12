#include <signal.h>
#include <stdbool.h>
#include <stdio.h>

#include "App/App.h"

static volatile sig_atomic_t keep_running = 1;

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

    if(!app_init(&app)) {
        return 1;
    }

    while(keep_running) {
        app_update(&app);
    }

    app_shutdown(&app);
    return 0;
}
