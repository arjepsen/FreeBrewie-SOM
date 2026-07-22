#ifndef FREEBREWIE_STYLE_CATALOG_H
#define FREEBREWIE_STYLE_CATALOG_H

/****************************************************************************************
 * @file Style_catalog.h
 * @brief Small beer-style catalog used by recipe editing screens.
 *
 * Responsibility: expose selectable beer style records loaded from project data.
 * Owns: bounded in-memory style catalog cache and fallback starter style records.
 * Must not own: draft recipe state, LVGL widgets, BeerXML/BeerJSON import, or storage UI.
 ****************************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#define STYLE_CATALOG_MAX_STYLES 16U
#define STYLE_CATALOG_TEXT_MAX_LENGTH 48U

typedef struct
{
    const char *style_name;     // Display name shown in the style picker and copied into a selected draft.
    const char *style_number;   // BJCP-style short code. Kept as text because external style systems differ.
    const char *style_category; // Human-readable category shown under the style name.
    const char *style_type;     // Broad fermentation family, for example Ale or Lager.
} style_catalog_style_t;

bool style_catalog_init();
uint8_t style_catalog_get_count();
const style_catalog_style_t *style_catalog_get_style(uint8_t index);
bool style_catalog_loaded_from_file();

#endif
