#include "Screen_manual.h"

#include <string.h>

#define SCREEN_MANUAL_PAD 8

typedef struct
{
    const char *title;
    const char *subtitle;
    const char *body;
} screen_manual_mode_info_t;

static void screen_manual_set_static(lv_obj_t *object);
static lv_obj_t *screen_manual_create_header(lv_obj_t *parent, screen_manual_t *manual);
static lv_obj_t *screen_manual_create_back_button(lv_obj_t *parent, screen_manual_t *manual);
static lv_obj_t *screen_manual_create_mode_row(lv_obj_t *parent,
                                               const screen_manual_mode_info_t *mode_info,
                                               screen_manual_mode_context_t *context);
static void screen_manual_select_mode(screen_manual_t *manual, screen_manual_mode_id_t mode_id);
static void screen_manual_back_event_cb(lv_event_t *event);
static void screen_manual_mode_event_cb(lv_event_t *event);

static const screen_manual_mode_info_t screen_manual_modes[SCREEN_MANUAL_MODE_COUNT] = {
    [SCREEN_MANUAL_MODE_SHORT_CLEAN] = {
        "Short Clean",
        "Quick rinse path",
        "Safe scaffold only. Later this should confirm water inlet, pump, heater, and drain "
        "interlocks before any MCU command is sent."},
    [SCREEN_MANUAL_MODE_SANITIZING_CLEAN] = {
        "Sanitizing Clean",
        "Cleaning agent cycle",
        "Safe scaffold only. Later this should guide the user through sanitizer preparation "
        "and then enter a guarded active-cleaning flow."},
    [SCREEN_MANUAL_MODE_FULL_CLEAN] = {
        "Full Clean",
        "Long maintenance cycle",
        "Safe scaffold only. Later this should combine water intake, heating, circulation, "
        "and drain steps with clear progress and stop handling."},
    [SCREEN_MANUAL_MODE_DRAIN_AFTER_BREW] = {
        "Drain After Brew",
        "Post-brew drain",
        "Safe scaffold only. Later this should ask for drain parameters and verify that the "
        "machine is in a state where draining is allowed."},
    [SCREEN_MANUAL_MODE_FULL_DRAIN] = {
        "Full Drain",
        "Empty machine paths",
        "Safe scaffold only. Later this should be a service action with confirmation and "
        "clear active-process feedback."},
    [SCREEN_MANUAL_MODE_UNCLOGGING] = {
        "Unclogging",
        "Guided service helper",
        "Safe scaffold only. Later this should show a step-by-step guide before any active "
        "pump or valve behavior is enabled."}};

/****************************************************************************************
 * @brief Make an object static so it cannot accidentally become a small scroll target.
 ****************************************************************************************/
static void screen_manual_set_static(lv_obj_t *object)
{
    if (object == NULL)
    {
        return;
    }

    lv_obj_clear_flag(object, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_OFF);
}

/****************************************************************************************
 * @brief Create the old-style header for Manual/Cleaning.
 *
 * The left button returns to the top-level menu. Starting actual clean/drain flows will
 * later require app-level safety routing, so the header does not emit hardware actions.
 ****************************************************************************************/
static lv_obj_t *screen_manual_create_header(lv_obj_t *parent, screen_manual_t *manual)
{
    lv_obj_t *header;
    lv_obj_t *title;

    header = lv_obj_create(parent);
    screen_manual_set_static(header);
    lv_obj_set_width(header, lv_pct(100));
    lv_obj_set_height(header, 64);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_all(header, 0, 0);

    screen_manual_create_back_button(header, manual);

    title = lv_label_create(header);
    lv_label_set_text(title, "Manual");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xC8C8C8), 0);
    lv_obj_center(title);

    return header;
}

static lv_obj_t *screen_manual_create_back_button(lv_obj_t *parent, screen_manual_t *manual)
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
    lv_obj_add_event_cb(button, screen_manual_back_event_cb, LV_EVENT_CLICKED, &manual->back_button_context);

    label = lv_label_create(button);
    lv_label_set_text(label, "<");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);

    return button;
}

/****************************************************************************************
 * @brief Create one selectable clean/drain/unclog row.
 *
 * Rows update the local detail panel only. The app can later replace this with a guarded
 * confirmation flow without changing how the top-level Manual screen is reached.
 ****************************************************************************************/
