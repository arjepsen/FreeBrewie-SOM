#ifndef LV_CONF_H
#define LV_CONF_H

/*
 * LVGL configuration for the real Olimex A13-SOM target.
 *
 * The target uses LVGL's Linux DRM backend on /dev/dri/card0. SDL is deliberately disabled
 * here because there is no desktop windowing environment on the appliance target.
 */

#define LV_COLOR_DEPTH 16
#define LV_USE_OS 0
#define LV_USE_LOG 1

/*
 * These are the physical LCD timings exposed by the current sun4i DRM driver. The panel
 * is mounted portrait in the appliance, but target-side LVGL rotation is disabled until
 * the DRM direct-buffer rotation path has been fixed and tested on the SOM.
 */
#define LV_HOR_RES_MAX 480
#define LV_VER_RES_MAX 272
#define LV_USE_SDL 0

#define LV_USE_LINUX_DRM 1
#define LV_USE_LINUX_FBDEV 1
#define LV_USE_EVDEV 1
#define LV_FONT_MONTSERRAT_20 1

#endif
