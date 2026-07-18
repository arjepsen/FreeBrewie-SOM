#include "Screen_brew_checklist.h"

#include <string.h>

#define SCREEN_BREW_CHECKLIST_PAD 8

typedef struct
{
    const char *title;
} screen_brew_checklist_item_info_t;

static void screen_brew_checklist_set_static(lv_obj_t *object);
static lv_obj_t *screen_brew_checklist_create_header(lv_obj_t *parent,
                                                     screen_brew_checklist_t *checklist);
static lv_obj_t *screen_brew_checklist_create_back_button(lv_obj_t *parent,
                                                          screen_brew_checklist_t *checklist);
static lv_obj_t *screen_brew_checklist_create_item_row(lv_obj_t *parent,
                                                       const screen_brew_checklist_item_info_t *item_info,
                                                       screen_brew_checklist_item_context_t *context,
                                                       lv_obj_t **check_fill);
static lv_obj_t *screen_brew_checklist_create_start_button(lv_obj_t *parent,
                                                           screen_brew_checklist_nav_context_t *context);
static void screen_brew_checklist_update_item(screen_brew_checklist_t *checklist,
                                              screen_brew_checklist_item_id_t item_id);
static void screen_brew_checklist_back_event_cb(lv_event_t *event);
static void screen_brew_checklist_item_event_cb(lv_event_t *event);

static const screen_brew_checklist_item_info_t
    screen_brew_checklist_items[SCREEN_BREW_CHECKLIST_ITEM_COUNT] = {
        [SCREEN_BREW_CHECKLIST_ITEM_RECIPE] = {
            "Recipe reviewed"},
        [SCREEN_BREW_CHECKLIST_ITEM_WATER] = {
            "Water path prepared"},
        [SCREEN_BREW_CHECKLIST_ITEM_COOLING] = {
            "Cooling water ready"},
        [SCREEN_BREW_CHECKLIST_ITEM_DRAIN] = {
            "Drain path clear"}};

/****************************************************************************************
 * @brief Make an object static so list rows remain predictable touch targets.
 ****************************************************************************************/
static void screen_brew_checklist_set_static(lv_obj_t *object)
{
    if (object == NULL)
    {
        return;
    }

    lv_obj_clear_flag(object, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_OFF);
}

static lv_obj_t *screen_brew_checklist_create_header(lv_obj_t *parent,
                                                     screen_brew_checklist_t *checklist)
{
    lv_obj_t *header;
    lv_obj_t *title;

    header = lv_obj_create(parent);
    screen_brew_checklist_set_static(header);
    lv_obj_set_width(header, lv_pct(100));
    lv_obj_set_height(header, 64);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_all(header, 0, 0);

    screen_brew_checklist_create_back_button(header, checklist);

    title = lv_label_create(header);
    lv_label_set_text(title, "Checklist");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xC8C8C8), 0);
    lv_obj_center(title);

    return header;
}

static lv_obj_t *screen_brew_checklist_create_back_button(lv_obj_t *parent,
                                                          screen_brew_checklist_t *checklist)
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
                        screen_brew_checklist_back_event_cb,
                        LV_EVENT_CLICKED,
                        &checklist->back_button_context);

    label = lv_label_create(button);
    lv_label_set_text(label, "<");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);
    return button;
}

/****************************************************************************************
 * @brief Create one local checklist row.
 *
 * These rows are an early UI scaffold. They are not proof that the real machine condition
 * is true; final validation must come from app and MCU state before active brewing starts.
 ****************************************************************************************/
