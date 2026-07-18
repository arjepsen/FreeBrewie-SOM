#include "Display_rotation.h"

#include <stddef.h>

/****************************************************************************************
 * @brief Rotate one LVGL dirty rectangle into the physical DRM framebuffer.
 *
 * LVGL renders the UI as a normal portrait image. The physical LCD is scanned out by DRM
 * as landscape because that is how the panel is wired. This routine performs only the
 * pixel-address mapping between those two views:
 *
 *   logical portrait x,y -> physical landscape x = logical_height - 1 - y
 *                           physical landscape y = x
 *
 * The loop is arranged so the inner loop writes to adjacent framebuffer pixels. That makes
 * the destination side cache-friendly and keeps writes simple for the A13 memory system.
 * The source reads then step through LVGL's dirty rectangle by columns. A future NEON
 * version can improve that by rotating small blocks, but this scalar path stays clear and
 * dependable for arbitrary dirty-rectangle sizes.
 ****************************************************************************************/
void display_rotation_copy_counterclockwise_rgb565(uint8_t *destination_buffer,
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

        /*
         * A fixed source x becomes one physical framebuffer row. The first source y maps
         * near the right side of that row, then following source pixels move left one
         * destination pixel at a time.
         */
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
