#include "Screen_recipe_builder.h"

#include <string.h>

#include "UI_scroll.h"

#define SCREEN_RECIPE_BUILDER_PAD 8
#define SCREEN_RECIPE_BUILDER_ROW_WIDTH_PCT 98

typedef struct
{
    const char *title;
    const char *subtitle;
    const char *body;
} screen_recipe_builder_field_info_t;

static void screen_recipe_builder_set_static(lv_obj_t *object);
static lv_obj_t *screen_recipe_builder_create_header(lv_obj_t *parent,
                                                     screen_recipe_builder_t *builder);
static lv_obj_t *screen_recipe_builder_create_nav_button(lv_obj_t *parent,
                                                         const char *text,
                                                         lv_align_t align,
                                                         screen_recipe_builder_nav_context_t *context);
static lv_obj_t *screen_recipe_builder_create_menu_button(lv_obj_t *parent,
                                                          screen_recipe_builder_t *builder);
static lv_obj_t *screen_recipe_builder_create_field_row(lv_obj_t *parent,
                                                        const screen_recipe_builder_field_info_t *field_info,
                                                        screen_recipe_builder_field_context_t *context);
static lv_obj_t *screen_recipe_builder_create_disabled_save_button(lv_obj_t *parent);
static void screen_recipe_builder_select_field(screen_recipe_builder_t *builder,
                                               screen_recipe_builder_field_id_t field_id);
static void screen_recipe_builder_nav_event_cb(lv_event_t *event);
static void screen_recipe_builder_field_event_cb(lv_event_t *event);

static const screen_recipe_builder_field_info_t
    screen_recipe_builder_fields[SCREEN_RECIPE_BUILDER_FIELD_COUNT] = {
        [SCREEN_RECIPE_BUILDER_FIELD_NAME] = {
            "Name",
            "Recipe title",
            "Later this will open a text entry overlay. For now this screen only shows the "
            "shape of the recipe editing flow."},
        [SCREEN_RECIPE_BUILDER_FIELD_STYLE] = {
            "Style",
            "Beer type",
            "Later this can become a style picker shared by the embedded UI and web UI."},
        [SCREEN_RECIPE_BUILDER_FIELD_BATCH] = {
            "Batch",
            "Volume and targets",
            "Later this will edit batch size, strength, bitterness, and gravity metadata."},
        [SCREEN_RECIPE_BUILDER_FIELD_INGREDIENTS] = {
            "Ingredients",
            "Malt, hops, yeast",
            "Later this will open structured ingredient lists instead of free text."},
        [SCREEN_RECIPE_BUILDER_FIELD_BREWING] = {
            "Brewing",
            "Mash and boil steps",
            "Later this will edit brewing steps that can be validated before a brew starts."},
        [SCREEN_RECIPE_BUILDER_FIELD_FERMENTATION] = {
            "Fermentation",
            "Temperature and duration",
            "Later this will store fermentation guidance without giving the SOM hardware "
            "control over fermentation equipment."}};

/****************************************************************************************
 * @brief Make an object static so it cannot become a tiny scroll target.
 ****************************************************************************************/
static void screen_recipe_builder_set_static(lv_obj_t *object)
{
    if (object == NULL)
    {
        return;
    }

    lv_obj_clear_flag(object, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_OFF);
}

static lv_obj_t *screen_recipe_builder_create_header(lv_obj_t *parent,
                                                     screen_recipe_builder_t *builder)
{
    lv_obj_t *header;
    lv_obj_t *title;

    header = lv_obj_create(parent);
    screen_recipe_builder_set_static(header);
    lv_obj_set_width(header, lv_pct(100));
    lv_obj_set_height(header, 64);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_all(header, 0, 0);

    screen_recipe_builder_create_nav_button(header,
                                            "<",
                                            LV_ALIGN_LEFT_MID,
                                            &builder->back_button_context);

    title = lv_label_create(header);
    lv_label_set_text(title, "New Recipe");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xC8C8C8), 0);
    lv_obj_center(title);

    screen_recipe_builder_create_menu_button(header, builder);
    return header;
}

static lv_obj_t *screen_recipe_builder_create_nav_button(lv_obj_t *parent,
                                                         const char *text,
                                                         lv_align_t align,
                                                         screen_recipe_builder_nav_context_t *context)
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
    lv_obj_add_event_cb(button, screen_recipe_builder_nav_event_cb, LV_EVENT_CLICKED, context);

    label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);
    return button;
}

