#ifndef FREEBREWIE_RECIPE_TYPES_H
#define FREEBREWIE_RECIPE_TYPES_H

/****************************************************************************************
 * @file Recipe_types.h
 * @brief Shared recipe-domain data types.
 *
 * Responsibility: define plain recipe data structures that can be used by embedded UI,
 * future storage, and a future web/API interface.
 * Owns: stable recipe identifiers and read-only recipe metadata shapes.
 * Must not own: LVGL widgets, file storage, brewing start side effects, or web transport.
 ****************************************************************************************/

#include <stdint.h>

typedef uint32_t recipe_id_t;

typedef struct
{
    recipe_id_t id;
    const char *name;
    const char *style;
    const char *summary;
    const char *batch_size;
    const char *abv;
    const char *ibu;
    const char *og;
    const char *fermentables;
    const char *hops;
    const char *additions;
    const char *yeast;
    const char *mash;
    const char *boil;
    const char *cooling;
    const char *water;
    const char *fermentation;
    const char *fermentation_temperature;
    const char *fermentation_duration;
    uint32_t accent_color;
} recipe_t;

#endif
