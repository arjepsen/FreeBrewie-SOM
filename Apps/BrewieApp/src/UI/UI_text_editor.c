#include "UI_text_editor.h"

#include "Logic/Recipe_draft.h"

#include <string.h>

#define UI_TEXT_EDITOR_KEY_WIDTH_NORMAL LV_BUTTONMATRIX_CTRL_WIDTH_1
#define UI_TEXT_EDITOR_KEY_WIDTH_MODE LV_BUTTONMATRIX_CTRL_WIDTH_2
#define UI_TEXT_EDITOR_KEY_WIDTH_DONE LV_BUTTONMATRIX_CTRL_WIDTH_3
#define UI_TEXT_EDITOR_KEY_WIDTH_SPACE LV_BUTTONMATRIX_CTRL_WIDTH_6
#define UI_TEXT_EDITOR_KEY_CONTROL(width) (LV_KEYBOARD_CTRL_BUTTON_FLAGS | (width))
#define UI_TEXT_EDITOR_KEY_ACTION(width) \
    (LV_BUTTONMATRIX_CTRL_NO_REPEAT | LV_BUTTONMATRIX_CTRL_CLICK_TRIG | LV_BUTTONMATRIX_CTRL_CHECKED | (width))
#define UI_TEXT_EDITOR_KEY_TEXT_LOWER "abc"
#define UI_TEXT_EDITOR_KEY_TEXT_UPPER "ABC"
#define UI_TEXT_EDITOR_KEY_SPECIAL "123"
#define UI_TEXT_EDITOR_KEY_SHIFT LV_SYMBOL_UP
#define UI_TEXT_EDITOR_KEY_DONE "Done"
#define UI_TEXT_EDITOR_COLOR_BACKGROUND 0x050909
#define UI_TEXT_EDITOR_COLOR_PANEL 0x070B0B
#define UI_TEXT_EDITOR_COLOR_FIELD 0x0B1010
#define UI_TEXT_EDITOR_COLOR_KEYBOARD 0x101414
#define UI_TEXT_EDITOR_COLOR_KEY 0x2A2F2F
#define UI_TEXT_EDITOR_COLOR_KEY_PRESSED 0x3A3F3F
#define UI_TEXT_EDITOR_COLOR_KEY_BORDER 0x5E6666
#define UI_TEXT_EDITOR_COLOR_TEXT 0xFFFFFF
#define UI_TEXT_EDITOR_COLOR_TEXT_MUTED 0xC7C2BE
#define UI_TEXT_EDITOR_COLOR_ACCENT 0xFF7A00
#define UI_TEXT_EDITOR_COLOR_ACCENT_PRESSED 0xD86500
#define UI_TEXT_EDITOR_NORMAL_KEY_ROW_7 \
    UI_TEXT_EDITOR_KEY_WIDTH_NORMAL, UI_TEXT_EDITOR_KEY_WIDTH_NORMAL, UI_TEXT_EDITOR_KEY_WIDTH_NORMAL, \
    UI_TEXT_EDITOR_KEY_WIDTH_NORMAL, UI_TEXT_EDITOR_KEY_WIDTH_NORMAL, UI_TEXT_EDITOR_KEY_WIDTH_NORMAL, \
    UI_TEXT_EDITOR_KEY_WIDTH_NORMAL
#define UI_TEXT_EDITOR_NORMAL_KEY_GRID_28 \
    UI_TEXT_EDITOR_NORMAL_KEY_ROW_7, UI_TEXT_EDITOR_NORMAL_KEY_ROW_7, \
    UI_TEXT_EDITOR_NORMAL_KEY_ROW_7, UI_TEXT_EDITOR_NORMAL_KEY_ROW_7
#define UI_TEXT_EDITOR_NORMAL_KEY_ROW_5 \
    UI_TEXT_EDITOR_KEY_WIDTH_NORMAL, UI_TEXT_EDITOR_KEY_WIDTH_NORMAL, UI_TEXT_EDITOR_KEY_WIDTH_NORMAL, \
    UI_TEXT_EDITOR_KEY_WIDTH_NORMAL, UI_TEXT_EDITOR_KEY_WIDTH_NORMAL

