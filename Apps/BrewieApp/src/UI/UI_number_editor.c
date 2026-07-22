#include "UI_number_editor.h"

#include <stdio.h>

#define UI_NUMBER_EDITOR_COLOR_PANEL 0x1F1D1B
#define UI_NUMBER_EDITOR_COLOR_FIELD 0x0B1010
#define UI_NUMBER_EDITOR_COLOR_MUTED 0xC7C2BE
#define UI_NUMBER_EDITOR_COLOR_ACCENT 0xE67526
#define UI_NUMBER_EDITOR_COLOR_ACCENT_PRESSED 0xC85F22
#define UI_NUMBER_EDITOR_COLOR_BUTTON 0x4B4741
#define UI_NUMBER_EDITOR_COLOR_BUTTON_PRESSED 0x343434
#define UI_NUMBER_EDITOR_COLOR_TEXT 0xFFFFFF

static void ui_number_editor_set_static(lv_obj_t *object);
static lv_obj_t *ui_number_editor_create_button(lv_obj_t *parent,
                                                const char *text,
                                                lv_color_t normal_color,
                                                lv_color_t pressed_color,
                                                lv_event_cb_t event_cb,
                                                ui_number_editor_t *editor);
static void ui_number_editor_refresh_value(ui_number_editor_t *editor);
static void ui_number_editor_decrement_event_cb(lv_event_t *event);
static void ui_number_editor_increment_event_cb(lv_event_t *event);
static void ui_number_editor_cancel_event_cb(lv_event_t *event);
static void ui_number_editor_done_event_cb(lv_event_t *event);

/****************************************************************************************
 * @brief Make a helper object static so it cannot become a tiny scroll target.
 ****************************************************************************************/
static void ui_number_editor_set_static(lv_obj_t *object)
{
    if (object == NULL)
    {
        return;
    }

    lv_obj_clear_flag(object, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_OFF);
}

/****************************************************************************************
 * @brief Create a fixed-height modal button.
 ****************************************************************************************/
static lv_obj_t *ui_number_editor_create_button(lv_obj_t *parent,
                                                const char *text,
                                                lv_color_t normal_color,
                                                lv_color_t pressed_color,
                                                lv_event_cb_t event_cb,
                                                ui_number_editor_t *editor)
{
    lv_obj_t *button;
    lv_obj_t *label;

    button = lv_button_create(parent);
    lv_obj_set_flex_grow(button, 1);
    lv_obj_set_height(button, 44);
    lv_obj_set_style_bg_color(button, normal_color, 0);
    lv_obj_set_style_bg_color(button, pressed_color, LV_STATE_PRESSED);
    lv_obj_set_style_radius(button, 5, 0);
    lv_obj_set_style_border_width(button, 0, 0);
    lv_obj_add_event_cb(button, event_cb, LV_EVENT_CLICKED, editor);

    label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_hex(UI_NUMBER_EDITOR_COLOR_TEXT), 0);
    lv_obj_center(label);
    lv_obj_remove_flag(label, LV_OBJ_FLAG_CLICKABLE);
    return button;
}

/****************************************************************************************
 * @brief Refresh the visible working value without using floating point.
 ****************************************************************************************/
static void ui_number_editor_refresh_value(ui_number_editor_t *editor)
{
    char value_text[16];

    if (editor == NULL || editor->value_label == NULL)
    {
        return;
    }

    if (editor->show_as_liters)
    {
        snprintf(value_text,
                 sizeof(value_text),
                 "%u.%u",
                 (unsigned int)(editor->value / 10U),
                 (unsigned int)(editor->value % 10U));
    }
    else
    {
        snprintf(value_text, sizeof(value_text), "%u", (unsigned int)editor->value);
    }

    lv_label_set_text(editor->value_label, value_text);
}

static void ui_number_editor_decrement_event_cb(lv_event_t *event)
{
    ui_number_editor_t *editor;

    editor = lv_event_get_user_data(event);
    if (editor == NULL)
    {
        return;
    }

    if (editor->value <= editor->minimum + editor->step)
    {
        editor->value = editor->minimum;
    }
    else
    {
        editor->value = (uint16_t)(editor->value - editor->step);
    }

    ui_number_editor_refresh_value(editor);
}

