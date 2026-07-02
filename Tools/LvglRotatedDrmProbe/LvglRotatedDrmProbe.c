#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <poll.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#include <drm_fourcc.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include "lvgl.h"

/*
 * LvglRotatedDrmProbe tests the display path we probably want for Brewie:
 *
 *   1. LVGL sees a normal portrait screen: 272x480.
 *   2. LVGL draws into a small RGB565 software buffer.
 *   3. This file's flush callback rotates the changed pixels into the real 480x272
 *      landscape DRM scanout buffer.
 *
 * This keeps the real app away from experimental display code until we know the CPU cost
 * and visual behavior on the A13 SOM. It also avoids LVGL's built-in DRM rotation paths,
 * which hung or crashed during testing on the Olimex Bullseye image.
 */

#define PROBE_PHYSICAL_WIDTH 480U
#define PROBE_PHYSICAL_HEIGHT 272U
#define PROBE_PORTRAIT_WIDTH PROBE_PHYSICAL_HEIGHT
#define PROBE_PORTRAIT_HEIGHT PROBE_PHYSICAL_WIDTH
#define PROBE_DEFAULT_SECONDS 10U
#define PROBE_DRAW_BUFFER_ROWS 48U
#define PROBE_MOVING_BOX_SIZE 44
#define PROBE_DRM_BUFFER_COUNT 2U

typedef struct
{
    uint32_t handle;
    uint32_t pitch;
    uint64_t size;
    uint32_t framebuffer_id;
    uint8_t *map;
} drm_buffer_t;

typedef struct
{
    int fd;
    uint32_t connector_id;
    uint32_t crtc_id;
    drmModeModeInfo mode;
    drmModeCrtc *saved_crtc;
    drm_buffer_t buffers[PROBE_DRM_BUFFER_COUNT];
    unsigned int front_buffer_index;
    unsigned int back_buffer_index;
} drm_display_t;

typedef struct
{
    bool full_invalidate;
    bool full_draw_buffer;
    unsigned int seconds;
} probe_options_t;

typedef struct
{
    drm_display_t drm;
    bool full_repaint_each_frame;
    bool frame_prepared;
    uint64_t flush_count;
    uint64_t flushed_pixels;
    uint64_t flush_ns_total;
    uint64_t frame_copy_ns_total;
    uint64_t page_flip_ns_total;
    uint64_t page_flip_count;
    uint64_t largest_area_pixels;
} probe_context_t;

static int parse_args(int argc, char **argv, probe_options_t *options);
static void print_usage(const char *program_name);
static uint64_t now_ns_monotonic();
static uint64_t now_ms_monotonic();
static void sleep_ms(uint32_t milliseconds);
static bool drm_display_open(drm_display_t *display, const char *device_path);
static void drm_display_close(drm_display_t *display);
static bool drm_open_card(drm_display_t *display, const char *device_path);
static bool drm_find_connector_and_crtc(drm_display_t *display);
static bool drm_create_buffers(drm_display_t *display);
static bool drm_create_buffer(drm_display_t *display, drm_buffer_t *buffer);
static bool drm_set_scanout(drm_display_t *display);
static bool drm_page_flip(drm_display_t *display);
static void drm_destroy_buffers(drm_display_t *display);
static void drm_destroy_buffer(drm_display_t *display, drm_buffer_t *buffer);
static void rotated_flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map);
static void copy_area_counterclockwise(probe_context_t *context, const lv_area_t *area, const uint16_t *pixels);
static void page_flip_handler(int fd, unsigned int sequence, unsigned int tv_sec, unsigned int tv_usec, void *user_data);
static lv_color_t color_from_hex(uint32_t rgb);
static lv_obj_t *create_color_box(lv_obj_t *parent, uint32_t color, lv_align_t align);
static lv_obj_t *build_test_scene();
static void update_animation(lv_obj_t *box, uint64_t elapsed_ms);
static void print_summary(const probe_context_t *context, uint64_t elapsed_ns);

