#include "Screen_status.h"

#include <stdio.h>
#include <string.h>

#include "Platform/Logging.h"

/*
 * The status screen is a bring-up/debug screen, not the final brewing UI. It is still
 * important that it uses the same portrait shape as the finished appliance interface,
 * because that keeps early development honest about the real amount of screen space.
 */
#define SCREEN_STATUS_LABEL_WIDTH 82
#define SCREEN_STATUS_ROW_GAP 4
#define SCREEN_STATUS_PANEL_PAD 8

static lv_obj_t *screen_status_create_row(lv_obj_t *parent, const char *label_text, lv_obj_t **value_out);

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

void screen_status_init(screen_status_t *status)
{
    lv_obj_t *container;

    if (status == NULL)
    {
        return;
    }

    memset(status, 0, sizeof(*status));

    status->screen = lv_screen_active();

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
    lv_obj_set_scrollbar_mode(container, LV_SCROLLBAR_MODE_AUTO);

    status->title_label = lv_label_create(container);
    lv_label_set_text(status->title_label, "STATUS");
    lv_obj_set_style_text_font(status->title_label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(status->title_label, lv_color_hex(0xFFFFFF), 0);

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

    log_info("screen_status: built");
}

void screen_status_update(screen_status_t *status, const status_screen_view_model_t *view_model)
{
    char text[32];

    if (status == NULL || view_model == NULL)
    {
        return;
    }

    lv_label_set_text(status->display_value, view_model->display_text);
    lv_label_set_text(status->serial_value, view_model->serial_text);
    lv_label_set_text(status->heartbeat_value, view_model->heartbeat_text);
    lv_label_set_text(status->last_rx_value, view_model->last_rx_text);
    lv_label_set_text(status->link_value, view_model->link_text);
    lv_label_set_text(status->mcu_status_value, view_model->mcu_status_text);
    lv_label_set_text(status->pressure_value, view_model->pressure_text);
    lv_label_set_text(status->pump_value, view_model->pump_text);
    lv_label_set_text(status->solenoid_value, view_model->solenoid_text);
    lv_label_set_text(status->fault_value, view_model->fault_text);

    snprintf(text, sizeof(text), "%lu", (unsigned long)view_model->heartbeat_count);
    lv_label_set_text(status->hb_counter_value, text);
}
