#include "UI.h"

#include <string.h>

#include "UI_theme.h"

typedef struct
{
    ui_action_t action;
    ui_t *ui;
} ui_placeholder_button_context_t;

static void ui_handle_action(ui_action_t action, void *user_data);
static lv_obj_t *ui_create_menu_screen(ui_t *ui);
static lv_obj_t *ui_create_menu_row(lv_obj_t *parent,
                                    const char *text,
                                    ui_action_t action,
                                    ui_placeholder_button_context_t *context,
                                    ui_t *ui);
static lv_obj_t *ui_create_placeholder_screen(ui_t *ui, const char *title, const char *state, const char *message);
static lv_obj_t *ui_create_placeholder_button(lv_obj_t *parent,
                                              const char *text,
                                              ui_action_t action,
                                              ui_t *ui,
                                              ui_placeholder_button_context_t *context);
static void ui_placeholder_button_event_cb(lv_event_t *event);
static void ui_show_screen(ui_t *ui, ui_screen_id_t screen_id);

static ui_placeholder_button_context_t manual_back_context;
static ui_placeholder_button_context_t clean_back_context;
static ui_placeholder_button_context_t settings_back_context;
static ui_placeholder_button_context_t menu_home_context;
static ui_placeholder_button_context_t menu_status_context;
static ui_placeholder_button_context_t menu_clean_context;
static ui_placeholder_button_context_t menu_manual_context;
static ui_placeholder_button_context_t menu_settings_context;

static void ui_handle_action(ui_action_t action, void *user_data)
{
    ui_t *ui;
    ui_screen_id_t screen_id;

    ui = user_data;
    if (ui == NULL)
    {
        return;
    }

    screen_id = UI_SCREEN_HOME;
    if (action == UI_ACTION_SHOW_HOME)
    {
        screen_id = UI_SCREEN_HOME;
    }
    else if (action == UI_ACTION_SHOW_MENU)
    {
        screen_id = UI_SCREEN_MENU;
    }
    else if (action == UI_ACTION_SHOW_STATUS)
    {
        screen_id = UI_SCREEN_STATUS;
    }
    else if (action == UI_ACTION_SHOW_MANUAL)
    {
        screen_id = UI_SCREEN_MANUAL;
    }
    else if (action == UI_ACTION_SHOW_CLEAN)
    {
        screen_id = UI_SCREEN_CLEAN;
    }
    else if (action == UI_ACTION_SHOW_SETTINGS)
    {
        screen_id = UI_SCREEN_SETTINGS;
    }

    /*
     * LVGL allows many operations from event callbacks, but changing screens while the
     * current click/release event is still bubbling can be fragile. Queue navigation and
     * apply it during the normal UI update instead; that keeps button callbacks tiny and
     * avoids re-entering the refresh/event path.
     */
    ui->pending_screen = screen_id;
    ui->has_pending_screen = true;
}

/**
 * Create the full-screen menu inspired by the original Brewie MainMenu.
 *
 * The old UI used a slide-down menu with large dark rows. We keep the large row structure
 * because it is touch-friendly and cheap for LVGL to render, but use normal screen
 * navigation for now rather than animation.
 */
