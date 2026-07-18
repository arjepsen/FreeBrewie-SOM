#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Platform/Display_rotation.h"

/****************************************************************************************
 * @file DisplayRotationBenchmark.c
 * @brief Standalone benchmark for rotated RGB565 dirty-rectangle copy kernels.
 *
 * Responsibility: compare the explicit scalar rotation path with the production
 * best-available rotation path on the real SOM target.
 * Owns: benchmark buffers, timing, and checksum validation.
 *
 * This tool does not touch DRM, LVGL timers, input, or brewie.service. It measures the
 * memory operation only: copy a logical portrait LVGL dirty rectangle into the physical
 * landscape framebuffer layout. That keeps the result focused on the rotation algorithm.
 ****************************************************************************************/

#define LOGICAL_WIDTH 272U
#define LOGICAL_HEIGHT 480U
#define PHYSICAL_WIDTH LOGICAL_HEIGHT
#define PHYSICAL_HEIGHT LOGICAL_WIDTH
#define BYTES_PER_PIXEL 2U
#define DESTINATION_PITCH_BYTES (PHYSICAL_WIDTH * BYTES_PER_PIXEL)
#define DESTINATION_BUFFER_BYTES (DESTINATION_PITCH_BYTES * PHYSICAL_HEIGHT)
#define DEFAULT_ITERATIONS 2000U

typedef struct
{
    const char *name;
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
    uint32_t iterations;
} benchmark_case_t;

typedef struct
{
    double total_ms;
    uint32_t checksum;
} benchmark_result_t;

static int parse_args(int argc, char **argv, uint32_t *iterations);
static void print_usage(const char *program_name);
static bool allocate_aligned(void **memory, size_t bytes);
static uint64_t now_ns_monotonic();
static uint16_t make_test_pixel(uint32_t x, uint32_t y);
static void fill_source(uint16_t *source_pixels, uint32_t width, uint32_t height);
static uint32_t checksum_bytes(const uint8_t *bytes, size_t byte_count);
static benchmark_result_t run_app_scalar_case(const benchmark_case_t *test_case,
                                              const uint16_t *source_pixels,
                                              uint8_t *destination_buffer);
static benchmark_result_t run_experimental_case(const benchmark_case_t *test_case,
                                                const uint16_t *source_pixels,
                                                uint8_t *destination_buffer);

/****************************************************************************************
 * main
 ****************************************************************************************/
