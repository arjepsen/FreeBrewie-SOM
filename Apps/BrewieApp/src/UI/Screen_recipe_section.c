#include "Screen_recipe_section.h"

#include <string.h>

static void screen_recipe_section_set_static(lv_obj_t *object);
static lv_obj_t *screen_recipe_section_create_header(lv_obj_t *parent, screen_recipe_section_t *section);
static lv_obj_t *screen_recipe_section_create_nav_button(lv_obj_t *parent,
                                                         const char *text,
                                                         lv_align_t align,
                                                         screen_recipe_section_button_context_t *context);
static lv_obj_t *screen_recipe_section_create_menu_button(lv_obj_t *parent, screen_recipe_section_t *section);
static const char *screen_recipe_section_title(recipe_section_id_t section_id);
static const char *screen_recipe_section_body(const recipe_t *recipe, recipe_section_id_t section_id);
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

static const char *screen_recipe_section_body(const recipe_t *recipe, recipe_section_id_t section_id)
{
    if (recipe == NULL)
    {
        return "--";
    }

    if (section_id == RECIPE_SECTION_INGREDIENTS)
    {
        return "Ingredients editing is not wired yet. This section will later show "
               "fermentables, hops, and additions from the recipe data model.";
    }

    if (section_id == RECIPE_SECTION_BREWING)
    {
        return recipe->mash;
    }

    if (section_id == RECIPE_SECTION_FERMENTATION)
    {
        return recipe->fermentation;
    }

    return recipe->summary;
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
    lv_obj_t *body_box;

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

    body_box = lv_obj_create(container);
    screen_recipe_section_set_static(body_box);
    lv_obj_set_width(body_box, lv_pct(100));
    lv_obj_set_flex_grow(body_box, 1);
    lv_obj_set_style_bg_color(body_box, lv_color_hex(0x141414), 0);
    lv_obj_set_style_bg_opa(body_box, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(body_box, lv_color_hex(0x2C2B2B), 0);
    lv_obj_set_style_border_width(body_box, 1, 0);
    lv_obj_set_style_pad_all(body_box, 10, 0);

    section->body_label = lv_label_create(body_box);
    lv_label_set_text(section->body_label, "--");
    lv_label_set_long_mode(section->body_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(section->body_label, lv_pct(100));
    lv_obj_set_style_text_color(section->body_label, lv_color_hex(0xD8D8D8), 0);
    lv_obj_align(section->body_label, LV_ALIGN_TOP_LEFT, 0, 0);
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
    lv_label_set_text(section->body_label, screen_recipe_section_body(recipe, section_id));
    section->shown_recipe_id = recipe->id;
    section->shown_section_id = section_id;
}
