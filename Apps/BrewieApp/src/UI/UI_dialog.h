#ifndef FREEBREWIE_UI_DIALOG_H
#define FREEBREWIE_UI_DIALOG_H

/****************************************************************************************
 * @file UI_dialog.h
 * @brief Shared modal dialog helpers.
 *
 * Responsibility: provide reusable lightweight modal UI patterns.
 * Owns: modal overlay/panel widgets and simple show/hide behavior.
 * Must not own: workflow decisions, persistence, hardware actions, or screen navigation.
 ****************************************************************************************/

#include "lvgl.h"

typedef struct
{
    /** Full-screen darkened overlay that receives the modal. */
    lv_obj_t *overlay;
    /** Dialog title label. */
    lv_obj_t *title_label;
    /** Dialog body label. */
    lv_obj_t *body_label;
} ui_dialog_t;

void ui_dialog_init(ui_dialog_t *dialog, lv_obj_t *parent, const char *button_text);
void ui_dialog_show(ui_dialog_t *dialog, const char *title, const char *body);
void ui_dialog_hide(ui_dialog_t *dialog);

#endif
