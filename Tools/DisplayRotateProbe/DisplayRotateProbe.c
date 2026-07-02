#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <linux/fb.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

/*
 * DisplayRotateProbe is a small standalone experiment for the Brewie display problem.
 *
 * The real LCD timing is 480x272 landscape, while the panel is mounted as portrait in the
 * appliance. Before wiring a custom rotation backend into LVGL, this tool measures the
 * exact operation we would need: rotate a logical 272x480 RGB565 portrait frame into the
 * physical 480x272 RGB565 framebuffer layout.
 *
 * It can run as a pure memory benchmark, or it can mmap /dev/fb0 and write an animated
 * test pattern while brewie.service is stopped. The code is intentionally plain C and
 * heavily commented so the result is easy to reason about on the small A13 SOM.
 */

#define PORTRAIT_WIDTH 272U
#define PORTRAIT_HEIGHT 480U
#define PHYSICAL_WIDTH PORTRAIT_HEIGHT
#define PHYSICAL_HEIGHT PORTRAIT_WIDTH
#define DEFAULT_FRAMES 300U

typedef enum
{
    ROTATION_CLOCKWISE,
    ROTATION_COUNTERCLOCKWISE
} rotation_direction_t;

typedef struct
{
    uint8_t *pixels;
    size_t mapped_bytes;
    size_t line_length;
    uint32_t bits_per_pixel;
    uint32_t red_length;
    uint32_t red_offset;
    uint32_t green_length;
    uint32_t green_offset;
    uint32_t blue_length;
    uint32_t blue_offset;
    int fd;
} framebuffer_t;

static int parse_args(int argc, char **argv, unsigned int *frames, bool *write_fb, rotation_direction_t *direction);
static void print_usage(const char *program_name);
static uint64_t now_ns_monotonic();
static uint16_t make_rgb565(uint8_t red, uint8_t green, uint8_t blue);
static void fill_portrait_pattern(uint16_t *source, unsigned int frame_number);
static void rotate_clockwise(const uint16_t *source, uint16_t *destination, size_t destination_stride_pixels);
static void rotate_counterclockwise(const uint16_t *source, uint16_t *destination, size_t destination_stride_pixels);
static uint32_t checksum_frame(const uint16_t *pixels, size_t count);
static bool open_framebuffer(framebuffer_t *framebuffer, const char *path);
static void close_framebuffer(framebuffer_t *framebuffer);
static void copy_rgb565_to_framebuffer(const uint16_t *source, const framebuffer_t *framebuffer);
static uint32_t expand_component(uint32_t value, uint32_t bits);
static uint32_t pack_framebuffer_pixel(const framebuffer_t *framebuffer, uint16_t rgb565);
static void sleep_until_next_frame(uint64_t frame_start_ns);

/***************************************************************************************************
 * main
 **************************************************************************************************/
