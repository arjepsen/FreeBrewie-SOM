#include "Screen_recipes.h"

#include <string.h>

#include "Logic/Recipe_catalog.h"
#include "UI_scroll.h"

#define SCREEN_RECIPES_PAD 8
#define SCREEN_RECIPES_ROW_WIDTH_PCT 98

static void screen_recipes_set_static(lv_obj_t *object);
static lv_obj_t *screen_recipes_create_header(lv_obj_t *parent, screen_recipes_t *recipes);
static lv_obj_t *screen_recipes_create_back_button(lv_obj_t *parent, screen_recipes_t *recipes);
static lv_obj_t *screen_recipes_create_menu_button(lv_obj_t *parent, screen_recipes_t *recipes);
static lv_obj_t *screen_recipes_create_search_box(lv_obj_t *parent);
static lv_obj_t *screen_recipes_create_recipe_row(lv_obj_t *parent,
                                                  const recipe_catalog_entry_t *recipe,
                                                  screen_recipes_button_context_t *context,
                                                  ui_action_handler_t action_handler,
                                                  void *user_data);
static lv_obj_t *screen_recipes_create_create_button(lv_obj_t *parent,
                                                     screen_recipes_button_context_t *context);
static void screen_recipes_button_event_cb(lv_event_t *event);

static void screen_recipes_set_static(lv_obj_t *object)
{
    if (object == NULL)
    {
        return;
    }

    lv_obj_clear_flag(object, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_OFF);
}

/**
 * Build the Recipes header using the same old-style shell language as Home.
 *
 * This keeps the first recipe browser familiar: back arrow on the left, centered title,
 * and hamburger menu on the right. The header only emits navigation requests.
 */
static lv_obj_t *screen_recipes_create_header(lv_obj_t *parent, screen_recipes_t *recipes)
{
    lv_obj_t *header;
    lv_obj_t *title;

    header = lv_obj_create(parent);
    screen_recipes_set_static(header);
    lv_obj_set_width(header, lv_pct(100));
    lv_obj_set_height(header, 64);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_all(header, 0, 0);

    screen_recipes_create_back_button(header, recipes);

    title = lv_label_create(header);
    lv_label_set_text(title, "Recipes");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xC8C8C8), 0);
    lv_obj_center(title);

    screen_recipes_create_menu_button(header, recipes);
    return header;
}

static lv_obj_t *screen_recipes_create_back_button(lv_obj_t *parent, screen_recipes_t *recipes)
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
                        screen_recipes_button_event_cb,
                        LV_EVENT_CLICKED,
                        &recipes->back_button_context);

    label = lv_label_create(button);
    lv_label_set_text(label, "<");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);

    return button;
}

/**
 * Draw a hamburger button from simple LVGL objects.
 *
 * Avoiding icon fonts keeps this predictable in the minimal target image and in the
 * simulator. The three small rectangles are static and cheap to draw.
 */
