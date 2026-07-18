#include "Screen_menu.h"

#include <string.h>

static lv_obj_t *screen_menu_create_row(lv_obj_t *parent,
                                        const char *title,
                                        const char *subtitle,
                                        ui_action_t action,
                                        screen_menu_button_context_t *context,
                                        ui_action_handler_t action_handler,
                                        void *user_data);
static void screen_menu_button_event_cb(lv_event_t *event);

static lv_obj_t *screen_menu_create_row(lv_obj_t *parent,
                                        const char *title,
                                        const char *subtitle,
                                        ui_action_t action,
                                        screen_menu_button_context_t *context,
                                        ui_action_handler_t action_handler,
                                        void *user_data)
{
    lv_obj_t *button;
    lv_obj_t *title_label;
    lv_obj_t *subtitle_label;

    if (parent == NULL || context == NULL)
    {
        return NULL;
    }

    context->action = action;
    context->handler = action_handler;
    context->user_data = user_data;

    button = lv_button_create(parent);
    lv_obj_set_width(button, lv_pct(100));
    lv_obj_set_height(button, 58);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x3B332D), LV_STATE_PRESSED);
    lv_obj_set_style_border_color(button, lv_color_hex(0x38322D), 0);
    lv_obj_set_style_border_width(button, 1, 0);
    lv_obj_set_style_radius(button, 0, 0);
    lv_obj_set_style_pad_left(button, 12, 0);
    lv_obj_set_style_pad_right(button, 12, 0);
    lv_obj_set_style_pad_top(button, 6, 0);
    lv_obj_set_style_pad_bottom(button, 6, 0);
    lv_obj_add_event_cb(button, screen_menu_button_event_cb, LV_EVENT_CLICKED, context);

    /*
     * LVGL buttons normally use label children for text. Keep those children passive and
     * positioned directly inside the button, so the row behaves like one large hit target.
     */
    title_label = lv_label_create(button);
    lv_label_set_text(title_label, title);
    lv_label_set_long_mode(title_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(title_label, lv_pct(100));
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_20, 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_remove_flag(title_label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(title_label, LV_OBJ_FLAG_SCROLLABLE);

    subtitle_label = lv_label_create(button);
    lv_label_set_text(subtitle_label, subtitle);
    lv_label_set_long_mode(subtitle_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(subtitle_label, lv_pct(100));
    lv_obj_set_style_text_color(subtitle_label, lv_color_hex(0xE67526), 0);
    lv_obj_set_style_text_font(subtitle_label, &lv_font_montserrat_14, 0);
    lv_obj_align(subtitle_label, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_remove_flag(subtitle_label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(subtitle_label, LV_OBJ_FLAG_SCROLLABLE);

    return button;
}

static void screen_menu_button_event_cb(lv_event_t *event)
{
    screen_menu_button_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL || context->handler == NULL)
    {
        return;
    }

    context->handler(context->action, context->value, context->user_data);
}

void screen_menu_init(screen_menu_t *menu, ui_action_handler_t action_handler, void *user_data)
{
    lv_obj_t *container;
    lv_obj_t *title;
    lv_obj_t *subtitle;
    lv_obj_t *spacer;

    if (menu == NULL)
    {
        return;
    }

    memset(menu, 0, sizeof(*menu));

    menu->screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(menu->screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(menu->screen, LV_OPA_COVER, 0);
    lv_obj_clear_flag(menu->screen, LV_OBJ_FLAG_SCROLLABLE);

    container = lv_obj_create(menu->screen);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 8, 0);
    lv_obj_set_style_pad_row(container, 7, 0);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);

    title = lv_label_create(container);
    lv_label_set_text(title, "Main Menu");
    lv_obj_set_width(title, lv_pct(100));
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);

    subtitle = lv_label_create(container);
    lv_label_set_text(subtitle, "FreeBrewie");
    lv_obj_set_width(subtitle, lv_pct(100));
    lv_obj_set_style_text_align(subtitle, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0xE67526), 0);
    lv_obj_set_style_text_font(subtitle, &lv_font_montserrat_14, 0);

    screen_menu_create_row(container,
                           "HOME",
                           "Machine overview",
                           UI_ACTION_SHOW_HOME,
                           &menu->home_context,
                           action_handler,
                           user_data);
    screen_menu_create_row(container,
                           "RECIPES",
                           "Choose or prepare recipe",
                           UI_ACTION_SHOW_RECIPES,
                           &menu->recipes_context,
                           action_handler,
                           user_data);
    screen_menu_create_row(container,
                           "MANUAL / CLEANING",
                           "Safe service flows later",
                           UI_ACTION_SHOW_MANUAL,
                           &menu->manual_context,
                           action_handler,
                           user_data);
    screen_menu_create_row(container,
                           "SETTINGS",
                           "Display, touch, system",
                           UI_ACTION_SHOW_SETTINGS,
                           &menu->settings_context,
                           action_handler,
                           user_data);
    screen_menu_create_row(container,
                           "STATUS",
                           "Comms and diagnostics",
                           UI_ACTION_SHOW_STATUS,
                           &menu->status_context,
                           action_handler,
                           user_data);

    spacer = lv_obj_create(container);
    lv_obj_remove_style_all(spacer);
    lv_obj_set_width(spacer, lv_pct(100));
    lv_obj_set_flex_grow(spacer, 1);
}