/***************************************************************************************************
 * main
 **************************************************************************************************/
int main(int argc, char **argv)
{
    probe_options_t options;
    probe_context_t context;
    lv_display_t *lv_display;
    lv_obj_t *moving_box;
    size_t draw_buffer_pixels;
    uint16_t *draw_buffer_1;
    uint16_t *draw_buffer_2;
    uint64_t start_ms;
    uint64_t start_ns;
    uint64_t last_tick_ms;
    int exit_code;

    options.full_invalidate = false;
    options.full_draw_buffer = false;
    options.seconds = PROBE_DEFAULT_SECONDS;
    memset(&context, 0, sizeof(context));
    context.drm.fd = -1;
    draw_buffer_1 = NULL;
    draw_buffer_2 = NULL;
    exit_code = 0;

    if (parse_args(argc, argv, &options) != 0)
    {
        return 2;
    }
    context.full_repaint_each_frame = options.full_invalidate;

    printf("lvgl_rotated_drm_probe: seconds=%u full_invalidate=%s full_draw_buffer=%s\n",
           options.seconds,
           options.full_invalidate ? "yes" : "no",
           options.full_draw_buffer ? "yes" : "no");

    if (!drm_display_open(&context.drm, "/dev/dri/card0"))
    {
        return 1;
    }

    draw_buffer_pixels = PROBE_PORTRAIT_WIDTH *
                         (options.full_draw_buffer ? PROBE_PORTRAIT_HEIGHT : PROBE_DRAW_BUFFER_ROWS);
    draw_buffer_1 = calloc(draw_buffer_pixels, sizeof(*draw_buffer_1));
    draw_buffer_2 = calloc(draw_buffer_pixels, sizeof(*draw_buffer_2));
    if (draw_buffer_1 == NULL || draw_buffer_2 == NULL)
    {
        fprintf(stderr, "ERROR: not enough memory for LVGL draw buffers\n");
        exit_code = 1;
        goto done;
    }

    lv_init();
    lv_display = lv_display_create((int32_t)PROBE_PORTRAIT_WIDTH, (int32_t)PROBE_PORTRAIT_HEIGHT);
    if (lv_display == NULL)
    {
        fprintf(stderr, "ERROR: lv_display_create failed\n");
        exit_code = 1;
        goto done;
    }

    lv_display_set_driver_data(lv_display, &context);
    lv_display_set_flush_cb(lv_display, rotated_flush_cb);
    lv_display_set_buffers(lv_display,
                           draw_buffer_1,
                           draw_buffer_2,
                           (uint32_t)(draw_buffer_pixels * sizeof(*draw_buffer_1)),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);

    moving_box = build_test_scene();
    start_ms = now_ms_monotonic();
    start_ns = now_ns_monotonic();
    last_tick_ms = start_ms;

    while ((now_ms_monotonic() - start_ms) < ((uint64_t)options.seconds * 1000ULL))
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

        update_animation(moving_box, now_ms - start_ms);
        if (options.full_invalidate)
        {
            lv_obj_invalidate(lv_screen_active());
        }

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

    print_summary(&context, now_ns_monotonic() - start_ns);

done:
    free(draw_buffer_2);
    free(draw_buffer_1);
    drm_display_close(&context.drm);
    return exit_code;
}

/***************************************************************************************************
 * Argument parsing
 **************************************************************************************************/
