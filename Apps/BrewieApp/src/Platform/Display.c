#include "Display.h"

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "Platform/Display_rotation.h"
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
#include <drm_fourcc.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include "src/drivers/evdev/lv_evdev.h"
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
#define DISPLAY_DRAW_BUFFER_ROWS 48U
#define DISPLAY_DRM_BUFFER_COUNT 2U
#define DISPLAY_TOUCH_DEVICE_PATH "/dev/input/event0"
#define DISPLAY_TOUCH_RAW_X_MIN 0
#define DISPLAY_TOUCH_RAW_X_MAX 799
#define DISPLAY_TOUCH_RAW_Y_MIN 0
#define DISPLAY_TOUCH_RAW_Y_MAX 479

#if defined(__arm__) || defined(__aarch64__)
#if !defined(BREWIE_TARGET_DISPLAY_BACKEND_fbdev)
/*
 * One DRM "dumb buffer". Dumb buffers are simple CPU-writable framebuffers allocated by
 * the kernel. They are not GPU accelerated, but they are predictable and work well for this
 * small appliance display.
 */
typedef struct
{
    uint32_t handle;  // Kernel handle used when destroying the buffer.
    uint32_t pitch;  // Bytes per physical row; may be wider than width * bpp.
    uint64_t size;  // Total mapped byte size.
    uint32_t framebuffer_id;  // DRM framebuffer object id used by page flip.
    uint8_t *map;  // CPU pointer returned by mmap().
} display_drm_buffer_t;

/*
 * Runtime state for the custom rotated DRM backend.
 *
 * DRM scans the panel in physical landscape order. LVGL draws logical portrait rectangles.
 * Two DRM buffers let us draw the next frame off-screen, then swap it onto the panel during
 * vblank. front_buffer_index is visible; back_buffer_index is where the next frame is built.
 */
typedef struct
{
    int fd;  // File descriptor for /dev/dri/card0.
    uint32_t connector_id;
    uint32_t crtc_id;
    drmModeModeInfo mode;
    drmModeCrtc *saved_crtc;
    display_drm_buffer_t buffers[DISPLAY_DRM_BUFFER_COUNT];
    unsigned int front_buffer_index;
    unsigned int back_buffer_index;
    bool frame_prepared;
} display_drm_context_t;

static bool display_drm_init(display_drm_context_t *context);
static bool display_drm_open_card(display_drm_context_t *context, const char *device_path);
static bool display_drm_find_connector_and_crtc(display_drm_context_t *context);
static bool display_drm_create_buffers(display_drm_context_t *context);
static bool display_drm_create_buffer(display_drm_context_t *context, display_drm_buffer_t *buffer);
static bool display_drm_set_scanout(display_drm_context_t *context);
static bool display_drm_page_flip(display_drm_context_t *context);
static void display_drm_destroy_buffers(display_drm_context_t *context);
static void display_drm_destroy_buffer(display_drm_context_t *context, display_drm_buffer_t *buffer);
static void display_drm_flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map);
static void display_drm_page_flip_handler(int fd,
                                          unsigned int sequence,
                                          unsigned int tv_sec,
                                          unsigned int tv_usec,
                                          void *user_data);
static void display_touch_init(lv_display_t *lv_display);

static display_drm_context_t target_drm_context;
#endif
#endif

