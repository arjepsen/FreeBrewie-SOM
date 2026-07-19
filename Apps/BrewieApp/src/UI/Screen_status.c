#include "Screen_status.h"

#include <stdio.h>
#include <string.h>

#include "Platform/Logging.h"
#include "UI_scroll.h"

/*
 * The status screen is a bring-up/debug screen, not the final brewing UI. It is still
 * important that it uses the same portrait shape as the finished appliance interface,
 * because that keeps early development honest about the real amount of screen space.
 */
#define SCREEN_STATUS_LABEL_WIDTH 82
#define SCREEN_STATUS_ROW_GAP 4
#define SCREEN_STATUS_PANEL_PAD 8

static lv_obj_t *screen_status_create_row(lv_obj_t *parent, const char *label_text, lv_obj_t **value_out);
static lv_obj_t *screen_status_create_home_button(lv_obj_t *parent, screen_status_t *status);
static lv_obj_t *screen_status_create_touch_button(lv_obj_t *parent, screen_status_t *status);
static void screen_status_set_label_text(lv_obj_t *label, const char *text);
static void screen_status_home_event_cb(lv_event_t *event);
static void screen_status_touch_event_cb(lv_event_t *event);
static void screen_status_button_event_cb(lv_event_t *event);

static lv_obj_t *screen_status_create_row(lv_obj_t *parent, const char *label_text, lv_obj_t **value_out)
{
    lv_obj_t *row;
    lv_obj_t *label;
    lv_obj_t *value;

    row = lv_obj_create(parent);
    lv_obj_remove_style_all(row);
    lv_obj_set_width(row, lv_pct(100));
    lv_obj_set_height(row, LV_SIZE_CONTENT);
    lv_obj_set_layout(row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_top(row, 1, 0);
    lv_obj_set_style_pad_bottom(row, 1, 0);
    lv_obj_set_style_pad_column(row, SCREEN_STATUS_ROW_GAP, 0);

    label = lv_label_create(row);
    lv_label_set_text(label, label_text);
    lv_obj_set_width(label, SCREEN_STATUS_LABEL_WIDTH);
    lv_obj_set_style_text_color(label, lv_color_hex(0xA8C7FF), 0);

    value = lv_label_create(row);
    lv_label_set_text(value, "-");
    /*
     * Values can become longer than the portrait screen is wide, especially while the
     * protocol and fault reporting are still changing. Wrapping keeps the debug screen
     * readable instead of silently clipping the useful part of a message.
     */
    lv_label_set_long_mode(value, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(value, 0);
    lv_obj_set_flex_grow(value, 1);
    lv_obj_set_style_text_color(value, lv_color_hex(0xFFFFFF), 0);

    *value_out = value;
    return row;
}

/****************************************************************************************
 * @brief Update an LVGL label only when the visible text actually changed.
 *
 * LVGL marks objects dirty when their text is set. Calling lv_label_set_text() repeatedly
 * with the same text can therefore cause avoidable layout/render work. This helper keeps
 * the status screen cheap while it remains a diagnostic view made mostly of labels.
 ****************************************************************************************/
static void screen_status_set_label_text(lv_obj_t *label, const char *text)
{
    const char *current_text;

    if (label == NULL || text == NULL)
    {
        return;
    }

    current_text = lv_label_get_text(label);
    if (current_text == NULL || strcmp(current_text, text) != 0)
    {
        lv_label_set_text(label, text);
    }
}

static lv_obj_t *screen_status_create_home_button(lv_obj_t *parent, screen_status_t *status)
{
    lv_obj_t *button;
    lv_obj_t *label;

    button = lv_button_create(parent);
    lv_obj_set_width(button, lv_pct(100));
    lv_obj_set_height(button, 34);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x393939), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x4A4A4A), LV_STATE_PRESSED);
    lv_obj_set_style_radius(button, 4, 0);
    lv_obj_set_style_pad_all(button, 0, 0);
    lv_obj_add_event_cb(button, screen_status_home_event_cb, LV_EVENT_CLICKED, status);

    label = lv_label_create(button);
    lv_label_set_text(label, "Home");
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);

    return button;
}

static lv_obj_t *screen_status_create_touch_button(lv_obj_t *parent, screen_status_t *status)
{
    lv_obj_t *button;
    lv_obj_t *label;

    button = lv_button_create(parent);
    lv_obj_set_width(button, lv_pct(100));
    lv_obj_set_height(button, 38);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x1B7D5A), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x249B70), LV_STATE_PRESSED);
    lv_obj_set_style_radius(button, 4, 0);
    lv_obj_set_style_pad_all(button, 0, 0);
    lv_obj_add_event_cb(button, screen_status_button_event_cb, LV_EVENT_CLICKED, status);

    label = lv_label_create(button);
    lv_label_set_text(label, "Touch OK");
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);

    return button;
}

static void screen_status_home_event_cb(lv_event_t *event)
{
    screen_status_t *status;

    status = lv_event_get_user_data(event);
    if (status == NULL || status->action_handler == NULL)
    {
        return;
    }

    status->action_handler(UI_ACTION_SHOW_HOME, 0U, status->action_user_data);
}

