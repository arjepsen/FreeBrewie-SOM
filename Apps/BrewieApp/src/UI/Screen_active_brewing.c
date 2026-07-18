#include "Screen_active_brewing.h"

#include <string.h>

#define SCREEN_ACTIVE_BREWING_PAD 8

static void screen_active_brewing_set_static(lv_obj_t *object);
static lv_obj_t *screen_active_brewing_create_header(lv_obj_t *parent,
                                                     screen_active_brewing_t *active_brewing);
static lv_obj_t *screen_active_brewing_create_nav_button(lv_obj_t *parent,
                                                         const char *text,
                                                         lv_align_t align,
                                                         screen_active_brewing_nav_context_t *context);
static lv_obj_t *screen_active_brewing_create_menu_button(lv_obj_t *parent,
                                                          screen_active_brewing_t *active_brewing);
static lv_obj_t *screen_active_brewing_create_tab_button(lv_obj_t *parent,
                                                         const char *text,
                                                         screen_active_brewing_tab_context_t *context);
static void screen_active_brewing_create_overall_page(lv_obj_t *parent);
static void screen_active_brewing_create_actions_page(lv_obj_t *parent);
static void screen_active_brewing_select_tab(screen_active_brewing_t *active_brewing,
                                             screen_active_brewing_tab_id_t tab_id);
static void screen_active_brewing_nav_event_cb(lv_event_t *event);
static void screen_active_brewing_tab_event_cb(lv_event_t *event);

/****************************************************************************************
 * @brief Make an object static so small visual pieces do not become scroll targets.
 ****************************************************************************************/
static void screen_active_brewing_set_static(lv_obj_t *object)
{
    if (object == NULL)
    {
        return;
    }

    lv_obj_clear_flag(object, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_OFF);
}

static lv_obj_t *screen_active_brewing_create_header(lv_obj_t *parent,
                                                     screen_active_brewing_t *active_brewing)
{
    lv_obj_t *header;
    lv_obj_t *title;

    header = lv_obj_create(parent);
    screen_active_brewing_set_static(header);
    lv_obj_set_width(header, lv_pct(100));
    lv_obj_set_height(header, 64);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_all(header, 0, 0);

    screen_active_brewing_create_nav_button(header,
                                            "<",
                                            LV_ALIGN_LEFT_MID,
                                            &active_brewing->back_button_context);

    title = lv_label_create(header);
    lv_label_set_text(title, "Brewing");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xC8C8C8), 0);
    lv_obj_center(title);

    screen_active_brewing_create_menu_button(header, active_brewing);
    return header;
}

static lv_obj_t *screen_active_brewing_create_nav_button(lv_obj_t *parent,
                                                         const char *text,
                                                         lv_align_t align,
                                                         screen_active_brewing_nav_context_t *context)
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
    lv_obj_add_event_cb(button, screen_active_brewing_nav_event_cb, LV_EVENT_CLICKED, context);

    label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);
    return button;
}

static lv_obj_t *screen_active_brewing_create_menu_button(lv_obj_t *parent,
                                                          screen_active_brewing_t *active_brewing)
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
                        screen_active_brewing_nav_event_cb,
                        LV_EVENT_CLICKED,
                        &active_brewing->menu_button_context);

    for (offset_y = -7; offset_y <= 7; offset_y = (int8_t)(offset_y + 7))
    {
        line = lv_obj_create(button);
        screen_active_brewing_set_static(line);
        lv_obj_set_size(line, 20, 2);
        lv_obj_set_style_bg_color(line, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_bg_opa(line, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(line, 0, 0);
        lv_obj_align(line, LV_ALIGN_CENTER, 0, offset_y);
    }

    return button;
}

static lv_obj_t *screen_active_brewing_create_tab_button(lv_obj_t *parent,
                                                         const char *text,
                                                         screen_active_brewing_tab_context_t *context)
{
    lv_obj_t *button;
    lv_obj_t *label;

    button = lv_button_create(parent);
    lv_obj_set_width(button, lv_pct(50));
    lv_obj_set_height(button, 34);
    lv_obj_set_style_radius(button, 0, 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0xD96F28), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0xF47B32), LV_STATE_CHECKED);
    lv_obj_set_style_border_width(button, 0, 0);
    lv_obj_add_event_cb(button, screen_active_brewing_tab_event_cb, LV_EVENT_CLICKED, context);

    label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_hex(0x111111), 0);
    lv_obj_center(label);
    return button;
}

