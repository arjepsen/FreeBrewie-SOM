#include "Screen_recipe_section.h"

#include <string.h>

#include "UI_scroll.h"

static void screen_recipe_section_set_static(lv_obj_t *object);
static lv_obj_t *screen_recipe_section_create_header(lv_obj_t *parent, screen_recipe_section_t *section);
static lv_obj_t *screen_recipe_section_create_nav_button(lv_obj_t *parent,
                                                         const char *text,
                                                         lv_align_t align,
                                                         screen_recipe_section_button_context_t *context);
static lv_obj_t *screen_recipe_section_create_menu_button(lv_obj_t *parent, screen_recipe_section_t *section);
static lv_obj_t *screen_recipe_section_create_row(lv_obj_t *parent,
                                                  lv_obj_t **title_label,
                                                  lv_obj_t **value_label);
static const char *screen_recipe_section_title(recipe_section_id_t section_id);
static uint32_t screen_recipe_section_fill_rows(screen_recipe_section_t *section,
                                                const recipe_t *recipe,
                                                recipe_section_id_t section_id);
static void screen_recipe_section_set_row(screen_recipe_section_t *section,
                                          uint32_t row_index,
                                          const char *title,
                                          const char *value);
static void screen_recipe_section_button_event_cb(lv_event_t *event);

static void screen_recipe_section_set_static(lv_obj_t *object)
{
    if (object == NULL)
    {
        return;
    }

    lv_obj_clear_flag(object, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_OFF);
}

static lv_obj_t *screen_recipe_section_create_header(lv_obj_t *parent, screen_recipe_section_t *section)
{
    lv_obj_t *header;

    header = lv_obj_create(parent);
    screen_recipe_section_set_static(header);
    lv_obj_set_width(header, lv_pct(100));
    lv_obj_set_height(header, 64);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_all(header, 0, 0);

    screen_recipe_section_create_nav_button(header, "<", LV_ALIGN_LEFT_MID, &section->back_button_context);

    section->title_label = lv_label_create(header);
    lv_label_set_text(section->title_label, "Section");
    lv_obj_set_style_text_font(section->title_label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(section->title_label, lv_color_hex(0xC8C8C8), 0);
    lv_obj_center(section->title_label);

    screen_recipe_section_create_menu_button(header, section);
    return header;
}

static lv_obj_t *screen_recipe_section_create_nav_button(lv_obj_t *parent,
                                                         const char *text,
                                                         lv_align_t align,
                                                         screen_recipe_section_button_context_t *context)
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
    lv_obj_add_event_cb(button, screen_recipe_section_button_event_cb, LV_EVENT_CLICKED, context);

    label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);
    return button;
}