int main(int argc, char **argv)
{
    unsigned int frames;
    bool write_fb;
    rotation_direction_t direction;
    uint16_t *source;
    uint16_t *destination;
    framebuffer_t framebuffer;
    uint64_t start_ns;
    uint64_t rotate_ns_total;
    uint64_t elapsed_ns;
    uint32_t final_checksum;
    unsigned int frame;
    int exit_code;

    frames = DEFAULT_FRAMES;
    write_fb = false;
    direction = ROTATION_CLOCKWISE;
    source = NULL;
    destination = NULL;
    memset(&framebuffer, 0, sizeof(framebuffer));
    framebuffer.fd = -1;
    rotate_ns_total = 0U;
    final_checksum = 0U;
    exit_code = 0;

    if (parse_args(argc, argv, &frames, &write_fb, &direction) != 0)
    {
        return 2;
    }

    source = calloc(PORTRAIT_WIDTH * PORTRAIT_HEIGHT, sizeof(*source));
    destination = calloc(PHYSICAL_WIDTH * PHYSICAL_HEIGHT, sizeof(*destination));
    if (source == NULL || destination == NULL)
    {
        fprintf(stderr, "ERROR: not enough memory for test buffers\n");
        exit_code = 1;
        goto done;
    }

    if (write_fb && !open_framebuffer(&framebuffer, "/dev/fb0"))
    {
        exit_code = 1;
        goto done;
    }

    printf("rotate_probe: source=%ux%u portrait, destination=%ux%u landscape, frames=%u, output=%s\n",
           PORTRAIT_WIDTH,
           PORTRAIT_HEIGHT,
           PHYSICAL_WIDTH,
           PHYSICAL_HEIGHT,
           frames,
           write_fb ? "/dev/fb0" : "memory");

    start_ns = now_ns_monotonic();
    for (frame = 0U; frame < frames; ++frame)
    {
        uint64_t frame_start_ns;
        uint64_t rotate_start_ns;
        uint64_t rotate_end_ns;

        frame_start_ns = now_ns_monotonic();
        fill_portrait_pattern(source, frame);

        rotate_start_ns = now_ns_monotonic();
        if (direction == ROTATION_CLOCKWISE)
        {
            rotate_clockwise(source, destination, PHYSICAL_WIDTH);
        }
        else
        {
            rotate_counterclockwise(source, destination, PHYSICAL_WIDTH);
        }
        rotate_end_ns = now_ns_monotonic();
        rotate_ns_total += rotate_end_ns - rotate_start_ns;

        if (write_fb)
        {
            copy_rgb565_to_framebuffer(destination, &framebuffer);
            sleep_until_next_frame(frame_start_ns);
        }

        final_checksum = checksum_frame(destination, PHYSICAL_WIDTH * PHYSICAL_HEIGHT);
    }

    elapsed_ns = now_ns_monotonic() - start_ns;
    printf("rotate_probe: total %.3f ms, rotate-only %.3f ms, avg rotate %.3f ms/frame, %.1f rotate-fps, checksum=%" PRIu32 "\n",
           (double)elapsed_ns / 1000000.0,
           (double)rotate_ns_total / 1000000.0,
           ((double)rotate_ns_total / 1000000.0) / (double)frames,
           1000000000.0 / ((double)rotate_ns_total / (double)frames),
           final_checksum);

done:
    close_framebuffer(&framebuffer);
    free(destination);
    free(source);
    return exit_code;
}

/***************************************************************************************************
 * Argument parsing and help
 **************************************************************************************************/
static int parse_args(int argc, char **argv, unsigned int *frames, bool *write_fb, rotation_direction_t *direction)
{
    int index;

    for (index = 1; index < argc; ++index)
    {
        if (strcmp(argv[index], "--frames") == 0 && (index + 1) < argc)
        {
            *frames = (unsigned int)strtoul(argv[++index], NULL, 10);
            if (*frames == 0U)
            {
                fprintf(stderr, "ERROR: --frames must be greater than zero\n");
                return -1;
            }
        }
        else if (strcmp(argv[index], "--write-fb") == 0)
        {
            *write_fb = true;
        }
        else if (strcmp(argv[index], "--cw") == 0)
        {
            *direction = ROTATION_CLOCKWISE;
        }
        else if (strcmp(argv[index], "--ccw") == 0)
        {
            *direction = ROTATION_COUNTERCLOCKWISE;
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
            "usage: %s [--frames N] [--write-fb] [--cw|--ccw]\n"
            "\n"
            "  --frames N   Number of generated frames. Default: %u\n"
            "  --write-fb   Also write the rotated frames to /dev/fb0. Stop brewie.service first.\n"
            "  --cw         Rotate portrait source clockwise into landscape output. Default.\n"
            "  --ccw        Rotate portrait source counterclockwise into landscape output.\n",
            program_name,
            DEFAULT_FRAMES);
}

/***************************************************************************************************
 * Time and test-pattern helpers
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

static uint16_t make_rgb565(uint8_t red, uint8_t green, uint8_t blue)
{
    return (uint16_t)((red & 0xF8U) << 8U) |
           (uint16_t)((green & 0xFCU) << 3U) |
           (uint16_t)(blue >> 3U);
}

static void fill_portrait_pattern(uint16_t *source, unsigned int frame_number)
{
    size_t x;
    size_t y;
    size_t moving_x;
    size_t moving_y;

    /*
     * The pattern deliberately has different colors in each corner and a moving white box.
     * That makes it easy to tell which rotation direction is correct when writing to /dev/fb0.
     */
    moving_x = frame_number % (PORTRAIT_WIDTH - 36U);
    moving_y = (frame_number * 2U) % (PORTRAIT_HEIGHT - 36U);

    for (y = 0U; y < PORTRAIT_HEIGHT; ++y)
    {
        for (x = 0U; x < PORTRAIT_WIDTH; ++x)
        {
            uint8_t red;
            uint8_t green;
            uint8_t blue;

            red = (uint8_t)((x * 255U) / (PORTRAIT_WIDTH - 1U));
            green = (uint8_t)((y * 255U) / (PORTRAIT_HEIGHT - 1U));
            blue = (uint8_t)(((x + y + frame_number) * 3U) & 0xFFU);

            if (x < 28U && y < 28U)
            {
                source[y * PORTRAIT_WIDTH + x] = make_rgb565(255U, 0U, 0U);
            }
            else if (x >= PORTRAIT_WIDTH - 28U && y < 28U)
            {
                source[y * PORTRAIT_WIDTH + x] = make_rgb565(0U, 255U, 0U);
            }
            else if (x < 28U && y >= PORTRAIT_HEIGHT - 28U)
            {
                source[y * PORTRAIT_WIDTH + x] = make_rgb565(0U, 0U, 255U);
            }
            else if (x >= moving_x && x < moving_x + 36U && y >= moving_y && y < moving_y + 36U)
            {
                source[y * PORTRAIT_WIDTH + x] = make_rgb565(255U, 255U, 255U);
            }
            else
            {
                source[y * PORTRAIT_WIDTH + x] = make_rgb565(red, green, blue);
            }
        }
    }
}

