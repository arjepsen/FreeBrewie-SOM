#include "Screen_recipe_draft_ingredients.h"

#include <stdio.h>
#include <string.h>

#include "UI_scroll.h"

static void screen_recipe_draft_ingredients_set_static(lv_obj_t *object);
static lv_obj_t *screen_recipe_draft_ingredients_create_header(
    lv_obj_t *parent,
    screen_recipe_draft_ingredients_t *ingredients);
static lv_obj_t *screen_recipe_draft_ingredients_create_nav_button(
    lv_obj_t *parent,
    const char *text,
    lv_align_t align,
    screen_recipe_draft_ingredients_nav_context_t *context);
static lv_obj_t *screen_recipe_draft_ingredients_create_tab_button(
    lv_obj_t *parent,
    const char *text,
    screen_recipe_draft_ingredients_tab_context_t *context);
static lv_obj_t *screen_recipe_draft_ingredients_create_body(lv_obj_t *parent);
static lv_obj_t *screen_recipe_draft_ingredients_create_group(lv_obj_t *parent,
                                                              const char *caption);
static void screen_recipe_draft_ingredients_create_value_row(lv_obj_t *parent,
                                                             const char *label_text,
                                                             const char *value_text);
static lv_obj_t *screen_recipe_draft_ingredients_create_disabled_modify_button(lv_obj_t *parent);
static void screen_recipe_draft_ingredients_set_text_if_changed(lv_obj_t *label, const char *text);
static void screen_recipe_draft_ingredients_rebuild_fermentables(
    screen_recipe_draft_ingredients_t *ingredients,
    const recipe_draft_t *draft);
static void screen_recipe_draft_ingredients_rebuild_hops(screen_recipe_draft_ingredients_t *ingredients,
                                                         const recipe_draft_t *draft);
static void screen_recipe_draft_ingredients_set_active_tab(
    screen_recipe_draft_ingredients_t *ingredients,
    screen_recipe_draft_ingredients_tab_t tab);
static void screen_recipe_draft_ingredients_nav_event_cb(lv_event_t *event);
static void screen_recipe_draft_ingredients_tab_event_cb(lv_event_t *event);

/****************************************************************************************
 * @brief Make an object static so it cannot become a tiny scroll target.
 ****************************************************************************************/
static void screen_recipe_draft_ingredients_set_static(lv_obj_t *object)
{
    if (object == NULL)
    {
        return;
    }

    lv_obj_clear_flag(object, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_OFF);
}

static lv_obj_t *screen_recipe_draft_ingredients_create_header(
    lv_obj_t *parent,
    screen_recipe_draft_ingredients_t *ingredients)
{
    lv_obj_t *header;
    lv_obj_t *title;

    header = lv_obj_create(parent);
    screen_recipe_draft_ingredients_set_static(header);
    lv_obj_set_width(header, lv_pct(100));
    lv_obj_set_height(header, 64);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_all(header, 0, 0);

    screen_recipe_draft_ingredients_create_nav_button(header,
                                                      "<",
                                                      LV_ALIGN_LEFT_MID,
                                                      &ingredients->back_button_context);

    title = lv_label_create(header);
    lv_label_set_text(title, "Ingredients");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(title);
    return header;
}

static lv_obj_t *screen_recipe_draft_ingredients_create_nav_button(
    lv_obj_t *parent,
    const char *text,
    lv_align_t align,
    screen_recipe_draft_ingredients_nav_context_t *context)
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
    lv_obj_add_event_cb(button, screen_recipe_draft_ingredients_nav_event_cb, LV_EVENT_CLICKED, context);

    label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);
    return button;
}

