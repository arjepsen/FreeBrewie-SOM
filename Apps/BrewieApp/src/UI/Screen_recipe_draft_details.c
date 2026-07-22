#include "Screen_recipe_draft_details.h"

#include <stdio.h>
#include <string.h>

#include "UI_scroll.h"

static void screen_recipe_draft_details_set_static(lv_obj_t *object);
static lv_obj_t *screen_recipe_draft_details_create_header(lv_obj_t *parent,
                                                           screen_recipe_draft_details_t *details);
static lv_obj_t *screen_recipe_draft_details_create_nav_button(
    lv_obj_t *parent,
    const char *text,
    lv_align_t align,
    screen_recipe_draft_details_nav_context_t *context);
static lv_obj_t *screen_recipe_draft_details_create_panel(lv_obj_t *parent, const char *caption);
static lv_obj_t *screen_recipe_draft_details_create_value_row(lv_obj_t *parent,
                                                              const char *label_text,
                                                              const char *value_text,
                                                              lv_obj_t **value_label);
static lv_obj_t *screen_recipe_draft_details_create_modify_button(lv_obj_t *parent,
                                                                  screen_recipe_draft_details_t *details);
static void screen_recipe_draft_details_create_style_picker(screen_recipe_draft_details_t *details);
static lv_obj_t *screen_recipe_draft_details_create_style_option_button(
    lv_obj_t *parent,
    screen_recipe_draft_details_style_context_t *context);
static void screen_recipe_draft_details_hide_style_picker(screen_recipe_draft_details_t *details);
static void screen_recipe_draft_details_show_style_picker(screen_recipe_draft_details_t *details);
static void screen_recipe_draft_details_format_percent(char *buffer,
                                                       size_t buffer_size,
                                                       uint16_t value);
static void screen_recipe_draft_details_format_liters(char *buffer,
                                                      size_t buffer_size,
                                                      uint16_t value_dl);
static void screen_recipe_draft_details_format_abv(char *buffer,
                                                   size_t buffer_size,
                                                   uint16_t value_tenths);
static void screen_recipe_draft_details_format_whole(char *buffer,
                                                     size_t buffer_size,
                                                     uint16_t value);
static void screen_recipe_draft_details_format_gravity(char *buffer,
                                                       size_t buffer_size,
                                                       uint16_t value_points);
static void screen_recipe_draft_details_set_text_if_changed(lv_obj_t *label, const char *text);
static void screen_recipe_draft_details_nav_event_cb(lv_event_t *event);
static void screen_recipe_draft_details_batch_size_event_cb(lv_event_t *event);
static void screen_recipe_draft_details_batch_size_commit_cb(uint16_t value, void *user_data);
static void screen_recipe_draft_details_modify_event_cb(lv_event_t *event);
static void screen_recipe_draft_details_style_option_event_cb(lv_event_t *event);

/****************************************************************************************
 * @brief Make an object static so it cannot become a tiny scroll target.
 ****************************************************************************************/
static void screen_recipe_draft_details_set_static(lv_obj_t *object)
{
    if (object == NULL)
    {
        return;
    }

    lv_obj_clear_flag(object, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_OFF);
}

static lv_obj_t *screen_recipe_draft_details_create_header(lv_obj_t *parent,
                                                           screen_recipe_draft_details_t *details)
{
    lv_obj_t *header;
    lv_obj_t *title;

    header = lv_obj_create(parent);
    screen_recipe_draft_details_set_static(header);
    lv_obj_set_width(header, lv_pct(100));
    lv_obj_set_height(header, 64);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_all(header, 0, 0);

    screen_recipe_draft_details_create_nav_button(header,
                                                  "<",
                                                  LV_ALIGN_LEFT_MID,
                                                  &details->back_button_context);

    title = lv_label_create(header);
    lv_label_set_text(title, "DETAILS");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(title);
    return header;
}