static lv_obj_t *ui_create_menu_screen(ui_t *ui)
{
    lv_obj_t *screen;
    lv_obj_t *container;
    lv_obj_t *spacer;

    screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    container = lv_obj_create(screen);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 8, 0);
    lv_obj_set_style_pad_row(container, 10, 0);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);

    spacer = lv_obj_create(container);
    lv_obj_remove_style_all(spacer);
    lv_obj_set_width(spacer, lv_pct(100));
    lv_obj_set_height(spacer, 24);

    ui_create_menu_row(container, "HOME", UI_ACTION_SHOW_HOME, &menu_home_context, ui);
    ui_create_menu_row(container, "STATUS", UI_ACTION_SHOW_STATUS, &menu_status_context, ui);
    ui_create_menu_row(container, "CLEAN", UI_ACTION_SHOW_CLEAN, &menu_clean_context, ui);
    ui_create_menu_row(container, "MANUAL", UI_ACTION_SHOW_MANUAL, &menu_manual_context, ui);
    ui_create_menu_row(container, "SETTINGS", UI_ACTION_SHOW_SETTINGS, &menu_settings_context, ui);

    spacer = lv_obj_create(container);
    lv_obj_remove_style_all(spacer);
    lv_obj_set_width(spacer, lv_pct(100));
    lv_obj_set_flex_grow(spacer, 1);

    ui_create_menu_row(container, "< BACK", UI_ACTION_SHOW_HOME, &menu_home_context, ui);

    return screen;
}

static lv_obj_t *ui_create_menu_row(lv_obj_t *parent,
                                    const char *text,
                                    ui_action_t action,
                                    ui_placeholder_button_context_t *context,
                                    ui_t *ui)
{
    lv_obj_t *button;
    lv_obj_t *label;

    context->action = action;
    context->ui = ui;

    button = lv_button_create(parent);
    lv_obj_set_width(button, lv_pct(100));
    lv_obj_set_height(button, 60);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x343434), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x454545), LV_STATE_PRESSED);
    lv_obj_set_style_border_width(button, 0, 0);
    lv_obj_set_style_radius(button, 0, 0);
    lv_obj_add_event_cb(button, ui_placeholder_button_event_cb, LV_EVENT_CLICKED, context);

    label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_center(label);

    return button;
}

/**
 * Create a safe placeholder screen for sections whose workflows are not implemented yet.
 *
 * These screens are intentionally real navigation targets. They prove the touch/navigation
 * shell while making it obvious that brewing, cleaning, settings, and service behavior have
 * not been wired to hardware actions.
 */
