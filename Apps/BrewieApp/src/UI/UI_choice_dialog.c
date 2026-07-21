#include "UI_choice_dialog.h"

#include "UI_scroll.h"

static void ui_choice_dialog_set_static(lv_obj_t *object);
static lv_obj_t *ui_choice_dialog_create_choice_row(lv_obj_t *parent,
                                                    ui_choice_dialog_row_context_t *context,
                                                    lv_obj_t **label);
static void ui_choice_dialog_choice_event_cb(lv_event_t *event);
static void ui_choice_dialog_cancel_event_cb(lv_event_t *event);

/****************************************************************************************
 * @brief Make a dialog object non-scrollable when it should behave as fixed chrome.
 ****************************************************************************************/
static void ui_choice_dialog_set_static(lv_obj_t *object)
{
    if (object == NULL)
    {
        return;
    }

    lv_obj_clear_flag(object, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_OFF);
}

/****************************************************************************************
 * @brief Create one reusable choice row.
 ****************************************************************************************/
static lv_obj_t *ui_choice_dialog_create_choice_row(lv_obj_t *parent,
                                                    ui_choice_dialog_row_context_t *context,
                                                    lv_obj_t **label)
{
    lv_obj_t *button;

    button = lv_button_create(parent);
    lv_obj_set_width(button, lv_pct(98));
    lv_obj_set_height(button, 42);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x282828), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x3B332D), LV_STATE_PRESSED);
    lv_obj_set_style_border_color(button, lv_color_hex(0x343434), 0);
    lv_obj_set_style_border_width(button, 1, 0);
    lv_obj_set_style_radius(button, 0, 0);
    lv_obj_add_event_cb(button, ui_choice_dialog_choice_event_cb, LV_EVENT_CLICKED, context);

    *label = lv_label_create(button);
    lv_label_set_text(*label, "--");
    lv_label_set_long_mode(*label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(*label, lv_pct(100));
    lv_obj_set_style_text_color(*label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(*label);
    lv_obj_remove_flag(*label, LV_OBJ_FLAG_CLICKABLE);
    return button;
}

/****************************************************************************************
 * @brief Send the chosen index to the caller, then close the dialog.
 ****************************************************************************************/
static void ui_choice_dialog_choice_event_cb(lv_event_t *event)
{
    ui_choice_dialog_row_context_t *context;
    ui_choice_dialog_t *dialog;

    context = lv_event_get_user_data(event);
    if (context == NULL || context->dialog == NULL)
    {
        return;
    }

    dialog = context->dialog;
    if (context->choice_index < dialog->choice_count && dialog->handler != NULL)
    {
        dialog->handler(context->choice_index, dialog->user_data);
    }

    ui_choice_dialog_hide(dialog);
}

/****************************************************************************************
 * @brief Close the dialog without firing a choice callback.
 ****************************************************************************************/
static void ui_choice_dialog_cancel_event_cb(lv_event_t *event)
{
    ui_choice_dialog_t *dialog;

    dialog = lv_event_get_user_data(event);
    ui_choice_dialog_hide(dialog);
}

/****************************************************************************************
 * @brief Create a hidden reusable choice dialog under parent.
 ****************************************************************************************/
void ui_choice_dialog_init(ui_choice_dialog_t *dialog, lv_obj_t *parent)
{
    lv_obj_t *panel;
    lv_obj_t *list;
    lv_obj_t *cancel_button;
    lv_obj_t *cancel_label;
    uint8_t choice_index;

    if (dialog == NULL || parent == NULL)
    {
        return;
    }

    dialog->choice_count = 0U;
    dialog->handler = NULL;
    dialog->user_data = NULL;

    dialog->overlay = lv_obj_create(parent);
    ui_choice_dialog_set_static(dialog->overlay);
    lv_obj_set_size(dialog->overlay, lv_pct(100), lv_pct(100));
    lv_obj_center(dialog->overlay);
    lv_obj_set_style_bg_color(dialog->overlay, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(dialog->overlay, LV_OPA_70, 0);
    lv_obj_set_style_border_width(dialog->overlay, 0, 0);
    lv_obj_set_style_pad_all(dialog->overlay, 14, 0);
    lv_obj_add_flag(dialog->overlay, LV_OBJ_FLAG_HIDDEN);

    panel = lv_obj_create(dialog->overlay);
    ui_choice_dialog_set_static(panel);
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

    list = lv_obj_create(panel);
    lv_obj_set_width(list, lv_pct(100));
    lv_obj_set_height(list, 252);
    lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(list, 0, 0);
    lv_obj_set_style_pad_all(list, 0, 0);
    lv_obj_set_style_pad_row(list, 8, 0);
    ui_scroll_apply_gutter(list);
    lv_obj_set_layout(list, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);

    for (choice_index = 0U; choice_index < UI_CHOICE_DIALOG_MAX_CHOICES; ++choice_index)
    {
        dialog->row_contexts[choice_index].choice_index = choice_index;
        dialog->row_contexts[choice_index].dialog = dialog;
        dialog->choice_buttons[choice_index] =
            ui_choice_dialog_create_choice_row(list,
                                               &dialog->row_contexts[choice_index],
                                               &dialog->choice_labels[choice_index]);
        lv_obj_add_flag(dialog->choice_buttons[choice_index], LV_OBJ_FLAG_HIDDEN);
    }

    cancel_button = lv_button_create(panel);
    lv_obj_set_width(cancel_button, lv_pct(100));
    lv_obj_set_height(cancel_button, 42);
    lv_obj_set_style_bg_color(cancel_button, lv_color_hex(0x4B4741), 0);
    lv_obj_set_style_bg_color(cancel_button, lv_color_hex(0x343434), LV_STATE_PRESSED);
    lv_obj_set_style_radius(cancel_button, 5, 0);
    lv_obj_add_event_cb(cancel_button, ui_choice_dialog_cancel_event_cb, LV_EVENT_CLICKED, dialog);

    cancel_label = lv_label_create(cancel_button);
    lv_label_set_text(cancel_label, "Cancel");
    lv_obj_set_style_text_color(cancel_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(cancel_label);
}

/****************************************************************************************
 * @brief Show a list of static choices and remember the caller's selection callback.
 ****************************************************************************************/
void ui_choice_dialog_show(ui_choice_dialog_t *dialog,
                           const char *title,
                           const char *const *choices,
                           uint8_t choice_count,
                           ui_choice_dialog_handler_t handler,
                           void *user_data)
{
    uint8_t choice_index;
    uint8_t visible_count;

    if (dialog == NULL || dialog->overlay == NULL || choices == NULL)
    {
        return;
    }

    visible_count = choice_count;
    if (visible_count > UI_CHOICE_DIALOG_MAX_CHOICES)
    {
        visible_count = UI_CHOICE_DIALOG_MAX_CHOICES;
    }

    dialog->choice_count = visible_count;
    dialog->handler = handler;
    dialog->user_data = user_data;
    lv_label_set_text(dialog->title_label, title != NULL ? title : "");

    for (choice_index = 0U; choice_index < UI_CHOICE_DIALOG_MAX_CHOICES; ++choice_index)
    {
        if (choice_index < visible_count)
        {
            lv_label_set_text(dialog->choice_labels[choice_index], choices[choice_index]);
            lv_obj_remove_flag(dialog->choice_buttons[choice_index], LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            lv_obj_add_flag(dialog->choice_buttons[choice_index], LV_OBJ_FLAG_HIDDEN);
        }
    }

    lv_obj_remove_flag(dialog->overlay, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(dialog->overlay);
}

/****************************************************************************************
 * @brief Hide the dialog without changing its last choice labels.
 ****************************************************************************************/
void ui_choice_dialog_hide(ui_choice_dialog_t *dialog)
{
    if (dialog == NULL || dialog->overlay == NULL)
    {
        return;
    }

    lv_obj_add_flag(dialog->overlay, LV_OBJ_FLAG_HIDDEN);
}