static lv_obj_t *screen_recipes_create_menu_button(lv_obj_t *parent, screen_recipes_t *recipes)
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
                        screen_recipes_button_event_cb,
                        LV_EVENT_CLICKED,
                        &recipes->menu_button_context);

    for (offset_y = -7; offset_y <= 7; offset_y = (int8_t)(offset_y + 7))
    {
        line = lv_obj_create(button);
        screen_recipes_set_static(line);
        lv_obj_set_size(line, 20, 2);
        lv_obj_set_style_bg_color(line, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_bg_opa(line, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(line, 0, 0);
        lv_obj_align(line, LV_ALIGN_CENTER, 0, offset_y);
    }

    return button;
}

/**
 * Create a non-editing search/filter field.
 *
 * The original recipe chooser had search. We show the shape now, but keep it inert until
 * there is real recipe storage and a real input overlay behind it.
 */
static lv_obj_t *screen_recipes_create_search_box(lv_obj_t *parent)
{
    lv_obj_t *box;
    lv_obj_t *label;

    box = lv_obj_create(parent);
    screen_recipes_set_static(box);
    lv_obj_set_width(box, lv_pct(100));
    lv_obj_set_height(box, 42);
    lv_obj_set_style_bg_color(box, lv_color_hex(0x1F1F1F), 0);
    lv_obj_set_style_bg_opa(box, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(box, lv_color_hex(0x393939), 0);
    lv_obj_set_style_border_width(box, 1, 0);
    lv_obj_set_style_radius(box, 0, 0);
    lv_obj_set_style_pad_left(box, 10, 0);

    label = lv_label_create(box);
    lv_label_set_text(label, "Search recipes");
    lv_obj_set_style_text_color(label, lv_color_hex(0x8C8C8C), 0);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);

    return box;
}

/**
 * Create one static recipe-list row.
 *
 * The chooser only needs the title and the most important category/style text. Longer
 * descriptions belong on the detail screen, where there is enough vertical room for them.
 */
static lv_obj_t *screen_recipes_create_recipe_row(lv_obj_t *parent,
                                                  const recipe_catalog_entry_t *recipe,
                                                  screen_recipes_button_context_t *context,
                                                  ui_action_handler_t action_handler,
                                                  void *user_data)
{
    lv_obj_t *row;
    lv_obj_t *accent;
    lv_obj_t *text_block;
    lv_obj_t *name_label;
    lv_obj_t *style_label;

    if (parent == NULL || recipe == NULL || context == NULL)
    {
        return NULL;
    }

    context->action = UI_ACTION_SHOW_RECIPE_DETAIL;
    context->value = recipe->id;
    context->handler = action_handler;
    context->user_data = user_data;

    row = lv_button_create(parent);
    lv_obj_set_width(row, lv_pct(SCREEN_RECIPES_ROW_WIDTH_PCT));
    lv_obj_set_height(row, 64);
    lv_obj_set_style_align(row, LV_ALIGN_LEFT_MID, 0);
    lv_obj_set_style_bg_color(row, lv_color_hex(0x282828), 0);
    lv_obj_set_style_bg_color(row, lv_color_hex(0x3B332D), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(row, lv_color_hex(0x343434), 0);
    lv_obj_set_style_border_width(row, 1, 0);
    lv_obj_set_style_radius(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_layout(row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_event_cb(row, screen_recipes_button_event_cb, LV_EVENT_CLICKED, context);

    accent = lv_obj_create(row);
    screen_recipes_set_static(accent);
    lv_obj_set_size(accent, 10, 62);
    lv_obj_set_style_bg_color(accent, lv_color_hex(recipe->accent_color), 0);
    lv_obj_set_style_bg_opa(accent, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(accent, 0, 0);
    lv_obj_set_style_radius(accent, 0, 0);

    text_block = lv_obj_create(row);
    lv_obj_remove_style_all(text_block);
    screen_recipes_set_static(text_block);
    lv_obj_set_width(text_block, 228);
    lv_obj_set_height(text_block, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_left(text_block, 8, 0);
    lv_obj_set_style_pad_right(text_block, 6, 0);
    lv_obj_set_style_pad_row(text_block, 5, 0);
    lv_obj_set_layout(text_block, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(text_block, LV_FLEX_FLOW_COLUMN);

    name_label = lv_label_create(text_block);
    lv_label_set_text(name_label, recipe->name);
    lv_label_set_long_mode(name_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(name_label, lv_pct(100));
    lv_obj_set_style_text_color(name_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(name_label, &lv_font_montserrat_20, 0);

    style_label = lv_label_create(text_block);
    lv_label_set_text(style_label, recipe->style);
    lv_label_set_long_mode(style_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(style_label, lv_pct(100));
    lv_obj_set_style_text_color(style_label, lv_color_hex(0xE67526), 0);

    /*
     * The parent row is the only clickable object. Children render text/accent only, which
     * keeps hit testing predictable on the physical touch panel.
     */
    lv_obj_remove_flag(accent, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(text_block, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(name_label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(style_label, LV_OBJ_FLAG_CLICKABLE);

    return row;
}

static lv_obj_t *screen_recipes_create_create_button(lv_obj_t *parent,
                                                     screen_recipes_button_context_t *context)
{
    lv_obj_t *button;
    lv_obj_t *label;

    button = lv_button_create(parent);
    lv_obj_set_width(button, lv_pct(100));
    lv_obj_set_height(button, 48);
    lv_obj_set_style_bg_color(button, lv_color_hex(0xF47B32), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0xC85F22), LV_STATE_PRESSED);
    lv_obj_set_style_radius(button, 5, 0);
    lv_obj_add_event_cb(button, screen_recipes_button_event_cb, LV_EVENT_CLICKED, context);

    label = lv_label_create(button);
    lv_label_set_text(label, "CREATE RECIPE");
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);

    return button;
}

static void screen_recipes_button_event_cb(lv_event_t *event)
{
    screen_recipes_button_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL || context->handler == NULL)
    {
        return;
    }

    context->handler(context->action, context->value, context->user_data);
}

void screen_recipes_init(screen_recipes_t *recipes, ui_action_handler_t action_handler, void *user_data)
{
    lv_obj_t *container;
    lv_obj_t *list;
    lv_obj_t *notice;
    lv_obj_t *spacer;
    size_t recipe_index;
    size_t recipe_count;

    if (recipes == NULL)
    {
        return;
    }

    memset(recipes, 0, sizeof(*recipes));
    recipes->back_button_context.action = UI_ACTION_SHOW_HOME;
    recipes->back_button_context.handler = action_handler;
    recipes->back_button_context.user_data = user_data;
    recipes->menu_button_context.action = UI_ACTION_SHOW_MENU;
    recipes->menu_button_context.handler = action_handler;
    recipes->menu_button_context.user_data = user_data;
    recipes->create_button_context.action = UI_ACTION_SHOW_RECIPE_BUILDER;
    recipes->create_button_context.handler = action_handler;
    recipes->create_button_context.user_data = user_data;

    recipes->screen = lv_obj_create(NULL);
    screen_recipes_set_static(recipes->screen);
    lv_obj_set_style_bg_color(recipes->screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(recipes->screen, LV_OPA_COVER, 0);

    container = lv_obj_create(recipes->screen);
    screen_recipes_set_static(container);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, SCREEN_RECIPES_PAD, 0);
    lv_obj_set_style_pad_row(container, 8, 0);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);

    screen_recipes_create_header(container, recipes);
    screen_recipes_create_search_box(container);

    list = lv_obj_create(container);
    lv_obj_set_width(list, lv_pct(100));
    lv_obj_set_height(list, 180);
    lv_obj_set_flex_grow(list, 1);
    lv_obj_set_style_bg_color(list, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(list, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(list, 0, 0);
    lv_obj_set_style_pad_all(list, 0, 0);
    lv_obj_set_style_pad_row(list, 8, 0);
    ui_scroll_apply_gutter(list);
    lv_obj_add_flag(list, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_layout(list, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);

    recipe_count = recipe_catalog_count();
    if (recipe_count > (sizeof(recipes->recipe_row_contexts) / sizeof(recipes->recipe_row_contexts[0])))
    {
        recipe_count = sizeof(recipes->recipe_row_contexts) / sizeof(recipes->recipe_row_contexts[0]);
    }

    for (recipe_index = 0U; recipe_index < recipe_count; ++recipe_index)
    {
        screen_recipes_create_recipe_row(list,
                                         recipe_catalog_get_by_index(recipe_index),
                                         &recipes->recipe_row_contexts[recipe_index],
                                         action_handler,
                                         user_data);
    }

    notice = lv_label_create(container);
    lv_label_set_text(notice, "Recipe storage and brewing actions are not wired yet.");
    lv_label_set_long_mode(notice, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(notice, lv_pct(100));
    lv_obj_set_style_text_color(notice, lv_color_hex(0x8C8C8C), 0);

    spacer = lv_obj_create(container);
    lv_obj_remove_style_all(spacer);
    screen_recipes_set_static(spacer);
    lv_obj_set_width(spacer, lv_pct(100));
    lv_obj_set_height(spacer, 0);

    screen_recipes_create_create_button(container, &recipes->create_button_context);
}
