#ifndef FREEBREWIE_DISPLAY_ROTATION_H
#define FREEBREWIE_DISPLAY_ROTATION_H

/****************************************************************************************
 * @file Display_rotation.h
 * @brief Pixel rotation helpers for the mounted Brewie display.
 *
 * Responsibility: copy LVGL's logical portrait RGB565 pixels into the physical landscape
 * DRM framebuffer layout used by the A13 display controller.
 * Owns: rotation math and rotation-loop implementation details.
 ****************************************************************************************/

#include <stdint.h>

#include "lvgl.h"

void display_rotation_copy_counterclockwise_rgb565(uint8_t *destination_buffer,
                                                   uint32_t destination_pitch_bytes,
                                                   const lv_area_t *source_area,
                                                   const uint16_t *source_pixels,
                                                   uint32_t logical_height);
void display_rotation_copy_counterclockwise_rgb565_scalar(uint8_t *destination_buffer,
                                                          uint32_t destination_pitch_bytes,
                                                          const lv_area_t *source_area,
                                                          const uint16_t *source_pixels,
                                                          uint32_t logical_height);

#endif
