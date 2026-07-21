#include "Screen_recipe_builder.h"

#include <stdio.h>
#include <string.h>

#define SCREEN_RECIPE_BUILDER_PAD 8
#define SCREEN_RECIPE_BUILDER_NAME_VALUE_HEIGHT 18

static void screen_recipe_builder_set_static(lv_obj_t *object);
static lv_obj_t *screen_recipe_builder_create_header(lv_obj_t *parent);
static lv_obj_t *screen_recipe_builder_create_nav_button(lv_obj_t *parent,
                                                         const char *text,
                                                         lv_align_t align,
                                                         screen_recipe_builder_nav_context_t *context);
static lv_obj_t *screen_recipe_builder_create_name_row(lv_obj_t *parent,
                                                       screen_recipe_builder_name_context_t *context,
                                                       lv_obj_t **value_label);
static lv_obj_t *screen_recipe_builder_create_bottom_button(lv_obj_t *parent,
                                                            const char *text,
                                                            lv_align_t align,
                                                            bool enabled,
                                                            screen_recipe_builder_nav_context_t *context);
static void screen_recipe_builder_set_done_enabled(screen_recipe_builder_t *builder, bool enabled);
static void screen_recipe_builder_refresh_name(screen_recipe_builder_t *builder);
static void screen_recipe_builder_show_name_dialog(screen_recipe_builder_t *builder);
static void screen_recipe_builder_use_sample_name(void *user_data);
static void screen_recipe_builder_nav_event_cb(lv_event_t *event);
static void screen_recipe_builder_name_event_cb(lv_event_t *event);

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

/****************************************************************************************
 * @brief Create the old-style title area for the first recipe creation step.
 ****************************************************************************************/
static lv_obj_t *screen_recipe_builder_create_header(lv_obj_t *parent)
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

    title = lv_label_create(header);
    lv_label_set_text(title, "Creating Recipe");
    lv_label_set_long_mode(title, LV_LABEL_LONG_DOT);
    lv_obj_set_width(title, lv_pct(100));
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(title);
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

/****************************************************************************************
 * @brief Create the one editable local draft-name row.
 ****************************************************************************************/
