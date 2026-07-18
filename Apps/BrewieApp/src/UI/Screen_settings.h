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
    /** Category selected when this row is clicked. */
    screen_settings_category_id_t category_id;
    /** Screen instance that receives the local selection. */
    struct screen_settings_t *settings;
} screen_settings_category_context_t;

typedef struct
{
    /** Navigation action emitted when the top-left back button is clicked. */
    ui_action_t action;
    /** Callback owned by the UI shell. */
    ui_action_handler_t handler;
    /** Opaque pointer passed back to the callback, normally the ui_t instance. */
    void *user_data;
} screen_settings_nav_context_t;

typedef struct screen_settings_t
{
    /** Root LVGL screen object for Settings. */
    lv_obj_t *screen;
    /** Label showing the selected category title. */
    lv_obj_t *selected_title_label;
    /** Label showing the selected category explanation. */
    lv_obj_t *selected_body_label;
    /** Event callback context for returning to the top-level menu. */
    screen_settings_nav_context_t back_button_context;
    /** Event callback contexts for local settings rows. */
    screen_settings_category_context_t category_contexts[SCREEN_SETTINGS_CATEGORY_COUNT];
    /** Last selected category, used only for local presentation state. */
    screen_settings_category_id_t selected_category_id;
} screen_settings_t;

void screen_settings_init(screen_settings_t *settings, ui_action_handler_t action_handler, void *user_data);

#endif
