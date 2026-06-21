#include "Display.h"

#include <unistd.h>

#include "Platform/Logging.h"

#if !defined(__arm__) && !defined(__aarch64__)
#include "src/drivers/sdl/lv_sdl_keyboard.h"
#include "src/drivers/sdl/lv_sdl_mouse.h"
#include "src/drivers/sdl/lv_sdl_mousewheel.h"
#include "src/drivers/sdl/lv_sdl_window.h"
#endif

bool display_init(display_t *display)
{
    if (display == NULL)
    {
        return false;
    }

    display->ready = false;
    display->simulator = false;

    lv_init();

#if !defined(__arm__) && !defined(__aarch64__)
    if (lv_sdl_window_create(480, 272) == NULL)
    {
        log_error("display_init: simulator display create failed");
        return false;
    }

    lv_sdl_mouse_create();
    lv_sdl_mousewheel_create();
    lv_sdl_keyboard_create();
    display->ready = true;
    display->simulator = true;
    return true;
#else
    log_info("display_init: target display bypassed");
    return false;
#endif
}

void display_update(display_t *display, uint64_t now_ms)
{
    (void)now_ms;

    if (display == NULL || !display->ready)
    {
        return;
    }

    lv_timer_handler();
    lv_tick_inc(5);
    usleep(5000);
}
