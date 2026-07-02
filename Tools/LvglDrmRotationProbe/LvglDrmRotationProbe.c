#define _POSIX_C_SOURCE 200809L

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "lvgl.h"
#include "src/drivers/display/drm/lv_linux_drm.h"

/*
 * LvglDrmRotationProbe is an isolated test for LVGL's own DRM rotation behavior.
 *
 * BrewieApp must stay on the known-good display path while rotation is investigated. This
 * tool opens /dev/dri/card0 directly, draws a simple portrait/landscape test scene, and
 * tries exactly one rotation mode per run. If a mode crashes or shows a black screen, the
 * failure is isolated from the real service.
 */

#define PROBE_PHYSICAL_WIDTH 480
#define PROBE_PHYSICAL_HEIGHT 272
#define PROBE_PORTRAIT_WIDTH PROBE_PHYSICAL_HEIGHT
#define PROBE_PORTRAIT_HEIGHT PROBE_PHYSICAL_WIDTH
#define PROBE_DEFAULT_SECONDS 10U

typedef enum
{
    PROBE_MODE_NONE,
    PROBE_MODE_ROTATION,
    PROBE_MODE_MATRIX
} probe_mode_t;

typedef enum
{
    PROBE_ROTATION_90,
    PROBE_ROTATION_270
} probe_rotation_t;

static int parse_args(int argc, char **argv, probe_mode_t *mode, probe_rotation_t *rotation, unsigned int *seconds);
static void print_usage(const char *program_name);
static uint64_t now_ms_monotonic();
static lv_color_t color_from_hex(uint32_t rgb);
static lv_obj_t *create_color_box(lv_obj_t *parent, uint32_t color, lv_align_t align);
static lv_obj_t *build_test_scene();
static void update_moving_box(lv_obj_t *box, uint64_t now_ms);
static void sleep_ms(uint32_t milliseconds);
static const char *mode_name(probe_mode_t mode);
static const char *rotation_name(probe_rotation_t rotation);
static lv_display_rotation_t to_lvgl_rotation(probe_rotation_t rotation);

/***************************************************************************************************
 * main
 **************************************************************************************************/
int main(int argc, char **argv)
{
    probe_mode_t mode;
    probe_rotation_t rotation;
    unsigned int seconds;
    lv_display_t *display;
    lv_obj_t *moving_box;
    uint64_t start_ms;
    uint64_t last_tick_ms;

    mode = PROBE_MODE_NONE;
    rotation = PROBE_ROTATION_270;
    seconds = PROBE_DEFAULT_SECONDS;

    if (parse_args(argc, argv, &mode, &rotation, &seconds) != 0)
    {
        return 2;
    }

    printf("lvgl_drm_rotation_probe: mode=%s rotation=%s seconds=%u\n",
           mode_name(mode),
           rotation_name(rotation),
           seconds);

    lv_init();
    display = lv_linux_drm_create();
    if (display == NULL)
    {
        fprintf(stderr, "ERROR: lv_linux_drm_create failed\n");
        return 1;
    }

    if (lv_linux_drm_set_file(display, "/dev/dri/card0", -1) != LV_RESULT_OK)
    {
        fprintf(stderr, "ERROR: lv_linux_drm_set_file(/dev/dri/card0) failed\n");
        return 1;
    }

    /*
     * Normal LVGL rotation changes the logical resolution and asks the display backend to
     * present that correctly. Matrix rotation is a separate LVGL path that transforms draw
     * operations before they reach the direct display buffer. We test them independently.
     */
    if (mode == PROBE_MODE_ROTATION || mode == PROBE_MODE_MATRIX)
    {
        lv_display_set_rotation(display, to_lvgl_rotation(rotation));
    }

    if (mode == PROBE_MODE_MATRIX)
    {
#if LV_DRAW_TRANSFORM_USE_MATRIX
        lv_display_set_matrix_rotation(display, true);
#else
        fprintf(stderr, "ERROR: matrix mode requested, but LV_DRAW_TRANSFORM_USE_MATRIX is not enabled\n");
        return 1;
#endif
    }

    moving_box = build_test_scene();
    start_ms = now_ms_monotonic();
    last_tick_ms = start_ms;

    while ((now_ms_monotonic() - start_ms) < ((uint64_t)seconds * 1000ULL))
    {
        uint64_t now_ms;
        uint64_t elapsed_ms;
        uint32_t wait_ms;

        now_ms = now_ms_monotonic();
        elapsed_ms = now_ms - last_tick_ms;
        if (elapsed_ms > 0U)
        {
            if (elapsed_ms > UINT32_MAX)
            {
                elapsed_ms = UINT32_MAX;
            }

            lv_tick_inc((uint32_t)elapsed_ms);
            last_tick_ms = now_ms;
        }

        update_moving_box(moving_box, now_ms - start_ms);
        wait_ms = lv_timer_handler();
        if (wait_ms == LV_NO_TIMER_READY || wait_ms > 16U)
        {
            wait_ms = 16U;
        }

        if (wait_ms > 0U)
        {
            sleep_ms(wait_ms);
        }
    }

    printf("lvgl_drm_rotation_probe: completed\n");
    return 0;
}

/***************************************************************************************************
 * Argument parsing
 **************************************************************************************************/