static lv_obj_t *screen_recipe_draft_details_create_nav_button(
    lv_obj_t *parent,
    const char *text,
    lv_align_t align,
    screen_recipe_draft_details_nav_context_t *context)
{
    lv_obj_t *button;
    lv_obj_t *label;

    button = lv_button_create(parent);
    lv_obj_set_size(button, 42, 42);
    lv_obj_align(button, align, (align == LV_ALIGN_LEFT_MID) ? 2 : -2, 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x343434), LV_STATE_PRESSED);
    lv_obj_set_style_border_width(button, 0, 0);
    lv_obj_set_style_radius(button, 4, 0);
    lv_obj_add_event_cb(button, screen_recipe_draft_details_nav_event_cb, LV_EVENT_CLICKED, context);

    label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);
    return button;
}

/****************************************************************************************
 * @brief Create one old-style details sub-panel with an orange caption.
 ****************************************************************************************/
static lv_obj_t *screen_recipe_draft_details_create_panel(lv_obj_t *parent, const char *caption)
{
    lv_obj_t *panel;
    lv_obj_t *caption_label;

    panel = lv_obj_create(parent);
    lv_obj_set_width(panel, lv_pct(97));
    lv_obj_set_height(panel, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x292929), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(0x3F3F3F), 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_radius(panel, 0, 0);
    lv_obj_set_style_pad_all(panel, 8, 0);
    lv_obj_set_style_pad_row(panel, 7, 0);
    lv_obj_set_layout(panel, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);

    caption_label = lv_label_create(panel);
    lv_label_set_text(caption_label, caption);
    lv_obj_set_width(caption_label, lv_pct(100));
    lv_obj_set_style_text_color(caption_label, lv_color_hex(0xE67526), 0);
    return panel;
}

/****************************************************************************************
 * @brief Create one compact label/value row inside a details panel.
 ****************************************************************************************/
static lv_obj_t *screen_recipe_draft_details_create_value_row(lv_obj_t *parent,
                                                              const char *label_text,
                                                              const char *value_text,
                                                              lv_obj_t **value_label)
{
    lv_obj_t *row;
    lv_obj_t *label;
    lv_obj_t *value;

    row = lv_obj_create(parent);
    screen_recipe_draft_details_set_static(row);
    lv_obj_set_width(row, lv_pct(100));
    lv_obj_set_height(row, 24);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);

    label = lv_label_create(row);
    lv_label_set_text(label, label_text);
    lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(label, lv_pct(48));
    lv_obj_set_style_text_color(label, lv_color_hex(0xC8C8C8), 0);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);

    *value_label = lv_label_create(row);
    lv_label_set_text(*value_label, value_text);
    lv_label_set_long_mode(*value_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(*value_label, lv_pct(48));
    lv_obj_set_style_text_color(*value_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(*value_label, LV_ALIGN_RIGHT_MID, 0, 0);

    lv_obj_remove_flag(label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(*value_label, LV_OBJ_FLAG_CLICKABLE);
    return row;
}

static lv_obj_t *screen_recipe_draft_details_create_modify_button(lv_obj_t *parent,
                                                                  screen_recipe_draft_details_t *details)
{
    lv_obj_t *button;
    lv_obj_t *label;

    button = lv_button_create(parent);
    lv_obj_set_width(button, lv_pct(100));
    lv_obj_set_height(button, 44);
    lv_obj_set_style_bg_color(button, lv_color_hex(0xE67526), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0xC85F22), LV_STATE_PRESSED);
    lv_obj_set_style_radius(button, 5, 0);
    lv_obj_add_event_cb(button, screen_recipe_draft_details_modify_event_cb, LV_EVENT_CLICKED, details);

    label = lv_label_create(button);
    lv_label_set_text(label, "SELECT STYLE");
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);
    return button;
}

/****************************************************************************************
 * @brief Create the hidden style picker overlay.
 ****************************************************************************************/
