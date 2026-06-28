#ifndef LV_CONF_H
#define LV_CONF_H

/*
 * Default LVGL configuration for local development.
 *
 * CMake now selects either lv_conf_sim.h or lv_conf_target.h explicitly through
 * LV_BUILD_CONF_PATH, so this file is mainly a safe fallback for tools that expect a plain
 * top-level lv_conf.h. Keep it simulator-oriented; the target config lives in
 * lv_conf_target.h and should not be copied over this file.
 */

#define LV_COLOR_DEPTH 16
#define LV_USE_OS 0
#define LV_USE_LOG 1

/*
 * Fallback tools should see the same user-facing portrait shape as the simulator. The
 * target config keeps the physical 480x272 panel size because LVGL rotates it after DRM
 * has opened the real display.
 */
#define LV_HOR_RES_MAX 272
#define LV_VER_RES_MAX 480
#define LV_USE_SDL 1

#define LV_USE_LINUX_DRM 0
#define LV_USE_LINUX_FBDEV 0
#define LV_USE_EVDEV 0
#define LV_FONT_MONTSERRAT_20 1

#endif