static int parse_args(int argc, char **argv, probe_options_t *options)
{
    int index;

    for (index = 1; index < argc; ++index)
    {
        if (strcmp(argv[index], "--seconds") == 0 && (index + 1) < argc)
        {
            options->seconds = (unsigned int)strtoul(argv[++index], NULL, 10);
            if (options->seconds == 0U)
            {
                fprintf(stderr, "ERROR: --seconds must be greater than zero\n");
                return -1;
            }
        }
        else if (strcmp(argv[index], "--full-invalidate") == 0)
        {
            options->full_invalidate = true;
        }
        else if (strcmp(argv[index], "--full-draw-buffer") == 0)
        {
            options->full_draw_buffer = true;
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
            "usage: %s [--seconds N] [--full-invalidate]\n"
            "\n"
            "  --seconds N          Test duration. Default: %u\n"
            "  --full-invalidate    Force a full portrait-screen redraw every loop.\n"
            "  --full-draw-buffer   Give LVGL a full-height draw buffer instead of 48 rows.\n"
            "\n"
            "Run while brewie.service is stopped, because this probe needs DRM master.\n",
            program_name,
            PROBE_DEFAULT_SECONDS);
}

/***************************************************************************************************
 * Time helpers
 **************************************************************************************************/
static uint64_t now_ns_monotonic()
{
    struct timespec time_value;

    if (clock_gettime(CLOCK_MONOTONIC, &time_value) != 0)
    {
        return 0U;
    }

    return ((uint64_t)time_value.tv_sec * 1000000000ULL) + (uint64_t)time_value.tv_nsec;
}

static uint64_t now_ms_monotonic()
{
    return now_ns_monotonic() / 1000000ULL;
}

static void sleep_ms(uint32_t milliseconds)
{
    struct timespec sleep_time;

    sleep_time.tv_sec = (time_t)(milliseconds / 1000U);
    sleep_time.tv_nsec = (long)(milliseconds % 1000U) * 1000000L;
    nanosleep(&sleep_time, NULL);
}

/***************************************************************************************************
 * Minimal DRM setup
 **************************************************************************************************/
static bool drm_display_open(drm_display_t *display, const char *device_path)
{
    if (!drm_open_card(display, device_path))
    {
        return false;
    }

    if (!drm_find_connector_and_crtc(display))
    {
        drm_display_close(display);
        return false;
    }

    if (!drm_create_buffers(display))
    {
        drm_display_close(display);
        return false;
    }

    if (!drm_set_scanout(display))
    {
        drm_display_close(display);
        return false;
    }

    printf("lvgl_rotated_drm_probe: drm %ux%u pitch=%u bytes\n",
           display->mode.hdisplay,
           display->mode.vdisplay,
           display->buffers[display->front_buffer_index].pitch);
    return true;
}

static void drm_display_close(drm_display_t *display)
{
    if (display == NULL)
    {
        return;
    }

    if (display->saved_crtc != NULL)
    {
        drmModeSetCrtc(display->fd,
                       display->saved_crtc->crtc_id,
                       display->saved_crtc->buffer_id,
                       display->saved_crtc->x,
                       display->saved_crtc->y,
                       &display->connector_id,
                       1,
                       &display->saved_crtc->mode);
        drmModeFreeCrtc(display->saved_crtc);
        display->saved_crtc = NULL;
    }

    drm_destroy_buffers(display);

    if (display->fd >= 0)
    {
        close(display->fd);
        display->fd = -1;
    }
}

static bool drm_open_card(drm_display_t *display, const char *device_path)
{
    uint64_t has_dumb_buffer;

    display->fd = open(device_path, O_RDWR | O_CLOEXEC);
    if (display->fd < 0)
    {
        fprintf(stderr, "ERROR: open(%s) failed: %s\n", device_path, strerror(errno));
        return false;
    }

    if (drmGetCap(display->fd, DRM_CAP_DUMB_BUFFER, &has_dumb_buffer) != 0 || has_dumb_buffer == 0U)
    {
        fprintf(stderr, "ERROR: DRM device does not support dumb buffers\n");
        return false;
    }

    return true;
}

static bool drm_find_connector_and_crtc(drm_display_t *display)
{
    drmModeRes *resources;
    drmModeConnector *connector;
    drmModeEncoder *encoder;
    int crtc_index;
    int connector_index;
    int encoder_index;

    resources = drmModeGetResources(display->fd);
    if (resources == NULL)
    {
        fprintf(stderr, "ERROR: drmModeGetResources failed\n");
        return false;
    }

    for (connector_index = 0; connector_index < resources->count_connectors; ++connector_index)
    {
        connector = drmModeGetConnector(display->fd, resources->connectors[connector_index]);
        if (connector == NULL)
        {
            continue;
        }

        if (connector->connection == DRM_MODE_CONNECTED && connector->count_modes > 0)
        {
            display->connector_id = connector->connector_id;
            display->mode = connector->modes[0];

            for (encoder_index = 0; encoder_index < connector->count_encoders; ++encoder_index)
            {
                encoder = drmModeGetEncoder(display->fd, connector->encoders[encoder_index]);
                if (encoder == NULL)
                {
                    continue;
                }

                if (encoder->crtc_id != 0U)
                {
                    display->crtc_id = encoder->crtc_id;
                    drmModeFreeEncoder(encoder);
                    break;
                }

                /*
                 * If no CRTC is active yet, choose the first CRTC this encoder can drive.
                 * That makes the probe work both after brewie.service has been running and
                 * on a freshly booted display stack where nothing has claimed the pipe.
                 */
                for (crtc_index = 0; crtc_index < resources->count_crtcs; ++crtc_index)
                {
                    if ((encoder->possible_crtcs & (1 << crtc_index)) != 0)
                    {
                        display->crtc_id = resources->crtcs[crtc_index];
                        break;
                    }
                }

                drmModeFreeEncoder(encoder);
                if (display->crtc_id != 0U)
                {
                    break;
                }
            }

            drmModeFreeConnector(connector);
            break;
        }

        drmModeFreeConnector(connector);
    }

    drmModeFreeResources(resources);

    if (display->connector_id == 0U || display->crtc_id == 0U)
    {
        fprintf(stderr, "ERROR: connected DRM connector/CRTC not found\n");
        return false;
    }

    if (display->mode.hdisplay != PROBE_PHYSICAL_WIDTH || display->mode.vdisplay != PROBE_PHYSICAL_HEIGHT)
    {
        fprintf(stderr,
                "ERROR: expected %ux%u physical mode, got %ux%u\n",
                PROBE_PHYSICAL_WIDTH,
                PROBE_PHYSICAL_HEIGHT,
                display->mode.hdisplay,
                display->mode.vdisplay);
        return false;
    }

    display->saved_crtc = drmModeGetCrtc(display->fd, display->crtc_id);
    return true;
}

static bool drm_create_buffers(drm_display_t *display)
{
    display->front_buffer_index = 0U;
    display->back_buffer_index = 1U;

    if (!drm_create_buffer(display, &display->buffers[0]))
    {
        return false;
    }

    if (!drm_create_buffer(display, &display->buffers[1]))
    {
        return false;
    }

    return true;
}

static bool drm_create_buffer(drm_display_t *display, drm_buffer_t *buffer)
{
    struct drm_mode_create_dumb create_request;
    struct drm_mode_map_dumb map_request;
    uint32_t handles[4];
    uint32_t pitches[4];
    uint32_t offsets[4];

    memset(&create_request, 0, sizeof(create_request));
    create_request.width = PROBE_PHYSICAL_WIDTH;
    create_request.height = PROBE_PHYSICAL_HEIGHT;
    create_request.bpp = 16U;

    if (drmIoctl(display->fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_request) != 0)
    {
        fprintf(stderr, "ERROR: DRM_IOCTL_MODE_CREATE_DUMB failed: %s\n", strerror(errno));
        return false;
    }

    buffer->handle = create_request.handle;
    buffer->pitch = create_request.pitch;
    buffer->size = create_request.size;

    memset(&map_request, 0, sizeof(map_request));
    map_request.handle = buffer->handle;
    if (drmIoctl(display->fd, DRM_IOCTL_MODE_MAP_DUMB, &map_request) != 0)
    {
        fprintf(stderr, "ERROR: DRM_IOCTL_MODE_MAP_DUMB failed: %s\n", strerror(errno));
        return false;
    }

    buffer->map = mmap(NULL,
                       buffer->size,
                       PROT_READ | PROT_WRITE,
                       MAP_SHARED,
                       display->fd,
                       map_request.offset);
    if (buffer->map == MAP_FAILED)
    {
        fprintf(stderr, "ERROR: mmap DRM buffer failed: %s\n", strerror(errno));
        buffer->map = NULL;
        return false;
    }

    memset(buffer->map, 0, (size_t)buffer->size);
    memset(handles, 0, sizeof(handles));
    memset(pitches, 0, sizeof(pitches));
    memset(offsets, 0, sizeof(offsets));
    handles[0] = buffer->handle;
    pitches[0] = buffer->pitch;

    if (drmModeAddFB2(display->fd,
                      PROBE_PHYSICAL_WIDTH,
                      PROBE_PHYSICAL_HEIGHT,
                      DRM_FORMAT_RGB565,
                      handles,
                      pitches,
                      offsets,
                      &buffer->framebuffer_id,
                      0) != 0)
    {
        fprintf(stderr, "ERROR: drmModeAddFB2 failed: %s\n", strerror(errno));
        return false;
    }

    return true;
}

static bool drm_set_scanout(drm_display_t *display)
{
    if (drmModeSetCrtc(display->fd,
                       display->crtc_id,
                       display->buffers[display->front_buffer_index].framebuffer_id,
                       0,
                       0,
                       &display->connector_id,
                       1,
                       &display->mode) != 0)
    {
        fprintf(stderr, "ERROR: drmModeSetCrtc failed: %s\n", strerror(errno));
        return false;
    }

    return true;
}

static bool drm_page_flip(drm_display_t *display)
{
    drmEventContext event_context;
    struct pollfd poll_fd;
    bool page_flip_complete;
    int page_flip_result;

    page_flip_complete = false;
    page_flip_result = drmModePageFlip(display->fd,
                                       display->crtc_id,
                                       display->buffers[display->back_buffer_index].framebuffer_id,
                                       DRM_MODE_PAGE_FLIP_EVENT,
                                       &page_flip_complete);
    if (page_flip_result != 0)
    {
        fprintf(stderr, "ERROR: drmModePageFlip failed: %s\n", strerror(errno));
        return false;
    }

    memset(&event_context, 0, sizeof(event_context));
    event_context.version = DRM_EVENT_CONTEXT_VERSION;
    event_context.page_flip_handler = page_flip_handler;

    /*
     * Wait for the vblank-scheduled flip to complete. This is the part that should remove
     * the visible tearing/glitching from the single-buffer probe.
     */
    while (!page_flip_complete)
    {
        poll_fd.fd = display->fd;
        poll_fd.events = POLLIN;
        poll_fd.revents = 0;

        if (poll(&poll_fd, 1, 1000) <= 0)
        {
            fprintf(stderr, "ERROR: timed out waiting for DRM page flip\n");
            return false;
        }

        if (drmHandleEvent(display->fd, &event_context) != 0)
        {
            fprintf(stderr, "ERROR: drmHandleEvent failed: %s\n", strerror(errno));
            return false;
        }
    }

    display->front_buffer_index = display->back_buffer_index;
    display->back_buffer_index = 1U - display->front_buffer_index;
    return true;
}

static void drm_destroy_buffers(drm_display_t *display)
{
    unsigned int index;

    for (index = 0U; index < PROBE_DRM_BUFFER_COUNT; ++index)
    {
        drm_destroy_buffer(display, &display->buffers[index]);
    }
}

static void drm_destroy_buffer(drm_display_t *display, drm_buffer_t *buffer)
{
    struct drm_mode_destroy_dumb destroy_request;

    if (buffer->map != NULL)
    {
        munmap(buffer->map, (size_t)buffer->size);
        buffer->map = NULL;
    }

    if (buffer->framebuffer_id != 0U)
    {
        drmModeRmFB(display->fd, buffer->framebuffer_id);
        buffer->framebuffer_id = 0U;
    }

    if (buffer->handle != 0U)
    {
        memset(&destroy_request, 0, sizeof(destroy_request));
        destroy_request.handle = buffer->handle;
        drmIoctl(display->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy_request);
        buffer->handle = 0U;
    }
}

/***************************************************************************************************
 * LVGL flush path
 **************************************************************************************************/
static void rotated_flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
    probe_context_t *context;
    uint64_t start_ns;
    uint64_t elapsed_ns;
    uint64_t area_pixels;
    uint64_t copy_start_ns;
    uint64_t page_flip_start_ns;
    drm_buffer_t *front_buffer;
    drm_buffer_t *back_buffer;

    context = lv_display_get_driver_data(display);
    if (context == NULL || area == NULL || px_map == NULL)
    {
        lv_display_flush_ready(display);
        return;
    }

    front_buffer = &context->drm.buffers[context->drm.front_buffer_index];
    back_buffer = &context->drm.buffers[context->drm.back_buffer_index];
    if (!context->frame_prepared)
    {
        /*
         * LVGL only flushes dirty rectangles. The back buffer therefore needs to start as a
         * copy of the currently visible frame; then the dirty rotated rectangles are applied
         * on top before the page flip. The screen is small enough that this full copy is
         * cheap, and it avoids touching the visible buffer while the LCD scans it.
         *
         * The probe's forced full-redraw mode is different: every frame repaints the whole
         * screen in strips, so copying the old frame first is wasted work. That special case
         * helps us estimate how expensive full-screen transitions can become after obvious
         * buffering overhead is removed.
         */
        if (!context->full_repaint_each_frame)
        {
            copy_start_ns = now_ns_monotonic();
            memcpy(back_buffer->map, front_buffer->map, (size_t)front_buffer->size);
            context->frame_copy_ns_total += now_ns_monotonic() - copy_start_ns;
        }

        context->frame_prepared = true;
    }

    area_pixels = (uint64_t)(area->x2 - area->x1 + 1) * (uint64_t)(area->y2 - area->y1 + 1);
    start_ns = now_ns_monotonic();
    copy_area_counterclockwise(context, area, (const uint16_t *)px_map);
    elapsed_ns = now_ns_monotonic() - start_ns;

    context->flush_count++;
    context->flushed_pixels += area_pixels;
    context->flush_ns_total += elapsed_ns;
    if (area_pixels > context->largest_area_pixels)
    {
        context->largest_area_pixels = area_pixels;
    }

    if (lv_display_flush_is_last(display))
    {
        page_flip_start_ns = now_ns_monotonic();
        if (drm_page_flip(&context->drm))
        {
            context->page_flip_ns_total += now_ns_monotonic() - page_flip_start_ns;
            context->page_flip_count++;
        }

        context->frame_prepared = false;
    }

    lv_display_flush_ready(display);
}