static lv_obj_t *screen_brew_checklist_create_item_row(lv_obj_t *parent,
                                                       const screen_brew_checklist_item_info_t *item_info,
                                                       screen_brew_checklist_item_context_t *context,
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
    lv_obj_add_event_cb(button, screen_brew_checklist_item_event_cb, LV_EVENT_CLICKED, context);

    check_box = lv_obj_create(button);
    screen_brew_checklist_set_static(check_box);
    lv_obj_set_size(check_box, 24, 24);
    lv_obj_set_style_bg_color(check_box, lv_color_hex(0x151515), 0);
    lv_obj_set_style_bg_opa(check_box, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(check_box, lv_color_hex(0x55B047), 0);
    lv_obj_set_style_border_width(check_box, 2, 0);
    lv_obj_set_style_radius(check_box, 3, 0);
    lv_obj_set_style_pad_all(check_box, 0, 0);

    *check_fill = lv_obj_create(check_box);
    screen_brew_checklist_set_static(*check_fill);
    lv_obj_set_size(*check_fill, 12, 12);
    lv_obj_set_style_bg_color(*check_fill, lv_color_hex(0x55B047), 0);
    lv_obj_set_style_bg_opa(*check_fill, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(*check_fill, 0, 0);
    lv_obj_set_style_radius(*check_fill, 2, 0);
    lv_obj_center(*check_fill);

    title_label = lv_label_create(button);
    lv_label_set_text(title_label, item_info->title);
    lv_label_set_long_mode(title_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(title_label, 200);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), 0);

    lv_obj_remove_flag(check_box, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(*check_fill, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(title_label, LV_OBJ_FLAG_CLICKABLE);
    return button;
}

static lv_obj_t *screen_brew_checklist_create_start_button(lv_obj_t *parent,
                                                           screen_brew_checklist_nav_context_t *context)
{
    lv_obj_t *button;
    lv_obj_t *label;

    button = lv_button_create(parent);
    lv_obj_set_width(button, lv_pct(100));
    lv_obj_set_height(button, 48);
    lv_obj_set_style_bg_color(button, lv_color_hex(0xF47B32), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0xC85F22), LV_STATE_PRESSED);
    lv_obj_set_style_radius(button, 5, 0);
    lv_obj_add_event_cb(button, screen_brew_checklist_back_event_cb, LV_EVENT_CLICKED, context);

    label = lv_label_create(button);
    lv_label_set_text(label, "START");
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);
    return button;
}

static void screen_brew_checklist_update_item(screen_brew_checklist_t *checklist,
                                              screen_brew_checklist_item_id_t item_id)
{
    lv_obj_t *check_fill;

    if (checklist == NULL || item_id >= SCREEN_BREW_CHECKLIST_ITEM_COUNT)
    {
        return;
    }

    check_fill = checklist->item_check_fills[item_id];
    if (checklist->item_checked[item_id])
    {
        lv_obj_remove_flag(check_fill, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_add_flag(check_fill, LV_OBJ_FLAG_HIDDEN);
    }
}

static void screen_brew_checklist_back_event_cb(lv_event_t *event)
{
    screen_brew_checklist_nav_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL || context->handler == NULL)
    {
        return;
    }

    context->handler(context->action, context->value, context->user_data);
}

static void screen_brew_checklist_item_event_cb(lv_event_t *event)
{
    screen_brew_checklist_item_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL || context->checklist == NULL)
    {
        return;
    }

    context->checklist->item_checked[context->item_id] =
        !context->checklist->item_checked[context->item_id];
    screen_brew_checklist_update_item(context->checklist, context->item_id);
}

void screen_brew_checklist_init(screen_brew_checklist_t *checklist,
                                ui_action_handler_t action_handler,
                                void *user_data)
{
    lv_obj_t *container;
    lv_obj_t *intro;
    lv_obj_t *spacer;
    uint32_t item_index;

    if (checklist == NULL)
    {
        return;
    }

    memset(checklist, 0, sizeof(*checklist));
    checklist->back_button_context.action = UI_ACTION_SHOW_BREW_SETUP;
    checklist->back_button_context.handler = action_handler;
    checklist->back_button_context.user_data = user_data;
    checklist->start_button_context.action = UI_ACTION_SHOW_ACTIVE_BREWING;
    checklist->start_button_context.handler = action_handler;
    checklist->start_button_context.user_data = user_data;
    for (item_index = 0U; item_index < SCREEN_BREW_CHECKLIST_ITEM_COUNT; ++item_index)
    {
        checklist->item_checked[item_index] = true;
    }

    checklist->screen = lv_obj_create(NULL);
    screen_brew_checklist_set_static(checklist->screen);
    lv_obj_set_style_bg_color(checklist->screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(checklist->screen, LV_OPA_COVER, 0);

    container = lv_obj_create(checklist->screen);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, SCREEN_BREW_CHECKLIST_PAD, 0);
    lv_obj_set_style_pad_row(container, 8, 0);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);

    screen_brew_checklist_create_header(container, checklist);

    intro = lv_label_create(container);
    lv_label_set_text(intro, "CHECK BEFORE BREWING");
    lv_obj_set_width(intro, lv_pct(100));
    lv_obj_set_style_text_color(intro, lv_color_hex(0xE67526), 0);

    checklist->recipe_label = lv_label_create(container);
    lv_label_set_text(checklist->recipe_label, "--");
    lv_label_set_long_mode(checklist->recipe_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(checklist->recipe_label, lv_pct(100));
    lv_obj_set_style_text_color(checklist->recipe_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(checklist->recipe_label, &lv_font_montserrat_20, 0);

    for (item_index = 0U; item_index < SCREEN_BREW_CHECKLIST_ITEM_COUNT; ++item_index)
    {
        checklist->item_contexts[item_index].item_id = (screen_brew_checklist_item_id_t)item_index;
        checklist->item_contexts[item_index].checklist = checklist;
        screen_brew_checklist_create_item_row(container,
                                              &screen_brew_checklist_items[item_index],
                                              &checklist->item_contexts[item_index],
                                              &checklist->item_check_fills[item_index]);
        screen_brew_checklist_update_item(checklist, (screen_brew_checklist_item_id_t)item_index);
    }

    spacer = lv_obj_create(container);
    lv_obj_remove_style_all(spacer);
    screen_brew_checklist_set_static(spacer);
    lv_obj_set_width(spacer, lv_pct(100));
    lv_obj_set_flex_grow(spacer, 1);

    screen_brew_checklist_create_start_button(container, &checklist->start_button_context);
}

void screen_brew_checklist_show_recipe(screen_brew_checklist_t *checklist, const recipe_t *recipe)
{
    if (checklist == NULL || recipe == NULL || checklist->shown_recipe_id == recipe->id)
    {
        return;
    }

    lv_label_set_text(checklist->recipe_label, recipe->name);
    checklist->back_button_context.value = recipe->id;
    checklist->start_button_context.value = recipe->id;
    checklist->shown_recipe_id = recipe->id;
}
