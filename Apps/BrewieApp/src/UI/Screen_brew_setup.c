#include "Screen_brew_setup.h"

#include <string.h>

#define SCREEN_BREW_SETUP_PAD 8

typedef struct
{
    const char *title;
} screen_brew_setup_option_info_t;

static void screen_brew_setup_set_static(lv_obj_t *object);
static lv_obj_t *screen_brew_setup_create_header(lv_obj_t *parent, screen_brew_setup_t *setup);
static lv_obj_t *screen_brew_setup_create_nav_button(lv_obj_t *parent,
                                                     const char *text,
                                                     lv_align_t align,
                                                     screen_brew_setup_nav_context_t *context);
static lv_obj_t *screen_brew_setup_create_option_row(lv_obj_t *parent,
                                                     const screen_brew_setup_option_info_t *option_info,
                                                     screen_brew_setup_option_context_t *context,
                                                     lv_obj_t **check_fill);
static lv_obj_t *screen_brew_setup_create_disabled_start_button(lv_obj_t *parent);
static void screen_brew_setup_update_option_label(screen_brew_setup_t *setup,
                                                  screen_brew_setup_option_id_t option_id);
static void screen_brew_setup_nav_event_cb(lv_event_t *event);
static void screen_brew_setup_option_event_cb(lv_event_t *event);

static const screen_brew_setup_option_info_t screen_brew_setup_options[SCREEN_BREW_SETUP_OPTION_COUNT] = {
    [SCREEN_BREW_SETUP_OPTION_WATER_INLET] = {
        "Automatic Water Inlet"},
    [SCREEN_BREW_SETUP_OPTION_COOLING] = {
        "Automatic Cooling"}};

/****************************************************************************************
 * @brief Make an object static so it does not become an accidental scroll target.
 ****************************************************************************************/
static void screen_brew_setup_set_static(lv_obj_t *object)
{
    if (object == NULL)
    {
        return;
    }

    lv_obj_clear_flag(object, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_OFF);
}

/****************************************************************************************
 * @brief Create the old-style brew setup header.
 *
 * This mirrors the old Water Inlet style: a back button and centered title. It emits
 * navigation only. Starting a real brew must later route through app logic.
 ****************************************************************************************/
static lv_obj_t *screen_brew_setup_create_header(lv_obj_t *parent, screen_brew_setup_t *setup)
{
    lv_obj_t *header;
    lv_obj_t *title;

    header = lv_obj_create(parent);
    screen_brew_setup_set_static(header);
    lv_obj_set_width(header, lv_pct(100));
    lv_obj_set_height(header, 64);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_all(header, 0, 0);

    screen_brew_setup_create_nav_button(header, "<", LV_ALIGN_LEFT_MID, &setup->back_button_context);

    title = lv_label_create(header);
    lv_label_set_text(title, "First settings");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xC8C8C8), 0);
    lv_obj_center(title);

    return header;
}

static lv_obj_t *screen_brew_setup_create_nav_button(lv_obj_t *parent,
                                                     const char *text,
                                                     lv_align_t align,
                                                     screen_brew_setup_nav_context_t *context)
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
    lv_obj_add_event_cb(button, screen_brew_setup_nav_event_cb, LV_EVENT_CLICKED, context);

    label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);
    return button;
}

/****************************************************************************************
 * @brief Create one local brew option row.
 *
 * The old UI represented these choices as simple checkbox lines. Keep that shape here:
 * one large touch row, one checkbox marker, and one label. The value remains display-only
 * until future app-level safety checks are added.
 ****************************************************************************************/
