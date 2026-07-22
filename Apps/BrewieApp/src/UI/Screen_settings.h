#ifndef FREEBREWIE_SCREEN_SETTINGS_H
#define FREEBREWIE_SCREEN_SETTINGS_H

/****************************************************************************************
 * @file Screen_settings.h
 * @brief Old-Brewie-inspired Settings scaffold.
 *
 * Responsibility: show safe settings categories and local category information.
 * Owns: Settings LVGL objects, row callback contexts, and selected-category labels.
 * Must not own: persistent settings, network changes, calibration writes, or MCU commands.
 ****************************************************************************************/

#include "UI_types.h"
#include "lvgl.h"

typedef enum
{
    SCREEN_SETTINGS_CATEGORY_WIFI = 0,
    SCREEN_SETTINGS_CATEGORY_UNITS,
    SCREEN_SETTINGS_CATEGORY_TIME,
    SCREEN_SETTINGS_CATEGORY_WATER,
    SCREEN_SETTINGS_CATEGORY_CALIBRATION,
    SCREEN_SETTINGS_CATEGORY_LANGUAGE,
    SCREEN_SETTINGS_CATEGORY_ABOUT,
    SCREEN_SETTINGS_CATEGORY_COUNT
} screen_settings_category_id_t;

typedef struct
{
    screen_settings_category_id_t category_id;
    struct screen_settings_t *settings;
} screen_settings_category_context_t;

typedef struct
{
    ui_action_t action;
    ui_action_handler_t handler;
    void *user_data;
} screen_settings_nav_context_t;

/**
 * Settings presentation state.
 *
 * This screen is still informational. Selecting a category only swaps local text until
 * persistent settings and safety-sensitive calibration flows are designed.
 */
typedef struct screen_settings_t
{
    lv_obj_t *screen;
    lv_obj_t *selected_title_label;
    lv_obj_t *selected_body_label;
    screen_settings_nav_context_t back_button_context;
    screen_settings_category_context_t category_contexts[SCREEN_SETTINGS_CATEGORY_COUNT];
    screen_settings_category_id_t selected_category_id;
} screen_settings_t;

void screen_settings_init(screen_settings_t *settings, ui_action_handler_t action_handler, void *user_data);

#endif
