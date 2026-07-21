#include "Screen_recipe_draft_menu.h"

#include <string.h>

#define SCREEN_RECIPE_DRAFT_MENU_SECTION_COUNT 4U

static void screen_recipe_draft_menu_set_static(lv_obj_t *object);
static lv_obj_t *screen_recipe_draft_menu_create_header(lv_obj_t *parent,
                                                        screen_recipe_draft_menu_t *draft_menu);
static lv_obj_t *screen_recipe_draft_menu_create_nav_button(lv_obj_t *parent,
                                                            const char *text,
                                                            lv_align_t align,
                                                            screen_recipe_draft_menu_nav_context_t *context);
static lv_obj_t *screen_recipe_draft_menu_create_menu_button(lv_obj_t *parent,
                                                             screen_recipe_draft_menu_t *draft_menu);
static lv_obj_t *screen_recipe_draft_menu_create_recipe_bar(lv_obj_t *parent,
                                                            screen_recipe_draft_menu_t *draft_menu);
static lv_obj_t *screen_recipe_draft_menu_create_section_button(
    lv_obj_t *parent,
    const char *title,
    screen_recipe_draft_menu_section_context_t *context);
static lv_obj_t *screen_recipe_draft_menu_create_disabled_brew_button(lv_obj_t *parent);
static void screen_recipe_draft_menu_nav_event_cb(lv_event_t *event);
static void screen_recipe_draft_menu_section_event_cb(lv_event_t *event);

static const char *const screen_recipe_draft_menu_section_titles[SCREEN_RECIPE_DRAFT_MENU_SECTION_COUNT] = {
    "DETAILS",
    "INGREDIENTS",
    "BREWING",
    "FERMENTATION"};

static const char *const screen_recipe_draft_menu_section_bodies[SCREEN_RECIPE_DRAFT_MENU_SECTION_COUNT] = {
    "Details editing will come after local recipe storage is designed.",
    "Ingredient editing will be split into proper fermentables and hops screens later.",
    "Brewing editing will use separate mash, water, boil, and cooling screens later.",
    "Fermentation editing will stay separate from SOM hardware control."};

/****************************************************************************************
 * @brief Make an object static so it cannot become a tiny scroll target.
 ****************************************************************************************/
static void screen_recipe_draft_menu_set_static(lv_obj_t *object)
{
    if (object == NULL)
    {
        return;
    }

    lv_obj_clear_flag(object, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_OFF);
}

static lv_obj_t *screen_recipe_draft_menu_create_header(lv_obj_t *parent,
                                                        screen_recipe_draft_menu_t *draft_menu)
{
    lv_obj_t *header;
    lv_obj_t *title;

    header = lv_obj_create(parent);
    screen_recipe_draft_menu_set_static(header);
    lv_obj_set_width(header, lv_pct(100));
    lv_obj_set_height(header, 50);
    lv_obj_set_style_bg_opa(header, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_all(header, 0, 0);

    screen_recipe_draft_menu_create_nav_button(header,
                                               "<",
                                               LV_ALIGN_LEFT_MID,
                                               &draft_menu->back_button_context);

    title = lv_label_create(header);
    lv_label_set_text(title, "Recipe");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(title);

    screen_recipe_draft_menu_create_menu_button(header, draft_menu);
    return header;
}

static lv_obj_t *screen_recipe_draft_menu_create_nav_button(
    lv_obj_t *parent,
    const char *text,
    lv_align_t align,
    screen_recipe_draft_menu_nav_context_t *context)
{
    lv_obj_t *button;
    lv_obj_t *label;

    button = lv_button_create(parent);
    lv_obj_set_size(button, 42, 42);
    lv_obj_align(button, align, (align == LV_ALIGN_LEFT_MID) ? 2 : -2, 0);
    lv_obj_set_style_bg_opa(button, LV_OPA_TRANSP, 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x343434), LV_STATE_PRESSED);
    lv_obj_set_style_border_width(button, 0, 0);
    lv_obj_set_style_radius(button, 4, 0);
    lv_obj_add_event_cb(button, screen_recipe_draft_menu_nav_event_cb, LV_EVENT_CLICKED, context);

    label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);
    return button;
}

