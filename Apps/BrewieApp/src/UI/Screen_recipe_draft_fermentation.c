#include "Screen_recipe_draft_fermentation.h"

#include <stdio.h>
#include <string.h>

#include "UI_scroll.h"

static void screen_recipe_draft_fermentation_set_static(lv_obj_t *object);
static lv_obj_t *screen_recipe_draft_fermentation_create_header(
    lv_obj_t *parent,
    screen_recipe_draft_fermentation_t *fermentation);
static lv_obj_t *screen_recipe_draft_fermentation_create_nav_button(
    lv_obj_t *parent,
    const char *text,
    screen_recipe_draft_fermentation_nav_context_t *context);
static lv_obj_t *screen_recipe_draft_fermentation_create_group(lv_obj_t *parent,
                                                               const char *caption);
static lv_obj_t *screen_recipe_draft_fermentation_create_value_row(
    lv_obj_t *parent,
    const char *label_text,
    const char *value_text,
    screen_recipe_draft_fermentation_edit_context_t *context);
static lv_obj_t *screen_recipe_draft_fermentation_create_disabled_modify_button(lv_obj_t *parent);
static void screen_recipe_draft_fermentation_init_edit_contexts(
    screen_recipe_draft_fermentation_t *fermentation);
static void screen_recipe_draft_fermentation_format_temperature(char *buffer,
                                                                size_t buffer_size,
                                                                uint8_t value_c);
static void screen_recipe_draft_fermentation_format_days(char *buffer,
                                                         size_t buffer_size,
                                                         uint16_t value_days);
static void screen_recipe_draft_fermentation_set_text_if_changed(lv_obj_t *label,
                                                                 const char *text);
static void screen_recipe_draft_fermentation_rebuild_body(
    screen_recipe_draft_fermentation_t *fermentation,
    const recipe_draft_t *draft);
static void screen_recipe_draft_fermentation_nav_event_cb(lv_event_t *event);
static void screen_recipe_draft_fermentation_value_event_cb(lv_event_t *event);
static void screen_recipe_draft_fermentation_value_commit_cb(uint16_t value,
                                                             void *user_data);

/****************************************************************************************
 * @brief Make an object static so it cannot become a tiny scroll target.
 ****************************************************************************************/
static void screen_recipe_draft_fermentation_set_static(lv_obj_t *object)
{
    if (object == NULL)
    {
        return;
    }

    lv_obj_clear_flag(object, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_OFF);
}

static lv_obj_t *screen_recipe_draft_fermentation_create_header(
    lv_obj_t *parent,
    screen_recipe_draft_fermentation_t *fermentation)
{
    lv_obj_t *header;
    lv_obj_t *title;

    header = lv_obj_create(parent);
    screen_recipe_draft_fermentation_set_static(header);
    lv_obj_set_width(header, lv_pct(100));
    lv_obj_set_height(header, 64);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_all(header, 0, 0);

    screen_recipe_draft_fermentation_create_nav_button(header,
                                                       "<",
                                                       &fermentation->back_button_context);

    title = lv_label_create(header);
    lv_label_set_text(title, "Fermentation");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(title);
    return header;
}

static lv_obj_t *screen_recipe_draft_fermentation_create_nav_button(
    lv_obj_t *parent,
    const char *text,
    screen_recipe_draft_fermentation_nav_context_t *context)
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
    lv_obj_add_event_cb(button, screen_recipe_draft_fermentation_nav_event_cb, LV_EVENT_CLICKED, context);

    label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);
    return button;
}

/****************************************************************************************
 * @brief Create one old-style Fermentation sub-panel.
 ****************************************************************************************/
static lv_obj_t *screen_recipe_draft_fermentation_create_group(lv_obj_t *parent,
                                                               const char *caption)
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

/****************************************************************************************
 * @brief Create one editable Fermentation row and connect it to the shared number editor.
 ****************************************************************************************/