static void copy_area_counterclockwise(probe_context_t *context, const lv_area_t *area, const uint16_t *pixels)
{
    int32_t area_width;
    int32_t source_x;
    int32_t source_y;

    area_width = area->x2 - area->x1 + 1;

    /*
     * Counterclockwise mapping, matching the earlier raw display probe:
     *
     *   portrait x,y -> landscape x = portrait_height - 1 - y
     *                   landscape y = x
     *
     * The physical buffer may have padding at the end of each line, so use the DRM pitch
     * instead of assuming width * two bytes.
     */
    /*
     * Iterate source X first so each inner loop writes along one physical framebuffer row.
     * The first probe wrote one pixel into many different rows per inner loop, which is
     * harder on the A13 cache and memory bus during full-screen animation.
     */
    for (source_x = area->x1; source_x <= area->x2; ++source_x)
    {
        uint32_t destination_y;
        uint8_t *destination_row;
        uint16_t *destination_pixels;
        int32_t local_x;

        destination_y = (uint32_t)source_x;
        destination_row = context->drm.buffers[context->drm.back_buffer_index].map +
                          ((size_t)destination_y * context->drm.buffers[context->drm.back_buffer_index].pitch);
        destination_pixels = (uint16_t *)destination_row;
        local_x = source_x - area->x1;

        for (source_y = area->y1; source_y <= area->y2; ++source_y)
        {
            int32_t local_y;
            uint32_t destination_x;

            local_y = source_y - area->y1;
            destination_x = (uint32_t)((int32_t)PROBE_PORTRAIT_HEIGHT - 1 - source_y);
            destination_pixels[destination_x] = pixels[(size_t)local_y * (size_t)area_width + (size_t)local_x];
        }
    }
}

