#include "Screen_home.h"

#include <stdio.h>
#include <string.h>

#define SCREEN_HOME_PAD 8
#define SCREEN_HOME_GAP 8
#define SCREEN_HOME_TANK_WIDTH 116
#define SCREEN_HOME_BUTTON_WIDTH 112
#define SCREEN_HOME_ACTION_HEIGHT 48

static lv_obj_t *screen_home_create_panel(lv_obj_t *parent);
static lv_obj_t *screen_home_create_action_button(lv_obj_t *parent,
                                                  const char *text,
                                                  bool primary,
                                                  bool enabled,
                                                  screen_home_button_context_t *context);
static lv_obj_t *screen_home_create_tank_panel(lv_obj_t *parent, const char *name, lv_obj_t **temp_value);
static void screen_home_button_event_cb(lv_event_t *event);

/**
 * Build a plain rectangular panel.
 *
 * LVGL widgets are cheap when the layout is stable. These panels mirror the HTML design
 * spec and give us large, predictable regions that should redraw only when their labels
 * actually change.
 */
static lv_obj_t *screen_home_create_panel(lv_obj_t *parent)
{
    lv_obj_t *panel;

    panel = lv_obj_create(parent);
    lv_obj_set_width(panel, lv_pct(100));
    lv_obj_set_height(panel, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x141414), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(0x3F3F3F), 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_radius(panel, 4, 0);
    lv_obj_set_style_pad_all(panel, 8, 0);

    return panel;
}

/**
 * Build one home-screen action button.
 *
 * Disabled buttons are still visible because they explain the intended product shape, but
 * they do not emit actions yet. That lets us test navigation and touch without exposing
 * unfinished brewing, cleaning, or manual-service behavior.
 */
static lv_obj_t *screen_home_create_action_button(lv_obj_t *parent,
                                                  const char *text,
                                                  bool primary,
                                                  bool enabled,
                                                  screen_home_button_context_t *context)
{
    lv_obj_t *button;
    lv_obj_t *label;

    button = lv_button_create(parent);
    lv_obj_set_width(button, primary ? lv_pct(100) : SCREEN_HOME_BUTTON_WIDTH);
    lv_obj_set_height(button, primary ? 56 : SCREEN_HOME_ACTION_HEIGHT);
    lv_obj_set_style_radius(button, 4, 0);
    lv_obj_set_style_bg_color(button, enabled ? lv_color_hex(0xF47B32) : lv_color_hex(0x30373A), 0);
    lv_obj_set_style_bg_color(button, enabled ? lv_color_hex(0xC85F22) : lv_color_hex(0x30373A), LV_STATE_PRESSED);
    lv_obj_set_style_border_width(button, 1, 0);
    lv_obj_set_style_border_color(button, enabled ? lv_color_hex(0xC85F22) : lv_color_hex(0x464F53), 0);

    if (enabled && context != NULL)
    {
        lv_obj_add_event_cb(button, screen_home_button_event_cb, LV_EVENT_CLICKED, context);
    }
    else
    {
        lv_obj_add_state(button, LV_STATE_DISABLED);
    }

    label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, enabled ? lv_color_hex(0xFFFFFF) : lv_color_hex(0x9EA8AC), 0);
    lv_obj_center(label);

    return button;
}

/**
 * Build one tank summary box for the home screen.
 *
 * These boxes intentionally show only the headline values for now. The detailed raw
 * information belongs on the diagnostic status screen until we know which values the
 * normal brewing user actually needs every day.
 */