static const char *const ui_text_editor_keyboard_lower_map[] = {
    "a", "b", "c", "d", "e", "f", "g", "\n",
    "h", "i", "j", "k", "l", "m", "n", "\n",
    "o", "p", "q", "r", "s", "t", "u", "\n",
    "v", "w", "x", "y", "z", UI_TEXT_EDITOR_KEY_SHIFT, LV_SYMBOL_BACKSPACE, "\n",
    UI_TEXT_EDITOR_KEY_SPECIAL, " ", UI_TEXT_EDITOR_KEY_DONE, ""};

static const lv_buttonmatrix_ctrl_t ui_text_editor_keyboard_lower_ctrl[] = {
    UI_TEXT_EDITOR_NORMAL_KEY_ROW_7, UI_TEXT_EDITOR_NORMAL_KEY_ROW_7, UI_TEXT_EDITOR_NORMAL_KEY_ROW_7,
    UI_TEXT_EDITOR_NORMAL_KEY_ROW_5,
    UI_TEXT_EDITOR_KEY_ACTION(UI_TEXT_EDITOR_KEY_WIDTH_NORMAL),
    UI_TEXT_EDITOR_KEY_ACTION(UI_TEXT_EDITOR_KEY_WIDTH_NORMAL),
    UI_TEXT_EDITOR_KEY_ACTION(UI_TEXT_EDITOR_KEY_WIDTH_MODE),
    UI_TEXT_EDITOR_KEY_WIDTH_SPACE,
    UI_TEXT_EDITOR_KEY_CONTROL(UI_TEXT_EDITOR_KEY_WIDTH_DONE)};

static const char *const ui_text_editor_keyboard_upper_map[] = {
    "A", "B", "C", "D", "E", "F", "G", "\n",
    "H", "I", "J", "K", "L", "M", "N", "\n",
    "O", "P", "Q", "R", "S", "T", "U", "\n",
    "V", "W", "X", "Y", "Z", UI_TEXT_EDITOR_KEY_SHIFT, LV_SYMBOL_BACKSPACE, "\n",
    UI_TEXT_EDITOR_KEY_SPECIAL, " ", UI_TEXT_EDITOR_KEY_DONE, ""};

static const lv_buttonmatrix_ctrl_t ui_text_editor_keyboard_upper_ctrl[] = {
    UI_TEXT_EDITOR_NORMAL_KEY_ROW_7, UI_TEXT_EDITOR_NORMAL_KEY_ROW_7, UI_TEXT_EDITOR_NORMAL_KEY_ROW_7,
    UI_TEXT_EDITOR_NORMAL_KEY_ROW_5,
    UI_TEXT_EDITOR_KEY_ACTION(UI_TEXT_EDITOR_KEY_WIDTH_NORMAL),
    UI_TEXT_EDITOR_KEY_ACTION(UI_TEXT_EDITOR_KEY_WIDTH_NORMAL),
    UI_TEXT_EDITOR_KEY_ACTION(UI_TEXT_EDITOR_KEY_WIDTH_MODE),
    UI_TEXT_EDITOR_KEY_WIDTH_SPACE,
    UI_TEXT_EDITOR_KEY_CONTROL(UI_TEXT_EDITOR_KEY_WIDTH_DONE)};

static const char *const ui_text_editor_keyboard_special_map[] = {
    "1", "2", "3", "4", "5", "6", "7", "\n",
    "8", "9", "0", "/", ":", ";", "%", "\n",
    "æ", "ø", "å", "Æ", "Ø", "Å", "_", "\n",
    ".", ",", "?", "!", "+", "#", "@", "\n",
    UI_TEXT_EDITOR_KEY_TEXT_UPPER, " ", UI_TEXT_EDITOR_KEY_DONE, ""};

static const lv_buttonmatrix_ctrl_t ui_text_editor_keyboard_special_ctrl[] = {
    UI_TEXT_EDITOR_NORMAL_KEY_GRID_28,
    UI_TEXT_EDITOR_KEY_ACTION(UI_TEXT_EDITOR_KEY_WIDTH_MODE),
    UI_TEXT_EDITOR_KEY_WIDTH_SPACE,
    UI_TEXT_EDITOR_KEY_CONTROL(UI_TEXT_EDITOR_KEY_WIDTH_DONE),
};