static lv_obj_t *ui_create_placeholder_screen(ui_t *ui, const char *title, const char *state, const char *message)
{
    lv_obj_t *screen;
    lv_obj_t *container;
    lv_obj_t *topbar;
    lv_obj_t *label;
    lv_obj_t *state_label;
    lv_obj_t *body;

    screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    container = lv_obj_create(screen);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 8, 0);
    lv_obj_set_style_pad_row(container, 8, 0);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);

    topbar = lv_obj_create(container);
    lv_obj_set_width(topbar, lv_pct(100));
    lv_obj_set_height(topbar, 46);
    lv_obj_set_style_bg_color(topbar, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_opa(topbar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(topbar, 0, 0);
    lv_obj_set_style_pad_all(topbar, 6, 0);
    lv_obj_set_layout(topbar, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(topbar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(topbar, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    label = lv_label_create(topbar);
    lv_label_set_text(label, title);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);

    state_label = lv_label_create(topbar);
    lv_label_set_text(state_label, state);
    lv_obj_set_style_bg_color(state_label, lv_color_hex(0xD39C35), 0);
    lv_obj_set_style_bg_opa(state_label, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(state_label, 4, 0);
    lv_obj_set_style_pad_left(state_label, 8, 0);
    lv_obj_set_style_pad_right(state_label, 8, 0);
    lv_obj_set_style_pad_top(state_label, 5, 0);
    lv_obj_set_style_pad_bottom(state_label, 5, 0);
    lv_obj_set_style_text_color(state_label, lv_color_hex(0x111111), 0);

    body = lv_label_create(container);
    lv_label_set_text(body, message);
    lv_label_set_long_mode(body, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(body, lv_pct(100));
    lv_obj_set_style_text_color(body, lv_color_hex(0xC8C8C8), 0);
    lv_obj_set_style_pad_top(body, 90, 0);

    if (ui->manual_screen == NULL)
    {
        ui_create_placeholder_button(container, "Back", UI_ACTION_SHOW_HOME, ui, &manual_back_context);
    }
    else if (ui->clean_screen == NULL)
    {
        ui_create_placeholder_button(container, "Back", UI_ACTION_SHOW_HOME, ui, &clean_back_context);
    }
    else
    {
        ui_create_placeholder_button(container, "Back", UI_ACTION_SHOW_HOME, ui, &settings_back_context);
    }

    return screen;
}

static lv_obj_t *ui_create_placeholder_button(lv_obj_t *parent,
                                              const char *text,
                                              ui_action_t action,
                                              ui_t *ui,
                                              ui_placeholder_button_context_t *context)
{
    lv_obj_t *button;
    lv_obj_t *label;

    if (parent == NULL || context == NULL)
    {
        return NULL;
    }

    context->action = action;
    context->ui = ui;

    button = lv_button_create(parent);
    lv_obj_set_width(button, lv_pct(100));
    lv_obj_set_height(button, 48);
    lv_obj_set_style_radius(button, 4, 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x393939), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x4A4A4A), LV_STATE_PRESSED);
    lv_obj_add_event_cb(button, ui_placeholder_button_event_cb, LV_EVENT_CLICKED, context);

    label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);

    return button;
}

static void ui_placeholder_button_event_cb(lv_event_t *event)
{
    ui_placeholder_button_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL || context->ui == NULL)
    {
        return;
    }

    ui_handle_action(context->action, context->ui);
}

static void ui_show_screen(ui_t *ui, ui_screen_id_t screen_id)
{
    lv_obj_t *screen;

    if (ui == NULL)
    {
        return;
    }

    screen = ui->home.screen;
    if (screen_id == UI_SCREEN_MENU)
    {
        screen = ui->menu_screen;
    }
    else if (screen_id == UI_SCREEN_STATUS)
    {
        screen = ui->status.screen;
    }
    else if (screen_id == UI_SCREEN_MANUAL)
    {
        screen = ui->manual_screen;
    }
    else if (screen_id == UI_SCREEN_CLEAN)
    {
        screen = ui->clean_screen;
    }
    else if (screen_id == UI_SCREEN_SETTINGS)
    {
        screen = ui->settings_screen;
    }

    if (screen == NULL)
    {
        return;
    }

    ui->current_screen = screen_id;
    lv_screen_load(screen);
    lv_refr_now(NULL);
}

void ui_init(ui_t *ui)
{
    if (ui == NULL)
    {
        return;
    }

    memset(ui, 0, sizeof(*ui));
    ui_theme_init();
    screen_home_init(&ui->home, ui_handle_action, ui);
    screen_status_init(&ui->status, ui_handle_action, ui);
    ui->menu_screen = ui_create_menu_screen(ui);
    ui->manual_screen = ui_create_placeholder_screen(ui,
                                                     "Manual",
                                                     "Locked",
                                                     "Manual controls are not enabled yet. "
                                                     "This screen will require app logic and "
                                                     "MCU interlocks before it can move hardware.");
    ui->clean_screen = ui_create_placeholder_screen(ui,
                                                    "Clean",
                                                    "Later",
                                                    "Clean workflows will be added after the "
                                                    "navigation shell is proven. Short clean, "
                                                    "full clean, drain, and unclogging are likely "
                                                    "future destinations.");
    ui->settings_screen = ui_create_placeholder_screen(ui,
                                                       "Settings",
                                                       "Later",
                                                       "Settings starts with display and touch facts, "
                                                       "then can grow into network, system, and "
                                                       "service options.");
    ui_show_screen(ui, UI_SCREEN_HOME);
}

void ui_update(ui_t *ui, const status_screen_view_model_t *view_model)
{
    if (ui == NULL)
    {
        return;
    }

    if (ui->has_pending_screen)
    {
        ui->has_pending_screen = false;
        ui_show_screen(ui, ui->pending_screen);
    }

    screen_home_update(&ui->home, view_model);
    screen_status_update(&ui->status, view_model);
}
