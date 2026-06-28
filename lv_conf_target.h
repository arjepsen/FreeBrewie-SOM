#ifndef LV_CONF_H
#define LV_CONF_H

/*
 * LVGL configuration for the real Olimex A13-SOM target.
 *
 * The target can be compiled for the known-good DRM backend or the experimental fbdev
 * portrait backend. SDL is deliberately disabled because there is no desktop windowing
 * environment on the appliance target.
 */

#define LV_COLOR_DEPTH 16
#define LV_USE_OS 0
#define LV_USE_LOG 1

/*
 * These are the physical LCD timings exposed by the current sun4i display stack. The panel
 * is mounted portrait in the appliance. DRM currently stays in this physical orientation;
 * the experimental fbdev build asks LVGL to rotate into portrait.
 */
#define LV_HOR_RES_MAX 480
#define LV_VER_RES_MAX 272
#define LV_USE_SDL 0

#define LV_USE_LINUX_DRM 1
#define LV_USE_LINUX_FBDEV 1
#define LV_USE_EVDEV 1
#define LV_FONT_MONTSERRAT_20 1

#endif