static lv_obj_t *screen_recipe_draft_menu_create_menu_button(lv_obj_t *parent,
                                                             screen_recipe_draft_menu_t *draft_menu)
{
    lv_obj_t *button;
    lv_obj_t *line;
    int8_t offset_y;

    button = lv_button_create(parent);
    lv_obj_set_size(button, 42, 42);
    lv_obj_align(button, LV_ALIGN_RIGHT_MID, -2, 0);
    lv_obj_set_style_bg_opa(button, LV_OPA_TRANSP, 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x343434), LV_STATE_PRESSED);
    lv_obj_set_style_border_width(button, 0, 0);
    lv_obj_set_style_radius(button, 4, 0);
    lv_obj_set_style_pad_all(button, 0, 0);
    lv_obj_add_event_cb(button,
                        screen_recipe_draft_menu_nav_event_cb,
                        LV_EVENT_CLICKED,
                        &draft_menu->menu_button_context);

    for (offset_y = -7; offset_y <= 7; offset_y = (int8_t)(offset_y + 7))
    {
        line = lv_obj_create(button);
        screen_recipe_draft_menu_set_static(line);
        lv_obj_set_size(line, 20, 2);
        lv_obj_set_style_bg_color(line, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_bg_opa(line, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(line, 0, 0);
        lv_obj_align(line, LV_ALIGN_CENTER, 0, offset_y);
    }

    return button;
}

/****************************************************************************************
 * @brief Create the old-style dark recipe bar shown under the header area.
 ****************************************************************************************/
static lv_obj_t *screen_recipe_draft_menu_create_recipe_bar(lv_obj_t *parent,
                                                            screen_recipe_draft_menu_t *draft_menu)
{
    lv_obj_t *bar;

    bar = lv_obj_create(parent);
    screen_recipe_draft_menu_set_static(bar);
    lv_obj_set_width(bar, lv_pct(100));
    lv_obj_set_height(bar, 74);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x292929), 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_obj_set_style_pad_all(bar, 8, 0);
    lv_obj_set_layout(bar, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(bar, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(bar, 4, 0);

    draft_menu->name_label = lv_label_create(bar);
    lv_label_set_text(draft_menu->name_label, "--");
    lv_label_set_long_mode(draft_menu->name_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(draft_menu->name_label, lv_pct(100));
    lv_obj_set_style_text_color(draft_menu->name_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(draft_menu->name_label, &lv_font_montserrat_20, 0);

    draft_menu->style_label = lv_label_create(bar);
    lv_label_set_text(draft_menu->style_label, "DRAFT RECIPE");
    lv_label_set_long_mode(draft_menu->style_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(draft_menu->style_label, lv_pct(100));
    lv_obj_set_style_text_color(draft_menu->style_label, lv_color_hex(0xE67526), 0);
    return bar;
}

static lv_obj_t *screen_recipe_draft_menu_create_section_button(
    lv_obj_t *parent,
    const char *title,
    screen_recipe_draft_menu_section_context_t *context)
{
    lv_obj_t *button;
    lv_obj_t *label;

    button = lv_button_create(parent);
    lv_obj_set_width(button, lv_pct(48));
    lv_obj_set_height(button, 70);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x292929), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x3B332D), LV_STATE_PRESSED);
    lv_obj_set_style_border_color(button, lv_color_hex(0x3F3F3F), 0);
    lv_obj_set_style_border_width(button, 1, 0);
    lv_obj_set_style_radius(button, 0, 0);
    lv_obj_add_event_cb(button, screen_recipe_draft_menu_section_event_cb, LV_EVENT_CLICKED, context);

    label = lv_label_create(button);
    lv_label_set_text(label, title);
    lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(label, lv_pct(100));
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);
    lv_obj_remove_flag(label, LV_OBJ_FLAG_CLICKABLE);
    return button;
}

static lv_obj_t *screen_recipe_draft_menu_create_disabled_brew_button(lv_obj_t *parent)
{
    lv_obj_t *button;
    lv_obj_t *label;

    button = lv_button_create(parent);
    lv_obj_set_width(button, lv_pct(100));
    lv_obj_set_height(button, 44);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x4B4741), 0);
    lv_obj_set_style_radius(button, 5, 0);
    lv_obj_add_state(button, LV_STATE_DISABLED);

    label = lv_label_create(button);
    lv_label_set_text(label, "BREW LATER");
    lv_obj_set_style_text_color(label, lv_color_hex(0xC8C8C8), 0);
    lv_obj_center(label);
    return button;
}

static void screen_recipe_draft_menu_nav_event_cb(lv_event_t *event)
{
    screen_recipe_draft_menu_nav_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL || context->handler == NULL)
    {
        return;
    }

    context->handler(context->action, context->value, context->user_data);
}

static void screen_recipe_draft_menu_section_event_cb(lv_event_t *event)
{
    screen_recipe_draft_menu_section_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL || context->draft_menu == NULL)
    {
        return;
    }

    if (context->handler != NULL)
    {
        context->handler(context->action, 0U, context->user_data);
        return;
    }

    ui_dialog_show(&context->draft_menu->section_dialog, context->title, context->body);
}