static lv_obj_t *screen_manual_create_mode_row(lv_obj_t *parent,
                                               const screen_manual_mode_info_t *mode_info,
                                               screen_manual_mode_context_t *context)
{
    lv_obj_t *button;
    lv_obj_t *title_label;
    lv_obj_t *subtitle_label;

    button = lv_button_create(parent);
    lv_obj_set_width(button, lv_pct(95));
    lv_obj_set_height(button, 58);
    lv_obj_set_style_align(button, LV_ALIGN_LEFT_MID, 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x282828), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x3B332D), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(button, lv_color_hex(0x343434), 0);
    lv_obj_set_style_border_width(button, 1, 0);
    lv_obj_set_style_radius(button, 0, 0);
    lv_obj_set_style_pad_all(button, 8, 0);
    lv_obj_add_event_cb(button, screen_manual_mode_event_cb, LV_EVENT_CLICKED, context);

    title_label = lv_label_create(button);
    lv_label_set_text(title_label, mode_info->title);
    lv_label_set_long_mode(title_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(title_label, lv_pct(100));
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_20, 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_LEFT, 0, 0);

    subtitle_label = lv_label_create(button);
    lv_label_set_text(subtitle_label, mode_info->subtitle);
    lv_label_set_long_mode(subtitle_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(subtitle_label, lv_pct(100));
    lv_obj_set_style_text_color(subtitle_label, lv_color_hex(0xE67526), 0);
    lv_obj_align(subtitle_label, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    lv_obj_remove_flag(title_label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(subtitle_label, LV_OBJ_FLAG_CLICKABLE);
    return button;
}

static void screen_manual_select_mode(screen_manual_t *manual, screen_manual_mode_id_t mode_id)
{
    const screen_manual_mode_info_t *mode_info;

    if (manual == NULL || mode_id >= SCREEN_MANUAL_MODE_COUNT)
    {
        return;
    }

    mode_info = &screen_manual_modes[mode_id];
    lv_label_set_text(manual->selected_title_label, mode_info->title);
    lv_label_set_text(manual->selected_body_label, mode_info->body);
    manual->selected_mode_id = mode_id;
}

static void screen_manual_back_event_cb(lv_event_t *event)
{
    screen_manual_nav_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL || context->handler == NULL)
    {
        return;
    }

    context->handler(context->action, 0U, context->user_data);
}

static void screen_manual_mode_event_cb(lv_event_t *event)
{
    screen_manual_mode_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL)
    {
        return;
    }

    screen_manual_select_mode(context->manual, context->mode_id);
}

void screen_manual_init(screen_manual_t *manual, ui_action_handler_t action_handler, void *user_data)
{
    lv_obj_t *container;
    lv_obj_t *intro;
    lv_obj_t *list;
    uint32_t mode_index;

    if (manual == NULL)
    {
        return;
    }

    memset(manual, 0, sizeof(*manual));
    manual->back_button_context.action = UI_ACTION_SHOW_MENU;
    manual->back_button_context.handler = action_handler;
    manual->back_button_context.user_data = user_data;

    manual->screen = lv_obj_create(NULL);
    screen_manual_set_static(manual->screen);
    lv_obj_set_style_bg_color(manual->screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(manual->screen, LV_OPA_COVER, 0);

    container = lv_obj_create(manual->screen);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, SCREEN_MANUAL_PAD, 0);
    lv_obj_set_style_pad_row(container, 8, 0);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);

    screen_manual_create_header(container, manual);

    intro = lv_label_create(container);
    lv_label_set_text(intro, "Clean, drain, and service helpers");
    lv_obj_set_width(intro, lv_pct(100));
    lv_obj_set_style_text_color(intro, lv_color_hex(0xE67526), 0);

    manual->selected_title_label = lv_label_create(container);
    lv_label_set_text(manual->selected_title_label, "--");
    lv_label_set_long_mode(manual->selected_title_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(manual->selected_title_label, lv_pct(100));
    lv_obj_set_style_text_color(manual->selected_title_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(manual->selected_title_label, &lv_font_montserrat_20, 0);

    manual->selected_body_label = lv_label_create(container);
    lv_label_set_text(manual->selected_body_label, "--");
    lv_label_set_long_mode(manual->selected_body_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(manual->selected_body_label, lv_pct(100));
    lv_obj_set_style_text_color(manual->selected_body_label, lv_color_hex(0xC8C8C8), 0);

    list = lv_obj_create(container);
    lv_obj_set_width(list, lv_pct(100));
    lv_obj_set_flex_grow(list, 1);
    lv_obj_set_style_bg_color(list, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(list, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(list, 0, 0);
    lv_obj_set_style_pad_all(list, 0, 0);
    lv_obj_set_style_pad_row(list, 8, 0);
    lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_layout(list, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);

    for (mode_index = 0U; mode_index < SCREEN_MANUAL_MODE_COUNT; ++mode_index)
    {
        manual->mode_contexts[mode_index].mode_id = (screen_manual_mode_id_t)mode_index;
        manual->mode_contexts[mode_index].manual = manual;
        screen_manual_create_mode_row(list,
                                      &screen_manual_modes[mode_index],
                                      &manual->mode_contexts[mode_index]);
    }

    screen_manual_select_mode(manual, SCREEN_MANUAL_MODE_SHORT_CLEAN);
}