static lv_obj_t *screen_home_create_tank_panel(lv_obj_t *parent, const char *name, lv_obj_t **temp_value)
{
    lv_obj_t *tank;
    lv_obj_t *label;
    lv_obj_t *small_text;

    tank = lv_obj_create(parent);
    lv_obj_set_size(tank, SCREEN_HOME_TANK_WIDTH, 82);
    lv_obj_set_style_bg_color(tank, lv_color_hex(0x292929), 0);
    lv_obj_set_style_bg_opa(tank, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(tank, lv_color_hex(0x3F3F3F), 0);
    lv_obj_set_style_border_width(tank, 1, 0);
    lv_obj_set_style_radius(tank, 4, 0);
    lv_obj_set_style_pad_all(tank, 7, 0);
    lv_obj_set_layout(tank, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(tank, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(tank, 4, 0);

    label = lv_label_create(tank);
    lv_label_set_text(label, name);
    lv_obj_set_style_text_color(label, lv_color_hex(0xF47B32), 0);

    *temp_value = lv_label_create(tank);
    lv_label_set_text(*temp_value, "--.-");
    lv_obj_set_style_text_font(*temp_value, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(*temp_value, lv_color_hex(0xFFFFFF), 0);

    small_text = lv_label_create(tank);
    lv_label_set_text(small_text, "target -- C\npump off");
    lv_obj_set_style_text_color(small_text, lv_color_hex(0x9B9B9B), 0);

    return tank;
}

static void screen_home_button_event_cb(lv_event_t *event)
{
    screen_home_button_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL || context->handler == NULL)
    {
        return;
    }

    context->handler(context->action, context->user_data);
}

void screen_home_init(screen_home_t *home, ui_action_handler_t action_handler, void *user_data)
{
    lv_obj_t *container;
    lv_obj_t *topbar;
    lv_obj_t *brand_block;
    lv_obj_t *brand;
    lv_obj_t *mcu;
    lv_obj_t *state;
    lv_obj_t *machine_panel;
    lv_obj_t *machine_label;
    lv_obj_t *tank_row;
    lv_obj_t *action_panel;
    lv_obj_t *action_label;
    lv_obj_t *button_row;
    lv_obj_t *footer;

    if (home == NULL)
    {
        return;
    }

    memset(home, 0, sizeof(*home));

    home->status_button_context.action = UI_ACTION_SHOW_STATUS;
    home->status_button_context.handler = action_handler;
    home->status_button_context.user_data = user_data;
    home->manual_button_context.action = UI_ACTION_SHOW_MANUAL;
    home->manual_button_context.handler = action_handler;
    home->manual_button_context.user_data = user_data;
    home->clean_button_context.action = UI_ACTION_SHOW_CLEAN;
    home->clean_button_context.handler = action_handler;
    home->clean_button_context.user_data = user_data;
    home->settings_button_context.action = UI_ACTION_SHOW_SETTINGS;
    home->settings_button_context.handler = action_handler;
    home->settings_button_context.user_data = user_data;

    home->screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(home->screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(home->screen, LV_OPA_COVER, 0);

    container = lv_obj_create(home->screen);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_remove_style(container, NULL, LV_PART_SCROLLBAR);
    lv_obj_set_style_bg_color(container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, SCREEN_HOME_PAD, 0);
    lv_obj_set_style_pad_row(container, SCREEN_HOME_GAP, 0);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);

    topbar = lv_obj_create(container);
    lv_obj_set_width(topbar, lv_pct(100));
    lv_obj_set_height(topbar, 46);
    lv_obj_set_style_bg_color(topbar, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_opa(topbar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(topbar, 0, 0);
    lv_obj_set_style_pad_all(topbar, 6, 0);
    lv_obj_set_layout(topbar, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(topbar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(topbar, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    brand_block = lv_obj_create(topbar);
    lv_obj_remove_style_all(brand_block);
    lv_obj_set_width(brand_block, 160);
    lv_obj_set_height(brand_block, LV_SIZE_CONTENT);
    lv_obj_set_layout(brand_block, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(brand_block, LV_FLEX_FLOW_COLUMN);

    brand = lv_label_create(brand_block);
    lv_label_set_text(brand, "FreeBrewie");
    lv_obj_set_style_text_font(brand, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(brand, lv_color_hex(0xFFFFFF), 0);

    mcu = lv_label_create(brand_block);
    lv_label_set_text(mcu, "MCU unknown");
    lv_obj_set_style_text_color(mcu, lv_color_hex(0x9B9B9B), 0);
    home->mcu_value = mcu;

    state = lv_label_create(topbar);
    lv_label_set_text(state, "Ready");
    lv_obj_set_style_bg_color(state, lv_color_hex(0x557D45), 0);
    lv_obj_set_style_bg_opa(state, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(state, 4, 0);
    lv_obj_set_style_pad_left(state, 8, 0);
    lv_obj_set_style_pad_right(state, 8, 0);
    lv_obj_set_style_pad_top(state, 5, 0);
    lv_obj_set_style_pad_bottom(state, 5, 0);
    lv_obj_set_style_text_color(state, lv_color_hex(0xFFFFFF), 0);

    machine_panel = screen_home_create_panel(container);
    lv_obj_set_layout(machine_panel, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(machine_panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(machine_panel, 7, 0);

    machine_label = lv_label_create(machine_panel);
    lv_label_set_text(machine_label, "MACHINE");
    lv_obj_set_style_text_color(machine_label, lv_color_hex(0xF47B32), 0);

    tank_row = lv_obj_create(machine_panel);
    lv_obj_remove_style_all(tank_row);
    lv_obj_set_width(tank_row, lv_pct(100));
    lv_obj_set_height(tank_row, LV_SIZE_CONTENT);
    lv_obj_set_layout(tank_row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(tank_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(tank_row, 8, 0);

    screen_home_create_tank_panel(tank_row, "Mash", &home->mash_temp_value);
    screen_home_create_tank_panel(tank_row, "Boil", &home->boil_temp_value);

    action_panel = screen_home_create_panel(container);
    lv_obj_set_layout(action_panel, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(action_panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(action_panel, 8, 0);

    action_label = lv_label_create(action_panel);
    lv_label_set_text(action_label, "NEXT ACTION");
    lv_obj_set_style_text_color(action_label, lv_color_hex(0xF47B32), 0);

    screen_home_create_action_button(action_panel, "Brew later", true, false, NULL);

    button_row = lv_obj_create(action_panel);
    lv_obj_remove_style_all(button_row);
    lv_obj_set_width(button_row, lv_pct(100));
    lv_obj_set_height(button_row, LV_SIZE_CONTENT);
    lv_obj_set_layout(button_row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(button_row, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_style_pad_row(button_row, 8, 0);
    lv_obj_set_style_pad_column(button_row, 8, 0);

    screen_home_create_action_button(button_row, "Clean", false, true, &home->clean_button_context);
    screen_home_create_action_button(button_row, "Manual", false, true, &home->manual_button_context);
    screen_home_create_action_button(button_row, "Status", false, true, &home->status_button_context);
    screen_home_create_action_button(button_row, "Settings", false, true, &home->settings_button_context);

    footer = lv_label_create(container);
    lv_label_set_text(footer, "Idle                                      no faults");
    lv_obj_set_style_text_color(footer, lv_color_hex(0x9B9B9B), 0);
}

void screen_home_update(screen_home_t *home, const status_screen_view_model_t *view_model)
{
    char text[24];

    if (home == NULL || view_model == NULL)
    {
        return;
    }

    if (home->mcu_value != NULL)
    {
        lv_label_set_text_fmt(home->mcu_value, "MCU %s", view_model->link_text);
    }

    /*
     * The current status view model exposes compact text rather than separate numeric tank
     * values. Until the logic layer grows a dedicated home view model, show stable placeholder
     * temperatures while the home screen proves layout and navigation.
     */
    snprintf(text, sizeof(text), "--.-");
    lv_label_set_text(home->mash_temp_value, text);
    lv_label_set_text(home->boil_temp_value, text);
}
