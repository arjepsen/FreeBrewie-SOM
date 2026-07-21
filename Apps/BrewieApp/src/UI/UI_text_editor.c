#include "UI_text_editor.h"

static void ui_text_editor_set_static(lv_obj_t *object);
static lv_obj_t *ui_text_editor_create_button(lv_obj_t *parent,
                                              const char *text,
                                              lv_color_t normal_color,
                                              lv_color_t pressed_color,
                                              lv_event_cb_t event_cb,
                                              ui_text_editor_t *editor);
static void ui_text_editor_cancel_event_cb(lv_event_t *event);
static void ui_text_editor_ok_event_cb(lv_event_t *event);

/****************************************************************************************
 * @brief Make an editor object non-scrollable unless it is the actual text/keyboard input.
 ****************************************************************************************/
static void ui_text_editor_set_static(lv_obj_t *object)
{
    if (object == NULL)
    {
        return;
    }

    lv_obj_clear_flag(object, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_OFF);
}

static lv_obj_t *ui_text_editor_create_button(lv_obj_t *parent,
                                              const char *text,
                                              lv_color_t normal_color,
                                              lv_color_t pressed_color,
                                              lv_event_cb_t event_cb,
                                              ui_text_editor_t *editor)
{
    lv_obj_t *button;
    lv_obj_t *label;

    button = lv_button_create(parent);
    lv_obj_set_flex_grow(button, 1);
    lv_obj_set_height(button, 38);
    lv_obj_set_style_bg_color(button, normal_color, 0);
    lv_obj_set_style_bg_color(button, pressed_color, LV_STATE_PRESSED);
    lv_obj_set_style_radius(button, 5, 0);
    lv_obj_add_event_cb(button, event_cb, LV_EVENT_CLICKED, editor);

    label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);
    return button;
}

static void ui_text_editor_cancel_event_cb(lv_event_t *event)
{
    ui_text_editor_t *editor;

    editor = lv_event_get_user_data(event);
    ui_text_editor_hide(editor);
}

static void ui_text_editor_ok_event_cb(lv_event_t *event)
{
    ui_text_editor_t *editor;

    editor = lv_event_get_user_data(event);
    if (editor != NULL && editor->commit_handler != NULL)
    {
        editor->commit_handler(lv_textarea_get_text(editor->textarea), editor->commit_user_data);
    }

    ui_text_editor_hide(editor);
}

/****************************************************************************************
 * @brief Create a hidden reusable text editor under parent.
 ****************************************************************************************/
void ui_text_editor_init(ui_text_editor_t *editor, lv_obj_t *parent)
{
    lv_obj_t *panel;
    lv_obj_t *button_row;

    if (editor == NULL || parent == NULL)
    {
        return;
    }

    editor->commit_handler = NULL;
    editor->commit_user_data = NULL;

    editor->overlay = lv_obj_create(parent);
    ui_text_editor_set_static(editor->overlay);
    lv_obj_set_size(editor->overlay, lv_pct(100), lv_pct(100));
    lv_obj_center(editor->overlay);
    lv_obj_set_style_bg_color(editor->overlay, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(editor->overlay, LV_OPA_70, 0);
    lv_obj_set_style_border_width(editor->overlay, 0, 0);
    lv_obj_set_style_pad_all(editor->overlay, 8, 0);
    lv_obj_add_flag(editor->overlay, LV_OBJ_FLAG_HIDDEN);

    panel = lv_obj_create(editor->overlay);
    ui_text_editor_set_static(panel);
    lv_obj_set_size(panel, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(0xE67526), 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_radius(panel, 5, 0);
    lv_obj_set_style_pad_all(panel, 8, 0);
    lv_obj_set_style_pad_row(panel, 7, 0);
    lv_obj_set_layout(panel, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);

    editor->title_label = lv_label_create(panel);
    lv_label_set_text(editor->title_label, "--");
    lv_label_set_long_mode(editor->title_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(editor->title_label, lv_pct(100));
    lv_obj_set_style_text_color(editor->title_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(editor->title_label, &lv_font_montserrat_20, 0);

    editor->textarea = lv_textarea_create(panel);
    lv_obj_set_width(editor->textarea, lv_pct(100));
    lv_obj_set_height(editor->textarea, 42);
    lv_textarea_set_one_line(editor->textarea, true);
    lv_textarea_set_max_length(editor->textarea, 39);
    lv_obj_set_style_text_color(editor->textarea, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_color(editor->textarea, lv_color_hex(0x282828), 0);
    lv_obj_set_style_border_color(editor->textarea, lv_color_hex(0xE67526), LV_STATE_FOCUSED);

    button_row = lv_obj_create(panel);
    ui_text_editor_set_static(button_row);
    lv_obj_set_width(button_row, lv_pct(100));
    lv_obj_set_height(button_row, 38);
    lv_obj_set_style_bg_opa(button_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(button_row, 0, 0);
    lv_obj_set_style_pad_all(button_row, 0, 0);
    lv_obj_set_style_pad_column(button_row, 8, 0);
    lv_obj_set_layout(button_row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(button_row, LV_FLEX_FLOW_ROW);

    ui_text_editor_create_button(button_row,
                                 "Cancel",
                                 lv_color_hex(0x4B4741),
                                 lv_color_hex(0x343434),
                                 ui_text_editor_cancel_event_cb,
                                 editor);
    ui_text_editor_create_button(button_row,
                                 "OK",
                                 lv_color_hex(0xF47B32),
                                 lv_color_hex(0xC85F22),
                                 ui_text_editor_ok_event_cb,
                                 editor);

    editor->keyboard = lv_keyboard_create(panel);
    lv_obj_set_width(editor->keyboard, lv_pct(100));
    lv_obj_set_flex_grow(editor->keyboard, 1);
    lv_keyboard_set_textarea(editor->keyboard, editor->textarea);
}

/****************************************************************************************
 * @brief Show the editor with caller-owned commit behavior.
 ****************************************************************************************/
void ui_text_editor_show(ui_text_editor_t *editor,
                         const char *title,
                         const char *initial_text,
                         ui_text_editor_commit_handler_t commit_handler,
                         void *user_data)
{
    if (editor == NULL || editor->overlay == NULL)
    {
        return;
    }

    editor->commit_handler = commit_handler;
    editor->commit_user_data = user_data;
    lv_label_set_text(editor->title_label, title != NULL ? title : "");
    lv_textarea_set_text(editor->textarea, initial_text != NULL ? initial_text : "");
    lv_obj_remove_flag(editor->overlay, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(editor->overlay);
}

/****************************************************************************************
 * @brief Hide the editor without committing any value.
 ****************************************************************************************/
void ui_text_editor_hide(ui_text_editor_t *editor)
{
    if (editor == NULL || editor->overlay == NULL)
    {
        return;
    }

    lv_obj_add_flag(editor->overlay, LV_OBJ_FLAG_HIDDEN);
}