static lv_obj_t *screen_recipe_draft_ingredients_create_tab_button(
    lv_obj_t *parent,
    const char *text,
    screen_recipe_draft_ingredients_tab_context_t *context)
{
    lv_obj_t *button;
    lv_obj_t *label;

    button = lv_button_create(parent);
    lv_obj_set_flex_grow(button, 1);
    lv_obj_set_height(button, 40);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x292929), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x3B332D), LV_STATE_PRESSED);
    lv_obj_set_style_border_color(button, lv_color_hex(0x3F3F3F), 0);
    lv_obj_set_style_border_width(button, 1, 0);
    lv_obj_set_style_radius(button, 0, 0);
    lv_obj_add_event_cb(button, screen_recipe_draft_ingredients_tab_event_cb, LV_EVENT_CLICKED, context);

    label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(label, lv_pct(100));
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);
    lv_obj_remove_flag(label, LV_OBJ_FLAG_CLICKABLE);
    return button;
}

static lv_obj_t *screen_recipe_draft_ingredients_create_body(lv_obj_t *parent)
{
    lv_obj_t *body;

    body = lv_obj_create(parent);
    lv_obj_set_width(body, lv_pct(100));
    lv_obj_set_flex_grow(body, 1);
    lv_obj_set_style_bg_opa(body, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(body, 0, 0);
    lv_obj_set_style_pad_all(body, 0, 0);
    lv_obj_set_style_pad_row(body, 8, 0);
    ui_scroll_apply_gutter(body);
    lv_obj_set_layout(body, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(body, LV_FLEX_FLOW_COLUMN);
    return body;
}

static lv_obj_t *screen_recipe_draft_ingredients_create_group(lv_obj_t *parent,
                                                              const char *caption)
{
    lv_obj_t *group;
    lv_obj_t *caption_label;

    group = lv_obj_create(parent);
    lv_obj_set_width(group, lv_pct(97));
    lv_obj_set_height(group, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(group, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(group, 0, 0);
    lv_obj_set_style_pad_all(group, 8, 0);
    lv_obj_set_style_pad_row(group, 7, 0);
    lv_obj_set_layout(group, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(group, LV_FLEX_FLOW_COLUMN);

    caption_label = lv_label_create(group);
    lv_label_set_text(caption_label, caption);
    lv_obj_set_width(caption_label, lv_pct(100));
    lv_obj_set_style_text_color(caption_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(caption_label, &lv_font_montserrat_20, 0);
    return group;
}

static void screen_recipe_draft_ingredients_create_value_row(lv_obj_t *parent,
                                                             const char *label_text,
                                                             const char *value_text)
{
    lv_obj_t *row;
    lv_obj_t *label;
    lv_obj_t *value;

    row = lv_obj_create(parent);
    screen_recipe_draft_ingredients_set_static(row);
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
}

static lv_obj_t *screen_recipe_draft_ingredients_create_disabled_modify_button(lv_obj_t *parent)
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
 * @brief Avoid writing unchanged label text into LVGL.
 ****************************************************************************************/
static void screen_recipe_draft_ingredients_set_text_if_changed(lv_obj_t *label, const char *text)
{
    if (label == NULL || text == NULL || strcmp(lv_label_get_text(label), text) == 0)
    {
        return;
    }

    lv_label_set_text(label, text);
}

/****************************************************************************************
 * @brief Rebuild the small fermentables group from the RAM-only draft model.
 *
 * This is not a per-frame path; it runs before showing the Ingredients screen. Rebuilding
 * a few rows keeps the code simple while the draft model is still growing.
 ****************************************************************************************/
static void screen_recipe_draft_ingredients_rebuild_fermentables(
    screen_recipe_draft_ingredients_t *ingredients,
    const recipe_draft_t *draft)
{
    char amount_text[16];
    uint8_t index;

    if (ingredients == NULL || draft == NULL)
    {
        return;
    }

    if (ingredients->fermentables_group != NULL)
    {
        lv_obj_delete(ingredients->fermentables_group);
    }

    ingredients->fermentables_group =
        screen_recipe_draft_ingredients_create_group(ingredients->fermentables_body, "BAG 1");
    if (draft->fermentable_count == 0U)
    {
        screen_recipe_draft_ingredients_create_value_row(ingredients->fermentables_group,
                                                         "No fermentables",
                                                         "--");
        return;
    }

    for (index = 0U; index < draft->fermentable_count && index < RECIPE_DRAFT_MAX_FERMENTABLES; ++index)
    {
        snprintf(amount_text, sizeof(amount_text), "%u g", (unsigned int)draft->fermentables[index].amount_g);
        screen_recipe_draft_ingredients_create_value_row(ingredients->fermentables_group,
                                                         draft->fermentables[index].name,
                                                         amount_text);
    }
}

/****************************************************************************************
 * @brief Rebuild the small hops group from the RAM-only draft model.
 ****************************************************************************************/
static void screen_recipe_draft_ingredients_rebuild_hops(screen_recipe_draft_ingredients_t *ingredients,
                                                         const recipe_draft_t *draft)
{
    char amount_text[24];
    uint8_t index;

    if (ingredients == NULL || draft == NULL)
    {
        return;
    }

    if (ingredients->hops_group != NULL)
    {
        lv_obj_delete(ingredients->hops_group);
    }

    ingredients->hops_group = screen_recipe_draft_ingredients_create_group(ingredients->hops_body, "HOP CAGE 1");
    if (draft->hop_count == 0U)
    {
        screen_recipe_draft_ingredients_create_value_row(ingredients->hops_group, "No hops", "--");
        return;
    }

    for (index = 0U; index < draft->hop_count && index < RECIPE_DRAFT_MAX_HOPS; ++index)
    {
        snprintf(amount_text,
                 sizeof(amount_text),
                 "%u g / %u min",
                 (unsigned int)draft->hops[index].amount_g,
                 (unsigned int)draft->hops[index].boil_time_min);
        screen_recipe_draft_ingredients_create_value_row(ingredients->hops_group,
                                                         draft->hops[index].name,
                                                         amount_text);
    }
}

/****************************************************************************************
 * @brief Show one tab body and style the active tab in orange.
 ****************************************************************************************/
static void screen_recipe_draft_ingredients_set_active_tab(
    screen_recipe_draft_ingredients_t *ingredients,
    screen_recipe_draft_ingredients_tab_t tab)
{
    if (ingredients == NULL)
    {
        return;
    }

    ingredients->active_tab = tab;
    if (tab == SCREEN_RECIPE_DRAFT_INGREDIENTS_TAB_FERMENTABLES)
    {
        lv_obj_remove_flag(ingredients->fermentables_body, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ingredients->hops_body, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_add_flag(ingredients->fermentables_body, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(ingredients->hops_body, LV_OBJ_FLAG_HIDDEN);
    }

    lv_obj_set_style_bg_color(ingredients->fermentables_button,
                              tab == SCREEN_RECIPE_DRAFT_INGREDIENTS_TAB_FERMENTABLES
                                  ? lv_color_hex(0xE67526)
                                  : lv_color_hex(0x292929),
                              0);
    lv_obj_set_style_bg_color(ingredients->hops_button,
                              tab == SCREEN_RECIPE_DRAFT_INGREDIENTS_TAB_HOPS
                                  ? lv_color_hex(0xE67526)
                                  : lv_color_hex(0x292929),
                              0);
}

static void screen_recipe_draft_ingredients_nav_event_cb(lv_event_t *event)
{
    screen_recipe_draft_ingredients_nav_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL || context->handler == NULL)
    {
        return;
    }

    context->handler(context->action, context->value, context->user_data);
}

static void screen_recipe_draft_ingredients_tab_event_cb(lv_event_t *event)
{
    screen_recipe_draft_ingredients_tab_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL || context->ingredients == NULL)
    {
        return;
    }

    screen_recipe_draft_ingredients_set_active_tab(context->ingredients, context->tab);
}

void screen_recipe_draft_ingredients_init(screen_recipe_draft_ingredients_t *ingredients,
                                          ui_action_handler_t action_handler,
                                          void *user_data)
{
    lv_obj_t *container;
    lv_obj_t *tabs;

    if (ingredients == NULL)
    {
        return;
    }

    memset(ingredients, 0, sizeof(*ingredients));
    ingredients->back_button_context.action = UI_ACTION_SHOW_RECIPE_DRAFT_MENU;
    ingredients->back_button_context.handler = action_handler;
    ingredients->back_button_context.user_data = user_data;
    ingredients->tab_contexts[0].tab = SCREEN_RECIPE_DRAFT_INGREDIENTS_TAB_FERMENTABLES;
    ingredients->tab_contexts[0].ingredients = ingredients;
    ingredients->tab_contexts[1].tab = SCREEN_RECIPE_DRAFT_INGREDIENTS_TAB_HOPS;
    ingredients->tab_contexts[1].ingredients = ingredients;

    ingredients->screen = lv_obj_create(NULL);
    screen_recipe_draft_ingredients_set_static(ingredients->screen);
    lv_obj_set_style_bg_color(ingredients->screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(ingredients->screen, LV_OPA_COVER, 0);

    container = lv_obj_create(ingredients->screen);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container, lv_color_hex(0x141414), 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 8, 0);
    lv_obj_set_style_pad_row(container, 8, 0);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);

    screen_recipe_draft_ingredients_create_header(container, ingredients);

    ingredients->name_label = lv_label_create(container);
    lv_label_set_text(ingredients->name_label, "--");
    lv_label_set_long_mode(ingredients->name_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(ingredients->name_label, lv_pct(100));
    lv_obj_set_style_text_color(ingredients->name_label, lv_color_hex(0xE67526), 0);

    tabs = lv_obj_create(container);
    screen_recipe_draft_ingredients_set_static(tabs);
    lv_obj_set_width(tabs, lv_pct(100));
    lv_obj_set_height(tabs, 40);
    lv_obj_set_style_bg_opa(tabs, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(tabs, 0, 0);
    lv_obj_set_style_pad_all(tabs, 0, 0);
    lv_obj_set_style_pad_column(tabs, 8, 0);
    lv_obj_set_layout(tabs, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(tabs, LV_FLEX_FLOW_ROW);

    ingredients->fermentables_button =
        screen_recipe_draft_ingredients_create_tab_button(tabs,
                                                          "Fermentables",
                                                          &ingredients->tab_contexts[0]);
    ingredients->hops_button =
        screen_recipe_draft_ingredients_create_tab_button(tabs,
                                                          "Hops",
                                                          &ingredients->tab_contexts[1]);

    ingredients->fermentables_body = screen_recipe_draft_ingredients_create_body(container);

    ingredients->hops_body = screen_recipe_draft_ingredients_create_body(container);

    screen_recipe_draft_ingredients_create_disabled_modify_button(container);
    screen_recipe_draft_ingredients_set_active_tab(ingredients,
                                                   SCREEN_RECIPE_DRAFT_INGREDIENTS_TAB_FERMENTABLES);
}

/****************************************************************************************
 * @brief Show RAM-only draft ingredients without reading or saving a real recipe.
 ****************************************************************************************/
void screen_recipe_draft_ingredients_show(screen_recipe_draft_ingredients_t *ingredients,
                                          const recipe_draft_t *draft)
{
    const char *draft_name;

    if (ingredients == NULL || draft == NULL)
    {
        return;
    }

    draft_name = recipe_draft_get_name(draft);
    if (ingredients->shown_name != draft_name)
    {
        screen_recipe_draft_ingredients_set_text_if_changed(ingredients->name_label, draft_name);
        ingredients->shown_name = draft_name;
    }

    screen_recipe_draft_ingredients_rebuild_fermentables(ingredients, draft);
    screen_recipe_draft_ingredients_rebuild_hops(ingredients, draft);
}
