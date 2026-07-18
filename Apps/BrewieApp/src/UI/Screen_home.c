#include "Screen_home.h"

#include <stdio.h>
#include <string.h>

#define SCREEN_HOME_PAD 8

static void screen_home_set_static(lv_obj_t *object);
static lv_obj_t *screen_home_create_header(lv_obj_t *parent, screen_home_t *home);
static lv_obj_t *screen_home_create_menu_button(lv_obj_t *parent, screen_home_t *home);
static lv_obj_t *screen_home_create_profile_row(lv_obj_t *parent);
static lv_obj_t *screen_home_create_machine_row(lv_obj_t *parent);
static lv_obj_t *screen_home_create_brew_button(lv_obj_t *parent, screen_home_t *home);
static void screen_home_button_event_cb(lv_event_t *event);

static void screen_home_set_static(lv_obj_t *object)
{
    if (object == NULL)
    {
        return;
    }

    lv_obj_clear_flag(object, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_OFF);
}

/**
 * Build the shared Home-style header.
 *
 * This intentionally leans toward the old Brewie header: dark charcoal bar, centered title,
 * and a small top-right menu button. The title is not mixed with machine state, which keeps
 * the screen calmer and avoids the earlier overlap around the Ready badge.
 */
static lv_obj_t *screen_home_create_header(lv_obj_t *parent, screen_home_t *home)
{
    lv_obj_t *header;
    lv_obj_t *title;

    header = lv_obj_create(parent);
    screen_home_set_static(header);
    lv_obj_set_width(header, lv_pct(100));
    lv_obj_set_height(header, 64);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_all(header, 0, 0);

    title = lv_label_create(header);
    lv_label_set_text(title, "Home");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xC8C8C8), 0);
    lv_obj_center(title);

    screen_home_create_menu_button(header, home);
    return header;
}

/**
 * Draw a small hamburger menu button without relying on icon fonts.
 *
 * The original UI used a FontAwesome hamburger glyph. Drawing three bars ourselves is more
 * predictable on the minimal SOM image and in the simulator.
 */