static lv_obj_t *screen_recipe_draft_fermentation_create_value_row(
    lv_obj_t *parent,
    const char *label_text,
    const char *value_text,
    screen_recipe_draft_fermentation_edit_context_t *context)
{
    lv_obj_t *row;
    lv_obj_t *label;
    lv_obj_t *value;
    lv_obj_t *edit_icon;

    row = lv_obj_create(parent);
    screen_recipe_draft_fermentation_set_static(row);
    lv_obj_set_width(row, lv_pct(100));
    lv_obj_set_height(row, 24);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(row, lv_color_hex(0x3B332D), LV_STATE_PRESSED);
    lv_obj_add_event_cb(row, screen_recipe_draft_fermentation_value_event_cb, LV_EVENT_CLICKED, context);

    label = lv_label_create(row);
    lv_label_set_text(label, label_text);
    lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(label, lv_pct(52));
    lv_obj_set_style_text_color(label, lv_color_hex(0xC8C8C8), 0);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);

    value = lv_label_create(row);
    lv_label_set_text(value, value_text);
    lv_label_set_long_mode(value, LV_LABEL_LONG_DOT);
    lv_obj_set_width(value, lv_pct(34));
    lv_obj_set_style_text_color(value, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(value, LV_ALIGN_RIGHT_MID, -22, 0);

    edit_icon = lv_label_create(row);
    lv_label_set_text(edit_icon, LV_SYMBOL_EDIT);
    lv_obj_set_style_text_color(edit_icon, lv_color_hex(0xE67526), 0);
    lv_obj_align(edit_icon, LV_ALIGN_RIGHT_MID, 0, 0);

    lv_obj_remove_flag(label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(value, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(edit_icon, LV_OBJ_FLAG_CLICKABLE);
    return row;
}

static lv_obj_t *screen_recipe_draft_fermentation_create_disabled_modify_button(lv_obj_t *parent)
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

/****************************************************************************************
 * @brief Prepare stable callback contexts for rebuilt editable rows.
 ****************************************************************************************/
static void screen_recipe_draft_fermentation_init_edit_contexts(
    screen_recipe_draft_fermentation_t *fermentation)
{
    uint8_t index;

    if (fermentation == NULL)
    {
        return;
    }

    for (index = 0U; index < RECIPE_MAX_FERMENTATION_STEPS; ++index)
    {
        fermentation->temperature_contexts[index].fermentation = fermentation;
        fermentation->temperature_contexts[index].step_index = index;
        fermentation->temperature_contexts[index].edit_kind =
            SCREEN_RECIPE_DRAFT_FERMENTATION_EDIT_TEMPERATURE;
        fermentation->duration_contexts[index].fermentation = fermentation;
        fermentation->duration_contexts[index].step_index = index;
        fermentation->duration_contexts[index].edit_kind =
            SCREEN_RECIPE_DRAFT_FERMENTATION_EDIT_DURATION;
    }
}

static void screen_recipe_draft_fermentation_format_temperature(char *buffer,
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

static void screen_recipe_draft_fermentation_format_days(char *buffer,
                                                         size_t buffer_size,
                                                         uint16_t value_days)
{
    if (value_days == 0U)
    {
        snprintf(buffer, buffer_size, "--");
        return;
    }

    snprintf(buffer, buffer_size, "%u days", (unsigned int)value_days);
}

static void screen_recipe_draft_fermentation_set_text_if_changed(lv_obj_t *label,
                                                                 const char *text)
{
    if (label == NULL || text == NULL || strcmp(lv_label_get_text(label), text) == 0)
    {
        return;
    }

    lv_label_set_text(label, text);
}

/****************************************************************************************
 * @brief Rebuild the local fermentation schedule from the RAM-only draft model.
 ****************************************************************************************/
static void screen_recipe_draft_fermentation_rebuild_body(
    screen_recipe_draft_fermentation_t *fermentation,
    const recipe_draft_t *draft)
{
    lv_obj_t *group;
    char value_text[24];
    uint8_t index;

    lv_obj_clean(fermentation->body);

    for (index = 0U;
         index < draft->fermentation.step_count && index < RECIPE_MAX_FERMENTATION_STEPS;
         ++index)
    {
        group = screen_recipe_draft_fermentation_create_group(fermentation->body,
                                                              draft->fermentation.steps[index].name);
        screen_recipe_draft_fermentation_format_temperature(value_text,
                                                            sizeof(value_text),
                                                            draft->fermentation.steps[index].temperature_c);
        screen_recipe_draft_fermentation_create_value_row(group,
                                                          "Temperature",
                                                          value_text,
                                                          &fermentation->temperature_contexts[index]);

        screen_recipe_draft_fermentation_format_days(value_text,
                                                     sizeof(value_text),
                                                     draft->fermentation.steps[index].duration_days);
        screen_recipe_draft_fermentation_create_value_row(group,
                                                          "Duration",
                                                          value_text,
                                                          &fermentation->duration_contexts[index]);
    }
}

static void screen_recipe_draft_fermentation_nav_event_cb(lv_event_t *event)
{
    screen_recipe_draft_fermentation_nav_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL || context->handler == NULL)
    {
        return;
    }

    context->handler(context->action, context->value, context->user_data);
}

static void screen_recipe_draft_fermentation_value_event_cb(lv_event_t *event)
{
    screen_recipe_draft_fermentation_edit_context_t *context;
    screen_recipe_draft_fermentation_t *fermentation;
    uint16_t current_value;

    context = lv_event_get_user_data(event);
    if (context == NULL || context->fermentation == NULL || context->fermentation->draft == NULL)
    {
        return;
    }

    fermentation = context->fermentation;
    fermentation->active_step_index = context->step_index;
    fermentation->active_edit_kind = context->edit_kind;
    if (context->edit_kind == SCREEN_RECIPE_DRAFT_FERMENTATION_EDIT_TEMPERATURE)
    {
        current_value = fermentation->draft->fermentation.steps[context->step_index].temperature_c;
        if (current_value == 0U)
        {
            current_value = 20U;
        }
        ui_number_editor_show(&fermentation->number_editor,
                              "TEMPERATURE",
                              "C",
                              current_value,
                              1U,
                              40U,
                              1U,
                              false,
                              screen_recipe_draft_fermentation_value_commit_cb,
                              fermentation);
        return;
    }

    current_value = fermentation->draft->fermentation.steps[context->step_index].duration_days;
    if (current_value == 0U)
    {
        current_value = 7U;
    }
    ui_number_editor_show(&fermentation->number_editor,
                          "DURATION",
                          "days",
                          current_value,
                          0U,
                          60U,
                          1U,
                          false,
                          screen_recipe_draft_fermentation_value_commit_cb,
                          fermentation);
}

static void screen_recipe_draft_fermentation_value_commit_cb(uint16_t value,
                                                             void *user_data)
{
    screen_recipe_draft_fermentation_t *fermentation;

    fermentation = user_data;
    if (fermentation == NULL || fermentation->draft == NULL)
    {
        return;
    }

    if (fermentation->active_edit_kind == SCREEN_RECIPE_DRAFT_FERMENTATION_EDIT_TEMPERATURE)
    {
        recipe_draft_set_fermentation_temperature_c(fermentation->draft,
                                                    fermentation->active_step_index,
                                                    (uint8_t)value);
    }
    else
    {
        recipe_draft_set_fermentation_duration_days(fermentation->draft,
                                                    fermentation->active_step_index,
                                                    value);
    }

    screen_recipe_draft_fermentation_show(fermentation, fermentation->draft);
}

void screen_recipe_draft_fermentation_init(screen_recipe_draft_fermentation_t *fermentation,
                                           recipe_draft_t *draft,
                                           ui_action_handler_t action_handler,
                                           void *user_data)
{
    lv_obj_t *container;

    if (fermentation == NULL)
    {
        return;
    }

    memset(fermentation, 0, sizeof(*fermentation));
    fermentation->draft = draft;
    fermentation->back_button_context.action = UI_ACTION_SHOW_RECIPE_DRAFT_MENU;
    fermentation->back_button_context.handler = action_handler;
    fermentation->back_button_context.user_data = user_data;
    screen_recipe_draft_fermentation_init_edit_contexts(fermentation);

    fermentation->screen = lv_obj_create(NULL);
    screen_recipe_draft_fermentation_set_static(fermentation->screen);
    lv_obj_set_style_bg_color(fermentation->screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(fermentation->screen, LV_OPA_COVER, 0);

    container = lv_obj_create(fermentation->screen);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container, lv_color_hex(0x141414), 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 8, 0);
    lv_obj_set_style_pad_row(container, 8, 0);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);

    screen_recipe_draft_fermentation_create_header(container, fermentation);

    fermentation->name_label = lv_label_create(container);
    lv_label_set_text(fermentation->name_label, "--");
    lv_label_set_long_mode(fermentation->name_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(fermentation->name_label, lv_pct(100));
    lv_obj_set_style_text_color(fermentation->name_label, lv_color_hex(0xE67526), 0);

    fermentation->body = lv_obj_create(container);
    lv_obj_set_width(fermentation->body, lv_pct(100));
    lv_obj_set_flex_grow(fermentation->body, 1);
    lv_obj_set_style_bg_opa(fermentation->body, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(fermentation->body, 0, 0);
    lv_obj_set_style_pad_all(fermentation->body, 0, 0);
    lv_obj_set_style_pad_row(fermentation->body, 8, 0);
    ui_scroll_apply_gutter(fermentation->body);
    lv_obj_set_layout(fermentation->body, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(fermentation->body, LV_FLEX_FLOW_COLUMN);

    screen_recipe_draft_fermentation_create_disabled_modify_button(container);
    ui_number_editor_init(&fermentation->number_editor, fermentation->screen);
}

/****************************************************************************************
 * @brief Show RAM-only draft fermentation values without reading or saving a real recipe.
 ****************************************************************************************/
void screen_recipe_draft_fermentation_show(screen_recipe_draft_fermentation_t *fermentation,
                                           const recipe_draft_t *draft)
{
    const char *draft_name;

    if (fermentation == NULL || draft == NULL)
    {
        return;
    }

    draft_name = recipe_draft_get_name(draft);
    if (fermentation->shown_name != draft_name)
    {
        screen_recipe_draft_fermentation_set_text_if_changed(fermentation->name_label, draft_name);
        fermentation->shown_name = draft_name;
    }

    screen_recipe_draft_fermentation_rebuild_body(fermentation, draft);
}