static lv_obj_t *screen_recipe_builder_create_name_row(lv_obj_t *parent,
                                                       screen_recipe_builder_name_context_t *context,
                                                       lv_obj_t **value_label)
{
    lv_obj_t *button;
    lv_obj_t *title_label;

    button = lv_button_create(parent);
    lv_obj_set_width(button, lv_pct(100));
    lv_obj_set_height(button, 58);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x282828), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x3B332D), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(button, lv_color_hex(0x343434), 0);
    lv_obj_set_style_border_width(button, 1, 0);
    lv_obj_set_style_radius(button, 0, 0);
    lv_obj_set_style_pad_all(button, 8, 0);
    lv_obj_add_event_cb(button, screen_recipe_builder_name_event_cb, LV_EVENT_CLICKED, context);

    title_label = lv_label_create(button);
    lv_label_set_text(title_label, "Name");
    lv_label_set_long_mode(title_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(title_label, lv_pct(100));
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_20, 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_LEFT, 0, 0);

    *value_label = lv_label_create(button);
    lv_label_set_text(*value_label, "--");
    lv_label_set_long_mode(*value_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(*value_label, lv_pct(100));
    lv_obj_set_height(*value_label, SCREEN_RECIPE_BUILDER_NAME_VALUE_HEIGHT);
    lv_obj_set_style_text_color(*value_label, lv_color_hex(0xE67526), 0);
    lv_obj_align(*value_label, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    lv_obj_remove_flag(title_label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(*value_label, LV_OBJ_FLAG_CLICKABLE);
    return button;
}

/****************************************************************************************
 * @brief Create one old-style bottom action button.
 ****************************************************************************************/
static lv_obj_t *screen_recipe_builder_create_bottom_button(lv_obj_t *parent,
                                                            const char *text,
                                                            lv_align_t align,
                                                            bool enabled,
                                                            screen_recipe_builder_nav_context_t *context)
{
    lv_obj_t *button;
    lv_obj_t *label;

    button = lv_button_create(parent);
    lv_obj_set_width(button, lv_pct(38));
    lv_obj_set_height(button, 44);
    lv_obj_align(button, align, 0, -8);
    lv_obj_set_style_bg_color(button, enabled ? lv_color_hex(0xF47B32) : lv_color_hex(0x4B4741), 0);
    lv_obj_set_style_bg_color(button, enabled ? lv_color_hex(0xC85F22) : lv_color_hex(0x343434), LV_STATE_PRESSED);
    lv_obj_set_style_radius(button, 5, 0);
    lv_obj_add_event_cb(button, screen_recipe_builder_nav_event_cb, LV_EVENT_CLICKED, context);

    if (!enabled)
    {
        lv_obj_add_state(button, LV_STATE_DISABLED);
    }

    label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);
    return button;
}

/****************************************************************************************
 * @brief Enable or disable the local DONE button and keep its color state obvious.
 ****************************************************************************************/
static void screen_recipe_builder_set_done_enabled(screen_recipe_builder_t *builder, bool enabled)
{
    if (builder == NULL || builder->done_button == NULL)
    {
        return;
    }

    if (enabled)
    {
        lv_obj_remove_state(builder->done_button, LV_STATE_DISABLED);
    }
    else
    {
        lv_obj_add_state(builder->done_button, LV_STATE_DISABLED);
    }

    lv_obj_set_style_bg_color(builder->done_button,
                              enabled ? lv_color_hex(0xF47B32) : lv_color_hex(0x4B4741),
                              0);
    lv_obj_set_style_bg_color(builder->done_button,
                              enabled ? lv_color_hex(0xC85F22) : lv_color_hex(0x343434),
                              LV_STATE_PRESSED);
}

/****************************************************************************************
 * @brief Push the current local draft name into the visible name row.
 ****************************************************************************************/
static void screen_recipe_builder_refresh_name(screen_recipe_builder_t *builder)
{
    if (builder == NULL || builder->name_value_label == NULL)
    {
        return;
    }

    lv_label_set_text(builder->name_value_label, recipe_draft_get_name(builder->draft));
    screen_recipe_builder_set_done_enabled(builder, recipe_draft_has_name(builder->draft));
}

/****************************************************************************************
 * @brief Show the temporary local name helper until a real keyboard exists.
 ****************************************************************************************/
static void screen_recipe_builder_show_name_dialog(screen_recipe_builder_t *builder)
{
    if (builder == NULL)
    {
        return;
    }

    snprintf(builder->editor_dialog_body,
             sizeof(builder->editor_dialog_body),
             "Current name:\n%s\n\nUse Sample changes only this local draft. A real keyboard comes later.",
             recipe_draft_get_name(builder->draft));
    ui_dialog_show(&builder->editor_dialog, "Name your recipe", builder->editor_dialog_body);
}

/****************************************************************************************
 * @brief Apply a temporary sample name to the local draft only.
 ****************************************************************************************/
static void screen_recipe_builder_use_sample_name(void *user_data)
{
    screen_recipe_builder_t *builder;

    builder = user_data;
    if (builder == NULL)
    {
        return;
    }

    recipe_draft_apply_sample(builder->draft);
    screen_recipe_builder_refresh_name(builder);
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

static void screen_recipe_builder_name_event_cb(lv_event_t *event)
{
    screen_recipe_builder_name_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL)
    {
        return;
    }

    screen_recipe_builder_show_name_dialog(context->builder);
}

void screen_recipe_builder_init(screen_recipe_builder_t *builder,
                                recipe_draft_t *draft,
                                ui_action_handler_t action_handler,
                                void *user_data)
{
    lv_obj_t *container;
    lv_obj_t *primary_header;
    lv_obj_t *secondary_header;
    lv_obj_t *spacer;
    lv_obj_t *bottom_area;

    if (builder == NULL)
    {
        return;
    }

    memset(builder, 0, sizeof(*builder));
    builder->draft = draft;
    builder->back_button_context.action = UI_ACTION_SHOW_RECIPES;
    builder->back_button_context.handler = action_handler;
    builder->back_button_context.user_data = user_data;
    builder->done_button_context.action = UI_ACTION_SHOW_RECIPE_DRAFT_MENU;
    builder->done_button_context.handler = action_handler;
    builder->done_button_context.user_data = user_data;
    builder->name_context.builder = builder;

    builder->screen = lv_obj_create(NULL);
    screen_recipe_builder_set_static(builder->screen);
    lv_obj_set_style_bg_color(builder->screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(builder->screen, LV_OPA_COVER, 0);

    container = lv_obj_create(builder->screen);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container, lv_color_hex(0x141414), 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, SCREEN_RECIPE_BUILDER_PAD, 0);
    lv_obj_set_style_pad_row(container, 10, 0);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);

    screen_recipe_builder_create_header(container);
    ui_dialog_init(&builder->editor_dialog, builder->screen, "Cancel", "Use Sample");
    ui_dialog_set_secondary_action(&builder->editor_dialog,
                                   screen_recipe_builder_use_sample_name,
                                   builder);

    primary_header = lv_label_create(container);
    lv_label_set_text(primary_header, "FIRST STEP");
    lv_obj_set_width(primary_header, lv_pct(100));
    lv_obj_set_style_text_color(primary_header, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(primary_header, &lv_font_montserrat_20, 0);

    secondary_header = lv_label_create(container);
    lv_label_set_text(secondary_header, "Name your recipe");
    lv_obj_set_width(secondary_header, lv_pct(100));
    lv_obj_set_style_text_color(secondary_header, lv_color_hex(0x8C8C8C), 0);
    lv_obj_set_style_text_font(secondary_header, &lv_font_montserrat_20, 0);

    screen_recipe_builder_create_name_row(container,
                                          &builder->name_context,
                                          &builder->name_value_label);

    spacer = lv_obj_create(container);
    screen_recipe_builder_set_static(spacer);
    lv_obj_set_width(spacer, lv_pct(100));
    lv_obj_set_flex_grow(spacer, 1);
    lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(spacer, 0, 0);

    bottom_area = lv_obj_create(container);
    screen_recipe_builder_set_static(bottom_area);
    lv_obj_set_width(bottom_area, lv_pct(100));
    lv_obj_set_height(bottom_area, 58);
    lv_obj_set_style_bg_opa(bottom_area, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(bottom_area, 0, 0);
    lv_obj_set_style_pad_all(bottom_area, 0, 0);

    screen_recipe_builder_create_bottom_button(bottom_area,
                                               "CANCEL",
                                               LV_ALIGN_BOTTOM_LEFT,
                                               true,
                                               &builder->back_button_context);
    builder->done_button =
        screen_recipe_builder_create_bottom_button(bottom_area,
                                                   "DONE",
                                                   LV_ALIGN_BOTTOM_RIGHT,
                                                   false,
                                                   &builder->done_button_context);

    screen_recipe_builder_refresh_name(builder);
}

/****************************************************************************************
 * @brief Refresh visible builder widgets from the logic-owned draft before showing it.
 ****************************************************************************************/
void screen_recipe_builder_show(screen_recipe_builder_t *builder)
{
    if (builder == NULL)
    {
        return;
    }

    screen_recipe_builder_refresh_name(builder);
}