int main(int argc, char **argv)
{
    static const benchmark_case_t default_cases[] = {
        {"small_button_40x40", 24U, 40U, 40U, 40U, DEFAULT_ITERATIONS},
        {"medium_panel_160x80", 48U, 120U, 160U, 80U, DEFAULT_ITERATIONS},
        {"lvgl_strip_272x48", 0U, 96U, 272U, 48U, DEFAULT_ITERATIONS},
        {"half_screen_272x240", 0U, 120U, 272U, 240U, DEFAULT_ITERATIONS / 2U},
        {"full_screen_272x480", 0U, 0U, 272U, 480U, DEFAULT_ITERATIONS / 4U},
    };
    uint32_t iteration_override;
    uint16_t *source_pixels;
    uint8_t *destination_buffer;
    size_t source_pixels_count;
    size_t case_index;
    int exit_code;

    iteration_override = 0U;
    source_pixels = NULL;
    destination_buffer = NULL;
    exit_code = 0;

    if (parse_args(argc, argv, &iteration_override) != 0)
    {
        return 2;
    }

    source_pixels_count = LOGICAL_WIDTH * LOGICAL_HEIGHT;
    if (!allocate_aligned((void **)&source_pixels, source_pixels_count * sizeof(*source_pixels)) ||
        !allocate_aligned((void **)&destination_buffer, DESTINATION_BUFFER_BYTES))
    {
        fprintf(stderr, "display_rotation_benchmark: allocation failed\n");
        exit_code = 1;
        goto done;
    }

    fill_source(source_pixels, LOGICAL_WIDTH, LOGICAL_HEIGHT);

    printf("display_rotation_benchmark: logical=%ux%u physical=%ux%u pitch=%u bytes\n",
           LOGICAL_WIDTH,
           LOGICAL_HEIGHT,
           PHYSICAL_WIDTH,
           PHYSICAL_HEIGHT,
           DESTINATION_PITCH_BYTES);
    printf("display_rotation_benchmark: app_scalar is the explicit scalar fallback path\n");
    printf("display_rotation_benchmark: app_best is the production best-available path\n\n");

    for (case_index = 0U; case_index < (sizeof(default_cases) / sizeof(default_cases[0])); ++case_index)
    {
        benchmark_case_t test_case;
        benchmark_result_t scalar_result;
        benchmark_result_t experimental_result;
        double scalar_us;
        double experimental_us;

        test_case = default_cases[case_index];
        if (iteration_override > 0U)
        {
            test_case.iterations = iteration_override;
        }

        scalar_result = run_app_scalar_case(&test_case, source_pixels, destination_buffer);
        experimental_result = run_experimental_case(&test_case, source_pixels, destination_buffer);

        scalar_us = (scalar_result.total_ms * 1000.0) / (double)test_case.iterations;
        experimental_us = (experimental_result.total_ms * 1000.0) / (double)test_case.iterations;

        printf("%-22s %3ux%-3u iterations=%5u\n",
               test_case.name,
               test_case.width,
               test_case.height,
               test_case.iterations);
        printf("  app_scalar : %9.3f ms total  %8.3f us/copy  checksum=%" PRIu32 "\n",
               scalar_result.total_ms,
               scalar_us,
               scalar_result.checksum);
        printf("  app_best   : %9.3f ms total  %8.3f us/copy  checksum=%" PRIu32,
               experimental_result.total_ms,
               experimental_us,
               experimental_result.checksum);

        if (scalar_result.checksum != experimental_result.checksum)
        {
            printf("  MISMATCH\n\n");
            exit_code = 1;
        }
        else if (experimental_us > 0.0)
        {
            printf("  speedup=%.2fx\n\n", scalar_us / experimental_us);
        }
        else
        {
            printf("\n\n");
        }
    }

done:
    free(destination_buffer);
    free(source_pixels);
    return exit_code;
}

/****************************************************************************************
 * Argument Parsing
 ****************************************************************************************/
