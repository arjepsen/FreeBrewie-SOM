#include "Screen_recipe_detail.h"

#include <string.h>

#include "UI_scroll.h"

#define SCREEN_RECIPE_DETAIL_SECTION_WIDTH_PCT 97

/*
 * This file builds the selected-recipe overview:
 * - a fixed header with back/menu navigation,
 * - a hero strip showing the selected recipe,
 * - four section buttons that drill into read-only recipe details,
 * - and a Brew button that only navigates to setup.
 */

static void screen_recipe_detail_set_static(lv_obj_t *object);
static lv_obj_t *screen_recipe_detail_create_header(lv_obj_t *parent, screen_recipe_detail_t *detail);
static lv_obj_t *screen_recipe_detail_create_nav_button(lv_obj_t *parent,
                                                        const char *text,
                                                        lv_align_t align,
                                                        screen_recipe_detail_button_context_t *context);
static lv_obj_t *screen_recipe_detail_create_menu_button(lv_obj_t *parent, screen_recipe_detail_t *detail);
static lv_obj_t *screen_recipe_detail_create_section_button(lv_obj_t *parent,
                                                            const char *title,
                                                            const char *subtitle,
                                                            screen_recipe_detail_button_context_t *context);
static lv_obj_t *screen_recipe_detail_create_brew_button(lv_obj_t *parent,
                                                         screen_recipe_detail_button_context_t *context);
static void screen_recipe_detail_button_event_cb(lv_event_t *event);

static void screen_recipe_detail_set_static(lv_obj_t *object)
{
    if (object == NULL)
    {
        return;
    }

    lv_obj_clear_flag(object, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_OFF);
}

static lv_obj_t *screen_recipe_detail_create_header(lv_obj_t *parent, screen_recipe_detail_t *detail)
{
    lv_obj_t *header;
    lv_obj_t *title;

    header = lv_obj_create(parent);
    screen_recipe_detail_set_static(header);
    lv_obj_set_width(header, lv_pct(100));
    lv_obj_set_height(header, 64);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_all(header, 0, 0);

    screen_recipe_detail_create_nav_button(header, "<", LV_ALIGN_LEFT_MID, &detail->back_button_context);

    title = lv_label_create(header);
    lv_label_set_text(title, "Recipe");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xC8C8C8), 0);
    lv_obj_center(title);

    screen_recipe_detail_create_menu_button(header, detail);
    return header;
}

static lv_obj_t *screen_recipe_detail_create_nav_button(lv_obj_t *parent,
                                                        const char *text,
                                                        lv_align_t align,
                                                        screen_recipe_detail_button_context_t *context)
{
    lv_obj_t *button;
    lv_obj_t *label;
    int32_t offset_x;

    offset_x = (align == LV_ALIGN_LEFT_MID) ? 2 : -2;
    button = lv_button_create(parent);
    lv_obj_set_size(button, 42, 42);
    lv_obj_align(button, align, offset_x, 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x343434), LV_STATE_PRESSED);
    lv_obj_set_style_border_width(button, 0, 0);
    lv_obj_set_style_radius(button, 4, 0);
    lv_obj_add_event_cb(button, screen_recipe_detail_button_event_cb, LV_EVENT_CLICKED, context);

    label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);
    return button;
}