bool display_init(display_t *display)
{
    lv_display_t *lv_display;
    size_t draw_buffer_pixels;
    uint16_t *draw_buffer_1;
    uint16_t *draw_buffer_2;

    if (display == NULL)
    {
        return false;
    }

    display->ready = false;
    display->simulator = false;
    display->last_tick_ms = 0U;
    draw_buffer_1 = NULL;
    draw_buffer_2 = NULL;

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
    /*
     * Normal target path.
     *
     * LVGL's built-in DRM rotation was tested on the A13 SOM and either hung or crashed.
     * The framebuffer backend rotated correctly but used too much CPU. This custom backend
     * keeps the reliable DRM scanout path while letting LVGL draw in the user-facing
     * portrait orientation. Each LVGL dirty rectangle is rotated into a non-visible DRM
     * buffer, then page-flipped on vblank so moving widgets do not tear.
     */
    memset(&target_drm_context, 0, sizeof(target_drm_context));
    target_drm_context.fd = -1;
    if (!display_drm_init(&target_drm_context))
    {
        log_error("display_init: rotated DRM init failed");
        return false;
    }

    draw_buffer_pixels = DISPLAY_PORTRAIT_WIDTH * DISPLAY_DRAW_BUFFER_ROWS;
    draw_buffer_1 = calloc(draw_buffer_pixels, sizeof(*draw_buffer_1));
    draw_buffer_2 = calloc(draw_buffer_pixels, sizeof(*draw_buffer_2));
    if (draw_buffer_1 == NULL || draw_buffer_2 == NULL)
    {
        log_error("display_init: LVGL draw buffer allocation failed");
        free(draw_buffer_2);
        free(draw_buffer_1);
        return false;
    }

    lv_display = lv_display_create(DISPLAY_PORTRAIT_WIDTH, DISPLAY_PORTRAIT_HEIGHT);
    if (lv_display == NULL)
    {
        log_error("display_init: rotated DRM LVGL display create failed");
        free(draw_buffer_2);
        free(draw_buffer_1);
        return false;
    }

    lv_display_set_driver_data(lv_display, &target_drm_context);
    lv_display_set_flush_cb(lv_display, display_drm_flush_cb);
    lv_display_set_buffers(lv_display,
                           draw_buffer_1,
                           draw_buffer_2,
                           (uint32_t)(draw_buffer_pixels * sizeof(*draw_buffer_1)),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
    display_touch_init(lv_display);

    display->ready = true;
    display->simulator = false;
    log_info("display_init: rotated DRM display ready");
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

#if defined(__arm__) || defined(__aarch64__)
#if !defined(BREWIE_TARGET_DISPLAY_BACKEND_fbdev)
static bool display_drm_init(display_drm_context_t *context)
{
    /*
     * Bring-up order matters: open card, find the connected display pipe, allocate buffers,
     * then tell DRM which buffer should be scanned out.
     */
    if (!display_drm_open_card(context, "/dev/dri/card0"))
    {
        return false;
    }

    if (!display_drm_find_connector_and_crtc(context))
    {
        return false;
    }

    if (!display_drm_create_buffers(context))
    {
        return false;
    }

    if (!display_drm_set_scanout(context))
    {
        return false;
    }

    log_info("display_drm_init: physical 480x272 scanout, logical 272x480 UI");
    return true;
}

static bool display_drm_open_card(display_drm_context_t *context, const char *device_path)
{
    uint64_t has_dumb_buffer;

    context->fd = open(device_path, O_RDWR | O_CLOEXEC);
    if (context->fd < 0)
    {
        log_error("display_drm_open_card: open failed");
        return false;
    }

    if (drmGetCap(context->fd, DRM_CAP_DUMB_BUFFER, &has_dumb_buffer) != 0 || has_dumb_buffer == 0U)
    {
        log_error("display_drm_open_card: DRM dumb buffers unavailable");
        return false;
    }

    return true;
}

static bool display_drm_find_connector_and_crtc(display_drm_context_t *context)
{
    drmModeRes *resources;
    drmModeConnector *connector;
    drmModeEncoder *encoder;
    int crtc_index;
    int connector_index;
    int encoder_index;

    resources = drmModeGetResources(context->fd);
    if (resources == NULL)
    {
        log_error("display_drm_find_connector_and_crtc: resources unavailable");
        return false;
    }

    for (connector_index = 0; connector_index < resources->count_connectors; ++connector_index)
    {
        connector = drmModeGetConnector(context->fd, resources->connectors[connector_index]);
        if (connector == NULL)
        {
            continue;
        }

        if (connector->connection == DRM_MODE_CONNECTED && connector->count_modes > 0)
        {
            /*
             * Connector is the physical/logical output. CRTC is the scanout engine that
             * drives it. On this machine there is only one real panel, but the code still
             * asks DRM instead of hard-coding object ids.
             */
            context->connector_id = connector->connector_id;
            context->mode = connector->modes[0];

            for (encoder_index = 0; encoder_index < connector->count_encoders; ++encoder_index)
            {
                encoder = drmModeGetEncoder(context->fd, connector->encoders[encoder_index]);
                if (encoder == NULL)
                {
                    continue;
                }

                if (encoder->crtc_id != 0U)
                {
                    context->crtc_id = encoder->crtc_id;
                }
                else
                {
                    /*
                     * If the display was idle before the service started, no CRTC may be
                     * active yet. Pick the first compatible CRTC from the encoder mask.
                     */
                    for (crtc_index = 0; crtc_index < resources->count_crtcs; ++crtc_index)
                    {
                        if ((encoder->possible_crtcs & (1 << crtc_index)) != 0)
                        {
                            context->crtc_id = resources->crtcs[crtc_index];
                            break;
                        }
                    }
                }

                drmModeFreeEncoder(encoder);
                if (context->crtc_id != 0U)
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

    if (context->connector_id == 0U || context->crtc_id == 0U)
    {
        log_error("display_drm_find_connector_and_crtc: connected pipe not found");
        return false;
    }

    if (context->mode.hdisplay != DISPLAY_PHYSICAL_WIDTH || context->mode.vdisplay != DISPLAY_PHYSICAL_HEIGHT)
    {
        log_error("display_drm_find_connector_and_crtc: unexpected physical mode");
        return false;
    }

    context->saved_crtc = drmModeGetCrtc(context->fd, context->crtc_id);
    return true;
}

static bool display_drm_create_buffers(display_drm_context_t *context)
{
    context->front_buffer_index = 0U;
    context->back_buffer_index = 1U;

    if (!display_drm_create_buffer(context, &context->buffers[0]))
    {
        return false;
    }

    if (!display_drm_create_buffer(context, &context->buffers[1]))
    {
        return false;
    }

    return true;
}

static bool display_drm_create_buffer(display_drm_context_t *context, display_drm_buffer_t *buffer)
{
    struct drm_mode_create_dumb create_request;
    struct drm_mode_map_dumb map_request;
    uint32_t handles[4];
    uint32_t pitches[4];
    uint32_t offsets[4];

    memset(&create_request, 0, sizeof(create_request));
    /*
     * RGB565 matches LVGL's target color depth and halves memory bandwidth compared with
     * 32-bit pixels, which matters on the A13 SOM.
     */
    create_request.width = DISPLAY_PHYSICAL_WIDTH;
    create_request.height = DISPLAY_PHYSICAL_HEIGHT;
    create_request.bpp = 16U;

    if (drmIoctl(context->fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_request) != 0)
    {
        log_error("display_drm_create_buffer: create dumb buffer failed");
        return false;
    }

    buffer->handle = create_request.handle;
    buffer->pitch = create_request.pitch;
    buffer->size = create_request.size;

    memset(&map_request, 0, sizeof(map_request));
    map_request.handle = buffer->handle;
    if (drmIoctl(context->fd, DRM_IOCTL_MODE_MAP_DUMB, &map_request) != 0)
    {
        log_error("display_drm_create_buffer: map dumb buffer failed");
        return false;
    }

    buffer->map = mmap(NULL, buffer->size, PROT_READ | PROT_WRITE, MAP_SHARED, context->fd, map_request.offset);
    if (buffer->map == MAP_FAILED)
    {
        buffer->map = NULL;
        log_error("display_drm_create_buffer: mmap failed");
        return false;
    }

    memset(buffer->map, 0, (size_t)buffer->size);
    memset(handles, 0, sizeof(handles));
    memset(pitches, 0, sizeof(pitches));
    memset(offsets, 0, sizeof(offsets));
    handles[0] = buffer->handle;
    pitches[0] = buffer->pitch;

    if (drmModeAddFB2(context->fd,
                      DISPLAY_PHYSICAL_WIDTH,
                      DISPLAY_PHYSICAL_HEIGHT,
                      DRM_FORMAT_RGB565,
                      handles,
                      pitches,
                      offsets,
                      &buffer->framebuffer_id,
                      0) != 0)
    {
        log_error("display_drm_create_buffer: framebuffer create failed");
        return false;
    }

    return true;
}

static bool display_drm_set_scanout(display_drm_context_t *context)
{
    if (drmModeSetCrtc(context->fd,
                       context->crtc_id,
                       context->buffers[context->front_buffer_index].framebuffer_id,
                       0,
                       0,
                       &context->connector_id,
                       1,
                       &context->mode) != 0)
    {
        log_error("display_drm_set_scanout: set CRTC failed");
        return false;
    }

    return true;
}

static bool display_drm_page_flip(display_drm_context_t *context)
{
    drmEventContext event_context;
    struct pollfd page_flip_poll_request;
    bool page_flip_complete;

    page_flip_complete = false;
    /*
     * Page flip asks the kernel to make the back buffer visible on the next vblank. Waiting
     * for the event keeps our front/back buffer bookkeeping correct and avoids tearing.
     */
    if (drmModePageFlip(context->fd,
                        context->crtc_id,
                        context->buffers[context->back_buffer_index].framebuffer_id,
                        DRM_MODE_PAGE_FLIP_EVENT,
                        &page_flip_complete) != 0)
    {
        log_error("display_drm_page_flip: page flip request failed");
        return false;
    }

    memset(&event_context, 0, sizeof(event_context));
    event_context.version = DRM_EVENT_CONTEXT_VERSION;
    event_context.page_flip_handler = display_drm_page_flip_handler;

    while (!page_flip_complete)
    {
        page_flip_poll_request.fd = context->fd;
        page_flip_poll_request.events = POLLIN;
        page_flip_poll_request.revents = 0;

        if (poll(&page_flip_poll_request, 1, 1000) <= 0)
        {
            log_error("display_drm_page_flip: timed out waiting for vblank");
            return false;
        }

        if (drmHandleEvent(context->fd, &event_context) != 0)
        {
            log_error("display_drm_page_flip: event handling failed");
            return false;
        }
    }

    context->front_buffer_index = context->back_buffer_index;
    context->back_buffer_index = 1U - context->front_buffer_index;
    return true;
}

static void display_drm_destroy_buffers(display_drm_context_t *context)
{
    unsigned int index;

    for (index = 0U; index < DISPLAY_DRM_BUFFER_COUNT; ++index)
    {
        display_drm_destroy_buffer(context, &context->buffers[index]);
    }
}

static void display_drm_destroy_buffer(display_drm_context_t *context, display_drm_buffer_t *buffer)
{
    struct drm_mode_destroy_dumb destroy_request;

    if (buffer->map != NULL)
    {
        munmap(buffer->map, (size_t)buffer->size);
        buffer->map = NULL;
    }

    if (buffer->framebuffer_id != 0U)
    {
        drmModeRmFB(context->fd, buffer->framebuffer_id);
        buffer->framebuffer_id = 0U;
    }

    if (buffer->handle != 0U)
    {
        memset(&destroy_request, 0, sizeof(destroy_request));
        destroy_request.handle = buffer->handle;
        drmIoctl(context->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy_request);
        buffer->handle = 0U;
    }
}

static void display_drm_flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
    display_drm_context_t *context;
    display_drm_buffer_t *front_buffer;
    display_drm_buffer_t *back_buffer;

    context = lv_display_get_driver_data(display);
    if (context == NULL || area == NULL || px_map == NULL)
    {
        lv_display_flush_ready(display);
        return;
    }

    front_buffer = &context->buffers[context->front_buffer_index];
    back_buffer = &context->buffers[context->back_buffer_index];
    if (!context->frame_prepared)
    {
        /*
         * LVGL sends only dirty rectangles. Copy the visible frame into the back buffer
         * before applying those dirty rectangles, otherwise unchanged pixels would contain
         * stale data from an older frame.
         */
        memcpy(back_buffer->map, front_buffer->map, (size_t)front_buffer->size);
        context->frame_prepared = true;
    }

    display_rotation_copy_counterclockwise_rgb565(back_buffer->map,
                                                  back_buffer->pitch,
                                                  area,
                                                  (const uint16_t *)px_map,
                                                  DISPLAY_PORTRAIT_HEIGHT);

    if (lv_display_flush_is_last(display))
    {
        if (!display_drm_page_flip(context))
        {
            log_error("display_drm_flush_cb: page flip failed");
        }

        context->frame_prepared = false;
    }

    lv_display_flush_ready(display);
}

static void display_drm_page_flip_handler(int fd,
                                          unsigned int sequence,
                                          unsigned int tv_sec,
                                          unsigned int tv_usec,
                                          void *user_data)
{
    bool *page_flip_complete;

    (void)fd;
    (void)sequence;
    (void)tv_sec;
    (void)tv_usec;

    page_flip_complete = user_data;
    *page_flip_complete = true;
}

static void display_touch_init(lv_display_t *lv_display)
{
    lv_indev_t *touch;

    /*
     * The Goodix controller reports the touch surface in the panel's raw landscape axes:
     * X is 0..799 and Y is 0..479. The UI is logical portrait 272x480, so input must take
     * the same physical mounting into account as the display flush path:
     *
     *   logical x = inverted/scaled raw Y
     *   logical y = scaled raw X
     *
     * LVGL's evdev driver can express that with axis swap plus reversed raw-Y calibration.
     */
    touch = lv_evdev_create(LV_INDEV_TYPE_POINTER, DISPLAY_TOUCH_DEVICE_PATH);
    if (touch == NULL)
    {
        log_error("display_touch_init: Goodix evdev open failed");
        return;
    }

    lv_indev_set_display(touch, lv_display);
    lv_evdev_set_swap_axes(touch, true);
    lv_evdev_set_calibration(touch,
                             DISPLAY_TOUCH_RAW_Y_MAX,
                             DISPLAY_TOUCH_RAW_X_MIN,
                             DISPLAY_TOUCH_RAW_Y_MIN,
                             DISPLAY_TOUCH_RAW_X_MAX);
    log_info("display_touch_init: Goodix touch ready");
}
#endif
#endif
