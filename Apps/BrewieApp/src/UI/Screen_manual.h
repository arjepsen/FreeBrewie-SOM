#ifndef FREEBREWIE_SCREEN_MANUAL_H
#define FREEBREWIE_SCREEN_MANUAL_H

/****************************************************************************************
 * @file Screen_manual.h
 * @brief Old-Brewie-inspired Manual/Cleaning scaffold.
 *
 * Responsibility: show safe clean/drain/unclog mode choices and local mode information.
 * Owns: Manual/Cleaning LVGL objects, row callback contexts, and selected-mode labels.
 * Must not own: hardware control, interlock decisions, or MCU protocol commands.
 ****************************************************************************************/

#include "UI_types.h"
#include "lvgl.h"

typedef enum
{
    SCREEN_MANUAL_MODE_SHORT_CLEAN = 0,
    SCREEN_MANUAL_MODE_SANITIZING_CLEAN,
    SCREEN_MANUAL_MODE_FULL_CLEAN,
    SCREEN_MANUAL_MODE_DRAIN_AFTER_BREW,
    SCREEN_MANUAL_MODE_FULL_DRAIN,
    SCREEN_MANUAL_MODE_UNCLOGGING,
    SCREEN_MANUAL_MODE_COUNT
} screen_manual_mode_id_t;

typedef struct
{
    screen_manual_mode_id_t mode_id;
    struct screen_manual_t *manual;
} screen_manual_mode_context_t;

typedef struct
{
    ui_action_t action;
    ui_action_handler_t handler;
    void *user_data;
} screen_manual_nav_context_t;

/**
 * Manual/Cleaning presentation state.
 *
 * Selecting a row only changes local explanatory text. Real cleaning/drain commands must
 * later pass through app logic and machine safety checks before they reach the MCU.
 */
typedef struct screen_manual_t
{
    lv_obj_t *screen;
    lv_obj_t *selected_title_label;
    lv_obj_t *selected_body_label;
    screen_manual_nav_context_t back_button_context;
    screen_manual_mode_context_t mode_contexts[SCREEN_MANUAL_MODE_COUNT];
    screen_manual_mode_id_t selected_mode_id;
} screen_manual_t;

void screen_manual_init(screen_manual_t *manual, ui_action_handler_t action_handler, void *user_data);

#endif
