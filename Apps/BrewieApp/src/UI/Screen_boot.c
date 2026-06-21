#include "Screen_boot.h"

#include <stdio.h>
#include <string.h>

static lv_obj_t *screen_boot_create_row(lv_obj_t *parent, const char *label_text, lv_obj_t **value_out)
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

void screen_boot_init(screen_boot_t *boot)
{
    lv_obj_t *container;

    if (boot == NULL)
    {
        return;
    }

    memset(boot, 0, sizeof(*boot));

    boot->screen = lv_screen_active();
    lv_obj_set_style_bg_color(boot->screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_color(boot->screen, lv_color_hex(0xFFFFFF), 0);

    container = lv_obj_create(boot->screen);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_center(container);
    lv_obj_set_style_bg_color(container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 10, 0);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);

    boot->title_label = lv_label_create(container);
    lv_label_set_text(boot->title_label, "FreeBrewie SOM bring-up");
    lv_obj_set_style_text_font(boot->title_label, &lv_font_montserrat_14, 0);

    screen_boot_create_row(container, "display", &boot->display_value);
    screen_boot_create_row(container, "serial", &boot->serial_value);
    screen_boot_create_row(container, "heartbeat", &boot->heartbeat_value);
    screen_boot_create_row(container, "hb sent", &boot->hb_counter_value);
    screen_boot_create_row(container, "last rx", &boot->last_rx_value);
    screen_boot_create_row(container, "link", &boot->link_value);
}

void screen_boot_update(screen_boot_t *boot, const boot_screen_view_model_t *view_model)
{
    char text[32];

    if (boot == NULL || view_model == NULL)
    {
        return;
    }

    lv_label_set_text(boot->display_value, view_model->display_text);
    lv_label_set_text(boot->serial_value, view_model->serial_text);
    lv_label_set_text(boot->heartbeat_value, view_model->heartbeat_text);
    lv_label_set_text(boot->last_rx_value, view_model->last_rx_text);
    lv_label_set_text(boot->link_value, view_model->link_text);

    snprintf(text, sizeof(text), "%lu", (unsigned long)view_model->heartbeat_count);
    lv_label_set_text(boot->hb_counter_value, text);
}
