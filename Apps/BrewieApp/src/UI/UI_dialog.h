#ifndef FREEBREWIE_UI_DIALOG_H
#define FREEBREWIE_UI_DIALOG_H

/****************************************************************************************
 * @file UI_dialog.h
 * @brief Shared modal dialog helpers.
 *
 * Responsibility: provide reusable lightweight modal UI patterns.
 * Owns: modal overlay/panel widgets, button callbacks, and simple show/hide behavior.
 * Must not own: workflow decisions, persistence, hardware actions, or screen navigation.
 ****************************************************************************************/

#include "lvgl.h"

typedef void (*ui_dialog_action_handler_t)(void *user_data);

/**
 * Lightweight modal dialog state.
 *
 * Dialog button handlers are optional. The dialog hides itself after running the selected
 * action, so callers only provide the side effect they need.
 */
typedef struct
{
    lv_obj_t *overlay;
    lv_obj_t *title_label;
    lv_obj_t *body_label;
    ui_dialog_action_handler_t primary_handler;
    void *primary_user_data;
    ui_dialog_action_handler_t secondary_handler;
    void *secondary_user_data;
} ui_dialog_t;

void ui_dialog_init(ui_dialog_t *dialog,
                    lv_obj_t *parent,
                    const char *primary_text,
                    const char *secondary_text);
void ui_dialog_set_primary_action(ui_dialog_t *dialog,
                                  ui_dialog_action_handler_t handler,
                                  void *user_data);
void ui_dialog_set_secondary_action(ui_dialog_t *dialog,
                                    ui_dialog_action_handler_t handler,
                                    void *user_data);
void ui_dialog_show(ui_dialog_t *dialog, const char *title, const char *body);
void ui_dialog_hide(ui_dialog_t *dialog);

#endif
