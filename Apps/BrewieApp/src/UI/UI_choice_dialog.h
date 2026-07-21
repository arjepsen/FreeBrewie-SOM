#ifndef FREEBREWIE_UI_CHOICE_DIALOG_H
#define FREEBREWIE_UI_CHOICE_DIALOG_H

/****************************************************************************************
 * @file UI_choice_dialog.h
 * @brief Shared local choice-list modal helper.
 *
 * Responsibility: provide a small reusable modal for choosing one static text option.
 * Owns: modal overlay, fixed choice-row widgets, cancel button, and choice callbacks.
 * Must not own: persistence, validation, navigation, hardware actions, or business rules.
 ****************************************************************************************/

#include <stdint.h>

#include "lvgl.h"

#define UI_CHOICE_DIALOG_MAX_CHOICES 10U

typedef void (*ui_choice_dialog_handler_t)(uint8_t choice_index, void *user_data);

typedef struct
{
    /** Choice index represented by this row. */
    uint8_t choice_index;
    /** Dialog instance that owns the row. */
    struct ui_choice_dialog_t *dialog;
} ui_choice_dialog_row_context_t;

typedef struct ui_choice_dialog_t
{
    /** Full-screen darkened overlay that receives the modal. */
    lv_obj_t *overlay;
    /** Dialog title label. */
    lv_obj_t *title_label;
    /** Reused row buttons. Hidden rows are kept for later show calls. */
    lv_obj_t *choice_buttons[UI_CHOICE_DIALOG_MAX_CHOICES];
    /** Reused row labels. */
    lv_obj_t *choice_labels[UI_CHOICE_DIALOG_MAX_CHOICES];
    /** Stable callback contexts attached to the reused row buttons. */
    ui_choice_dialog_row_context_t row_contexts[UI_CHOICE_DIALOG_MAX_CHOICES];
    /** Number of visible/valid choices in the current show call. */
    uint8_t choice_count;
    /** Optional caller action fired when a row is chosen. */
    ui_choice_dialog_handler_t handler;
    /** Optional caller state passed to handler. */
    void *user_data;
} ui_choice_dialog_t;

void ui_choice_dialog_init(ui_choice_dialog_t *dialog, lv_obj_t *parent);
void ui_choice_dialog_show(ui_choice_dialog_t *dialog,
                           const char *title,
                           const char *const *choices,
                           uint8_t choice_count,
                           ui_choice_dialog_handler_t handler,
                           void *user_data);
void ui_choice_dialog_hide(ui_choice_dialog_t *dialog);

#endif
