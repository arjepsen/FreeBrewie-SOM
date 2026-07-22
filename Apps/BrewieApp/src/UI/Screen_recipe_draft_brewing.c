#include "Screen_recipe_draft_brewing.h"

#include <stdio.h>
#include <string.h>

#include "UI_scroll.h"

static void screen_recipe_draft_brewing_set_static(lv_obj_t *object);
static lv_obj_t *screen_recipe_draft_brewing_create_header(lv_obj_t *parent,
                                                           screen_recipe_draft_brewing_t *brewing);
static lv_obj_t *screen_recipe_draft_brewing_create_nav_button(
    lv_obj_t *parent,
    const char *text,
    lv_align_t align,
    screen_recipe_draft_brewing_nav_context_t *context);
static lv_obj_t *screen_recipe_draft_brewing_create_group(lv_obj_t *parent, const char *caption);
static lv_obj_t *screen_recipe_draft_brewing_create_value_row(lv_obj_t *parent,
                                                              const char *label_text,
                                                              const char *value_text);
static lv_obj_t *screen_recipe_draft_brewing_create_disabled_modify_button(lv_obj_t *parent);
static void screen_recipe_draft_brewing_format_liters(char *buffer,
                                                      size_t buffer_size,
                                                      uint16_t value_dl);
static void screen_recipe_draft_brewing_format_temp(char *buffer,
                                                    size_t buffer_size,
                                                    uint8_t value_c);
static void screen_recipe_draft_brewing_format_minutes(char *buffer,
                                                       size_t buffer_size,
                                                       uint16_t value_min);
static void screen_recipe_draft_brewing_set_text_if_changed(lv_obj_t *label, const char *text);
static void screen_recipe_draft_brewing_rebuild_body(screen_recipe_draft_brewing_t *brewing,
                                                     const recipe_draft_t *draft);
static void screen_recipe_draft_brewing_nav_event_cb(lv_event_t *event);
static void screen_recipe_draft_brewing_mash_water_event_cb(lv_event_t *event);
static void screen_recipe_draft_brewing_mash_water_commit_cb(uint16_t value, void *user_data);

/****************************************************************************************
 * @brief Make an object static so it cannot become a tiny scroll target.
 ****************************************************************************************/
static void screen_recipe_draft_brewing_set_static(lv_obj_t *object)
{
    if (object == NULL)
    {
        return;
    }

    lv_obj_clear_flag(object, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_OFF);
}

static lv_obj_t *screen_recipe_draft_brewing_create_header(lv_obj_t *parent,
                                                           screen_recipe_draft_brewing_t *brewing)
{
    lv_obj_t *header;
    lv_obj_t *title;

    header = lv_obj_create(parent);
    screen_recipe_draft_brewing_set_static(header);
    lv_obj_set_width(header, lv_pct(100));
    lv_obj_set_height(header, 64);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_all(header, 0, 0);

    screen_recipe_draft_brewing_create_nav_button(header,
                                                  "<",
                                                  LV_ALIGN_LEFT_MID,
                                                  &brewing->back_button_context);

    title = lv_label_create(header);
    lv_label_set_text(title, "Brewing");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(title);
    return header;
}

static lv_obj_t *screen_recipe_draft_brewing_create_nav_button(
    lv_obj_t *parent,
    const char *text,
    lv_align_t align,
    screen_recipe_draft_brewing_nav_context_t *context)
{
    lv_obj_t *button;
    lv_obj_t *label;

    button = lv_button_create(parent);
    lv_obj_set_size(button, 42, 42);
    lv_obj_align(button, align, (align == LV_ALIGN_LEFT_MID) ? 2 : -2, 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x343434), LV_STATE_PRESSED);
    lv_obj_set_style_border_width(button, 0, 0);
    lv_obj_set_style_radius(button, 4, 0);
    lv_obj_add_event_cb(button, screen_recipe_draft_brewing_nav_event_cb, LV_EVENT_CLICKED, context);

    label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);
    return button;
}

/****************************************************************************************
 * @brief Create one old-style Brewing sub-panel.
 ****************************************************************************************/