static void screen_status_touch_event_cb(lv_event_t *event)
{
    screen_status_t *status;
    lv_indev_t *indev;
    lv_point_t point;
    char text[48];

    status = lv_event_get_user_data(event);
    if (status == NULL || status->touch_value == NULL)
    {
        return;
    }

    indev = lv_indev_active();
    if (indev == NULL)
    {
        return;
    }

    lv_indev_get_point(indev, &point);
    status->touch_event_count++;

    /*
     * This is a temporary hardware bring-up indicator. It proves that the Linux input
     * device reaches LVGL and that the portrait coordinate mapping is plausible before
     * the real UI depends on touch buttons for machine control.
     */
    if (lv_event_get_code(event) == LV_EVENT_RELEASED)
    {
        snprintf(text,
                 sizeof(text),
                 "up %ld,%ld #%lu",
                 (long)point.x,
                 (long)point.y,
                 (unsigned long)status->touch_event_count);
    }
    else
    {
        snprintf(text,
                 sizeof(text),
                 "down %ld,%ld #%lu",
                 (long)point.x,
                 (long)point.y,
                 (unsigned long)status->touch_event_count);
    }

    lv_label_set_text(status->touch_value, text);
}

static void screen_status_button_event_cb(lv_event_t *event)
{
    screen_status_t *status;
    char text[40];

    status = lv_event_get_user_data(event);
    if (status == NULL || status->button_value == NULL)
    {
        return;
    }

    status->button_click_count++;

    /*
     * This proves the next layer after raw touch: LVGL hit-testing and button click events.
     * Keep it harmless and visible while touch is being brought up, then replace it with a
     * real navigation or service-mode action once we trust the input path.
     */
    snprintf(text,
             sizeof(text),
             "clicked #%lu",
             (unsigned long)status->button_click_count);
    lv_label_set_text(status->button_value, text);
}

void screen_status_init(screen_status_t *status, ui_action_handler_t action_handler, void *user_data)
{
    lv_obj_t *container;

    if (status == NULL)
    {
        return;
    }

    memset(status, 0, sizeof(*status));
    status->action_handler = action_handler;
    status->action_user_data = user_data;

    status->screen = lv_obj_create(NULL);

    lv_obj_set_style_bg_color(status->screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(status->screen, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(status->screen, lv_color_hex(0xFFFFFF), 0);

    container = lv_obj_create(status->screen);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_center(container);
    lv_obj_set_style_bg_color(container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, SCREEN_STATUS_PANEL_PAD, 0);
    lv_obj_set_style_pad_row(container, 3, 0);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    /*
     * The production UI should avoid requiring scrolling for normal workflows, but this
     * screen is intentionally a dense live diagnostic view. Allowing vertical scrolling is
     * better than hiding fields when a temporary debug value becomes too long.
     */
    lv_obj_set_scroll_dir(container, LV_DIR_VER);
    ui_scroll_apply_gutter(container);
    lv_obj_add_flag(container, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(container, screen_status_touch_event_cb, LV_EVENT_PRESSED, status);
    lv_obj_add_event_cb(container, screen_status_touch_event_cb, LV_EVENT_RELEASED, status);

    status->title_label = lv_label_create(container);
    lv_label_set_text(status->title_label, "STATUS");
    lv_obj_set_style_text_font(status->title_label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(status->title_label, lv_color_hex(0xFFFFFF), 0);

    screen_status_create_home_button(container, status);
    screen_status_create_touch_button(container, status);
    screen_status_create_row(container, "button", &status->button_value);
    lv_label_set_text(status->button_value, "not clicked");
    screen_status_create_row(container, "display", &status->display_value);
    screen_status_create_row(container, "serial", &status->serial_value);
    screen_status_create_row(container, "heartbeat", &status->heartbeat_value);
    screen_status_create_row(container, "hb sent", &status->hb_counter_value);
    screen_status_create_row(container, "last rx", &status->last_rx_value);
    screen_status_create_row(container, "link", &status->link_value);
    screen_status_create_row(container, "mcu", &status->mcu_status_value);
    screen_status_create_row(container, "pressure", &status->pressure_value);
    screen_status_create_row(container, "pumps", &status->pump_value);
    screen_status_create_row(container, "inlets", &status->solenoid_value);
    screen_status_create_row(container, "faults", &status->fault_value);
    screen_status_create_row(container, "touch", &status->touch_value);
    lv_label_set_text(status->touch_value, "tap screen");

    log_info("screen_status: built");
}

void screen_status_update(screen_status_t *status, const status_screen_view_model_t *view_model)
{
    if (status == NULL || view_model == NULL)
    {
        return;
    }

    screen_status_set_label_text(status->display_value, view_model->display_text);
    screen_status_set_label_text(status->serial_value, view_model->serial_text);
    screen_status_set_label_text(status->heartbeat_value, view_model->heartbeat_text);
    screen_status_set_label_text(status->last_rx_value, view_model->last_rx_text);
    screen_status_set_label_text(status->link_value, view_model->link_text);
    screen_status_set_label_text(status->mcu_status_value, view_model->mcu_status_text);
    screen_status_set_label_text(status->pressure_value, view_model->pressure_text);
    screen_status_set_label_text(status->pump_value, view_model->pump_text);
    screen_status_set_label_text(status->solenoid_value, view_model->solenoid_text);
    screen_status_set_label_text(status->fault_value, view_model->fault_text);

    /*
     * The heartbeat count normally changes once per second, while screen_status_update()
     * is called several times per second. Format this number only when it actually changed.
     */
    if (status->shown_heartbeat_count != view_model->heartbeat_count ||
        status->heartbeat_count_text[0] == '\0')
    {
        status->shown_heartbeat_count = view_model->heartbeat_count;
        snprintf(status->heartbeat_count_text,
                 sizeof(status->heartbeat_count_text),
                 "%lu",
                 (unsigned long)view_model->heartbeat_count);
        screen_status_set_label_text(status->hb_counter_value, status->heartbeat_count_text);
    }
}
