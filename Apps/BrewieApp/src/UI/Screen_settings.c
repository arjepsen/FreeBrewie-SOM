#include "Screen_settings.h"

#include <string.h>

#include "UI_scroll.h"

#define SCREEN_SETTINGS_PAD 8
#define SCREEN_SETTINGS_ROW_WIDTH_PCT 98

typedef struct
{
    const char *title;
    const char *subtitle;
    const char *body;
} screen_settings_category_info_t;

static void screen_settings_set_static(lv_obj_t *object);
static lv_obj_t *screen_settings_create_header(lv_obj_t *parent, screen_settings_t *settings);
static lv_obj_t *screen_settings_create_back_button(lv_obj_t *parent, screen_settings_t *settings);
static lv_obj_t *screen_settings_create_category_row(lv_obj_t *parent,
                                                     const screen_settings_category_info_t *category_info,
                                                     screen_settings_category_context_t *context);
static void screen_settings_select_category(screen_settings_t *settings,
                                            screen_settings_category_id_t category_id);
static void screen_settings_back_event_cb(lv_event_t *event);
static void screen_settings_category_event_cb(lv_event_t *event);

static const screen_settings_category_info_t screen_settings_categories[SCREEN_SETTINGS_CATEGORY_COUNT] = {
    [SCREEN_SETTINGS_CATEGORY_WIFI] = {
        "WiFi",
        "Network connection",
        "Safe scaffold only. Later this should show available networks and route changes "
        "through a system/network service instead of directly changing state from UI code."},
    [SCREEN_SETTINGS_CATEGORY_UNITS] = {
        "Units",
        "Temperature and volume",
        "Safe scaffold only. Later this should edit persistent display units and keep raw "
        "machine values separate from user-facing formatting."},
    [SCREEN_SETTINGS_CATEGORY_TIME] = {
        "Time",
        "Clock and timezone",
        "Safe scaffold only. Later this should expose clock/timezone facts without blocking "
        "the UI or assuming network time is always available."},
    [SCREEN_SETTINGS_CATEGORY_WATER] = {
        "Water Settings",
        "Water calibration defaults",
        "Safe scaffold only. Later this should route changes through app-level validation "
        "because water settings can affect brewing and cleaning behavior."},
    [SCREEN_SETTINGS_CATEGORY_CALIBRATION] = {
        "Calibration",
        "Guided machine setup",
        "Safe scaffold only. Later this should become a guided flow with clear prerequisites "
        "and no direct hardware action from the screen layer."},
    [SCREEN_SETTINGS_CATEGORY_LANGUAGE] = {
        "Language",
        "Interface language",
        "Safe scaffold only. Later this should select text resources while keeping screen "
        "layout robust for longer translated labels."},
    [SCREEN_SETTINGS_CATEGORY_ABOUT] = {
        "About",
        "Version and device facts",
        "Safe scaffold only. Later this should show app version, image/build facts, and "
        "service information."}};

/****************************************************************************************
 * @brief Make an object static so list rows remain predictable touch targets.
 ****************************************************************************************/
static void screen_settings_set_static(lv_obj_t *object)
{
    if (object == NULL)
    {
        return;
    }

    lv_obj_clear_flag(object, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_OFF);
}

static lv_obj_t *screen_settings_create_header(lv_obj_t *parent, screen_settings_t *settings)
{
    lv_obj_t *header;
    lv_obj_t *title;

    header = lv_obj_create(parent);
    screen_settings_set_static(header);
    lv_obj_set_width(header, lv_pct(100));
    lv_obj_set_height(header, 64);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_all(header, 0, 0);

    screen_settings_create_back_button(header, settings);

    title = lv_label_create(header);
    lv_label_set_text(title, "Settings");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xC8C8C8), 0);
    lv_obj_center(title);

    return header;
}

static lv_obj_t *screen_settings_create_back_button(lv_obj_t *parent, screen_settings_t *settings)
{
    lv_obj_t *button;
    lv_obj_t *label;

    button = lv_button_create(parent);
    lv_obj_set_size(button, 42, 42);
    lv_obj_align(button, LV_ALIGN_LEFT_MID, 2, 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x343434), LV_STATE_PRESSED);
    lv_obj_set_style_border_width(button, 0, 0);
    lv_obj_set_style_radius(button, 4, 0);
    lv_obj_add_event_cb(button,
                        screen_settings_back_event_cb,
                        LV_EVENT_CLICKED,
                        &settings->back_button_context);

    label = lv_label_create(button);
    lv_label_set_text(label, "<");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);

    return button;
}

/****************************************************************************************
 * @brief Create one selectable settings-category row.
 *
 * Settings rows are local selectors for now. Real persistence, networking, or calibration
 * behavior must be added below the UI layer once the app has safe routing for it.
 ****************************************************************************************/