static lv_obj_t *screen_recipe_builder_create_menu_button(lv_obj_t *parent,
                                                          screen_recipe_builder_t *builder)
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
                        screen_recipe_builder_nav_event_cb,
                        LV_EVENT_CLICKED,
                        &builder->menu_button_context);

    for (offset_y = -7; offset_y <= 7; offset_y = (int8_t)(offset_y + 7))
    {
        line = lv_obj_create(button);
        screen_recipe_builder_set_static(line);
        lv_obj_set_size(line, 20, 2);
        lv_obj_set_style_bg_color(line, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_bg_opa(line, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(line, 0, 0);
        lv_obj_align(line, LV_ALIGN_CENTER, 0, offset_y);
    }

    return button;
}

/****************************************************************************************
 * @brief Create one local recipe-builder field row.
 ****************************************************************************************/
static lv_obj_t *screen_recipe_builder_create_field_row(lv_obj_t *parent,
                                                        const screen_recipe_builder_field_info_t *field_info,
                                                        screen_recipe_builder_field_context_t *context)
{
    lv_obj_t *button;
    lv_obj_t *title_label;
    lv_obj_t *subtitle_label;

    button = lv_button_create(parent);
    lv_obj_set_width(button, lv_pct(SCREEN_RECIPE_BUILDER_ROW_WIDTH_PCT));
    lv_obj_set_height(button, 58);
    lv_obj_set_style_align(button, LV_ALIGN_LEFT_MID, 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x282828), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x3B332D), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(button, lv_color_hex(0x343434), 0);
    lv_obj_set_style_border_width(button, 1, 0);
    lv_obj_set_style_radius(button, 0, 0);
    lv_obj_set_style_pad_all(button, 8, 0);
    lv_obj_add_event_cb(button, screen_recipe_builder_field_event_cb, LV_EVENT_CLICKED, context);

    title_label = lv_label_create(button);
    lv_label_set_text(title_label, field_info->title);
    lv_label_set_long_mode(title_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(title_label, lv_pct(100));
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_20, 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_LEFT, 0, 0);

    subtitle_label = lv_label_create(button);
    lv_label_set_text(subtitle_label, field_info->subtitle);
    lv_label_set_long_mode(subtitle_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(subtitle_label, lv_pct(100));
    lv_obj_set_style_text_color(subtitle_label, lv_color_hex(0xE67526), 0);
    lv_obj_align(subtitle_label, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    lv_obj_remove_flag(title_label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(subtitle_label, LV_OBJ_FLAG_CLICKABLE);
    return button;
}

static lv_obj_t *screen_recipe_builder_create_disabled_save_button(lv_obj_t *parent)
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
    lv_label_set_text(label, "SAVE LATER");
    lv_obj_set_style_text_color(label, lv_color_hex(0xC8C8C8), 0);
    lv_obj_center(label);
    return button;
}

static void screen_recipe_builder_select_field(screen_recipe_builder_t *builder,
                                               screen_recipe_builder_field_id_t field_id)
{
    const screen_recipe_builder_field_info_t *field_info;

    if (builder == NULL || field_id >= SCREEN_RECIPE_BUILDER_FIELD_COUNT)
    {
        return;
    }

    field_info = &screen_recipe_builder_fields[field_id];
    lv_label_set_text(builder->selected_title_label, field_info->title);
    lv_label_set_text(builder->selected_body_label, field_info->body);
    builder->selected_field_id = field_id;
}

static void screen_recipe_builder_nav_event_cb(lv_event_t *event)
{
    screen_recipe_builder_nav_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL || context->handler == NULL)
    {
        return;
    }

    context->handler(context->action, context->value, context->user_data);
}

static void screen_recipe_builder_field_event_cb(lv_event_t *event)
{
    screen_recipe_builder_field_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL)
    {
        return;
    }

    screen_recipe_builder_select_field(context->builder, context->field_id);
}

void screen_recipe_builder_init(screen_recipe_builder_t *builder,
                                ui_action_handler_t action_handler,
                                void *user_data)
{
    lv_obj_t *container;
    lv_obj_t *intro;
    lv_obj_t *list;
    uint32_t field_index;

    if (builder == NULL)
    {
        return;
    }

    memset(builder, 0, sizeof(*builder));
    builder->back_button_context.action = UI_ACTION_SHOW_RECIPES;
    builder->back_button_context.handler = action_handler;
    builder->back_button_context.user_data = user_data;
    builder->menu_button_context.action = UI_ACTION_SHOW_MENU;
    builder->menu_button_context.handler = action_handler;
    builder->menu_button_context.user_data = user_data;

    builder->screen = lv_obj_create(NULL);
    screen_recipe_builder_set_static(builder->screen);
    lv_obj_set_style_bg_color(builder->screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(builder->screen, LV_OPA_COVER, 0);

    container = lv_obj_create(builder->screen);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, SCREEN_RECIPE_BUILDER_PAD, 0);
    lv_obj_set_style_pad_row(container, 8, 0);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);

    screen_recipe_builder_create_header(container, builder);

    intro = lv_label_create(container);
    lv_label_set_text(intro, "Recipe fields");
    lv_obj_set_width(intro, lv_pct(100));
    lv_obj_set_style_text_color(intro, lv_color_hex(0xE67526), 0);

    builder->selected_title_label = lv_label_create(container);
    lv_label_set_text(builder->selected_title_label, "--");
    lv_label_set_long_mode(builder->selected_title_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(builder->selected_title_label, lv_pct(100));
    lv_obj_set_style_text_color(builder->selected_title_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(builder->selected_title_label, &lv_font_montserrat_20, 0);

    builder->selected_body_label = lv_label_create(container);
    lv_label_set_text(builder->selected_body_label, "--");
    lv_label_set_long_mode(builder->selected_body_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(builder->selected_body_label, lv_pct(100));
    lv_obj_set_style_text_color(builder->selected_body_label, lv_color_hex(0xC8C8C8), 0);

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

    for (field_index = 0U; field_index < SCREEN_RECIPE_BUILDER_FIELD_COUNT; ++field_index)
    {
        builder->field_contexts[field_index].field_id =
            (screen_recipe_builder_field_id_t)field_index;
        builder->field_contexts[field_index].builder = builder;
        screen_recipe_builder_create_field_row(list,
                                               &screen_recipe_builder_fields[field_index],
                                               &builder->field_contexts[field_index]);
    }

    screen_recipe_builder_create_disabled_save_button(container);
    screen_recipe_builder_select_field(builder, SCREEN_RECIPE_BUILDER_FIELD_NAME);
}
