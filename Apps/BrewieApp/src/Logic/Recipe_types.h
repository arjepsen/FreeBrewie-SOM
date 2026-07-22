#ifndef FREEBREWIE_RECIPE_TYPES_H
#define FREEBREWIE_RECIPE_TYPES_H

/****************************************************************************************
 * @file Recipe_types.h
 * @brief Shared recipe-domain data types.
 *
 * Responsibility: define plain recipe data structures that can be used by embedded UI,
 * future storage, and a future web/API interface.
 * Owns: stable recipe identifiers, shared recipe section shapes, and read-only catalog
 * preview metadata shapes.
 * Must not own: LVGL widgets, file storage, brewing start side effects, or web transport.
 ****************************************************************************************/

#include <stdint.h>

#define RECIPE_NAME_MAX_LENGTH 40U
#define RECIPE_MAX_FERMENTABLES 8U
#define RECIPE_MAX_HOPS 8U
#define RECIPE_MAX_MASH_STEPS 6U
#define RECIPE_MAX_FERMENTATION_STEPS 6U

typedef uint32_t recipe_id_t;

typedef struct
{
    const char *name;  // Ingredient name. Points to stable string storage in this first scaffold.
    uint16_t amount_g;  // Addition amount in grams.
} recipe_fermentable_t;

typedef struct
{
    const char *name;  // Hop name. Points to stable string storage in this first scaffold.
    uint16_t amount_g;  // Addition amount in grams.
    uint16_t boil_time_min;  // Boil time in minutes before flameout/end of boil.
} recipe_hop_t;

typedef struct
{
    uint8_t temperature_c;  // Target mash step temperature in degrees C.
    uint16_t time_min;  // Step hold time in minutes.
} recipe_mash_step_t;

typedef struct
{
    uint16_t mash_in_water_dl;  // Mash-in water amount in deciliters.
    uint8_t mash_in_temperature_c;  // Mash-in target temperature in degrees C.
    uint8_t mash_step_count;  // Number of active mash steps in mash_steps[].
    recipe_mash_step_t mash_steps[RECIPE_MAX_MASH_STEPS];  // Ordered mash steps.
    uint16_t sparge_water_dl;  // Sparge water amount in deciliters.
    uint8_t sparge_temperature_c;  // Sparge water temperature in degrees C.
    uint16_t sparge_time_min;  // Sparge duration in minutes.
    uint16_t boil_time_min;  // Total boil time in minutes.
    uint16_t delayed_hopping_min;  // Delayed hopping time in minutes.
    uint8_t cooling_target_c;  // Cooling target temperature in degrees C.
} recipe_brewing_t;

typedef struct
{
    const char *name;  // Step name shown in the normal recipe editor, for example Primary.
    uint8_t temperature_c;  // Target fermentation temperature in degrees C.
    uint16_t duration_days;  // Step duration in days.
} recipe_fermentation_step_t;

typedef struct
{
    uint8_t step_count;  // Number of active fermentation steps in steps[].
    recipe_fermentation_step_t steps[RECIPE_MAX_FERMENTATION_STEPS];  // Ordered fermentation schedule.
} recipe_fermentation_t;

typedef struct
{
    const char *style_name;  // Display name of the selected beer style, or placeholder text when none is selected.
    const char *style_number;  // BJCP or style-guide number text. Kept as text because external systems vary here.
    const char *style_category;  // Style category text.
    const char *style_type;  // Style type text, for example Ale or Lager.
} recipe_style_t;

typedef struct
{
    uint8_t efficiency_percent;  // Expected brewhouse efficiency in whole percent.
    uint16_t batch_size_dl;  // Batch size in deciliters, so 200 means 20.0 L without floating point.
    uint16_t estimated_abv_tenths;  // Estimated alcohol in tenths of a percent, so 52 means 5.2%.
    uint16_t estimated_srm;  // Estimated color in whole SRM for now.
    uint16_t estimated_ibu;  // Estimated bitterness in whole IBU.
    uint16_t estimated_og_points;  // Estimated original gravity as gravity points, so 1050 means 1.050.
    uint16_t estimated_fg_points;  // Estimated final gravity as gravity points, so 1011 means 1.011.
} recipe_calculated_t;

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
