#ifndef LV_CONF_H
#define LV_CONF_H

/*
 * LVGL configuration for the local simulator build.
 *
 * The simulator runs on the Debian VM and uses SDL for its window, mouse, keyboard, and
 * wheel input. Keep the Linux DRM/fbdev/evdev backends disabled here so a local UI build
 * does not need target-only display headers such as drm.h.
 */

#define LV_COLOR_DEPTH 16
#define LV_USE_OS 0
#define LV_USE_LOG 1

/*
 * These dimensions match the current landscape bring-up view. The final appliance UI is
 * expected to become portrait after rotation work, but keeping the simulator aligned with
 * today's target output makes the current status screen easier to compare.
 */
#define LV_HOR_RES_MAX 480
#define LV_VER_RES_MAX 272

#define LV_USE_SDL 1
#define LV_USE_LINUX_DRM 0
#define LV_USE_LINUX_FBDEV 0
#define LV_USE_EVDEV 0

#define LV_FONT_MONTSERRAT_20 1

#endif