static void ui_text_editor_set_static(lv_obj_t *object);
static lv_obj_t *ui_text_editor_create_header_button(lv_obj_t *parent,
                                                     const char *text,
                                                     lv_event_cb_t event_cb,
                                                     ui_text_editor_t *editor);
static void ui_text_editor_cancel_event_cb(lv_event_t *event);
static void ui_text_editor_commit(ui_text_editor_t *editor);
static void ui_text_editor_keyboard_value_event_cb(lv_event_t *event);
static void ui_text_editor_ready_event_cb(lv_event_t *event);

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

static lv_obj_t *ui_text_editor_create_header_button(lv_obj_t *parent,
                                                     const char *text,
                                                     lv_event_cb_t event_cb,
                                                     ui_text_editor_t *editor)
{
    lv_obj_t *button;
    lv_obj_t *label;

    button = lv_button_create(parent);
    ui_text_editor_set_static(button);
    lv_obj_set_size(button, 38, 38);
    lv_obj_set_style_bg_opa(button, LV_OPA_TRANSP, 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(UI_TEXT_EDITOR_COLOR_KEY_PRESSED), LV_STATE_PRESSED);
    lv_obj_set_style_border_width(button, 0, 0);
    lv_obj_set_style_radius(button, 5, 0);
    lv_obj_set_style_pad_all(button, 0, 0);
    lv_obj_add_event_cb(button, event_cb, LV_EVENT_CLICKED, editor);

    label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_hex(UI_TEXT_EDITOR_COLOR_ACCENT), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_center(label);
    return button;
}

static void ui_text_editor_cancel_event_cb(lv_event_t *event)
{
    ui_text_editor_t *editor;

    editor = lv_event_get_user_data(event);
    ui_text_editor_hide(editor);
}

static void ui_text_editor_ready_event_cb(lv_event_t *event)
{
    ui_text_editor_t *editor;

    editor = lv_event_get_user_data(event);
    ui_text_editor_commit(editor);
}

/****************************************************************************************
 * @brief Commit the text currently shown in the textarea and close the editor.
 ****************************************************************************************/
static void ui_text_editor_commit(ui_text_editor_t *editor)
{
    if (editor != NULL && editor->commit_handler != NULL)
    {
        editor->commit_handler(lv_textarea_get_text(editor->textarea), editor->commit_user_data);
    }

    ui_text_editor_hide(editor);
}

/****************************************************************************************
 * @brief Handle custom keyboard labels before falling back to LVGL's default keyboard.
 *
 * LVGL's built-in keyboard changes mode only for its exact mode-label strings. The old
 * Brewie-style keyboard uses a visual shift arrow and a literal "Done" label, so those
 * controls need a tiny adapter while ordinary text/backspace behavior stays with LVGL.
 ****************************************************************************************/
static void ui_text_editor_keyboard_value_event_cb(lv_event_t *event)
{
    lv_obj_t *keyboard;
    uint32_t button_id;
    const char *button_text;
    ui_text_editor_t *editor;

    keyboard = lv_event_get_current_target(event);
    button_id = lv_buttonmatrix_get_selected_button(keyboard);
    if (button_id == LV_BUTTONMATRIX_BUTTON_NONE)
    {
        return;
    }

    button_text = lv_buttonmatrix_get_button_text(keyboard, button_id);
    if (button_text == NULL)
    {
        return;
    }

    if (strcmp(button_text, UI_TEXT_EDITOR_KEY_SHIFT) == 0)
    {
        if (lv_keyboard_get_mode(keyboard) == LV_KEYBOARD_MODE_TEXT_UPPER)
        {
            lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);
        }
        else
        {
            lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_TEXT_UPPER);
        }
        return;
    }

    if (strcmp(button_text, UI_TEXT_EDITOR_KEY_DONE) == 0)
    {
        editor = lv_event_get_user_data(event);
        ui_text_editor_commit(editor);
        return;
    }

    lv_keyboard_def_event_cb(event);
}

/****************************************************************************************
 * @brief Create a hidden reusable text editor under parent.
 ****************************************************************************************/