static void screen_active_brewing_create_overall_page(lv_obj_t *parent)
{
    lv_obj_t *progress;
    lv_obj_t *label;
    lv_obj_t *sub_label;
    lv_obj_t *pause_button;
    lv_obj_t *pause_label;

    progress = lv_obj_create(parent);
    screen_active_brewing_set_static(progress);
    lv_obj_set_size(progress, 116, 116);
    lv_obj_set_style_radius(progress, 58, 0);
    lv_obj_set_style_bg_color(progress, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_opa(progress, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(progress, lv_color_hex(0xF47B32), 0);
    lv_obj_set_style_border_width(progress, 5, 0);
    lv_obj_align(progress, LV_ALIGN_TOP_MID, 0, 4);

    label = lv_label_create(progress);
    lv_label_set_text(label, "0");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);

    label = lv_label_create(parent);
    lv_label_set_text(label, "Ready - 0%");
    lv_obj_set_width(label, lv_pct(100));
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 132);

    sub_label = lv_label_create(parent);
    lv_label_set_text(sub_label, "Remaining time: --:--");
    lv_obj_set_width(sub_label, lv_pct(100));
    lv_obj_set_style_text_align(sub_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(sub_label, lv_color_hex(0x8C8C8C), 0);
    lv_obj_align(sub_label, LV_ALIGN_TOP_MID, 0, 158);

    pause_button = lv_button_create(parent);
    lv_obj_set_width(pause_button, lv_pct(100));
    lv_obj_set_height(pause_button, 48);
    lv_obj_align(pause_button, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(pause_button, lv_color_hex(0xF47B32), 0);
    lv_obj_set_style_radius(pause_button, 5, 0);
    lv_obj_add_state(pause_button, LV_STATE_DISABLED);

    pause_label = lv_label_create(pause_button);
    lv_label_set_text(pause_label, "PAUSE");
    lv_obj_set_style_text_color(pause_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(pause_label);
}

static void screen_active_brewing_create_actions_page(lv_obj_t *parent)
{
    lv_obj_t *row;
    lv_obj_t *label;

    row = lv_obj_create(parent);
    screen_active_brewing_set_static(row);
    lv_obj_set_width(row, lv_pct(100));
    lv_obj_set_height(row, 64);
    lv_obj_set_style_bg_color(row, lv_color_hex(0x282828), 0);
    lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(row, lv_color_hex(0x343434), 0);
    lv_obj_set_style_border_width(row, 1, 0);
    lv_obj_set_style_radius(row, 0, 0);
    lv_obj_set_style_pad_all(row, 8, 0);

    label = lv_label_create(row);
    lv_label_set_text(label, "Mash tank        --.- C");
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);

    row = lv_obj_create(parent);
    screen_active_brewing_set_static(row);
    lv_obj_set_width(row, lv_pct(100));
    lv_obj_set_height(row, 64);
    lv_obj_set_style_bg_color(row, lv_color_hex(0x282828), 0);
    lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(row, lv_color_hex(0x343434), 0);
    lv_obj_set_style_border_width(row, 1, 0);
    lv_obj_set_style_radius(row, 0, 0);
    lv_obj_set_style_pad_all(row, 8, 0);

    label = lv_label_create(row);
    lv_label_set_text(label, "Boil tank        --.- C");
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);
}

static void screen_active_brewing_select_tab(screen_active_brewing_t *active_brewing,
                                             screen_active_brewing_tab_id_t tab_id)
{
    uint32_t tab_index;

    if (active_brewing == NULL || tab_id >= SCREEN_ACTIVE_BREWING_TAB_COUNT)
    {
        return;
    }

    for (tab_index = 0U; tab_index < SCREEN_ACTIVE_BREWING_TAB_COUNT; ++tab_index)
    {
        if (tab_index == (uint32_t)tab_id)
        {
            lv_obj_add_state(active_brewing->tab_buttons[tab_index], LV_STATE_CHECKED);
            lv_obj_remove_flag(active_brewing->tab_pages[tab_index], LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            lv_obj_remove_state(active_brewing->tab_buttons[tab_index], LV_STATE_CHECKED);
            lv_obj_add_flag(active_brewing->tab_pages[tab_index], LV_OBJ_FLAG_HIDDEN);
        }
    }

    active_brewing->selected_tab_id = tab_id;
}

static void screen_active_brewing_nav_event_cb(lv_event_t *event)
{
    screen_active_brewing_nav_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL || context->handler == NULL)
    {
        return;
    }

    context->handler(context->action, context->value, context->user_data);
}

static void screen_active_brewing_tab_event_cb(lv_event_t *event)
{
    screen_active_brewing_tab_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL || context->active_brewing == NULL)
    {
        return;
    }

    screen_active_brewing_select_tab(context->active_brewing, context->tab_id);
}