/***************************************************************************************************
 * Rotation kernels
 **************************************************************************************************/
static void rotate_clockwise(const uint16_t *source, uint16_t *destination, size_t destination_stride_pixels)
{
    size_t source_x;
    size_t source_y;

    for (source_y = 0U; source_y < PORTRAIT_HEIGHT; ++source_y)
    {
        for (source_x = 0U; source_x < PORTRAIT_WIDTH; ++source_x)
        {
            size_t destination_x;
            size_t destination_y;

            destination_x = source_y;
            destination_y = (PORTRAIT_WIDTH - 1U) - source_x;
            destination[destination_y * destination_stride_pixels + destination_x] =
                source[source_y * PORTRAIT_WIDTH + source_x];
        }
    }
}

static void rotate_counterclockwise(const uint16_t *source, uint16_t *destination, size_t destination_stride_pixels)
{
    size_t source_x;
    size_t source_y;

    for (source_y = 0U; source_y < PORTRAIT_HEIGHT; ++source_y)
    {
        for (source_x = 0U; source_x < PORTRAIT_WIDTH; ++source_x)
        {
            size_t destination_x;
            size_t destination_y;

            destination_x = (PORTRAIT_HEIGHT - 1U) - source_y;
            destination_y = source_x;
            destination[destination_y * destination_stride_pixels + destination_x] =
                source[source_y * PORTRAIT_WIDTH + source_x];
        }
    }
}

static uint32_t checksum_frame(const uint16_t *pixels, size_t count)
{
    size_t index;
    uint32_t checksum;

    checksum = 2166136261U;
    for (index = 0U; index < count; ++index)
    {
        checksum ^= pixels[index];
        checksum *= 16777619U;
    }

    return checksum;
}

/***************************************************************************************************
 * Framebuffer output
 **************************************************************************************************/
static bool open_framebuffer(framebuffer_t *framebuffer, const char *path)
{
    struct fb_var_screeninfo variable_info;
    struct fb_fix_screeninfo fixed_info;

    framebuffer->fd = open(path, O_RDWR);
    if (framebuffer->fd < 0)
    {
        fprintf(stderr, "ERROR: open(%s) failed: %s\n", path, strerror(errno));
        return false;
    }

    if (ioctl(framebuffer->fd, FBIOGET_VSCREENINFO, &variable_info) != 0 ||
        ioctl(framebuffer->fd, FBIOGET_FSCREENINFO, &fixed_info) != 0)
    {
        fprintf(stderr, "ERROR: framebuffer ioctl failed: %s\n", strerror(errno));
        close_framebuffer(framebuffer);
        return false;
    }

    if (variable_info.xres < PHYSICAL_WIDTH || variable_info.yres < PHYSICAL_HEIGHT)
    {
        fprintf(stderr,
                "ERROR: framebuffer is too small: got %ux%u, need at least %ux%u\n",
                variable_info.xres,
                variable_info.yres,
                PHYSICAL_WIDTH,
                PHYSICAL_HEIGHT);
        close_framebuffer(framebuffer);
        return false;
    }

    if (variable_info.bits_per_pixel != 16U && variable_info.bits_per_pixel != 32U)
    {
        fprintf(stderr,
                "ERROR: expected 16 bpp or 32 bpp framebuffer, got %u bpp\n",
                variable_info.bits_per_pixel);
        close_framebuffer(framebuffer);
        return false;
    }

    framebuffer->line_length = fixed_info.line_length;
    framebuffer->bits_per_pixel = variable_info.bits_per_pixel;
    framebuffer->red_length = variable_info.red.length;
    framebuffer->red_offset = variable_info.red.offset;
    framebuffer->green_length = variable_info.green.length;
    framebuffer->green_offset = variable_info.green.offset;
    framebuffer->blue_length = variable_info.blue.length;
    framebuffer->blue_offset = variable_info.blue.offset;
    framebuffer->mapped_bytes = fixed_info.smem_len;
    framebuffer->pixels = mmap(NULL, framebuffer->mapped_bytes, PROT_READ | PROT_WRITE, MAP_SHARED, framebuffer->fd, 0);
    if (framebuffer->pixels == MAP_FAILED)
    {
        fprintf(stderr, "ERROR: mmap(%s) failed: %s\n", path, strerror(errno));
        framebuffer->pixels = NULL;
        close_framebuffer(framebuffer);
        return false;
    }

    printf("rotate_probe: fb0 %ux%u, %u bpp, line_length=%u, rgb offsets=%u/%u/%u lengths=%u/%u/%u, mapped=%zu bytes\n",
           variable_info.xres,
           variable_info.yres,
           variable_info.bits_per_pixel,
           fixed_info.line_length,
           variable_info.red.offset,
           variable_info.green.offset,
           variable_info.blue.offset,
           variable_info.red.length,
           variable_info.green.length,
           variable_info.blue.length,
           framebuffer->mapped_bytes);

    return true;
}