static lv_obj_t *screen_home_create_menu_button(lv_obj_t *parent, screen_home_t *home)
{
    lv_obj_t *button;
    lv_obj_t *line;
    int8_t offset_y;

    button = lv_button_create(parent);
    lv_obj_set_size(button, 42, 42);
    lv_obj_align(button, LV_ALIGN_RIGHT_MID, -2, 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x343434), LV_STATE_PRESSED);
    lv_obj_set_style_border_width(button, 0, 0);
    lv_obj_set_style_radius(button, 4, 0);
    lv_obj_set_style_pad_all(button, 0, 0);
    lv_obj_add_event_cb(button, screen_home_button_event_cb, LV_EVENT_CLICKED, &home->menu_button_context);

    for (offset_y = -7; offset_y <= 7; offset_y = (int8_t)(offset_y + 7))
    {
        line = lv_obj_create(button);
        screen_home_set_static(line);
        lv_obj_set_size(line, 20, 2);
        lv_obj_set_style_bg_color(line, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_bg_opa(line, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(line, 0, 0);
        lv_obj_align(line, LV_ALIGN_CENTER, 0, offset_y);
    }

    return button;
}

/**
 * Build the profile/status row inspired by the old Home `ProfileBox`.
 *
 * We do not have real user/profile data yet, so this row acts as a friendly appliance
 * status card. It keeps the old visual rhythm: dark avatar block, white headline, orange
 * secondary line.
 */
static lv_obj_t *screen_home_create_profile_row(lv_obj_t *parent)
{
    lv_obj_t *row;
    lv_obj_t *avatar;
    lv_obj_t *avatar_label;
    lv_obj_t *text_block;
    lv_obj_t *headline;
    lv_obj_t *subline;

    row = lv_obj_create(parent);
    screen_home_set_static(row);
    lv_obj_set_width(row, lv_pct(100));
    lv_obj_set_height(row, 58);
    lv_obj_set_style_bg_color(row, lv_color_hex(0x141414), 0);
    lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(row, lv_color_hex(0x2C2B2B), 0);
    lv_obj_set_style_border_width(row, 1, 0);
    lv_obj_set_style_radius(row, 0, 0);
    lv_obj_set_style_pad_all(row, 1, 0);
    lv_obj_set_style_pad_column(row, 8, 0);
    lv_obj_set_layout(row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    avatar = lv_obj_create(row);
    screen_home_set_static(avatar);
    lv_obj_set_size(avatar, 54, 54);
    lv_obj_set_style_bg_color(avatar, lv_color_hex(0x393939), 0);
    lv_obj_set_style_bg_opa(avatar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(avatar, 0, 0);
    lv_obj_set_style_radius(avatar, 0, 0);

    avatar_label = lv_label_create(avatar);
    lv_label_set_text(avatar_label, "FB");
    lv_obj_set_style_text_color(avatar_label, lv_color_hex(0xF47B32), 0);
    lv_obj_set_style_text_font(avatar_label, &lv_font_montserrat_20, 0);
    lv_obj_center(avatar_label);

    text_block = lv_obj_create(row);
    lv_obj_remove_style_all(text_block);
    screen_home_set_static(text_block);
    lv_obj_set_width(text_block, 170);
    lv_obj_set_height(text_block, LV_SIZE_CONTENT);
    lv_obj_set_layout(text_block, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(text_block, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(text_block, 4, 0);

    headline = lv_label_create(text_block);
    lv_label_set_text(headline, "FREEBREWIE");
    lv_obj_set_style_text_color(headline, lv_color_hex(0xFFFFFF), 0);

    subline = lv_label_create(text_block);
    lv_label_set_text(subline, "Ready for next step");
    lv_obj_set_style_text_color(subline, lv_color_hex(0xE67526), 0);

    return row;
}

/**
 * Build a compact machine summary row.
 *
 * This is the one deliberate addition beyond the old Home screen. It gives the rewrite a
 * useful appliance dashboard feel, but stays static and small so it is cheap for LVGL to
 * redraw and not fiddly to use.
 */
static lv_obj_t *screen_home_create_machine_row(lv_obj_t *parent)
{
    lv_obj_t *row;
    lv_obj_t *label;
    lv_obj_t *value;

    row = lv_obj_create(parent);
    screen_home_set_static(row);
    lv_obj_set_width(row, lv_pct(100));
    lv_obj_set_height(row, 74);
    lv_obj_set_style_bg_color(row, lv_color_hex(0x141414), 0);
    lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(row, lv_color_hex(0x2C2B2B), 0);
    lv_obj_set_style_border_width(row, 1, 0);
    lv_obj_set_style_radius(row, 0, 0);
    lv_obj_set_style_pad_all(row, 8, 0);
    lv_obj_set_layout(row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(row, 8, 0);

    label = lv_label_create(row);
    lv_label_set_text(label, "MACHINE");
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);

    value = lv_label_create(row);
    lv_label_set_text(value, "Mash --.- C        Boil --.- C");
    lv_obj_set_style_text_color(value, lv_color_hex(0xE67526), 0);

    return row;
}

static lv_obj_t *screen_home_create_brew_button(lv_obj_t *parent, screen_home_t *home)
{
    lv_obj_t *button;
    lv_obj_t *label;

    button = lv_button_create(parent);
    lv_obj_set_width(button, lv_pct(100));
    lv_obj_set_height(button, 48);
    lv_obj_set_style_bg_color(button, lv_color_hex(0xF47B32), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0xC85F22), LV_STATE_PRESSED);
    lv_obj_set_style_radius(button, 5, 0);
    lv_obj_add_event_cb(button, screen_home_button_event_cb, LV_EVENT_CLICKED, &home->brew_button_context);

    label = lv_label_create(button);
    lv_label_set_text(label, "LET'S BREW");
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);

    return button;
}

static void screen_home_button_event_cb(lv_event_t *event)
{
    screen_home_button_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL || context->handler == NULL)
    {
        return;
    }

    context->handler(context->action, context->value, context->user_data);
}

void screen_home_init(screen_home_t *home, ui_action_handler_t action_handler, void *user_data)
{
    lv_obj_t *container;
    lv_obj_t *spacer;

    if (home == NULL)
    {
        return;
    }

    memset(home, 0, sizeof(*home));
    home->menu_button_context.action = UI_ACTION_SHOW_MENU;
    home->menu_button_context.handler = action_handler;
    home->menu_button_context.user_data = user_data;
    home->brew_button_context.action = UI_ACTION_SHOW_RECIPES;
    home->brew_button_context.handler = action_handler;
    home->brew_button_context.user_data = user_data;

    home->screen = lv_obj_create(NULL);
    screen_home_set_static(home->screen);
    lv_obj_set_style_bg_color(home->screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(home->screen, LV_OPA_COVER, 0);

    container = lv_obj_create(home->screen);
    screen_home_set_static(container);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, SCREEN_HOME_PAD, 0);
    lv_obj_set_style_pad_row(container, 8, 0);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);

    screen_home_create_header(container, home);
    screen_home_create_profile_row(container);

    home->mcu_value = lv_label_create(container);
    lv_label_set_text(home->mcu_value, "MCU unknown");
    lv_obj_set_style_text_color(home->mcu_value, lv_color_hex(0x9B9B9B), 0);

    screen_home_create_machine_row(container);

    spacer = lv_obj_create(container);
    lv_obj_remove_style_all(spacer);
    screen_home_set_static(spacer);
    lv_obj_set_width(spacer, lv_pct(100));
    lv_obj_set_flex_grow(spacer, 1);

    screen_home_create_brew_button(container, home);
}

void screen_home_update(screen_home_t *home, const status_screen_view_model_t *view_model)
{
    if (home == NULL || view_model == NULL || home->mcu_value == NULL)
    {
        return;
    }

    if (home->shown_link_text != view_model->link_text)
    {
        snprintf(home->mcu_text, sizeof(home->mcu_text), "MCU %s", view_model->link_text);
        lv_label_set_text(home->mcu_value, home->mcu_text);
        home->shown_link_text = view_model->link_text;
    }
}