static lv_obj_t *screen_brew_setup_create_option_row(lv_obj_t *parent,
                                                     const screen_brew_setup_option_info_t *option_info,
                                                     screen_brew_setup_option_context_t *context,
                                                     lv_obj_t **check_fill)
{
    lv_obj_t *button;
    lv_obj_t *check_box;
    lv_obj_t *title_label;

    button = lv_button_create(parent);
    lv_obj_set_width(button, lv_pct(100));
    lv_obj_set_height(button, 46);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x282828), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x3B332D), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(button, lv_color_hex(0x343434), 0);
    lv_obj_set_style_border_width(button, 1, 0);
    lv_obj_set_style_radius(button, 0, 0);
    lv_obj_set_style_pad_all(button, 8, 0);
    lv_obj_set_style_pad_column(button, 8, 0);
    lv_obj_set_layout(button, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(button, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(button, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_event_cb(button, screen_brew_setup_option_event_cb, LV_EVENT_CLICKED, context);

    check_box = lv_obj_create(button);
    screen_brew_setup_set_static(check_box);
    lv_obj_set_size(check_box, 24, 24);
    lv_obj_set_style_bg_color(check_box, lv_color_hex(0x151515), 0);
    lv_obj_set_style_bg_opa(check_box, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(check_box, lv_color_hex(0x55B047), 0);
    lv_obj_set_style_border_width(check_box, 2, 0);
    lv_obj_set_style_radius(check_box, 3, 0);
    lv_obj_set_style_pad_all(check_box, 0, 0);

    *check_fill = lv_obj_create(check_box);
    screen_brew_setup_set_static(*check_fill);
    lv_obj_set_size(*check_fill, 12, 12);
    lv_obj_set_style_bg_color(*check_fill, lv_color_hex(0x55B047), 0);
    lv_obj_set_style_bg_opa(*check_fill, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(*check_fill, 0, 0);
    lv_obj_set_style_radius(*check_fill, 2, 0);
    lv_obj_center(*check_fill);

    title_label = lv_label_create(button);
    lv_label_set_text(title_label, option_info->title);
    lv_label_set_long_mode(title_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(title_label, 200);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), 0);

    lv_obj_remove_flag(check_box, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(*check_fill, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(title_label, LV_OBJ_FLAG_CLICKABLE);
    return button;
}

static lv_obj_t *screen_brew_setup_create_disabled_start_button(lv_obj_t *parent)
{
    lv_obj_t *button;
    lv_obj_t *label;

    button = lv_button_create(parent);
    lv_obj_set_width(button, lv_pct(100));
    lv_obj_set_height(button, 48);
    lv_obj_set_style_bg_color(button, lv_color_hex(0xF47B32), 0);
    lv_obj_set_style_radius(button, 5, 0);
    lv_obj_add_state(button, LV_STATE_DISABLED);

    label = lv_label_create(button);
    lv_label_set_text(label, "START LATER");
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);
    return button;
}

static void screen_brew_setup_update_option_label(screen_brew_setup_t *setup,
                                                  screen_brew_setup_option_id_t option_id)
{
    lv_obj_t *check_fill;

    if (setup == NULL || option_id >= SCREEN_BREW_SETUP_OPTION_COUNT)
    {
        return;
    }

    check_fill = setup->option_check_fills[option_id];
    if (setup->option_enabled[option_id])
    {
        lv_obj_remove_flag(check_fill, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_add_flag(check_fill, LV_OBJ_FLAG_HIDDEN);
    }
}

static void screen_brew_setup_nav_event_cb(lv_event_t *event)
{
    screen_brew_setup_nav_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL || context->handler == NULL)
    {
        return;
    }

    context->handler(context->action, context->value, context->user_data);
}

static void screen_brew_setup_option_event_cb(lv_event_t *event)
{
    screen_brew_setup_option_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL || context->setup == NULL)
    {
        return;
    }

    context->setup->option_enabled[context->option_id] = !context->setup->option_enabled[context->option_id];
    screen_brew_setup_update_option_label(context->setup, context->option_id);
}

void screen_brew_setup_init(screen_brew_setup_t *setup,
                            ui_action_handler_t action_handler,
                            void *user_data)
{
    lv_obj_t *container;
    lv_obj_t *intro;
    lv_obj_t *checklist;
    uint32_t option_index;

    if (setup == NULL)
    {
        return;
    }

    memset(setup, 0, sizeof(*setup));
    setup->back_button_context.action = UI_ACTION_SHOW_RECIPE_DETAIL;
    setup->back_button_context.handler = action_handler;
    setup->back_button_context.user_data = user_data;
    setup->option_enabled[SCREEN_BREW_SETUP_OPTION_WATER_INLET] = true;
    setup->option_enabled[SCREEN_BREW_SETUP_OPTION_COOLING] = true;

    setup->screen = lv_obj_create(NULL);
    screen_brew_setup_set_static(setup->screen);
    lv_obj_set_style_bg_color(setup->screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(setup->screen, LV_OPA_COVER, 0);

    container = lv_obj_create(setup->screen);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, SCREEN_BREW_SETUP_PAD, 0);
    lv_obj_set_style_pad_row(container, 8, 0);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);

    screen_brew_setup_create_header(container, setup);

    intro = lv_label_create(container);
    lv_label_set_text(intro, "SET YOUR BREWING PARAMETERS");
    lv_obj_set_width(intro, lv_pct(100));
    lv_obj_set_style_text_color(intro, lv_color_hex(0xE67526), 0);

    setup->recipe_label = lv_label_create(container);
    lv_label_set_text(setup->recipe_label, "--");
    lv_label_set_long_mode(setup->recipe_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(setup->recipe_label, lv_pct(100));
    lv_obj_set_style_text_color(setup->recipe_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(setup->recipe_label, &lv_font_montserrat_20, 0);

    for (option_index = 0U; option_index < SCREEN_BREW_SETUP_OPTION_COUNT; ++option_index)
    {
        setup->option_contexts[option_index].option_id = (screen_brew_setup_option_id_t)option_index;
        setup->option_contexts[option_index].setup = setup;
        screen_brew_setup_create_option_row(container,
                                            &screen_brew_setup_options[option_index],
                                            &setup->option_contexts[option_index],
                                            &setup->option_check_fills[option_index]);
        screen_brew_setup_update_option_label(setup, (screen_brew_setup_option_id_t)option_index);
    }

    checklist = lv_obj_create(container);
    lv_obj_remove_style_all(checklist);
    screen_brew_setup_set_static(checklist);
    lv_obj_set_width(checklist, lv_pct(100));
    lv_obj_set_flex_grow(checklist, 1);

    screen_brew_setup_create_disabled_start_button(container);
}

void screen_brew_setup_show_recipe(screen_brew_setup_t *setup, const recipe_t *recipe)
{
    if (setup == NULL || recipe == NULL || setup->shown_recipe_id == recipe->id)
    {
        return;
    }

    lv_label_set_text(setup->recipe_label, recipe->name);
    setup->back_button_context.value = recipe->id;
    setup->shown_recipe_id = recipe->id;
}