static void screen_recipe_draft_details_create_style_picker(screen_recipe_draft_details_t *details)
{
    lv_obj_t *panel;
    lv_obj_t *title;
    lv_obj_t *list;
    lv_obj_t *close_button;
    lv_obj_t *close_label;
    uint8_t index;
    uint8_t style_count;

    details->style_picker_overlay = lv_obj_create(details->screen);
    screen_recipe_draft_details_set_static(details->style_picker_overlay);
    lv_obj_set_size(details->style_picker_overlay, lv_pct(100), lv_pct(100));
    lv_obj_center(details->style_picker_overlay);
    lv_obj_set_style_bg_color(details->style_picker_overlay, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(details->style_picker_overlay, LV_OPA_70, 0);
    lv_obj_set_style_border_width(details->style_picker_overlay, 0, 0);
    lv_obj_set_style_pad_all(details->style_picker_overlay, 10, 0);
    lv_obj_add_flag(details->style_picker_overlay, LV_OBJ_FLAG_HIDDEN);

    panel = lv_obj_create(details->style_picker_overlay);
    lv_obj_set_size(panel, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(0xE67526), 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_radius(panel, 5, 0);
    lv_obj_set_style_pad_all(panel, 8, 0);
    lv_obj_set_style_pad_row(panel, 8, 0);
    lv_obj_set_layout(panel, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);

    title = lv_label_create(panel);
    lv_label_set_text(title, "SELECT BEER STYLE");
    lv_obj_set_width(title, lv_pct(100));
    lv_obj_set_style_text_color(title, lv_color_hex(0xE67526), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);

    list = lv_obj_create(panel);
    lv_obj_set_width(list, lv_pct(100));
    lv_obj_set_flex_grow(list, 1);
    lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(list, 0, 0);
    lv_obj_set_style_pad_all(list, 0, 0);
    lv_obj_set_style_pad_row(list, 7, 0);
    ui_scroll_apply_gutter(list);
    lv_obj_set_layout(list, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);

    style_catalog_init();
    style_count = style_catalog_get_count();
    for (index = 0U; index < style_count && index < STYLE_CATALOG_MAX_STYLES; ++index)
    {
        details->style_option_contexts[index].details = details;
        details->style_option_contexts[index].option_index = index;
        screen_recipe_draft_details_create_style_option_button(list, &details->style_option_contexts[index]);
    }

    close_button = lv_button_create(panel);
    lv_obj_set_width(close_button, lv_pct(100));
    lv_obj_set_height(close_button, 42);
    lv_obj_set_style_bg_color(close_button, lv_color_hex(0x4B4741), 0);
    lv_obj_set_style_bg_color(close_button, lv_color_hex(0x343434), LV_STATE_PRESSED);
    lv_obj_set_style_radius(close_button, 5, 0);
    lv_obj_add_event_cb(close_button, screen_recipe_draft_details_modify_event_cb, LV_EVENT_CLICKED, details);

    close_label = lv_label_create(close_button);
    lv_label_set_text(close_label, "BACK");
    lv_obj_set_style_text_color(close_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(close_label);
}

/****************************************************************************************
 * @brief Create one selectable beer-style row in the picker.
 ****************************************************************************************/
static lv_obj_t *screen_recipe_draft_details_create_style_option_button(
    lv_obj_t *parent,
    screen_recipe_draft_details_style_context_t *context)
{
    const style_catalog_style_t *option;
    lv_obj_t *button;
    lv_obj_t *name_label;
    lv_obj_t *body_label;

    option = style_catalog_get_style(context->option_index);
    if (option == NULL)
    {
        return NULL;
    }

    button = lv_button_create(parent);
    lv_obj_set_width(button, lv_pct(97));
    lv_obj_set_height(button, 60);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x292929), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x3B332D), LV_STATE_PRESSED);
    lv_obj_set_style_border_color(button, lv_color_hex(0x3F3F3F), 0);
    lv_obj_set_style_border_width(button, 1, 0);
    lv_obj_set_style_radius(button, 0, 0);
    lv_obj_set_style_pad_all(button, 7, 0);
    lv_obj_set_layout(button, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(button, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(button, 3, 0);
    lv_obj_add_event_cb(button, screen_recipe_draft_details_style_option_event_cb, LV_EVENT_CLICKED, context);

    name_label = lv_label_create(button);
    lv_label_set_text(name_label, option->style_name);
    lv_label_set_long_mode(name_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(name_label, lv_pct(100));
    lv_obj_set_style_text_color(name_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_remove_flag(name_label, LV_OBJ_FLAG_CLICKABLE);

    body_label = lv_label_create(button);
    lv_label_set_text_fmt(body_label,
                          "%s - %s",
                          option->style_number,
                          option->style_category);
    lv_label_set_long_mode(body_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(body_label, lv_pct(100));
    lv_obj_set_style_text_color(body_label, lv_color_hex(0xE67526), 0);
    lv_obj_remove_flag(body_label, LV_OBJ_FLAG_CLICKABLE);
    return button;
}

static void screen_recipe_draft_details_hide_style_picker(screen_recipe_draft_details_t *details)
{
    if (details == NULL || details->style_picker_overlay == NULL)
    {
        return;
    }

    lv_obj_add_flag(details->style_picker_overlay, LV_OBJ_FLAG_HIDDEN);
}

static void screen_recipe_draft_details_show_style_picker(screen_recipe_draft_details_t *details)
{
    if (details == NULL || details->style_picker_overlay == NULL)
    {
        return;
    }

    lv_obj_remove_flag(details->style_picker_overlay, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(details->style_picker_overlay);
}

/****************************************************************************************
 * @brief Format a whole percent, using -- when the model still has a placeholder value.
 ****************************************************************************************/
static void screen_recipe_draft_details_format_percent(char *buffer,
                                                       size_t buffer_size,
                                                       uint16_t value)
{
    if (value == 0U)
    {
        snprintf(buffer, buffer_size, "--");
        return;
    }

    snprintf(buffer, buffer_size, "%u%%", (unsigned int)value);
}

/****************************************************************************************
 * @brief Format deciliters as liters with one decimal place.
 ****************************************************************************************/
static void screen_recipe_draft_details_format_liters(char *buffer,
                                                      size_t buffer_size,
                                                      uint16_t value_dl)
{
    if (value_dl == 0U)
    {
        snprintf(buffer, buffer_size, "--");
        return;
    }

    snprintf(buffer,
             buffer_size,
             "%u.%u L",
             (unsigned int)(value_dl / 10U),
             (unsigned int)(value_dl % 10U));
}

/****************************************************************************************
 * @brief Format tenths of a percent as an ABV value.
 ****************************************************************************************/
static void screen_recipe_draft_details_format_abv(char *buffer,
                                                   size_t buffer_size,
                                                   uint16_t value_tenths)
{
    if (value_tenths == 0U)
    {
        snprintf(buffer, buffer_size, "--");
        return;
    }

    snprintf(buffer,
             buffer_size,
             "%u.%u%%",
             (unsigned int)(value_tenths / 10U),
             (unsigned int)(value_tenths % 10U));
}

/****************************************************************************************
 * @brief Format a whole-number calculated value.
 ****************************************************************************************/
static void screen_recipe_draft_details_format_whole(char *buffer,
                                                     size_t buffer_size,
                                                     uint16_t value)
{
    if (value == 0U)
    {
        snprintf(buffer, buffer_size, "--");
        return;
    }

    snprintf(buffer, buffer_size, "%u", (unsigned int)value);
}

/****************************************************************************************
 * @brief Format gravity points as 1.xxx.
 ****************************************************************************************/
static void screen_recipe_draft_details_format_gravity(char *buffer,
                                                       size_t buffer_size,
                                                       uint16_t value_points)
{
    if (value_points == 0U)
    {
        snprintf(buffer, buffer_size, "--");
        return;
    }

    snprintf(buffer, buffer_size, "%u.%03u", (unsigned int)(value_points / 1000U),
             (unsigned int)(value_points % 1000U));
}

/****************************************************************************************
 * @brief Avoid writing unchanged label text into LVGL.
 ****************************************************************************************/
static void screen_recipe_draft_details_set_text_if_changed(lv_obj_t *label, const char *text)
{
    if (label == NULL || text == NULL || strcmp(lv_label_get_text(label), text) == 0)
    {
        return;
    }

    lv_label_set_text(label, text);
}

static void screen_recipe_draft_details_nav_event_cb(lv_event_t *event)
{
    screen_recipe_draft_details_nav_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL || context->handler == NULL)
    {
        return;
    }

    context->handler(context->action, context->value, context->user_data);
}

static void screen_recipe_draft_details_batch_size_event_cb(lv_event_t *event)
{
    screen_recipe_draft_details_t *details;
    uint16_t current_value;

    details = lv_event_get_user_data(event);
    if (details == NULL || details->draft == NULL)
    {
        return;
    }

    current_value = details->draft->calculated.batch_size_dl;
    if (current_value == 0U)
    {
        current_value = 200U;
    }

    ui_number_editor_show(&details->batch_size_editor,
                          "BATCH SIZE",
                          "L",
                          current_value,
                          50U,
                          300U,
                          5U,
                          true,
                          screen_recipe_draft_details_batch_size_commit_cb,
                          details);
}

static void screen_recipe_draft_details_batch_size_commit_cb(uint16_t value, void *user_data)
{
    screen_recipe_draft_details_t *details;

    details = user_data;
    if (details == NULL || details->draft == NULL)
    {
        return;
    }

    recipe_draft_set_batch_size_dl(details->draft, value);
    screen_recipe_draft_details_show(details, details->draft);
}

static void screen_recipe_draft_details_modify_event_cb(lv_event_t *event)
{
    screen_recipe_draft_details_t *details;

    details = lv_event_get_user_data(event);
    if (details == NULL || details->style_picker_overlay == NULL)
    {
        return;
    }

    if (lv_obj_has_flag(details->style_picker_overlay, LV_OBJ_FLAG_HIDDEN))
    {
        screen_recipe_draft_details_show_style_picker(details);
    }
    else
    {
        screen_recipe_draft_details_hide_style_picker(details);
    }
}

static void screen_recipe_draft_details_style_option_event_cb(lv_event_t *event)
{
    screen_recipe_draft_details_style_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL || context->details == NULL)
    {
        return;
    }

    recipe_draft_set_style(context->details->draft, style_catalog_get_style(context->option_index));
    screen_recipe_draft_details_show(context->details, context->details->draft);
    screen_recipe_draft_details_hide_style_picker(context->details);
}

void screen_recipe_draft_details_init(screen_recipe_draft_details_t *details,
                                      recipe_draft_t *draft,
                                      ui_action_handler_t action_handler,
                                      void *user_data)
{
    lv_obj_t *container;
    lv_obj_t *body;
    lv_obj_t *style_panel;
    lv_obj_t *calculated_panel;

    if (details == NULL)
    {
        return;
    }

    memset(details, 0, sizeof(*details));
    details->draft = draft;
    details->back_button_context.action = UI_ACTION_SHOW_RECIPE_DRAFT_MENU;
    details->back_button_context.handler = action_handler;
    details->back_button_context.user_data = user_data;
    details->screen = lv_obj_create(NULL);
    screen_recipe_draft_details_set_static(details->screen);
    lv_obj_set_style_bg_color(details->screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(details->screen, LV_OPA_COVER, 0);

    container = lv_obj_create(details->screen);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container, lv_color_hex(0x141414), 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 8, 0);
    lv_obj_set_style_pad_row(container, 8, 0);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);

    screen_recipe_draft_details_create_header(container, details);

    details->name_label = lv_label_create(container);
    lv_label_set_text(details->name_label, "--");
    lv_label_set_long_mode(details->name_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(details->name_label, lv_pct(100));
    lv_obj_set_style_text_color(details->name_label, lv_color_hex(0xE67526), 0);

    body = lv_obj_create(container);
    lv_obj_set_width(body, lv_pct(100));
    lv_obj_set_flex_grow(body, 1);
    lv_obj_set_style_bg_opa(body, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(body, 0, 0);
    lv_obj_set_style_pad_all(body, 0, 0);
    lv_obj_set_style_pad_row(body, 8, 0);
    ui_scroll_apply_gutter(body);
    lv_obj_set_layout(body, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(body, LV_FLEX_FLOW_COLUMN);

    style_panel = screen_recipe_draft_details_create_panel(body, "BEER STYLE");
    screen_recipe_draft_details_create_value_row(style_panel,
                                                 "Style",
                                                 "--",
                                                 &details->style_name_label);
    screen_recipe_draft_details_create_value_row(style_panel,
                                                 "BJCP number",
                                                 "--",
                                                 &details->style_number_label);
    screen_recipe_draft_details_create_value_row(style_panel,
                                                 "Category",
                                                 "--",
                                                 &details->style_category_label);
    screen_recipe_draft_details_create_value_row(style_panel,
                                                 "Type",
                                                 "--",
                                                 &details->style_type_label);

    calculated_panel = screen_recipe_draft_details_create_panel(body, "CALCULATED VALUES");
    screen_recipe_draft_details_create_value_row(calculated_panel,
                                                 "Efficiency",
                                                 "--",
                                                 &details->efficiency_label);
    details->batch_size_row = screen_recipe_draft_details_create_value_row(calculated_panel,
                                                                           "Batch size",
                                                                           "--",
                                                                           &details->batch_size_label);
    lv_obj_add_flag(details->batch_size_row, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(details->batch_size_row, lv_color_hex(0x3B332D), LV_STATE_PRESSED);
    lv_obj_add_event_cb(details->batch_size_row,
                        screen_recipe_draft_details_batch_size_event_cb,
                        LV_EVENT_CLICKED,
                        details);
    screen_recipe_draft_details_create_value_row(calculated_panel, "ABV", "--", &details->abv_label);
    screen_recipe_draft_details_create_value_row(calculated_panel, "SRM", "--", &details->srm_label);
    screen_recipe_draft_details_create_value_row(calculated_panel, "IBU", "--", &details->ibu_label);
    screen_recipe_draft_details_create_value_row(calculated_panel, "OG", "--", &details->og_label);
    screen_recipe_draft_details_create_value_row(calculated_panel, "FG", "--", &details->fg_label);

    screen_recipe_draft_details_create_modify_button(container, details);
    screen_recipe_draft_details_create_style_picker(details);
    ui_number_editor_init(&details->batch_size_editor, details->screen);
}

/****************************************************************************************
 * @brief Show RAM-only draft details without reading or saving a real recipe.
 ****************************************************************************************/
void screen_recipe_draft_details_show(screen_recipe_draft_details_t *details,
                                      const recipe_draft_t *draft)
{
    char value_text[16];
    const char *draft_name;

    if (details == NULL || draft == NULL)
    {
        return;
    }

    draft_name = recipe_draft_get_name(draft);
    if (details->shown_name != draft_name)
    {
        screen_recipe_draft_details_set_text_if_changed(details->name_label, draft_name);
        details->shown_name = draft_name;
    }

    screen_recipe_draft_details_set_text_if_changed(details->style_name_label, draft->style.style_name);
    screen_recipe_draft_details_set_text_if_changed(details->style_number_label, draft->style.style_number);
    screen_recipe_draft_details_set_text_if_changed(details->style_category_label, draft->style.style_category);
    screen_recipe_draft_details_set_text_if_changed(details->style_type_label, draft->style.style_type);

    screen_recipe_draft_details_format_percent(value_text,
                                               sizeof(value_text),
                                               draft->calculated.efficiency_percent);
    screen_recipe_draft_details_set_text_if_changed(details->efficiency_label, value_text);

    screen_recipe_draft_details_format_liters(value_text,
                                              sizeof(value_text),
                                              draft->calculated.batch_size_dl);
    screen_recipe_draft_details_set_text_if_changed(details->batch_size_label, value_text);

    screen_recipe_draft_details_format_abv(value_text,
                                           sizeof(value_text),
                                           draft->calculated.estimated_abv_tenths);
    screen_recipe_draft_details_set_text_if_changed(details->abv_label, value_text);

    screen_recipe_draft_details_format_whole(value_text,
                                             sizeof(value_text),
                                             draft->calculated.estimated_srm);
    screen_recipe_draft_details_set_text_if_changed(details->srm_label, value_text);

    screen_recipe_draft_details_format_whole(value_text,
                                             sizeof(value_text),
                                             draft->calculated.estimated_ibu);
    screen_recipe_draft_details_set_text_if_changed(details->ibu_label, value_text);

    screen_recipe_draft_details_format_gravity(value_text,
                                               sizeof(value_text),
                                               draft->calculated.estimated_og_points);
    screen_recipe_draft_details_set_text_if_changed(details->og_label, value_text);

    screen_recipe_draft_details_format_gravity(value_text,
                                               sizeof(value_text),
                                               draft->calculated.estimated_fg_points);
    screen_recipe_draft_details_set_text_if_changed(details->fg_label, value_text);
}
