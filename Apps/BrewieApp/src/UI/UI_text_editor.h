#ifndef FREEBREWIE_UI_TEXT_EDITOR_H
#define FREEBREWIE_UI_TEXT_EDITOR_H

/****************************************************************************************
 * @file UI_text_editor.h
 * @brief Reusable bounded text-entry modal.
 *
 * Responsibility: provide a small LVGL text-entry dialog that can edit model-owned text
 * through caller-provided commit callbacks.
 * Owns: text-entry overlay, textarea, keyboard, and commit/cancel callbacks.
 * Must not own: recipe data, search state, persistence, validation, or hardware actions.
 ****************************************************************************************/

#include "lvgl.h"

typedef void (*ui_text_editor_commit_handler_t)(const char *text, void *user_data);

/**
 * Reusable text editor widget set.
 *
 * Callers supply the title, initial text, and commit callback when showing the editor.
 * The editor owns only LVGL objects and callback storage; the accepted text is copied by
 * the caller's commit handler into the real model.
 */
typedef struct
{
    lv_obj_t *overlay;
    lv_obj_t *title_label;
    lv_obj_t *textarea;
    lv_obj_t *keyboard;
    ui_text_editor_commit_handler_t commit_handler;
    void *commit_user_data;
} ui_text_editor_t;

void ui_text_editor_init(ui_text_editor_t *editor, lv_obj_t *parent);
void ui_text_editor_show(ui_text_editor_t *editor,
                         const char *title,
                         const char *initial_text,
                         ui_text_editor_commit_handler_t commit_handler,
                         void *user_data);
void ui_text_editor_hide(ui_text_editor_t *editor);

#endif