static int parse_args(int argc, char **argv, probe_mode_t *mode, probe_rotation_t *rotation, unsigned int *seconds)
{
    int index;

    for (index = 1; index < argc; ++index)
    {
        if (strcmp(argv[index], "--mode") == 0 && (index + 1) < argc)
        {
            ++index;
            if (strcmp(argv[index], "none") == 0)
            {
                *mode = PROBE_MODE_NONE;
            }
            else if (strcmp(argv[index], "rotation") == 0)
            {
                *mode = PROBE_MODE_ROTATION;
            }
            else if (strcmp(argv[index], "matrix") == 0)
            {
                *mode = PROBE_MODE_MATRIX;
            }
            else
            {
                print_usage(argv[0]);
                return -1;
            }
        }
        else if (strcmp(argv[index], "--rotation") == 0 && (index + 1) < argc)
        {
            ++index;
            if (strcmp(argv[index], "90") == 0)
            {
                *rotation = PROBE_ROTATION_90;
            }
            else if (strcmp(argv[index], "270") == 0)
            {
                *rotation = PROBE_ROTATION_270;
            }
            else
            {
                print_usage(argv[0]);
                return -1;
            }
        }
        else if (strcmp(argv[index], "--seconds") == 0 && (index + 1) < argc)
        {
            *seconds = (unsigned int)strtoul(argv[++index], NULL, 10);
            if (*seconds == 0U)
            {
                fprintf(stderr, "ERROR: --seconds must be greater than zero\n");
                return -1;
            }
        }
        else if (strcmp(argv[index], "--help") == 0)
        {
            print_usage(argv[0]);
            exit(0);
        }
        else
        {
            print_usage(argv[0]);
            return -1;
        }
    }

    return 0;
}

static void print_usage(const char *program_name)
{
    fprintf(stderr,
            "usage: %s [--mode none|rotation|matrix] [--rotation 90|270] [--seconds N]\n"
            "\n"
            "  --mode none       Open DRM and draw without LVGL rotation. Default.\n"
            "  --mode rotation   Use lv_display_set_rotation only.\n"
            "  --mode matrix     Use lv_display_set_rotation plus lv_display_set_matrix_rotation.\n"
            "  --rotation 270    Expected Brewie portrait direction based on rotate probe. Default.\n"
            "  --rotation 90     Opposite test direction.\n",
            program_name);
}

/***************************************************************************************************
 * Test scene
 **************************************************************************************************/
static uint64_t now_ms_monotonic()
{
    struct timespec time_value;

    if (clock_gettime(CLOCK_MONOTONIC, &time_value) != 0)
    {
        return 0U;
    }

    return ((uint64_t)time_value.tv_sec * 1000ULL) + ((uint64_t)time_value.tv_nsec / 1000000ULL);
}

static lv_color_t color_from_hex(uint32_t rgb)
{
    return lv_color_hex(rgb);
}

static lv_obj_t *create_color_box(lv_obj_t *parent, uint32_t color, lv_align_t align)
{
    lv_obj_t *box;

    box = lv_obj_create(parent);
    lv_obj_remove_style_all(box);
    lv_obj_set_size(box, 48, 48);
    lv_obj_set_style_bg_color(box, color_from_hex(color), 0);
    lv_obj_set_style_bg_opa(box, LV_OPA_COVER, 0);
    lv_obj_align(box, align, 0, 0);
    return box;
}

static lv_obj_t *build_test_scene()
{
    lv_obj_t *screen;
    lv_obj_t *title;
    lv_obj_t *moving_box;

    screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, color_from_hex(0x101018), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    create_color_box(screen, 0xFF0000, LV_ALIGN_TOP_LEFT);
    create_color_box(screen, 0x00FF00, LV_ALIGN_TOP_RIGHT);
    create_color_box(screen, 0x0000FF, LV_ALIGN_BOTTOM_LEFT);
    create_color_box(screen, 0xFFFF00, LV_ALIGN_BOTTOM_RIGHT);

    moving_box = lv_obj_create(screen);
    lv_obj_remove_style_all(moving_box);
    lv_obj_set_size(moving_box, 36, 36);
    lv_obj_set_style_bg_color(moving_box, color_from_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(moving_box, LV_OPA_COVER, 0);

    title = lv_label_create(screen);
    lv_label_set_text(title, "LVGL DRM ROTATION PROBE");
    lv_obj_set_style_text_color(title, color_from_hex(0xFFFFFF), 0);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);

    return moving_box;
}

static void update_moving_box(lv_obj_t *box, uint64_t now_ms)
{
    int32_t usable_width;
    int32_t usable_height;
    int32_t phase;

    if (box == NULL)
    {
        return;
    }

    usable_width = lv_display_get_horizontal_resolution(NULL) - 36;
    usable_height = lv_display_get_vertical_resolution(NULL) - 36;
    if (usable_width <= 0 || usable_height <= 0)
    {
        return;
    }

    phase = (int32_t)((now_ms / 12U) % (uint64_t)usable_width);
    lv_obj_set_pos(box, phase, (phase * usable_height) / usable_width);
}

static void sleep_ms(uint32_t milliseconds)
{
    struct timespec sleep_time;

    sleep_time.tv_sec = (time_t)(milliseconds / 1000U);
    sleep_time.tv_nsec = (long)(milliseconds % 1000U) * 1000000L;
    nanosleep(&sleep_time, NULL);
}

/***************************************************************************************************
 * Small text helpers
 **************************************************************************************************/
static const char *mode_name(probe_mode_t mode)
{
    switch (mode)
    {
    case PROBE_MODE_NONE:
        return "none";
    case PROBE_MODE_ROTATION:
        return "rotation";
    case PROBE_MODE_MATRIX:
        return "matrix";
    default:
        return "unknown";
    }
}

static const char *rotation_name(probe_rotation_t rotation)
{
    return rotation == PROBE_ROTATION_90 ? "90" : "270";
}

static lv_display_rotation_t to_lvgl_rotation(probe_rotation_t rotation)
{
    return rotation == PROBE_ROTATION_90 ? LV_DISPLAY_ROTATION_90 : LV_DISPLAY_ROTATION_270;
}