static void ui_number_editor_increment_event_cb(lv_event_t *event)
{
    ui_number_editor_t *editor;

    editor = lv_event_get_user_data(event);
    if (editor == NULL)
    {
        return;
    }

    if (editor->value >= editor->maximum - editor->step)
    {
        editor->value = editor->maximum;
    }
    else
    {
        editor->value = (uint16_t)(editor->value + editor->step);
    }

    ui_number_editor_refresh_value(editor);
}

static void ui_number_editor_cancel_event_cb(lv_event_t *event)
{
    ui_number_editor_t *editor;

    editor = lv_event_get_user_data(event);
    ui_number_editor_hide(editor);
}

static void ui_number_editor_done_event_cb(lv_event_t *event)
{
    ui_number_editor_t *editor;

    editor = lv_event_get_user_data(event);
    if (editor != NULL && editor->commit_handler != NULL)
    {
        editor->commit_handler(editor->value, editor->commit_user_data);
    }

    ui_number_editor_hide(editor);
}

/****************************************************************************************
 * @brief Create a hidden reusable numeric editor under parent.
 ****************************************************************************************/
void ui_number_editor_init(ui_number_editor_t *editor, lv_obj_t *parent)
{
    lv_obj_t *panel;
    lv_obj_t *value_row;
    lv_obj_t *button_row;

    if (editor == NULL || parent == NULL)
    {
        return;
    }

    editor->commit_handler = NULL;
    editor->commit_user_data = NULL;

    editor->overlay = lv_obj_create(parent);
    ui_number_editor_set_static(editor->overlay);
    lv_obj_set_size(editor->overlay, lv_pct(100), lv_pct(100));
    lv_obj_center(editor->overlay);
    lv_obj_set_style_bg_color(editor->overlay, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(editor->overlay, LV_OPA_70, 0);
    lv_obj_set_style_border_width(editor->overlay, 0, 0);
    lv_obj_set_style_pad_all(editor->overlay, 12, 0);
    lv_obj_add_flag(editor->overlay, LV_OBJ_FLAG_HIDDEN);

    panel = lv_obj_create(editor->overlay);
    ui_number_editor_set_static(panel);
    lv_obj_set_width(panel, lv_pct(100));
    lv_obj_set_height(panel, LV_SIZE_CONTENT);
    lv_obj_center(panel);
    lv_obj_set_style_bg_color(panel, lv_color_hex(UI_NUMBER_EDITOR_COLOR_PANEL), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(UI_NUMBER_EDITOR_COLOR_ACCENT), 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_radius(panel, 5, 0);
    lv_obj_set_style_pad_all(panel, 12, 0);
    lv_obj_set_style_pad_row(panel, 10, 0);
    lv_obj_set_layout(panel, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);

    editor->title_label = lv_label_create(panel);
    lv_label_set_text(editor->title_label, "--");
    lv_label_set_long_mode(editor->title_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(editor->title_label, lv_pct(100));
    lv_obj_set_style_text_color(editor->title_label, lv_color_hex(UI_NUMBER_EDITOR_COLOR_ACCENT), 0);
    lv_obj_set_style_text_font(editor->title_label, &lv_font_montserrat_20, 0);

    value_row = lv_obj_create(panel);
    ui_number_editor_set_static(value_row);
    lv_obj_set_width(value_row, lv_pct(100));
    lv_obj_set_height(value_row, 58);
    lv_obj_set_style_bg_color(value_row, lv_color_hex(UI_NUMBER_EDITOR_COLOR_FIELD), 0);
    lv_obj_set_style_bg_opa(value_row, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(value_row, lv_color_hex(UI_NUMBER_EDITOR_COLOR_ACCENT), 0);
    lv_obj_set_style_border_width(value_row, 1, 0);
    lv_obj_set_style_radius(value_row, 5, 0);
    lv_obj_set_style_pad_all(value_row, 8, 0);

    editor->value_label = lv_label_create(value_row);
    lv_label_set_text(editor->value_label, "--");
    lv_obj_set_style_text_color(editor->value_label, lv_color_hex(UI_NUMBER_EDITOR_COLOR_TEXT), 0);
    lv_obj_set_style_text_font(editor->value_label, &lv_font_montserrat_20, 0);
    lv_obj_align(editor->value_label, LV_ALIGN_LEFT_MID, 8, 0);

    editor->unit_label = lv_label_create(value_row);
    lv_label_set_text(editor->unit_label, "");
    lv_obj_set_style_text_color(editor->unit_label, lv_color_hex(UI_NUMBER_EDITOR_COLOR_MUTED), 0);
    lv_obj_align(editor->unit_label, LV_ALIGN_RIGHT_MID, -8, 0);

    button_row = lv_obj_create(panel);
    ui_number_editor_set_static(button_row);
    lv_obj_set_width(button_row, lv_pct(100));
    lv_obj_set_height(button_row, 44);
    lv_obj_set_style_bg_opa(button_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(button_row, 0, 0);
    lv_obj_set_style_pad_all(button_row, 0, 0);
    lv_obj_set_style_pad_column(button_row, 8, 0);
    lv_obj_set_layout(button_row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(button_row, LV_FLEX_FLOW_ROW);

    ui_number_editor_create_button(button_row,
                                   "-",
                                   lv_color_hex(UI_NUMBER_EDITOR_COLOR_BUTTON),
                                   lv_color_hex(UI_NUMBER_EDITOR_COLOR_BUTTON_PRESSED),
                                   ui_number_editor_decrement_event_cb,
                                   editor);
    ui_number_editor_create_button(button_row,
                                   "+",
                                   lv_color_hex(UI_NUMBER_EDITOR_COLOR_BUTTON),
                                   lv_color_hex(UI_NUMBER_EDITOR_COLOR_BUTTON_PRESSED),
                                   ui_number_editor_increment_event_cb,
                                   editor);

    button_row = lv_obj_create(panel);
    ui_number_editor_set_static(button_row);
    lv_obj_set_width(button_row, lv_pct(100));
    lv_obj_set_height(button_row, 44);
    lv_obj_set_style_bg_opa(button_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(button_row, 0, 0);
    lv_obj_set_style_pad_all(button_row, 0, 0);
    lv_obj_set_style_pad_column(button_row, 8, 0);
    lv_obj_set_layout(button_row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(button_row, LV_FLEX_FLOW_ROW);

    ui_number_editor_create_button(button_row,
                                   "Cancel",
                                   lv_color_hex(UI_NUMBER_EDITOR_COLOR_BUTTON),
                                   lv_color_hex(UI_NUMBER_EDITOR_COLOR_BUTTON_PRESSED),
                                   ui_number_editor_cancel_event_cb,
                                   editor);
    ui_number_editor_create_button(button_row,
                                   "Done",
                                   lv_color_hex(UI_NUMBER_EDITOR_COLOR_ACCENT),
                                   lv_color_hex(UI_NUMBER_EDITOR_COLOR_ACCENT_PRESSED),
                                   ui_number_editor_done_event_cb,
                                   editor);
}

/****************************************************************************************
 * @brief Show the editor with caller-provided bounds and commit behavior.
 ****************************************************************************************/
void ui_number_editor_show(ui_number_editor_t *editor,
                           const char *title,
                           const char *unit_text,
                           uint16_t value,
                           uint16_t minimum,
                           uint16_t maximum,
                           uint16_t step,
                           bool show_as_liters,
                           ui_number_editor_commit_handler_t commit_handler,
                           void *user_data)
{
    if (editor == NULL || editor->overlay == NULL)
    {
        return;
    }

    editor->minimum = minimum;
    editor->maximum = maximum;
    editor->step = (step == 0U) ? 1U : step;
    editor->value = value;
    if (editor->value < minimum)
    {
        editor->value = minimum;
    }
    if (editor->value > maximum)
    {
        editor->value = maximum;
    }

    editor->show_as_liters = show_as_liters;
    editor->commit_handler = commit_handler;
    editor->commit_user_data = user_data;
    lv_label_set_text(editor->title_label, title != NULL ? title : "");
    lv_label_set_text(editor->unit_label, unit_text != NULL ? unit_text : "");
    ui_number_editor_refresh_value(editor);
    lv_obj_remove_flag(editor->overlay, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(editor->overlay);
}

/****************************************************************************************
 * @brief Hide the editor without committing the working value.
 ****************************************************************************************/
void ui_number_editor_hide(ui_number_editor_t *editor)
{
    if (editor == NULL || editor->overlay == NULL)
    {
        return;
    }

    lv_obj_add_flag(editor->overlay, LV_OBJ_FLAG_HIDDEN);
}
