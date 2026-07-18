#include "Display_rotation.h"

#include <stddef.h>

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
#include <arm_neon.h>
#define DISPLAY_ROTATION_HAS_NEON 1
#else
#define DISPLAY_ROTATION_HAS_NEON 0
#endif

static void display_rotation_copy_leftover_edges_scalar(uint8_t *destination_buffer,
                                                        uint32_t destination_pitch_bytes,
                                                        const lv_area_t *source_area,
                                                        const uint16_t *source_pixels,
                                                        uint32_t source_width,
                                                        uint32_t tile_width,
                                                        uint32_t tile_height,
                                                        uint32_t logical_height);
#if DISPLAY_ROTATION_HAS_NEON
static void display_rotation_copy_8x8_counterclockwise_neon(uint8_t *destination_buffer,
                                                            uint32_t destination_pitch_bytes,
                                                            const uint16_t *source_pixels,
                                                            uint32_t source_width,
                                                            uint32_t local_x,
                                                            uint32_t local_y,
                                                            uint32_t global_x,
                                                            uint32_t global_y,
                                                            uint32_t logical_height);
static uint16x8_t display_rotation_reverse_u16x8(uint16x8_t value);
#endif

/****************************************************************************************
 * @brief Rotate one LVGL dirty rectangle into the physical DRM framebuffer.
 *
 * This is the production entry point. On the A13 target it uses a NEON 8x8 tiled core for
 * the area that fits cleanly into tiles, then scalar code handles the right and bottom
 * edges. Simulator and non-NEON builds use the scalar path directly.
 ****************************************************************************************/
void display_rotation_copy_counterclockwise_rgb565(uint8_t *destination_buffer,
                                                   uint32_t destination_pitch_bytes,
                                                   const lv_area_t *source_area,
                                                   const uint16_t *source_pixels,
                                                   uint32_t logical_height)
{
    uint32_t source_width;
    uint32_t source_height;
    uint32_t tile_width;
    uint32_t tile_height;
    uint32_t local_x;
    uint32_t local_y;

    if (destination_buffer == NULL || source_area == NULL || source_pixels == NULL)
    {
        return;
    }

    source_width = (uint32_t)(source_area->x2 - source_area->x1 + 1);
    source_height = (uint32_t)(source_area->y2 - source_area->y1 + 1);
    tile_width = source_width & ~7U;
    tile_height = source_height & ~7U;

#if DISPLAY_ROTATION_HAS_NEON
    /*
     * NEON has no useful gather/scatter support on this ARMv7 target, so the fast path is
     * tile based: load eight contiguous source rows, transpose/reverse the 8x8 RGB565 tile
     * in NEON registers, then store eight contiguous destination rows.
     */
    for (local_y = 0U; local_y < tile_height; local_y += 8U)
    {
        for (local_x = 0U; local_x < tile_width; local_x += 8U)
        {
            display_rotation_copy_8x8_counterclockwise_neon(destination_buffer,
                                                            destination_pitch_bytes,
                                                            source_pixels,
                                                            source_width,
                                                            local_x,
                                                            local_y,
                                                            (uint32_t)source_area->x1 + local_x,
                                                            (uint32_t)source_area->y1 + local_y,
                                                            logical_height);
        }
    }
#else
    (void)local_x;
    (void)local_y;
#endif

    display_rotation_copy_leftover_edges_scalar(destination_buffer,
                                               destination_pitch_bytes,
                                               source_area,
                                               source_pixels,
                                               source_width,
                                               tile_width,
                                               tile_height,
                                               logical_height);
}

/****************************************************************************************
 * @brief Scalar reference/fallback rotation path.
 *
 * LVGL renders the UI as a normal portrait image. The physical LCD is scanned out by DRM
 * as landscape because that is how the panel is wired. This routine performs only the
 * pixel-address mapping between those two views:
 *
 *   logical portrait x,y -> physical landscape x = logical_height - 1 - y
 *                           physical landscape y = x
 *
 * The loop writes adjacent framebuffer pixels in the inner loop, which is the best simple
 * scalar tradeoff for this memory layout.
 ****************************************************************************************/
void display_rotation_copy_counterclockwise_rgb565_scalar(uint8_t *destination_buffer,
                                                          uint32_t destination_pitch_bytes,
                                                          const lv_area_t *source_area,
                                                          const uint16_t *source_pixels,
                                                          uint32_t logical_height)
{
    int32_t source_width;
    int32_t source_x;

    if (destination_buffer == NULL || source_area == NULL || source_pixels == NULL)
    {
        return;
    }

    source_width = source_area->x2 - source_area->x1 + 1;

    for (source_x = source_area->x1; source_x <= source_area->x2; ++source_x)
    {
        uint16_t *destination_pixel;
        const uint16_t *source_pixel;
        int32_t source_y;

        destination_pixel = (uint16_t *)(destination_buffer +
                                         ((size_t)source_x * (size_t)destination_pitch_bytes));
        destination_pixel += (size_t)logical_height - 1U - (size_t)source_area->y1;

        source_pixel = source_pixels + (size_t)(source_x - source_area->x1);

        for (source_y = source_area->y1; source_y <= source_area->y2; ++source_y)
        {
            *destination_pixel = *source_pixel;
            --destination_pixel;
            source_pixel += source_width;
        }
    }
}

