#include "UI_dialog.h"

static void ui_dialog_set_static(lv_obj_t *object);
static lv_obj_t *ui_dialog_create_button(lv_obj_t *parent,
                                         const char *text,
                                         lv_color_t normal_color,
                                         lv_color_t pressed_color,
                                         lv_event_cb_t event_cb,
                                         ui_dialog_t *dialog);
static void ui_dialog_primary_event_cb(lv_event_t *event);
static void ui_dialog_secondary_event_cb(lv_event_t *event);

/****************************************************************************************
 * @brief Make a dialog sub-object static and non-scrollable.
 ****************************************************************************************/
static void ui_dialog_set_static(lv_obj_t *object)
{
    if (object == NULL)
    {
        return;
    }

    lv_obj_clear_flag(object, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_OFF);
}

/****************************************************************************************
 * @brief Create one fixed-height dialog action button.
 ****************************************************************************************/
static lv_obj_t *ui_dialog_create_button(lv_obj_t *parent,
                                         const char *text,
                                         lv_color_t normal_color,
                                         lv_color_t pressed_color,
                                         lv_event_cb_t event_cb,
                                         ui_dialog_t *dialog)
{
    lv_obj_t *button;
    lv_obj_t *button_label;

    button = lv_button_create(parent);
    lv_obj_set_flex_grow(button, 1);
    lv_obj_set_height(button, 42);
    lv_obj_set_style_bg_color(button, normal_color, 0);
    lv_obj_set_style_bg_color(button, pressed_color, LV_STATE_PRESSED);
    lv_obj_set_style_radius(button, 5, 0);
    lv_obj_add_event_cb(button, event_cb, LV_EVENT_CLICKED, dialog);

    button_label = lv_label_create(button);
    lv_label_set_text(button_label, text != NULL ? text : "OK");
    lv_obj_set_style_text_color(button_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(button_label);
    return button;
}

/****************************************************************************************
 * @brief Run the optional primary action, then close the dialog.
 ****************************************************************************************/
static void ui_dialog_primary_event_cb(lv_event_t *event)
{
    ui_dialog_t *dialog;

    dialog = lv_event_get_user_data(event);
    if (dialog != NULL && dialog->primary_handler != NULL)
    {
        dialog->primary_handler(dialog->primary_user_data);
    }

    ui_dialog_hide(dialog);
}

/****************************************************************************************
 * @brief Run the optional secondary action, then close the dialog.
 ****************************************************************************************/
static void ui_dialog_secondary_event_cb(lv_event_t *event)
{
    ui_dialog_t *dialog;

    dialog = lv_event_get_user_data(event);
    if (dialog != NULL && dialog->secondary_handler != NULL)
    {
        dialog->secondary_handler(dialog->secondary_user_data);
    }

    ui_dialog_hide(dialog);
}

/****************************************************************************************
 * @brief Create a hidden reusable modal dialog under parent.
 ****************************************************************************************/
void ui_dialog_init(ui_dialog_t *dialog,
                    lv_obj_t *parent,
                    const char *primary_text,
                    const char *secondary_text)
{
    lv_obj_t *panel;
    lv_obj_t *button_row;

    if (dialog == NULL || parent == NULL)
    {
        return;
    }

    dialog->primary_handler = NULL;
    dialog->primary_user_data = NULL;
    dialog->secondary_handler = NULL;
    dialog->secondary_user_data = NULL;

    dialog->overlay = lv_obj_create(parent);
    ui_dialog_set_static(dialog->overlay);
    lv_obj_set_size(dialog->overlay, lv_pct(100), lv_pct(100));
    lv_obj_center(dialog->overlay);
    lv_obj_set_style_bg_color(dialog->overlay, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(dialog->overlay, LV_OPA_70, 0);
    lv_obj_set_style_border_width(dialog->overlay, 0, 0);
    lv_obj_set_style_pad_all(dialog->overlay, 14, 0);
    lv_obj_add_flag(dialog->overlay, LV_OBJ_FLAG_HIDDEN);

    panel = lv_obj_create(dialog->overlay);
    ui_dialog_set_static(panel);
    lv_obj_set_width(panel, lv_pct(100));
    lv_obj_set_height(panel, LV_SIZE_CONTENT);
    lv_obj_center(panel);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(0xE67526), 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_radius(panel, 5, 0);
    lv_obj_set_style_pad_all(panel, 12, 0);
    lv_obj_set_style_pad_row(panel, 10, 0);
    lv_obj_set_layout(panel, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);

    dialog->title_label = lv_label_create(panel);
    lv_label_set_text(dialog->title_label, "--");
    lv_label_set_long_mode(dialog->title_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(dialog->title_label, lv_pct(100));
    lv_obj_set_style_text_color(dialog->title_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(dialog->title_label, &lv_font_montserrat_20, 0);

    dialog->body_label = lv_label_create(panel);
    lv_label_set_text(dialog->body_label, "--");
    lv_label_set_long_mode(dialog->body_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(dialog->body_label, lv_pct(100));
    lv_obj_set_style_text_color(dialog->body_label, lv_color_hex(0xC8C8C8), 0);

    button_row = lv_obj_create(panel);
    ui_dialog_set_static(button_row);
    lv_obj_set_width(button_row, lv_pct(100));
    lv_obj_set_height(button_row, 42);
    lv_obj_set_style_bg_opa(button_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(button_row, 0, 0);
    lv_obj_set_style_pad_all(button_row, 0, 0);
    lv_obj_set_style_pad_column(button_row, 8, 0);
    lv_obj_set_layout(button_row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(button_row, LV_FLEX_FLOW_ROW);

    ui_dialog_create_button(button_row,
                            primary_text,
                            lv_color_hex(0x4B4741),
                            lv_color_hex(0x343434),
                            ui_dialog_primary_event_cb,
                            dialog);
    ui_dialog_create_button(button_row,
                            secondary_text,
                            lv_color_hex(0xF47B32),
                            lv_color_hex(0xC85F22),
                            ui_dialog_secondary_event_cb,
                            dialog);
}

/****************************************************************************************
 * @brief Set the optional action called by the left/primary dialog button.
 ****************************************************************************************/
void ui_dialog_set_primary_action(ui_dialog_t *dialog,
                                  ui_dialog_action_handler_t handler,
                                  void *user_data)
{
    if (dialog == NULL)
    {
        return;
    }

    dialog->primary_handler = handler;
    dialog->primary_user_data = user_data;
}

/****************************************************************************************
 * @brief Set the optional action called by the right/secondary dialog button.
 ****************************************************************************************/
void ui_dialog_set_secondary_action(ui_dialog_t *dialog,
                                    ui_dialog_action_handler_t handler,
                                    void *user_data)
{
    if (dialog == NULL)
    {
        return;
    }

    dialog->secondary_handler = handler;
    dialog->secondary_user_data = user_data;
}

/****************************************************************************************
 * @brief Show dialog with new title and body text.
 ****************************************************************************************/
void ui_dialog_show(ui_dialog_t *dialog, const char *title, const char *body)
{
    if (dialog == NULL || dialog->overlay == NULL)
    {
        return;
    }

    lv_label_set_text(dialog->title_label, title != NULL ? title : "");
    lv_label_set_text(dialog->body_label, body != NULL ? body : "");
    lv_obj_remove_flag(dialog->overlay, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(dialog->overlay);
}

/****************************************************************************************
 * @brief Hide dialog without changing its last text.
 ****************************************************************************************/
void ui_dialog_hide(ui_dialog_t *dialog)
{
    if (dialog == NULL || dialog->overlay == NULL)
    {
        return;
    }

    lv_obj_add_flag(dialog->overlay, LV_OBJ_FLAG_HIDDEN);
}