static lv_obj_t *screen_recipe_detail_create_menu_button(lv_obj_t *parent, screen_recipe_detail_t *detail)
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
    lv_obj_add_event_cb(button,
                        screen_recipe_detail_button_event_cb,
                        LV_EVENT_CLICKED,
                        &detail->menu_button_context);

    for (offset_y = -7; offset_y <= 7; offset_y = (int8_t)(offset_y + 7))
    {
        line = lv_obj_create(button);
        screen_recipe_detail_set_static(line);
        lv_obj_set_size(line, 20, 2);
        lv_obj_set_style_bg_color(line, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_bg_opa(line, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(line, 0, 0);
        lv_obj_align(line, LV_ALIGN_CENTER, 0, offset_y);
    }

    return button;
}

static lv_obj_t *screen_recipe_detail_create_section_button(lv_obj_t *parent,
                                                            const char *title,
                                                            const char *subtitle,
                                                            screen_recipe_detail_button_context_t *context)
{
    lv_obj_t *button;
    lv_obj_t *title_label;
    lv_obj_t *subtitle_label;

    button = lv_button_create(parent);
    lv_obj_set_width(button, lv_pct(SCREEN_RECIPE_DETAIL_SECTION_WIDTH_PCT));
    lv_obj_set_height(button, 58);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x141414), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x3B332D), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(button, lv_color_hex(0x2C2B2B), 0);
    lv_obj_set_style_border_width(button, 1, 0);
    lv_obj_set_style_radius(button, 0, 0);
    lv_obj_set_style_pad_all(button, 8, 0);
    lv_obj_add_event_cb(button, screen_recipe_detail_button_event_cb, LV_EVENT_CLICKED, context);

    title_label = lv_label_create(button);
    lv_label_set_text(title_label, title);
    lv_label_set_long_mode(title_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(title_label, lv_pct(100));
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_20, 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_LEFT, 0, 0);

    subtitle_label = lv_label_create(button);
    lv_label_set_text(subtitle_label, subtitle);
    lv_label_set_long_mode(subtitle_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(subtitle_label, lv_pct(100));
    lv_obj_set_style_text_color(subtitle_label, lv_color_hex(0xE67526), 0);
    lv_obj_align(subtitle_label, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    lv_obj_remove_flag(title_label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(subtitle_label, LV_OBJ_FLAG_CLICKABLE);
    return button;
}

static lv_obj_t *screen_recipe_detail_create_brew_button(lv_obj_t *parent,
                                                         screen_recipe_detail_button_context_t *context)
{
    lv_obj_t *button;
    lv_obj_t *label;

    button = lv_button_create(parent);
    lv_obj_set_width(button, lv_pct(100));
    lv_obj_set_height(button, 44);
    lv_obj_set_style_bg_color(button, lv_color_hex(0xF47B32), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0xC85F22), LV_STATE_PRESSED);
    lv_obj_set_style_radius(button, 5, 0);
    lv_obj_add_event_cb(button, screen_recipe_detail_button_event_cb, LV_EVENT_CLICKED, context);

    label = lv_label_create(button);
    lv_label_set_text(label, "BREW");
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);
    return button;
}

static void screen_recipe_detail_button_event_cb(lv_event_t *event)
{
    screen_recipe_detail_button_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL || context->handler == NULL)
    {
        return;
    }

    context->handler(context->action, context->value, context->user_data);
}