static void page_flip_handler(int fd, unsigned int sequence, unsigned int tv_sec, unsigned int tv_usec, void *user_data)
{
    bool *page_flip_complete;

    (void)fd;
    (void)sequence;
    (void)tv_sec;
    (void)tv_usec;

    page_flip_complete = user_data;
    *page_flip_complete = true;
}

/***************************************************************************************************
 * Test scene
 **************************************************************************************************/
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

    title = lv_label_create(screen);
    lv_label_set_text(title, "ROTATED DRM FLUSH");
    lv_obj_set_width(title, 220);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(title, color_from_hex(0xFFFFFF), 0);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, -18);

    moving_box = lv_obj_create(screen);
    lv_obj_remove_style_all(moving_box);
    lv_obj_set_size(moving_box, PROBE_MOVING_BOX_SIZE, PROBE_MOVING_BOX_SIZE);
    lv_obj_set_style_bg_color(moving_box, color_from_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(moving_box, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(moving_box, 4, 0);

    return moving_box;
}

static void update_animation(lv_obj_t *box, uint64_t elapsed_ms)
{
    int32_t usable_width;
    int32_t usable_height;
    int32_t phase;

    usable_width = (int32_t)PROBE_PORTRAIT_WIDTH - PROBE_MOVING_BOX_SIZE;
    usable_height = (int32_t)PROBE_PORTRAIT_HEIGHT - PROBE_MOVING_BOX_SIZE;
    phase = (int32_t)((elapsed_ms / 12U) % (uint64_t)usable_width);

    /*
     * The box moves diagonally so the probe constantly exercises both horizontal and
     * vertical changes. That is closer to real UI animation than a static full redraw.
     */
    lv_obj_set_pos(box, phase, (phase * usable_height) / usable_width);
}

static void print_summary(const probe_context_t *context, uint64_t elapsed_ns)
{
    double elapsed_seconds;
    double flush_ms;
    double average_flush_ms;
    double frame_copy_ms;
    double page_flip_ms;

    elapsed_seconds = (double)elapsed_ns / 1000000000.0;
    flush_ms = (double)context->flush_ns_total / 1000000.0;
    average_flush_ms = context->flush_count == 0U ? 0.0 : flush_ms / (double)context->flush_count;
    frame_copy_ms = (double)context->frame_copy_ns_total / 1000000.0;
    page_flip_ms = (double)context->page_flip_ns_total / 1000000.0;

    printf("lvgl_rotated_drm_probe: completed %.3f seconds\n", elapsed_seconds);
    printf("lvgl_rotated_drm_probe: flushes=%" PRIu64
           " flips=%" PRIu64
           " pixels=%" PRIu64
           " largest_area=%" PRIu64
           " flush_total=%.3f ms"
           " flush_avg=%.3f ms"
           " frame_copy_total=%.3f ms"
           " page_flip_total=%.3f ms\n",
           context->flush_count,
           context->page_flip_count,
           context->flushed_pixels,
           context->largest_area_pixels,
           flush_ms,
           average_flush_ms,
           frame_copy_ms,
           page_flip_ms);
}