void ui_text_editor_init(ui_text_editor_t *editor, lv_obj_t *parent)
{
    lv_obj_t *panel;
    lv_obj_t *header_row;
    lv_obj_t *header_spacer;
    lv_obj_t *hint_label;
    lv_obj_t *limit_label;

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
    lv_obj_set_style_bg_color(editor->overlay, lv_color_hex(UI_TEXT_EDITOR_COLOR_BACKGROUND), 0);
    lv_obj_set_style_bg_opa(editor->overlay, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(editor->overlay, 0, 0);
    lv_obj_set_style_pad_all(editor->overlay, 0, 0);
    lv_obj_add_flag(editor->overlay, LV_OBJ_FLAG_HIDDEN);

    panel = lv_obj_create(editor->overlay);
    ui_text_editor_set_static(panel);
    lv_obj_set_size(panel, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(panel, lv_color_hex(UI_TEXT_EDITOR_COLOR_PANEL), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(panel, 0, 0);
    lv_obj_set_style_radius(panel, 0, 0);
    lv_obj_set_style_pad_left(panel, 8, 0);
    lv_obj_set_style_pad_right(panel, 8, 0);
    lv_obj_set_style_pad_top(panel, 6, 0);
    lv_obj_set_style_pad_bottom(panel, 6, 0);
    lv_obj_set_style_pad_row(panel, 8, 0);
    lv_obj_set_layout(panel, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);

    header_row = lv_obj_create(panel);
    ui_text_editor_set_static(header_row);
    lv_obj_set_width(header_row, lv_pct(100));
    lv_obj_set_height(header_row, 42);
    lv_obj_set_style_bg_opa(header_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(header_row, 0, 0);
    lv_obj_set_style_pad_all(header_row, 0, 0);
    lv_obj_set_style_pad_column(header_row, 0, 0);
    lv_obj_set_layout(header_row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(header_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    ui_text_editor_create_header_button(header_row, LV_SYMBOL_LEFT, ui_text_editor_cancel_event_cb, editor);

    editor->title_label = lv_label_create(header_row);
    lv_label_set_text(editor->title_label, "--");
    lv_label_set_long_mode(editor->title_label, LV_LABEL_LONG_DOT);
    lv_obj_set_flex_grow(editor->title_label, 1);
    lv_obj_set_style_text_align(editor->title_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(editor->title_label, lv_color_hex(UI_TEXT_EDITOR_COLOR_ACCENT), 0);
    lv_obj_set_style_text_font(editor->title_label, &lv_font_montserrat_20, 0);

    header_spacer = lv_obj_create(header_row);
    ui_text_editor_set_static(header_spacer);
    lv_obj_set_size(header_spacer, 38, 38);
    lv_obj_set_style_bg_opa(header_spacer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(header_spacer, 0, 0);
    lv_obj_set_style_pad_all(header_spacer, 0, 0);

    hint_label = lv_label_create(panel);
    lv_label_set_text(hint_label, "Enter a name for this recipe");
    lv_obj_set_width(hint_label, lv_pct(100));
    lv_obj_set_style_text_align(hint_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(hint_label, lv_color_hex(UI_TEXT_EDITOR_COLOR_TEXT_MUTED), 0);

    editor->textarea = lv_textarea_create(panel);
    lv_obj_set_width(editor->textarea, lv_pct(100));
    lv_obj_set_height(editor->textarea, 56);
    lv_textarea_set_one_line(editor->textarea, true);
    lv_textarea_set_max_length(editor->textarea, RECIPE_DRAFT_NAME_MAX_LENGTH - 1U);
    lv_obj_set_style_text_color(editor->textarea, lv_color_hex(UI_TEXT_EDITOR_COLOR_TEXT), 0);
    lv_obj_set_style_text_font(editor->textarea, &lv_font_montserrat_20, 0);
    lv_obj_set_style_bg_color(editor->textarea, lv_color_hex(UI_TEXT_EDITOR_COLOR_FIELD), 0);
    lv_obj_set_style_border_color(editor->textarea, lv_color_hex(UI_TEXT_EDITOR_COLOR_ACCENT), 0);
    lv_obj_set_style_border_width(editor->textarea, 2, 0);
    lv_obj_set_style_radius(editor->textarea, 8, 0);
    lv_obj_set_style_pad_left(editor->textarea, 12, 0);
    lv_obj_set_style_pad_right(editor->textarea, 12, 0);

    limit_label = lv_label_create(panel);
    lv_label_set_text_fmt(limit_label, "1 - %u characters", (unsigned int)(RECIPE_DRAFT_NAME_MAX_LENGTH - 1U));
    lv_obj_set_width(limit_label, lv_pct(100));
    lv_obj_set_style_text_color(limit_label, lv_color_hex(UI_TEXT_EDITOR_COLOR_TEXT_MUTED), 0);

    header_spacer = lv_obj_create(panel);
    ui_text_editor_set_static(header_spacer);
    lv_obj_set_width(header_spacer, lv_pct(100));
    lv_obj_set_flex_grow(header_spacer, 1);
    lv_obj_set_style_bg_opa(header_spacer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(header_spacer, 0, 0);
    lv_obj_set_style_pad_all(header_spacer, 0, 0);

    editor->keyboard = lv_keyboard_create(panel);
    lv_obj_set_width(editor->keyboard, lv_pct(100));
    lv_obj_set_height(editor->keyboard, 190);
    lv_obj_set_style_bg_color(editor->keyboard, lv_color_hex(UI_TEXT_EDITOR_COLOR_KEYBOARD), 0);
    lv_obj_set_style_bg_opa(editor->keyboard, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(editor->keyboard, 0, 0);
    lv_obj_set_style_pad_all(editor->keyboard, 4, 0);
    lv_obj_set_style_pad_row(editor->keyboard, 4, 0);
    lv_obj_set_style_pad_column(editor->keyboard, 4, 0);
    lv_obj_set_style_bg_color(editor->keyboard, lv_color_hex(UI_TEXT_EDITOR_COLOR_KEY), LV_PART_ITEMS);
    lv_obj_set_style_bg_color(editor->keyboard, lv_color_hex(UI_TEXT_EDITOR_COLOR_KEY_PRESSED), LV_PART_ITEMS | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(editor->keyboard, lv_color_hex(UI_TEXT_EDITOR_COLOR_KEY), LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(editor->keyboard, lv_color_hex(UI_TEXT_EDITOR_COLOR_TEXT), LV_PART_ITEMS);
    lv_obj_set_style_text_color(editor->keyboard, lv_color_hex(UI_TEXT_EDITOR_COLOR_ACCENT), LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_radius(editor->keyboard, 4, LV_PART_ITEMS);
    lv_obj_set_style_border_width(editor->keyboard, 1, LV_PART_ITEMS);
    lv_obj_set_style_border_color(editor->keyboard, lv_color_hex(UI_TEXT_EDITOR_COLOR_KEY_BORDER), LV_PART_ITEMS);
    lv_keyboard_set_map(editor->keyboard,
                        LV_KEYBOARD_MODE_TEXT_LOWER,
                        ui_text_editor_keyboard_lower_map,
                        ui_text_editor_keyboard_lower_ctrl);
    lv_keyboard_set_map(editor->keyboard,
                        LV_KEYBOARD_MODE_TEXT_UPPER,
                        ui_text_editor_keyboard_upper_map,
                        ui_text_editor_keyboard_upper_ctrl);
    lv_keyboard_set_map(editor->keyboard,
                        LV_KEYBOARD_MODE_SPECIAL,
                        ui_text_editor_keyboard_special_map,
                        ui_text_editor_keyboard_special_ctrl);
    lv_keyboard_set_mode(editor->keyboard, LV_KEYBOARD_MODE_TEXT_UPPER);
    lv_keyboard_set_textarea(editor->keyboard, editor->textarea);
    lv_obj_remove_event_cb(editor->keyboard, lv_keyboard_def_event_cb);
    lv_obj_add_event_cb(editor->keyboard, ui_text_editor_keyboard_value_event_cb, LV_EVENT_VALUE_CHANGED, editor);
    lv_obj_add_event_cb(editor->keyboard, ui_text_editor_ready_event_cb, LV_EVENT_READY, editor);
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
    lv_keyboard_set_mode(editor->keyboard, LV_KEYBOARD_MODE_TEXT_UPPER);
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