static lv_obj_t *screen_recipe_section_create_menu_button(lv_obj_t *parent, screen_recipe_section_t *section)
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
                        screen_recipe_section_button_event_cb,
                        LV_EVENT_CLICKED,
                        &section->menu_button_context);

    for (offset_y = -7; offset_y <= 7; offset_y = (int8_t)(offset_y + 7))
    {
        line = lv_obj_create(button);
        screen_recipe_section_set_static(line);
        lv_obj_set_size(line, 20, 2);
        lv_obj_set_style_bg_color(line, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_bg_opa(line, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(line, 0, 0);
        lv_obj_align(line, LV_ALIGN_CENTER, 0, offset_y);
    }

    return button;
}

static lv_obj_t *screen_recipe_section_create_row(lv_obj_t *parent,
                                                  lv_obj_t **title_label,
                                                  lv_obj_t **value_label)
{
    lv_obj_t *row;

    row = lv_obj_create(parent);
    screen_recipe_section_set_static(row);
    lv_obj_set_width(row, lv_pct(100));
    lv_obj_set_height(row, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(row, lv_color_hex(0x141414), 0);
    lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(row, lv_color_hex(0x2C2B2B), 0);
    lv_obj_set_style_border_width(row, 1, 0);
    lv_obj_set_style_radius(row, 0, 0);
    lv_obj_set_style_pad_all(row, 8, 0);
    lv_obj_set_style_pad_row(row, 5, 0);
    lv_obj_set_layout(row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_COLUMN);

    *title_label = lv_label_create(row);
    lv_label_set_text(*title_label, "--");
    lv_label_set_long_mode(*title_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(*title_label, lv_pct(100));
    lv_obj_set_style_text_color(*title_label, lv_color_hex(0xE67526), 0);

    *value_label = lv_label_create(row);
    lv_label_set_text(*value_label, "--");
    lv_label_set_long_mode(*value_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(*value_label, lv_pct(100));
    lv_obj_set_style_text_color(*value_label, lv_color_hex(0xD8D8D8), 0);

    return row;
}

static const char *screen_recipe_section_title(recipe_section_id_t section_id)
{
    if (section_id == RECIPE_SECTION_INGREDIENTS)
    {
        return "Ingredients";
    }

    if (section_id == RECIPE_SECTION_BREWING)
    {
        return "Brewing";
    }

    if (section_id == RECIPE_SECTION_FERMENTATION)
    {
        return "Fermentation";
    }

    return "Details";
}

static uint32_t screen_recipe_section_fill_rows(screen_recipe_section_t *section,
                                                const recipe_t *recipe,
                                                recipe_section_id_t section_id)
{
    if (recipe == NULL)
    {
        return 0U;
    }

    if (section_id == RECIPE_SECTION_INGREDIENTS)
    {
        screen_recipe_section_set_row(section, 0U, "Fermentables", recipe->fermentables);
        screen_recipe_section_set_row(section, 1U, "Hops", recipe->hops);
        screen_recipe_section_set_row(section, 2U, "Additions", recipe->additions);
        screen_recipe_section_set_row(section, 3U, "Yeast", recipe->yeast);
        return 4U;
    }

    if (section_id == RECIPE_SECTION_BREWING)
    {
        screen_recipe_section_set_row(section, 0U, "Mash", recipe->mash);
        screen_recipe_section_set_row(section, 1U, "Boil", recipe->boil);
        screen_recipe_section_set_row(section, 2U, "Cooling", recipe->cooling);
        screen_recipe_section_set_row(section, 3U, "Water", recipe->water);
        return 4U;
    }

    if (section_id == RECIPE_SECTION_FERMENTATION)
    {
        screen_recipe_section_set_row(section, 0U, "Primary", recipe->fermentation);
        screen_recipe_section_set_row(section, 1U, "Temperature", recipe->fermentation_temperature);
        screen_recipe_section_set_row(section, 2U, "Duration", recipe->fermentation_duration);
        return 3U;
    }

    screen_recipe_section_set_row(section, 0U, "Style", recipe->style);
    screen_recipe_section_set_row(section, 1U, "Summary", recipe->summary);
    screen_recipe_section_set_row(section, 2U, "Batch size", recipe->batch_size);
    screen_recipe_section_set_row(section, 3U, "ABV", recipe->abv);
    screen_recipe_section_set_row(section, 4U, "IBU", recipe->ibu);
    screen_recipe_section_set_row(section, 5U, "Original gravity", recipe->og);
    return 6U;
}

static void screen_recipe_section_set_row(screen_recipe_section_t *section,
                                          uint32_t row_index,
                                          const char *title,
                                          const char *value)
{
    if (section == NULL || row_index >= SCREEN_RECIPE_SECTION_MAX_ROWS)
    {
        return;
    }

    lv_label_set_text(section->row_title_labels[row_index], title);
    lv_label_set_text(section->row_value_labels[row_index], value);
    lv_obj_remove_flag(section->row_objects[row_index], LV_OBJ_FLAG_HIDDEN);
}

static void screen_recipe_section_button_event_cb(lv_event_t *event)
{
    screen_recipe_section_button_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL || context->handler == NULL)
    {
        return;
    }

    context->handler(context->action, context->value, context->user_data);
}

void screen_recipe_section_init(screen_recipe_section_t *section,
                                ui_action_handler_t action_handler,
                                void *user_data)
{
    lv_obj_t *container;
    lv_obj_t *body_list;
    uint32_t row_index;

    if (section == NULL)
    {
        return;
    }

    memset(section, 0, sizeof(*section));
    section->back_button_context.action = UI_ACTION_SHOW_RECIPE_DETAIL;
    section->back_button_context.handler = action_handler;
    section->back_button_context.user_data = user_data;
    section->menu_button_context.action = UI_ACTION_SHOW_MENU;
    section->menu_button_context.handler = action_handler;
    section->menu_button_context.user_data = user_data;

    section->screen = lv_obj_create(NULL);
    screen_recipe_section_set_static(section->screen);
    lv_obj_set_style_bg_color(section->screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(section->screen, LV_OPA_COVER, 0);

    container = lv_obj_create(section->screen);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 8, 0);
    lv_obj_set_style_pad_row(container, 8, 0);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);

    screen_recipe_section_create_header(container, section);

    section->recipe_label = lv_label_create(container);
    lv_label_set_text(section->recipe_label, "--");
    lv_label_set_long_mode(section->recipe_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(section->recipe_label, lv_pct(100));
    lv_obj_set_style_text_color(section->recipe_label, lv_color_hex(0xE67526), 0);

    body_list = lv_obj_create(container);
    lv_obj_set_width(body_list, lv_pct(100));
    lv_obj_set_flex_grow(body_list, 1);
    lv_obj_set_style_bg_color(body_list, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(body_list, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(body_list, 0, 0);
    lv_obj_set_style_pad_all(body_list, 0, 0);
    lv_obj_set_style_pad_row(body_list, 8, 0);
    ui_scroll_apply_gutter(body_list);
    lv_obj_set_layout(body_list, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(body_list, LV_FLEX_FLOW_COLUMN);

    for (row_index = 0U; row_index < SCREEN_RECIPE_SECTION_MAX_ROWS; ++row_index)
    {
        section->row_objects[row_index] =
            screen_recipe_section_create_row(body_list,
                                             &section->row_title_labels[row_index],
                                             &section->row_value_labels[row_index]);
        lv_obj_add_flag(section->row_objects[row_index], LV_OBJ_FLAG_HIDDEN);
    }
}

void screen_recipe_section_show(screen_recipe_section_t *section,
                                const recipe_t *recipe,
                                recipe_section_id_t section_id)
{
    if (section == NULL || recipe == NULL)
    {
        return;
    }

    if (section->shown_recipe_id == recipe->id && section->shown_section_id == section_id)
    {
        return;
    }

    section->back_button_context.value = recipe->id;
    lv_label_set_text(section->title_label, screen_recipe_section_title(section_id));
    lv_label_set_text(section->recipe_label, recipe->name);

    for (uint32_t row_index = 0U; row_index < SCREEN_RECIPE_SECTION_MAX_ROWS; ++row_index)
    {
        lv_obj_add_flag(section->row_objects[row_index], LV_OBJ_FLAG_HIDDEN);
    }

    (void)screen_recipe_section_fill_rows(section, recipe, section_id);
    section->shown_recipe_id = recipe->id;
    section->shown_section_id = section_id;
}
