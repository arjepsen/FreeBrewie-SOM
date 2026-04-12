#include "UI.h"

#include <stdio.h>
#include <string.h>

static lv_obj_t *ui_create_row(lv_obj_t *parent, const char *label_text, lv_obj_t **value_out)
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
    lv_obj_set_style_pad_top(row, 2, 0);
    lv_obj_set_style_pad_bottom(row, 2, 0);

    label = lv_label_create(row);
    lv_label_set_text(label, label_text);
    lv_obj_set_width(label, 100);

    value = lv_label_create(row);
    lv_label_set_text(value, "-");
    lv_obj_set_flex_grow(value, 1);

    *value_out = value;
    return row;
}

void ui_init(ui_state_t *ui)
{
    lv_obj_t *container;

    if(ui == NULL) {
        return;
    }

    memset(ui, 0, sizeof(*ui));

    ui->screen = lv_screen_active();
    lv_obj_set_style_bg_color(ui->screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_color(ui->screen, lv_color_hex(0xFFFFFF), 0);

    container = lv_obj_create(ui->screen);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_center(container);
    lv_obj_set_style_bg_color(container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 10, 0);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);

    ui->title_label = lv_label_create(container);
    lv_label_set_text(ui->title_label, "FreeBrewie SOM bring-up");
    lv_obj_set_style_text_font(ui->title_label, &lv_font_montserrat_14, 0);

    ui_create_row(container, "display", &ui->display_value);
    ui_create_row(container, "serial", &ui->serial_value);
    ui_create_row(container, "heartbeat", &ui->heartbeat_value);
    ui_create_row(container, "hb sent", &ui->hb_counter_value);
    ui_create_row(container, "last rx", &ui->last_rx_value);
    ui_create_row(container, "link", &ui->link_value);
}

void ui_set_status(ui_state_t *ui, const char *key, const char *value)
{
    if(ui == NULL || key == NULL || value == NULL) {
        return;
    }

    if(strcmp(key, "display") == 0) {
        lv_label_set_text(ui->display_value, value);
    }
    else if(strcmp(key, "serial") == 0) {
        lv_label_set_text(ui->serial_value, value);
    }
    else if(strcmp(key, "heartbeat") == 0) {
        lv_label_set_text(ui->heartbeat_value, value);
    }
    else if(strcmp(key, "last rx") == 0) {
        lv_label_set_text(ui->last_rx_value, value);
    }
    else if(strcmp(key, "link") == 0) {
        lv_label_set_text(ui->link_value, value);
    }
}

void ui_set_counter(ui_state_t *ui, const char *key, uint32_t value)
{
    char text[32];

    if(ui == NULL || key == NULL) {
        return;
    }

    snprintf(text, sizeof(text), "%lu", (unsigned long)value);

    if(strcmp(key, "hb sent") == 0) {
        lv_label_set_text(ui->hb_counter_value, text);
    }
}
