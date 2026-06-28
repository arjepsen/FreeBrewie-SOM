#include "Display.h"

#include <unistd.h>

#include "Platform/Logging.h"

#if !defined(__arm__) && !defined(__aarch64__)
#include "src/drivers/sdl/lv_sdl_keyboard.h"
#include "src/drivers/sdl/lv_sdl_mouse.h"
#include "src/drivers/sdl/lv_sdl_mousewheel.h"
#include "src/drivers/sdl/lv_sdl_window.h"
#else
#if defined(BREWIE_TARGET_DISPLAY_BACKEND_fbdev)
#include "src/drivers/display/fb/lv_linux_fbdev.h"
#else
#include "src/drivers/display/drm/lv_linux_drm.h"
#endif
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
#define DISPLAY_IDLE_SLEEP_MAX_MS 25U

bool display_init(display_t *display)
{
    lv_display_t *lv_display;

    if (display == NULL)
    {
        return false;
    }

    display->ready = false;
    display->simulator = false;
    display->last_tick_ms = 0U;

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
#if defined(BREWIE_TARGET_DISPLAY_BACKEND_fbdev)
    /*
     * Experimental portrait path for the real SOM.
     *
     * The framebuffer device is provided by the same sun4i display stack as DRM, but
     * LVGL's fbdev backend has explicit software rotation support. This lets us test a
     * natural portrait UI on the mounted panel without touching the kernel or writing a
     * custom rotate/blit backend first.
     */
    lv_display = lv_linux_fbdev_create();
    if (lv_display == NULL)
    {
        log_error("display_init: fbdev display create failed");
        return false;
    }

    if (lv_linux_fbdev_set_file(lv_display, "/dev/fb0") != LV_RESULT_OK)
    {
        log_error("display_init: fbdev set file failed");
        return false;
    }

    lv_display_set_rotation(lv_display, LV_DISPLAY_ROTATION_90);

    display->ready = true;
    display->simulator = false;
    log_info("display_init: fbdev display ready");
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
#endif
}

void display_update(display_t *display, uint64_t now_ms)
{
    uint64_t elapsed_ms;
    uint32_t wait_ms;

    if (display == NULL || !display->ready)
    {
        return;
    }

    /*
     * LVGL needs its millisecond tick to move forward, but the old code advanced it by a
     * fixed 5 ms on every loop. That made the UI timing depend on how fast the main loop
     * happened to spin. Use real elapsed time instead, so the app can sleep when idle
     * without making animations or timers run at the wrong speed.
     */
    if (display->last_tick_ms == 0U)
    {
        display->last_tick_ms = now_ms;
    }

    elapsed_ms = now_ms - display->last_tick_ms;
    if (elapsed_ms > 0U)
    {
        if (elapsed_ms > UINT32_MAX)
        {
            elapsed_ms = UINT32_MAX;
        }

        lv_tick_inc((uint32_t)elapsed_ms);
        display->last_tick_ms = now_ms;
    }

    wait_ms = lv_timer_handler();

    /*
     * Let LVGL tell us when it needs to run again, but cap the sleep so comms and future
     * input handling still get checked promptly. This is especially important on the A13
     * SOM, where waking hundreds of times per second for a static status screen wastes CPU.
     */
    if (wait_ms == LV_NO_TIMER_READY || wait_ms > DISPLAY_IDLE_SLEEP_MAX_MS)
    {
        wait_ms = DISPLAY_IDLE_SLEEP_MAX_MS;
    }

    if (wait_ms > 0U)
    {
        usleep(wait_ms * 1000U);
    }
}