void screen_recipe_detail_init(screen_recipe_detail_t *detail,
                               ui_action_handler_t action_handler,
                               void *user_data)
{
    lv_obj_t *container;
    lv_obj_t *hero;
    lv_obj_t *section_list;
    lv_obj_t *actions;

    if (detail == NULL)
    {
        return;
    }

    memset(detail, 0, sizeof(*detail));
    detail->back_button_context.action = UI_ACTION_SHOW_RECIPES;
    detail->back_button_context.handler = action_handler;
    detail->back_button_context.user_data = user_data;
    detail->menu_button_context.action = UI_ACTION_SHOW_MENU;
    detail->menu_button_context.handler = action_handler;
    detail->menu_button_context.user_data = user_data;
    detail->details_context.action = UI_ACTION_SHOW_RECIPE_DETAILS_SECTION;
    detail->details_context.handler = action_handler;
    detail->details_context.user_data = user_data;
    detail->ingredients_context.action = UI_ACTION_SHOW_RECIPE_INGREDIENTS_SECTION;
    detail->ingredients_context.handler = action_handler;
    detail->ingredients_context.user_data = user_data;
    detail->brewing_context.action = UI_ACTION_SHOW_RECIPE_BREWING_SECTION;
    detail->brewing_context.handler = action_handler;
    detail->brewing_context.user_data = user_data;
    detail->fermentation_context.action = UI_ACTION_SHOW_RECIPE_FERMENTATION_SECTION;
    detail->fermentation_context.handler = action_handler;
    detail->fermentation_context.user_data = user_data;
    detail->brew_context.action = UI_ACTION_SHOW_BREW_SETUP;
    detail->brew_context.handler = action_handler;
    detail->brew_context.user_data = user_data;

    /* Build the object tree once. Later recipe changes only update labels and action values. */
    detail->screen = lv_obj_create(NULL);
    screen_recipe_detail_set_static(detail->screen);
    lv_obj_set_style_bg_color(detail->screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(detail->screen, LV_OPA_COVER, 0);

    container = lv_obj_create(detail->screen);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 8, 0);
    lv_obj_set_style_pad_row(container, 8, 0);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);

    screen_recipe_detail_create_header(container, detail);

    hero = lv_obj_create(container);
    screen_recipe_detail_set_static(hero);
    lv_obj_set_width(hero, lv_pct(100));
    lv_obj_set_height(hero, 82);
    lv_obj_set_style_bg_color(hero, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_opa(hero, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(hero, lv_color_hex(0x393939), 0);
    lv_obj_set_style_border_width(hero, 1, 0);
    lv_obj_set_style_radius(hero, 0, 0);
    lv_obj_set_style_pad_all(hero, 8, 0);
    lv_obj_set_layout(hero, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(hero, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(hero, 5, 0);

    detail->name_label = lv_label_create(hero);
    lv_label_set_text(detail->name_label, "--");
    lv_label_set_long_mode(detail->name_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(detail->name_label, lv_pct(100));
    lv_obj_set_style_text_color(detail->name_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(detail->name_label, &lv_font_montserrat_20, 0);

    detail->style_label = lv_label_create(hero);
    lv_label_set_text(detail->style_label, "--");
    lv_label_set_long_mode(detail->style_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(detail->style_label, lv_pct(100));
    lv_obj_set_style_text_color(detail->style_label, lv_color_hex(0xE67526), 0);

    section_list = lv_obj_create(container);
    lv_obj_set_width(section_list, lv_pct(100));
    lv_obj_set_flex_grow(section_list, 1);
    lv_obj_set_style_bg_color(section_list, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(section_list, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(section_list, 0, 0);
    lv_obj_set_style_pad_all(section_list, 0, 0);
    lv_obj_set_style_pad_row(section_list, 8, 0);
    ui_scroll_apply_gutter(section_list);
    lv_obj_set_layout(section_list, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(section_list, LV_FLEX_FLOW_COLUMN);

    screen_recipe_detail_create_section_button(section_list,
                                               "Details",
                                               "Style, notes, and metadata",
                                               &detail->details_context);
    screen_recipe_detail_create_section_button(section_list,
                                               "Ingredients",
                                               "Fermentables, hops, additions",
                                               &detail->ingredients_context);
    screen_recipe_detail_create_section_button(section_list,
                                               "Brewing",
                                               "Mash, boil, cooling setup",
                                               &detail->brewing_context);
    screen_recipe_detail_create_section_button(section_list,
                                               "Fermentation",
                                               "Fermentation plan",
                                               &detail->fermentation_context);

    actions = lv_obj_create(container);
    screen_recipe_detail_set_static(actions);
    lv_obj_set_width(actions, lv_pct(100));
    lv_obj_set_height(actions, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(actions, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(actions, 0, 0);
    lv_obj_set_style_pad_all(actions, 0, 0);
    lv_obj_set_style_pad_row(actions, 6, 0);
    lv_obj_set_layout(actions, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(actions, LV_FLEX_FLOW_COLUMN);

    screen_recipe_detail_create_brew_button(actions, &detail->brew_context);
}

void screen_recipe_detail_show_recipe(screen_recipe_detail_t *detail, const recipe_t *recipe)
{
    if (detail == NULL || recipe == NULL || detail->shown_recipe_id == recipe->id)
    {
        return;
    }

    lv_label_set_text(detail->name_label, recipe->name);
    lv_label_set_text(detail->style_label, recipe->style);

    /* Carry the selected recipe id through every navigation action from this screen. */
    detail->details_context.value = recipe->id;
    detail->ingredients_context.value = recipe->id;
    detail->brewing_context.value = recipe->id;
    detail->fermentation_context.value = recipe->id;
    detail->brew_context.value = recipe->id;
    detail->shown_recipe_id = recipe->id;
}
