#ifndef FREEBREWIE_UI_NUMBER_EDITOR_H
#define FREEBREWIE_UI_NUMBER_EDITOR_H

/****************************************************************************************
 * @file UI_number_editor.h
 * @brief Reusable bounded numeric editor modal.
 *
 * Responsibility: provide a small LVGL dialog for editing one unsigned integer value.
 * Owns: numeric editor overlay, value label, step buttons, and commit/cancel callbacks.
 * Must not own: recipe data, validation policy beyond caller-provided bounds, or hardware actions.
 ****************************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include "lvgl.h"

typedef void (*ui_number_editor_commit_handler_t)(uint16_t value, void *user_data);

/**
 * Reusable bounded unsigned-number editor.
 *
 * Bounds and units are supplied by the caller each time the editor is shown. This keeps
 * the widget reusable while leaving recipe validation policy in the model/logic layer.
 */
typedef struct
{
    lv_obj_t *overlay;
    lv_obj_t *title_label;
    lv_obj_t *value_label;
    lv_obj_t *unit_label;
    /** Current working value in caller-defined units. */
    uint16_t value;
    uint16_t minimum;
    uint16_t maximum;
    uint16_t step;
    /** True when value should display as deciliters with one decimal liter. */
    bool show_as_liters;
    ui_number_editor_commit_handler_t commit_handler;
    void *commit_user_data;
} ui_number_editor_t;

void ui_number_editor_init(ui_number_editor_t *editor, lv_obj_t *parent);
void ui_number_editor_show(ui_number_editor_t *editor,
                           const char *title,
                           const char *unit_text,
                           uint16_t value,
                           uint16_t minimum,
                           uint16_t maximum,
                           uint16_t step,
                           bool show_as_liters,
                           ui_number_editor_commit_handler_t commit_handler,
                           void *user_data);
void ui_number_editor_hide(ui_number_editor_t *editor);

#endif