void screen_active_brewing_init(screen_active_brewing_t *active_brewing,
                                ui_action_handler_t action_handler,
                                void *user_data)
{
    lv_obj_t *container;
    lv_obj_t *tabs;
    uint32_t tab_index;

    if (active_brewing == NULL)
    {
        return;
    }

    memset(active_brewing, 0, sizeof(*active_brewing));
    active_brewing->back_button_context.action = UI_ACTION_SHOW_BREW_CHECKLIST;
    active_brewing->back_button_context.handler = action_handler;
    active_brewing->back_button_context.user_data = user_data;
    active_brewing->menu_button_context.action = UI_ACTION_SHOW_MENU;
    active_brewing->menu_button_context.handler = action_handler;
    active_brewing->menu_button_context.user_data = user_data;

    active_brewing->screen = lv_obj_create(NULL);
    screen_active_brewing_set_static(active_brewing->screen);
    lv_obj_set_style_bg_color(active_brewing->screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(active_brewing->screen, LV_OPA_COVER, 0);

    container = lv_obj_create(active_brewing->screen);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, SCREEN_ACTIVE_BREWING_PAD, 0);
    lv_obj_set_style_pad_row(container, 8, 0);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);

    screen_active_brewing_create_header(container, active_brewing);

    active_brewing->recipe_label = lv_label_create(container);
    lv_label_set_text(active_brewing->recipe_label, "--");
    lv_label_set_long_mode(active_brewing->recipe_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(active_brewing->recipe_label, lv_pct(100));
    lv_obj_set_style_text_color(active_brewing->recipe_label, lv_color_hex(0xE67526), 0);

    tabs = lv_obj_create(container);
    screen_active_brewing_set_static(tabs);
    lv_obj_set_width(tabs, lv_pct(100));
    lv_obj_set_height(tabs, 34);
    lv_obj_set_style_bg_color(tabs, lv_color_hex(0xD96F28), 0);
    lv_obj_set_style_bg_opa(tabs, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(tabs, 0, 0);
    lv_obj_set_style_pad_all(tabs, 0, 0);
    lv_obj_set_layout(tabs, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(tabs, LV_FLEX_FLOW_ROW);

    active_brewing->tab_contexts[SCREEN_ACTIVE_BREWING_TAB_OVERALL].tab_id =
        SCREEN_ACTIVE_BREWING_TAB_OVERALL;
    active_brewing->tab_contexts[SCREEN_ACTIVE_BREWING_TAB_OVERALL].active_brewing =
        active_brewing;
    active_brewing->tab_contexts[SCREEN_ACTIVE_BREWING_TAB_ACTIONS].tab_id =
        SCREEN_ACTIVE_BREWING_TAB_ACTIONS;
    active_brewing->tab_contexts[SCREEN_ACTIVE_BREWING_TAB_ACTIONS].active_brewing =
        active_brewing;

    active_brewing->tab_buttons[SCREEN_ACTIVE_BREWING_TAB_OVERALL] =
        screen_active_brewing_create_tab_button(tabs,
                                                "Overall",
                                                &active_brewing->tab_contexts[SCREEN_ACTIVE_BREWING_TAB_OVERALL]);
    active_brewing->tab_buttons[SCREEN_ACTIVE_BREWING_TAB_ACTIONS] =
        screen_active_brewing_create_tab_button(tabs,
                                                "Actions",
                                                &active_brewing->tab_contexts[SCREEN_ACTIVE_BREWING_TAB_ACTIONS]);

    for (tab_index = 0U; tab_index < SCREEN_ACTIVE_BREWING_TAB_COUNT; ++tab_index)
    {
        active_brewing->tab_pages[tab_index] = lv_obj_create(container);
        screen_active_brewing_set_static(active_brewing->tab_pages[tab_index]);
        lv_obj_set_width(active_brewing->tab_pages[tab_index], lv_pct(100));
        lv_obj_set_flex_grow(active_brewing->tab_pages[tab_index], 1);
        lv_obj_set_style_bg_color(active_brewing->tab_pages[tab_index], lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_opa(active_brewing->tab_pages[tab_index], LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(active_brewing->tab_pages[tab_index], 0, 0);
        lv_obj_set_style_pad_all(active_brewing->tab_pages[tab_index], 0, 0);
    }

    screen_active_brewing_create_overall_page(active_brewing->tab_pages[SCREEN_ACTIVE_BREWING_TAB_OVERALL]);
    screen_active_brewing_create_actions_page(active_brewing->tab_pages[SCREEN_ACTIVE_BREWING_TAB_ACTIONS]);
    screen_active_brewing_select_tab(active_brewing, SCREEN_ACTIVE_BREWING_TAB_OVERALL);
}

void screen_active_brewing_show_recipe(screen_active_brewing_t *active_brewing,
                                       const recipe_t *recipe)
{
    if (active_brewing == NULL || recipe == NULL || active_brewing->shown_recipe_id == recipe->id)
    {
        return;
    }

    lv_label_set_text(active_brewing->recipe_label, recipe->name);
    active_brewing->back_button_context.value = recipe->id;
    active_brewing->shown_recipe_id = recipe->id;
}
