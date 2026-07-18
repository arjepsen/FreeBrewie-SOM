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
    /** Mode selected when this row is clicked. */
    screen_manual_mode_id_t mode_id;
    /** Screen instance that receives the local selection. */
    struct screen_manual_t *manual;
} screen_manual_mode_context_t;

typedef struct
{
    /** Navigation action emitted when the top-left back button is clicked. */
    ui_action_t action;
    /** Callback owned by the UI shell. */
    ui_action_handler_t handler;
    /** Opaque pointer passed back to the callback, normally the ui_t instance. */
    void *user_data;
} screen_manual_nav_context_t;

typedef struct screen_manual_t
{
    /** Root LVGL screen object for Manual/Cleaning. */
    lv_obj_t *screen;
    /** Label showing the selected mode title. */
    lv_obj_t *selected_title_label;
    /** Label showing the selected mode explanation. */
    lv_obj_t *selected_body_label;
    /** Event callback context for returning to the top-level menu. */
    screen_manual_nav_context_t back_button_context;
    /** Event callback contexts for local clean/drain/unclog rows. */
    screen_manual_mode_context_t mode_contexts[SCREEN_MANUAL_MODE_COUNT];
    /** Last selected mode, used only for local presentation state. */
    screen_manual_mode_id_t selected_mode_id;
} screen_manual_t;

void screen_manual_init(screen_manual_t *manual, ui_action_handler_t action_handler, void *user_data);

#endif