static lv_obj_t *screen_settings_create_category_row(lv_obj_t *parent,
                                                     const screen_settings_category_info_t *category_info,
                                                     screen_settings_category_context_t *context)
{
    lv_obj_t *button;
    lv_obj_t *title_label;
    lv_obj_t *subtitle_label;

    button = lv_button_create(parent);
    lv_obj_set_width(button, lv_pct(SCREEN_SETTINGS_ROW_WIDTH_PCT));
    lv_obj_set_height(button, 58);
    lv_obj_set_style_align(button, LV_ALIGN_LEFT_MID, 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x282828), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x3B332D), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(button, lv_color_hex(0x343434), 0);
    lv_obj_set_style_border_width(button, 1, 0);
    lv_obj_set_style_radius(button, 0, 0);
    lv_obj_set_style_pad_all(button, 8, 0);
    lv_obj_add_event_cb(button, screen_settings_category_event_cb, LV_EVENT_CLICKED, context);

    title_label = lv_label_create(button);
    lv_label_set_text(title_label, category_info->title);
    lv_label_set_long_mode(title_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(title_label, lv_pct(100));
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_20, 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_LEFT, 0, 0);

    subtitle_label = lv_label_create(button);
    lv_label_set_text(subtitle_label, category_info->subtitle);
    lv_label_set_long_mode(subtitle_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(subtitle_label, lv_pct(100));
    lv_obj_set_style_text_color(subtitle_label, lv_color_hex(0xE67526), 0);
    lv_obj_align(subtitle_label, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    lv_obj_remove_flag(title_label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(subtitle_label, LV_OBJ_FLAG_CLICKABLE);
    return button;
}

static void screen_settings_select_category(screen_settings_t *settings,
                                            screen_settings_category_id_t category_id)
{
    const screen_settings_category_info_t *category_info;

    if (settings == NULL || category_id >= SCREEN_SETTINGS_CATEGORY_COUNT)
    {
        return;
    }

    category_info = &screen_settings_categories[category_id];
    lv_label_set_text(settings->selected_title_label, category_info->title);
    lv_label_set_text(settings->selected_body_label, category_info->body);
    settings->selected_category_id = category_id;
}

static void screen_settings_back_event_cb(lv_event_t *event)
{
    screen_settings_nav_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL || context->handler == NULL)
    {
        return;
    }

    context->handler(context->action, 0U, context->user_data);
}

static void screen_settings_category_event_cb(lv_event_t *event)
{
    screen_settings_category_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL)
    {
        return;
    }

    screen_settings_select_category(context->settings, context->category_id);
}

void screen_settings_init(screen_settings_t *settings, ui_action_handler_t action_handler, void *user_data)
{
    lv_obj_t *container;
    lv_obj_t *intro;
    lv_obj_t *list;
    uint32_t category_index;

    if (settings == NULL)
    {
        return;
    }

    memset(settings, 0, sizeof(*settings));
    settings->back_button_context.action = UI_ACTION_SHOW_MENU;
    settings->back_button_context.handler = action_handler;
    settings->back_button_context.user_data = user_data;

    settings->screen = lv_obj_create(NULL);
    screen_settings_set_static(settings->screen);
    lv_obj_set_style_bg_color(settings->screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(settings->screen, LV_OPA_COVER, 0);

    container = lv_obj_create(settings->screen);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, SCREEN_SETTINGS_PAD, 0);
    lv_obj_set_style_pad_row(container, 8, 0);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);

    screen_settings_create_header(container, settings);

    intro = lv_label_create(container);
    lv_label_set_text(intro, "System setup and appliance information");
    lv_obj_set_width(intro, lv_pct(100));
    lv_obj_set_style_text_color(intro, lv_color_hex(0xE67526), 0);

    settings->selected_title_label = lv_label_create(container);
    lv_label_set_text(settings->selected_title_label, "--");
    lv_label_set_long_mode(settings->selected_title_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(settings->selected_title_label, lv_pct(100));
    lv_obj_set_style_text_color(settings->selected_title_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(settings->selected_title_label, &lv_font_montserrat_20, 0);

    settings->selected_body_label = lv_label_create(container);
    lv_label_set_text(settings->selected_body_label, "--");
    lv_label_set_long_mode(settings->selected_body_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(settings->selected_body_label, lv_pct(100));
    lv_obj_set_style_text_color(settings->selected_body_label, lv_color_hex(0xC8C8C8), 0);

    list = lv_obj_create(container);
    lv_obj_set_width(list, lv_pct(100));
    lv_obj_set_flex_grow(list, 1);
    lv_obj_set_style_bg_color(list, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(list, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(list, 0, 0);
    lv_obj_set_style_pad_all(list, 0, 0);
    lv_obj_set_style_pad_row(list, 8, 0);
    ui_scroll_apply_gutter(list);
    lv_obj_set_layout(list, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);

    for (category_index = 0U; category_index < SCREEN_SETTINGS_CATEGORY_COUNT; ++category_index)
    {
        settings->category_contexts[category_index].category_id =
            (screen_settings_category_id_t)category_index;
        settings->category_contexts[category_index].settings = settings;
        screen_settings_create_category_row(list,
                                            &screen_settings_categories[category_index],
                                            &settings->category_contexts[category_index]);
    }

    screen_settings_select_category(settings, SCREEN_SETTINGS_CATEGORY_WIFI);
}
