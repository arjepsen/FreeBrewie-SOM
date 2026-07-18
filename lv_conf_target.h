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
 * The A13 uses a Cortex-A8 CPU with NEON. LVGL's software renderer has NEON paths for
 * common RGB565 blending/conversion operations. Keep this target-only so the simulator
 * still builds for the host CPU.
 */
#define LV_USE_DRAW_SW_ASM 1

#if defined(BREWIE_ENABLE_LVGL_MATRIX_ROTATION) && BREWIE_ENABLE_LVGL_MATRIX_ROTATION
/*
 * Matrix rotation is disabled in normal target builds because it crashed during early
 * BrewieApp testing on the SOM. Keep it behind an explicit CMake switch so isolated probe
 * binaries can investigate LVGL's DRM behavior without changing the safe appliance build.
 */
#define LV_USE_FLOAT 1
#define LV_USE_MATRIX 1
#define LV_DRAW_TRANSFORM_USE_MATRIX 1
#endif

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