static void display_rotation_copy_leftover_edges_scalar(uint8_t *destination_buffer,
                                                        uint32_t destination_pitch_bytes,
                                                        const lv_area_t *source_area,
                                                        const uint16_t *source_pixels,
                                                        uint32_t source_width,
                                                        uint32_t tile_width,
                                                        uint32_t tile_height,
                                                        uint32_t logical_height)
{
    uint32_t source_height;
    uint32_t local_x;
    uint32_t local_y;

    source_height = (uint32_t)(source_area->y2 - source_area->y1 + 1);

    /*
     * The tiled fast path covers the top-left rectangle whose width and height are
     * divisible by eight. This scalar pass fills the right strip and bottom strip exactly
     * once, avoiding overlapping writes and keeping the edge code easy to verify.
     */
    for (local_y = 0U; local_y < source_height; ++local_y)
    {
        uint32_t local_x_start;

        local_x_start = (local_y < tile_height) ? tile_width : 0U;
        for (local_x = local_x_start; local_x < source_width; ++local_x)
        {
            uint32_t global_x;
            uint32_t global_y;
            uint32_t destination_x;
            uint32_t destination_y;
            uint16_t *destination_row;

            global_x = (uint32_t)source_area->x1 + local_x;
            global_y = (uint32_t)source_area->y1 + local_y;
            destination_x = logical_height - 1U - global_y;
            destination_y = global_x;
            destination_row = (uint16_t *)(destination_buffer +
                                           ((size_t)destination_y * destination_pitch_bytes));
            destination_row[destination_x] = source_pixels[(size_t)local_y * source_width + local_x];
        }
    }
}

