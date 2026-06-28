#include "Display.h"

#include <unistd.h>

#include "Platform/Logging.h"

#if !defined(__arm__) && !defined(__aarch64__)
#include "src/drivers/sdl/lv_sdl_keyboard.h"
#include "src/drivers/sdl/lv_sdl_mouse.h"
#include "src/drivers/sdl/lv_sdl_mousewheel.h"
#include "src/drivers/sdl/lv_sdl_window.h"
#else
#include "src/drivers/display/drm/lv_linux_drm.h"
#endif

/*
 * The Brewie LCD panel is physically a 480x272 landscape panel, but it is mounted in
 * the machine as a portrait display. Keep that fact centralized here so the UI code can
 * be written in the natural user-facing orientation instead of every screen having to
 * know that the panel is sideways.
 */
#define DISPLAY_PHYSICAL_WIDTH 480
#define DISPLAY_PHYSICAL_HEIGHT 272
#define DISPLAY_PORTRAIT_WIDTH DISPLAY_PHYSICAL_HEIGHT
#define DISPLAY_PORTRAIT_HEIGHT DISPLAY_PHYSICAL_WIDTH

bool display_init(display_t *display)
{
    lv_display_t *lv_display;

    if (display == NULL)
    {
        return false;
    }

    display->ready = false;
    display->simulator = false;

    lv_init();

#if !defined(__arm__) && !defined(__aarch64__)
    /*
     * The simulator is already a desktop window, so create it in the user-facing portrait
     * shape directly. That makes local UI work match the appliance without spending time
     * rotating pixels that only exist inside the developer's simulator window.
     */
    lv_display = lv_sdl_window_create(DISPLAY_PORTRAIT_WIDTH, DISPLAY_PORTRAIT_HEIGHT);
    if (lv_display == NULL)
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
    lv_display = lv_linux_drm_create();
    if (lv_display == NULL)
    {
        log_error("display_init: drm display create failed");
        return false;
    }

    if (lv_linux_drm_set_file(lv_display, "/dev/dri/card0", -1) != LV_RESULT_OK)
    {
        log_error("display_init: drm set file failed");
        return false;
    }

    /*
     * Keep the target in the physical DRM orientation for now. The current sun4i DRM
     * driver exposes only the 480x272 scanout mode, and early testing showed that LVGL
     * rotation on this direct-buffer DRM backend can fail badly on the SOM. Portrait UI
     * work continues in the simulator while target rotation gets a dedicated fix.
     */

    display->ready = true;
    display->simulator = false;
    log_info("display_init: drm display ready");
    return true;
#endif
}

void display_update(display_t *display, uint64_t now_ms)
{
    uint32_t wait_ms;

    (void)now_ms;

    if (display == NULL || !display->ready)
    {
        return;
    }

    wait_ms = lv_timer_handler();
    lv_tick_inc(5);

    if (wait_ms == LV_NO_TIMER_READY || wait_ms > 5)
    {
        wait_ms = 5;
    }

    usleep(wait_ms * 1000);
}