static int parse_args(int argc, char **argv, uint32_t *iterations)
{
    int index;

    for (index = 1; index < argc; ++index)
    {
        if (strcmp(argv[index], "--iterations") == 0 && (index + 1) < argc)
        {
            *iterations = (uint32_t)strtoul(argv[++index], NULL, 10);
            if (*iterations == 0U)
            {
                fprintf(stderr, "display_rotation_benchmark: --iterations must be greater than zero\n");
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
            "usage: %s [--iterations N]\n"
            "\n"
            "Runs fixed dirty-rectangle rotation cases and compares the explicit scalar\n"
            "fallback with the production best-available path. No display device is opened.\n",
            program_name);
}

/****************************************************************************************
 * Test Data And Timing
 ****************************************************************************************/
static bool allocate_aligned(void **memory, size_t bytes)
{
    int result;

    result = posix_memalign(memory, 16U, bytes);
    if (result != 0)
    {
        errno = result;
        return false;
    }

    memset(*memory, 0, bytes);
    return true;
}

static uint64_t now_ns_monotonic()
{
    struct timespec time_value;

    if (clock_gettime(CLOCK_MONOTONIC, &time_value) != 0)
    {
        return 0U;
    }

    return ((uint64_t)time_value.tv_sec * 1000000000ULL) + (uint64_t)time_value.tv_nsec;
}

static uint16_t make_test_pixel(uint32_t x, uint32_t y)
{
    uint32_t red;
    uint32_t green;
    uint32_t blue;

    /*
     * Build deterministic RGB565-ish noise. The exact colors do not matter; the important
     * thing is that neighboring pixels differ so a wrong rotate order changes checksum.
     */
    red = (x * 17U + y * 3U) & 0x1FU;
    green = (x * 5U + y * 11U) & 0x3FU;
    blue = (x * 13U + y * 7U) & 0x1FU;
    return (uint16_t)((red << 11U) | (green << 5U) | blue);
}

static void fill_source(uint16_t *source_pixels, uint32_t width, uint32_t height)
{
    uint32_t x;
    uint32_t y;

    for (y = 0U; y < height; ++y)
    {
        for (x = 0U; x < width; ++x)
        {
            source_pixels[(size_t)y * width + x] = make_test_pixel(x, y);
        }
    }
}

static uint32_t checksum_bytes(const uint8_t *bytes, size_t byte_count)
{
    size_t index;
    uint32_t checksum;

    checksum = 2166136261U;
    for (index = 0U; index < byte_count; ++index)
    {
        checksum ^= bytes[index];
        checksum *= 16777619U;
    }

    return checksum;
}

/****************************************************************************************
 * Benchmark Runners
 ****************************************************************************************/
static benchmark_result_t run_app_scalar_case(const benchmark_case_t *test_case,
                                              const uint16_t *source_pixels,
                                              uint8_t *destination_buffer)
{
    benchmark_result_t result;
    lv_area_t source_area;
    uint64_t start_ns;
    uint32_t iteration;

    source_area.x1 = (lv_coord_t)test_case->x;
    source_area.y1 = (lv_coord_t)test_case->y;
    source_area.x2 = (lv_coord_t)(test_case->x + test_case->width - 1U);
    source_area.y2 = (lv_coord_t)(test_case->y + test_case->height - 1U);

    memset(destination_buffer, 0, DESTINATION_BUFFER_BYTES);
    start_ns = now_ns_monotonic();
    for (iteration = 0U; iteration < test_case->iterations; ++iteration)
    {
        display_rotation_copy_counterclockwise_rgb565_scalar(destination_buffer,
                                                             DESTINATION_PITCH_BYTES,
                                                             &source_area,
                                                             source_pixels +
                                                                 ((size_t)test_case->y * LOGICAL_WIDTH) +
                                                                 test_case->x,
                                                             LOGICAL_HEIGHT);
    }

    result.total_ms = (double)(now_ns_monotonic() - start_ns) / 1000000.0;
    result.checksum = checksum_bytes(destination_buffer, DESTINATION_BUFFER_BYTES);
    return result;
}

static benchmark_result_t run_experimental_case(const benchmark_case_t *test_case,
                                                const uint16_t *source_pixels,
                                                uint8_t *destination_buffer)
{
    benchmark_result_t result;
    lv_area_t source_area;
    uint64_t start_ns;
    uint32_t iteration;

    source_area.x1 = (lv_coord_t)test_case->x;
    source_area.y1 = (lv_coord_t)test_case->y;
    source_area.x2 = (lv_coord_t)(test_case->x + test_case->width - 1U);
    source_area.y2 = (lv_coord_t)(test_case->y + test_case->height - 1U);

    memset(destination_buffer, 0, DESTINATION_BUFFER_BYTES);
    start_ns = now_ns_monotonic();
    for (iteration = 0U; iteration < test_case->iterations; ++iteration)
    {
        display_rotation_copy_counterclockwise_rgb565(destination_buffer,
                                                      DESTINATION_PITCH_BYTES,
                                                      &source_area,
                                                      source_pixels +
                                                          ((size_t)test_case->y * LOGICAL_WIDTH) +
                                                          test_case->x,
                                                      LOGICAL_HEIGHT);
    }

    result.total_ms = (double)(now_ns_monotonic() - start_ns) / 1000000.0;
    result.checksum = checksum_bytes(destination_buffer, DESTINATION_BUFFER_BYTES);
    return result;
}