#if DISPLAY_ROTATION_HAS_NEON
static void display_rotation_copy_8x8_counterclockwise_neon(uint8_t *destination_buffer,
                                                            uint32_t destination_pitch_bytes,
                                                            const uint16_t *source_pixels,
                                                            uint32_t source_width,
                                                            uint32_t local_x,
                                                            uint32_t local_y,
                                                            uint32_t global_x,
                                                            uint32_t global_y,
                                                            uint32_t logical_height)
{
    const uint16_t *tile_source;
    uint16_t *destination_row;
    uint32_t destination_x_start;
    uint16x8_t row_0;
    uint16x8_t row_1;
    uint16x8_t row_2;
    uint16x8_t row_3;
    uint16x8_t row_4;
    uint16x8_t row_5;
    uint16x8_t row_6;
    uint16x8_t row_7;
    uint16x8x2_t transpose_01_16;
    uint16x8x2_t transpose_23_16;
    uint16x8x2_t transpose_45_16;
    uint16x8x2_t transpose_67_16;
    uint32x4x2_t transpose_02_32;
    uint32x4x2_t transpose_13_32;
    uint32x4x2_t transpose_46_32;
    uint32x4x2_t transpose_57_32;
    uint16x8_t column_0;
    uint16x8_t column_1;
    uint16x8_t column_2;
    uint16x8_t column_3;
    uint16x8_t column_4;
    uint16x8_t column_5;
    uint16x8_t column_6;
    uint16x8_t column_7;

    tile_source = source_pixels + ((size_t)local_y * source_width) + local_x;
    row_0 = vld1q_u16(tile_source + ((size_t)0U * source_width));
    row_1 = vld1q_u16(tile_source + ((size_t)1U * source_width));
    row_2 = vld1q_u16(tile_source + ((size_t)2U * source_width));
    row_3 = vld1q_u16(tile_source + ((size_t)3U * source_width));
    row_4 = vld1q_u16(tile_source + ((size_t)4U * source_width));
    row_5 = vld1q_u16(tile_source + ((size_t)5U * source_width));
    row_6 = vld1q_u16(tile_source + ((size_t)6U * source_width));
    row_7 = vld1q_u16(tile_source + ((size_t)7U * source_width));

    /*
     * Transpose the 8x8 RGB565 tile. The u16 and u32 transpose stages gradually group the
     * original columns together without scalar lane loads.
     */
    transpose_01_16 = vtrnq_u16(row_0, row_1);
    transpose_23_16 = vtrnq_u16(row_2, row_3);
    transpose_45_16 = vtrnq_u16(row_4, row_5);
    transpose_67_16 = vtrnq_u16(row_6, row_7);

    transpose_02_32 = vtrnq_u32(vreinterpretq_u32_u16(transpose_01_16.val[0]),
                                vreinterpretq_u32_u16(transpose_23_16.val[0]));
    transpose_13_32 = vtrnq_u32(vreinterpretq_u32_u16(transpose_01_16.val[1]),
                                vreinterpretq_u32_u16(transpose_23_16.val[1]));
    transpose_46_32 = vtrnq_u32(vreinterpretq_u32_u16(transpose_45_16.val[0]),
                                vreinterpretq_u32_u16(transpose_67_16.val[0]));
    transpose_57_32 = vtrnq_u32(vreinterpretq_u32_u16(transpose_45_16.val[1]),
                                vreinterpretq_u32_u16(transpose_67_16.val[1]));

    column_0 = vcombine_u16(vget_low_u16(vreinterpretq_u16_u32(transpose_02_32.val[0])),
                            vget_low_u16(vreinterpretq_u16_u32(transpose_46_32.val[0])));
    column_1 = vcombine_u16(vget_low_u16(vreinterpretq_u16_u32(transpose_13_32.val[0])),
                            vget_low_u16(vreinterpretq_u16_u32(transpose_57_32.val[0])));
    column_2 = vcombine_u16(vget_low_u16(vreinterpretq_u16_u32(transpose_02_32.val[1])),
                            vget_low_u16(vreinterpretq_u16_u32(transpose_46_32.val[1])));
    column_3 = vcombine_u16(vget_low_u16(vreinterpretq_u16_u32(transpose_13_32.val[1])),
                            vget_low_u16(vreinterpretq_u16_u32(transpose_57_32.val[1])));
    column_4 = vcombine_u16(vget_high_u16(vreinterpretq_u16_u32(transpose_02_32.val[0])),
                            vget_high_u16(vreinterpretq_u16_u32(transpose_46_32.val[0])));
    column_5 = vcombine_u16(vget_high_u16(vreinterpretq_u16_u32(transpose_13_32.val[0])),
                            vget_high_u16(vreinterpretq_u16_u32(transpose_57_32.val[0])));
    column_6 = vcombine_u16(vget_high_u16(vreinterpretq_u16_u32(transpose_02_32.val[1])),
                            vget_high_u16(vreinterpretq_u16_u32(transpose_46_32.val[1])));
    column_7 = vcombine_u16(vget_high_u16(vreinterpretq_u16_u32(transpose_13_32.val[1])),
                            vget_high_u16(vreinterpretq_u16_u32(transpose_57_32.val[1])));

    destination_x_start = logical_height - 1U - global_y;

    destination_row = (uint16_t *)(destination_buffer + ((size_t)(global_x + 0U) * destination_pitch_bytes));
    vst1q_u16(destination_row + destination_x_start - 7U, display_rotation_reverse_u16x8(column_0));
    destination_row = (uint16_t *)(destination_buffer + ((size_t)(global_x + 1U) * destination_pitch_bytes));
    vst1q_u16(destination_row + destination_x_start - 7U, display_rotation_reverse_u16x8(column_1));
    destination_row = (uint16_t *)(destination_buffer + ((size_t)(global_x + 2U) * destination_pitch_bytes));
    vst1q_u16(destination_row + destination_x_start - 7U, display_rotation_reverse_u16x8(column_2));
    destination_row = (uint16_t *)(destination_buffer + ((size_t)(global_x + 3U) * destination_pitch_bytes));
    vst1q_u16(destination_row + destination_x_start - 7U, display_rotation_reverse_u16x8(column_3));
    destination_row = (uint16_t *)(destination_buffer + ((size_t)(global_x + 4U) * destination_pitch_bytes));
    vst1q_u16(destination_row + destination_x_start - 7U, display_rotation_reverse_u16x8(column_4));
    destination_row = (uint16_t *)(destination_buffer + ((size_t)(global_x + 5U) * destination_pitch_bytes));
    vst1q_u16(destination_row + destination_x_start - 7U, display_rotation_reverse_u16x8(column_5));
    destination_row = (uint16_t *)(destination_buffer + ((size_t)(global_x + 6U) * destination_pitch_bytes));
    vst1q_u16(destination_row + destination_x_start - 7U, display_rotation_reverse_u16x8(column_6));
    destination_row = (uint16_t *)(destination_buffer + ((size_t)(global_x + 7U) * destination_pitch_bytes));
    vst1q_u16(destination_row + destination_x_start - 7U, display_rotation_reverse_u16x8(column_7));
}

static uint16x8_t display_rotation_reverse_u16x8(uint16x8_t value)
{
    uint16x8_t reversed_halves;

    reversed_halves = vrev64q_u16(value);
    return vcombine_u16(vget_high_u16(reversed_halves), vget_low_u16(reversed_halves));
}
#endif