static lv_obj_t *screen_recipe_draft_brewing_create_group(lv_obj_t *parent, const char *caption)
{
    lv_obj_t *group;
    lv_obj_t *caption_label;

    group = lv_obj_create(parent);
    lv_obj_set_width(group, lv_pct(97));
    lv_obj_set_height(group, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(group, lv_color_hex(0x292929), 0);
    lv_obj_set_style_bg_opa(group, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(group, lv_color_hex(0x3F3F3F), 0);
    lv_obj_set_style_border_width(group, 1, 0);
    lv_obj_set_style_radius(group, 0, 0);
    lv_obj_set_style_pad_all(group, 8, 0);
    lv_obj_set_style_pad_row(group, 7, 0);
    lv_obj_set_layout(group, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(group, LV_FLEX_FLOW_COLUMN);

    caption_label = lv_label_create(group);
    lv_label_set_text(caption_label, caption);
    lv_obj_set_width(caption_label, lv_pct(100));
    lv_obj_set_style_text_color(caption_label, lv_color_hex(0xE67526), 0);
    return group;
}

static lv_obj_t *screen_recipe_draft_brewing_create_value_row(lv_obj_t *parent,
                                                              const char *label_text,
                                                              const char *value_text)
{
    lv_obj_t *row;
    lv_obj_t *label;
    lv_obj_t *value;

    row = lv_obj_create(parent);
    screen_recipe_draft_brewing_set_static(row);
    lv_obj_set_width(row, lv_pct(100));
    lv_obj_set_height(row, 24);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);

    label = lv_label_create(row);
    lv_label_set_text(label, label_text);
    lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(label, lv_pct(58));
    lv_obj_set_style_text_color(label, lv_color_hex(0xC8C8C8), 0);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);

    value = lv_label_create(row);
    lv_label_set_text(value, value_text);
    lv_label_set_long_mode(value, LV_LABEL_LONG_DOT);
    lv_obj_set_width(value, lv_pct(38));
    lv_obj_set_style_text_color(value, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(value, LV_ALIGN_RIGHT_MID, 0, 0);

    lv_obj_remove_flag(label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(value, LV_OBJ_FLAG_CLICKABLE);
    return row;
}

static lv_obj_t *screen_recipe_draft_brewing_create_disabled_modify_button(lv_obj_t *parent)
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
    lv_label_set_text(label, "MODIFY LATER");
    lv_obj_set_style_text_color(label, lv_color_hex(0xC8C8C8), 0);
    lv_obj_center(label);
    return button;
}

static void screen_recipe_draft_brewing_format_liters(char *buffer,
                                                      size_t buffer_size,
                                                      uint16_t value_dl)
{
    if (value_dl == 0U)
    {
        snprintf(buffer, buffer_size, "--");
        return;
    }

    snprintf(buffer,
             buffer_size,
             "%u.%u L",
             (unsigned int)(value_dl / 10U),
             (unsigned int)(value_dl % 10U));
}

static void screen_recipe_draft_brewing_format_temp(char *buffer,
                                                    size_t buffer_size,
                                                    uint8_t value_c)
{
    if (value_c == 0U)
    {
        snprintf(buffer, buffer_size, "--");
        return;
    }

    snprintf(buffer, buffer_size, "%u C", (unsigned int)value_c);
}

static void screen_recipe_draft_brewing_format_minutes(char *buffer,
                                                       size_t buffer_size,
                                                       uint16_t value_min)
{
    if (value_min == 0U)
    {
        snprintf(buffer, buffer_size, "--");
        return;
    }

    snprintf(buffer, buffer_size, "%u min", (unsigned int)value_min);
}

static void screen_recipe_draft_brewing_set_text_if_changed(lv_obj_t *label, const char *text)
{
    if (label == NULL || text == NULL || strcmp(lv_label_get_text(label), text) == 0)
    {
        return;
    }

    lv_label_set_text(label, text);
}

/****************************************************************************************
 * @brief Rebuild the compact Brewing panels from the RAM-only draft model.
 *
 * This is screen-show work, not a hot UI tick. Rebuilding a few rows keeps the scaffold
 * straightforward while the brewing model is still changing shape.
 ****************************************************************************************/
static void screen_recipe_draft_brewing_rebuild_body(screen_recipe_draft_brewing_t *brewing,
                                                     const recipe_draft_t *draft)
{
    lv_obj_t *mash_group;
    lv_obj_t *water_group;
    lv_obj_t *boil_group;
    lv_obj_t *mash_water_row;
    char value_text[24];
    char step_name[16];
    uint8_t index;

    lv_obj_clean(brewing->body);

    water_group = screen_recipe_draft_brewing_create_group(brewing->body, "WATER");
    screen_recipe_draft_brewing_format_liters(value_text,
                                              sizeof(value_text),
                                              draft->brewing.mash_in_water_dl);
    mash_water_row = screen_recipe_draft_brewing_create_value_row(water_group, "Mash water", value_text);
    lv_obj_add_flag(mash_water_row, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(mash_water_row, lv_color_hex(0x3B332D), LV_STATE_PRESSED);
    lv_obj_add_event_cb(mash_water_row,
                        screen_recipe_draft_brewing_mash_water_event_cb,
                        LV_EVENT_CLICKED,
                        brewing);
    screen_recipe_draft_brewing_format_temp(value_text,
                                            sizeof(value_text),
                                            draft->brewing.mash_in_temperature_c);
    screen_recipe_draft_brewing_create_value_row(water_group, "Mash in temp", value_text);
    screen_recipe_draft_brewing_format_liters(value_text,
                                              sizeof(value_text),
                                              draft->brewing.sparge_water_dl);
    screen_recipe_draft_brewing_create_value_row(water_group, "Sparge water", value_text);
    screen_recipe_draft_brewing_format_temp(value_text,
                                            sizeof(value_text),
                                            draft->brewing.sparge_temperature_c);
    screen_recipe_draft_brewing_create_value_row(water_group, "Sparge temp", value_text);
    screen_recipe_draft_brewing_format_minutes(value_text,
                                               sizeof(value_text),
                                               draft->brewing.sparge_time_min);
    screen_recipe_draft_brewing_create_value_row(water_group, "Sparge time", value_text);

    mash_group = screen_recipe_draft_brewing_create_group(brewing->body, "MASH");
    if (draft->brewing.mash_step_count == 0U)
    {
        screen_recipe_draft_brewing_create_value_row(mash_group, "No mash steps", "--");
    }
    for (index = 0U; index < draft->brewing.mash_step_count && index < RECIPE_DRAFT_MAX_MASH_STEPS; ++index)
    {
        snprintf(step_name, sizeof(step_name), "Step %u", (unsigned int)(index + 1U));
        snprintf(value_text,
                 sizeof(value_text),
                 "%u C / %u min",
                 (unsigned int)draft->brewing.mash_steps[index].temperature_c,
                 (unsigned int)draft->brewing.mash_steps[index].time_min);
        screen_recipe_draft_brewing_create_value_row(mash_group, step_name, value_text);
    }

    boil_group = screen_recipe_draft_brewing_create_group(brewing->body, "BOIL / COOLING");
    screen_recipe_draft_brewing_format_minutes(value_text,
                                               sizeof(value_text),
                                               draft->brewing.boil_time_min);
    screen_recipe_draft_brewing_create_value_row(boil_group, "Boil time", value_text);
    screen_recipe_draft_brewing_format_minutes(value_text,
                                               sizeof(value_text),
                                               draft->brewing.delayed_hopping_min);
    screen_recipe_draft_brewing_create_value_row(boil_group, "Delayed hops", value_text);
    screen_recipe_draft_brewing_format_temp(value_text,
                                            sizeof(value_text),
                                            draft->brewing.cooling_target_c);
    screen_recipe_draft_brewing_create_value_row(boil_group, "Cooling target", value_text);
}

static void screen_recipe_draft_brewing_nav_event_cb(lv_event_t *event)
{
    screen_recipe_draft_brewing_nav_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL || context->handler == NULL)
    {
        return;
    }

    context->handler(context->action, context->value, context->user_data);
}

static void screen_recipe_draft_brewing_mash_water_event_cb(lv_event_t *event)
{
    screen_recipe_draft_brewing_t *brewing;
    uint16_t current_value;

    brewing = lv_event_get_user_data(event);
    if (brewing == NULL || brewing->draft == NULL)
    {
        return;
    }

    current_value = brewing->draft->brewing.mash_in_water_dl;
    if (current_value == 0U)
    {
        current_value = 150U;
    }

    ui_number_editor_show(&brewing->mash_water_editor,
                          "MASH WATER",
                          "L",
                          current_value,
                          20U,
                          300U,
                          5U,
                          true,
                          screen_recipe_draft_brewing_mash_water_commit_cb,
                          brewing);
}

static void screen_recipe_draft_brewing_mash_water_commit_cb(uint16_t value, void *user_data)
{
    screen_recipe_draft_brewing_t *brewing;

    brewing = user_data;
    if (brewing == NULL || brewing->draft == NULL)
    {
        return;
    }

    recipe_draft_set_mash_in_water_dl(brewing->draft, value);
    screen_recipe_draft_brewing_show(brewing, brewing->draft);
}

void screen_recipe_draft_brewing_init(screen_recipe_draft_brewing_t *brewing,
                                      recipe_draft_t *draft,
                                      ui_action_handler_t action_handler,
                                      void *user_data)
{
    lv_obj_t *container;

    if (brewing == NULL)
    {
        return;
    }

    memset(brewing, 0, sizeof(*brewing));
    brewing->draft = draft;
    brewing->back_button_context.action = UI_ACTION_SHOW_RECIPE_DRAFT_MENU;
    brewing->back_button_context.handler = action_handler;
    brewing->back_button_context.user_data = user_data;

    brewing->screen = lv_obj_create(NULL);
    screen_recipe_draft_brewing_set_static(brewing->screen);
    lv_obj_set_style_bg_color(brewing->screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(brewing->screen, LV_OPA_COVER, 0);

    container = lv_obj_create(brewing->screen);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container, lv_color_hex(0x141414), 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 8, 0);
    lv_obj_set_style_pad_row(container, 8, 0);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);

    screen_recipe_draft_brewing_create_header(container, brewing);

    brewing->name_label = lv_label_create(container);
    lv_label_set_text(brewing->name_label, "--");
    lv_label_set_long_mode(brewing->name_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(brewing->name_label, lv_pct(100));
    lv_obj_set_style_text_color(brewing->name_label, lv_color_hex(0xE67526), 0);

    brewing->body = lv_obj_create(container);
    lv_obj_set_width(brewing->body, lv_pct(100));
    lv_obj_set_flex_grow(brewing->body, 1);
    lv_obj_set_style_bg_opa(brewing->body, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(brewing->body, 0, 0);
    lv_obj_set_style_pad_all(brewing->body, 0, 0);
    lv_obj_set_style_pad_row(brewing->body, 8, 0);
    ui_scroll_apply_gutter(brewing->body);
    lv_obj_set_layout(brewing->body, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(brewing->body, LV_FLEX_FLOW_COLUMN);

    screen_recipe_draft_brewing_create_disabled_modify_button(container);
    ui_number_editor_init(&brewing->mash_water_editor, brewing->screen);
}

/****************************************************************************************
 * @brief Show RAM-only draft brewing values without reading or saving a real recipe.
 ****************************************************************************************/
void screen_recipe_draft_brewing_show(screen_recipe_draft_brewing_t *brewing,
                                      const recipe_draft_t *draft)
{
    const char *draft_name;

    if (brewing == NULL || draft == NULL)
    {
        return;
    }

    draft_name = recipe_draft_get_name(draft);
    if (brewing->shown_name != draft_name)
    {
        screen_recipe_draft_brewing_set_text_if_changed(brewing->name_label, draft_name);
        brewing->shown_name = draft_name;
    }

    screen_recipe_draft_brewing_rebuild_body(brewing, draft);
}