void screen_recipe_draft_menu_init(screen_recipe_draft_menu_t *draft_menu,
                                   ui_action_handler_t action_handler,
                                   void *user_data)
{
    lv_obj_t *container;
    lv_obj_t *hero;
    lv_obj_t *section_grid;
    lv_obj_t *actions;
    uint8_t section_index;

    if (draft_menu == NULL)
    {
        return;
    }

    memset(draft_menu, 0, sizeof(*draft_menu));
    draft_menu->back_button_context.action = UI_ACTION_SHOW_RECIPE_BUILDER;
    draft_menu->back_button_context.handler = action_handler;
    draft_menu->back_button_context.user_data = user_data;
    draft_menu->menu_button_context.action = UI_ACTION_SHOW_MENU;
    draft_menu->menu_button_context.handler = action_handler;
    draft_menu->menu_button_context.user_data = user_data;

    draft_menu->screen = lv_obj_create(NULL);
    screen_recipe_draft_menu_set_static(draft_menu->screen);
    lv_obj_set_style_bg_color(draft_menu->screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(draft_menu->screen, LV_OPA_COVER, 0);

    container = lv_obj_create(draft_menu->screen);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container, lv_color_hex(0x141414), 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 8, 0);
    lv_obj_set_style_pad_row(container, 8, 0);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);

    hero = lv_obj_create(container);
    screen_recipe_draft_menu_set_static(hero);
    lv_obj_set_width(hero, lv_pct(100));
    lv_obj_set_height(hero, 132);
    lv_obj_set_style_bg_color(hero, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_opa(hero, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(hero, 0, 0);
    lv_obj_set_style_pad_all(hero, 0, 0);
    lv_obj_set_layout(hero, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(hero, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(hero, 0, 0);

    screen_recipe_draft_menu_create_header(hero, draft_menu);
    screen_recipe_draft_menu_create_recipe_bar(hero, draft_menu);

    section_grid = lv_obj_create(container);
    lv_obj_set_width(section_grid, lv_pct(100));
    lv_obj_set_flex_grow(section_grid, 1);
    lv_obj_set_style_bg_opa(section_grid, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(section_grid, 0, 0);
    lv_obj_set_style_pad_all(section_grid, 0, 0);
    lv_obj_set_style_pad_row(section_grid, 8, 0);
    lv_obj_set_style_pad_column(section_grid, 8, 0);
    lv_obj_set_layout(section_grid, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(section_grid, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(section_grid,
                          LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);

    for (section_index = 0U; section_index < SCREEN_RECIPE_DRAFT_MENU_SECTION_COUNT; ++section_index)
    {
        draft_menu->section_contexts[section_index].title =
            screen_recipe_draft_menu_section_titles[section_index];
        draft_menu->section_contexts[section_index].body =
            screen_recipe_draft_menu_section_bodies[section_index];
        draft_menu->section_contexts[section_index].draft_menu = draft_menu;
        if (section_index == 0U)
        {
            draft_menu->section_contexts[section_index].action =
                UI_ACTION_SHOW_RECIPE_DRAFT_DETAILS;
            draft_menu->section_contexts[section_index].handler = action_handler;
            draft_menu->section_contexts[section_index].user_data = user_data;
        }
        else if (section_index == 1U)
        {
            draft_menu->section_contexts[section_index].action =
                UI_ACTION_SHOW_RECIPE_DRAFT_INGREDIENTS;
            draft_menu->section_contexts[section_index].handler = action_handler;
            draft_menu->section_contexts[section_index].user_data = user_data;
        }
        screen_recipe_draft_menu_create_section_button(
            section_grid,
            screen_recipe_draft_menu_section_titles[section_index],
            &draft_menu->section_contexts[section_index]);
    }

    actions = lv_obj_create(container);
    screen_recipe_draft_menu_set_static(actions);
    lv_obj_set_width(actions, lv_pct(100));
    lv_obj_set_height(actions, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(actions, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(actions, 0, 0);
    lv_obj_set_style_pad_all(actions, 0, 0);
    screen_recipe_draft_menu_create_disabled_brew_button(actions);

    ui_dialog_init(&draft_menu->section_dialog, draft_menu->screen, "Close", "OK");
}

/****************************************************************************************
 * @brief Show a RAM-only draft name without creating or saving a real recipe.
 ****************************************************************************************/
void screen_recipe_draft_menu_show(screen_recipe_draft_menu_t *draft_menu,
                                   const char *draft_name)
{
    if (draft_menu == NULL || draft_name == NULL || draft_menu->shown_name == draft_name)
    {
        return;
    }

    lv_label_set_text(draft_menu->name_label, draft_name);
    draft_menu->shown_name = draft_name;
}