static void close_framebuffer(framebuffer_t *framebuffer)
{
    if (framebuffer->pixels != NULL)
    {
        munmap(framebuffer->pixels, framebuffer->mapped_bytes);
        framebuffer->pixels = NULL;
    }

    if (framebuffer->fd >= 0)
    {
        close(framebuffer->fd);
        framebuffer->fd = -1;
    }
}

static void copy_rgb565_to_framebuffer(const uint16_t *source, const framebuffer_t *framebuffer)
{
    size_t x;
    size_t y;

    /*
     * The rotation benchmark stays RGB565 because that is the compact format we would like
     * for an efficient custom backend. /dev/fb0 on the Olimex image may expose either
     * RGB565 or 32 bpp, so this final copy adapts the test frame to the actual fbdev format.
     */
    for (y = 0U; y < PHYSICAL_HEIGHT; ++y)
    {
        uint8_t *row;

        row = &framebuffer->pixels[y * framebuffer->line_length];
        for (x = 0U; x < PHYSICAL_WIDTH; ++x)
        {
            uint16_t rgb565;

            rgb565 = source[y * PHYSICAL_WIDTH + x];
            if (framebuffer->bits_per_pixel == 16U)
            {
                ((uint16_t *)row)[x] = rgb565;
            }
            else
            {
                ((uint32_t *)row)[x] = pack_framebuffer_pixel(framebuffer, rgb565);
            }
        }
    }
}

static uint32_t expand_component(uint32_t value, uint32_t bits)
{
    if (bits == 0U)
    {
        return 0U;
    }

    if (bits >= 8U)
    {
        return value << (bits - 8U);
    }

    return value >> (8U - bits);
}

static uint32_t pack_framebuffer_pixel(const framebuffer_t *framebuffer, uint16_t rgb565)
{
    uint32_t red;
    uint32_t green;
    uint32_t blue;

    red = ((uint32_t)(rgb565 >> 11U) & 0x1FU) << 3U;
    green = ((uint32_t)(rgb565 >> 5U) & 0x3FU) << 2U;
    blue = ((uint32_t)rgb565 & 0x1FU) << 3U;

    return (expand_component(red, framebuffer->red_length) << framebuffer->red_offset) |
           (expand_component(green, framebuffer->green_length) << framebuffer->green_offset) |
           (expand_component(blue, framebuffer->blue_length) << framebuffer->blue_offset);
}

static void sleep_until_next_frame(uint64_t frame_start_ns)
{
    const uint64_t target_frame_ns = 33333333ULL;
    uint64_t now_ns;

    now_ns = now_ns_monotonic();
    if ((now_ns - frame_start_ns) < target_frame_ns)
    {
        uint64_t remaining_ns;
        struct timespec sleep_time;

        remaining_ns = target_frame_ns - (now_ns - frame_start_ns);
        sleep_time.tv_sec = (time_t)(remaining_ns / 1000000000ULL);
        sleep_time.tv_nsec = (long)(remaining_ns % 1000000000ULL);
        nanosleep(&sleep_time, NULL);
    }
}
