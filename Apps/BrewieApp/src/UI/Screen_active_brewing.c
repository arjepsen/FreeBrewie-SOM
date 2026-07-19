#include "Screen_active_brewing.h"

#include <stdio.h>
#include <string.h>

#define SCREEN_ACTIVE_BREWING_PAD 8

static void screen_active_brewing_set_static(lv_obj_t *object);
static void screen_active_brewing_set_label_text(lv_obj_t *label, const char *text);
static bool screen_active_brewing_machine_matches(const status_machine_snapshot_t *left,
                                                  const status_machine_snapshot_t *right);
static bool screen_active_brewing_process_matches(const brewing_process_view_model_t *left,
                                                  const brewing_process_view_model_t *right);
static void screen_active_brewing_style_process_step(screen_active_brewing_t *active_brewing,
                                                     brewing_process_stage_t stage,
                                                     bool active);
static void screen_active_brewing_format_temperature(char *buffer,
                                                     size_t buffer_size,
                                                     int16_t temperature_c_x10,
                                                     uint8_t target_c,
                                                     bool valid);
static lv_obj_t *screen_active_brewing_create_header(lv_obj_t *parent,
                                                     screen_active_brewing_t *active_brewing);
static lv_obj_t *screen_active_brewing_create_nav_button(lv_obj_t *parent,
                                                         const char *text,
                                                         lv_align_t align,
                                                         screen_active_brewing_nav_context_t *context);
static lv_obj_t *screen_active_brewing_create_menu_button(lv_obj_t *parent,
                                                          screen_active_brewing_t *active_brewing);
static lv_obj_t *screen_active_brewing_create_tab_button(lv_obj_t *parent,
                                                         const char *text,
                                                         screen_active_brewing_tab_context_t *context);
static lv_obj_t *screen_active_brewing_create_tank(lv_obj_t *parent,
                                                   const char *title,
                                                   const char *temperature_text,
                                                   bool filled,
                                                   lv_obj_t **temperature_label);
static lv_obj_t *screen_active_brewing_create_disabled_control_button(lv_obj_t *parent,
                                                                      const char *text,
                                                                      uint32_t color);
static lv_obj_t *screen_active_brewing_create_process_step(lv_obj_t *parent,
                                                           screen_active_brewing_t *active_brewing,
                                                           const char *text,
                                                           brewing_process_stage_t stage);
static void screen_active_brewing_create_process_strip(screen_active_brewing_t *active_brewing,
                                                       lv_obj_t *parent);
static void screen_active_brewing_create_overall_page(screen_active_brewing_t *active_brewing,
                                                      lv_obj_t *parent);
static void screen_active_brewing_create_actions_page(screen_active_brewing_t *active_brewing,
                                                      lv_obj_t *parent);
static void screen_active_brewing_select_tab(screen_active_brewing_t *active_brewing,
                                             screen_active_brewing_tab_id_t tab_id);
static void screen_active_brewing_nav_event_cb(lv_event_t *event);
static void screen_active_brewing_tab_event_cb(lv_event_t *event);

/****************************************************************************************
 * @brief Make an object static so small visual pieces do not become scroll targets.
 ****************************************************************************************/
static void screen_active_brewing_set_static(lv_obj_t *object)
{
    if (object == NULL)
    {
        return;
    }

    lv_obj_clear_flag(object, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_OFF);
}

/****************************************************************************************
 * @brief Update an LVGL label only when the visible text really changed.
 ****************************************************************************************/
static void screen_active_brewing_set_label_text(lv_obj_t *label, const char *text)
{
    const char *current_text;

    if (label == NULL || text == NULL)
    {
        return;
    }

    current_text = lv_label_get_text(label);
    if (current_text != NULL && strcmp(current_text, text) == 0)
    {
        return;
    }

    lv_label_set_text(label, text);
}

/****************************************************************************************
 * @brief Compare the raw fields currently used by this screen.
 ****************************************************************************************/
static bool screen_active_brewing_machine_matches(const status_machine_snapshot_t *left,
                                                  const status_machine_snapshot_t *right)
{
    if (left == NULL || right == NULL)
    {
        return false;
    }

    return left->mcu_status_valid == right->mcu_status_valid &&
           left->mash_target_c == right->mash_target_c &&
           left->boil_target_c == right->boil_target_c &&
           left->mash_temp_c_x10 == right->mash_temp_c_x10 &&
           left->boil_temp_c_x10 == right->boil_temp_c_x10 &&
           left->mash_pump_setpoint == right->mash_pump_setpoint &&
           left->boil_pump_setpoint == right->boil_pump_setpoint &&
           left->mash_pump_running == right->mash_pump_running &&
           left->boil_pump_running == right->boil_pump_running &&
           left->pressure_count == right->pressure_count &&
           left->fault_flags == right->fault_flags;
}

/****************************************************************************************
 * @brief Compare the process presentation fields currently used by this screen.
 ****************************************************************************************/
static bool screen_active_brewing_process_matches(const brewing_process_view_model_t *left,
                                                  const brewing_process_view_model_t *right)
{
    if (left == NULL || right == NULL)
    {
        return false;
    }

    return left->current_stage == right->current_stage &&
           left->state_text == right->state_text &&
           left->detail_text == right->detail_text &&
           left->progress_percent == right->progress_percent &&
           left->pause_enabled == right->pause_enabled &&
           left->stop_enabled == right->stop_enabled;
}

/****************************************************************************************
 * @brief Apply active/inactive colors to one process-strip stage.
 ****************************************************************************************/
static void screen_active_brewing_style_process_step(screen_active_brewing_t *active_brewing,
                                                     brewing_process_stage_t stage,
                                                     bool active)
{
    if (active_brewing == NULL || stage >= BREWING_PROCESS_STAGE_COUNT)
    {
        return;
    }

    lv_obj_set_style_bg_color(active_brewing->process_step_markers[stage],
                              active ? lv_color_hex(0xF47B32) : lv_color_hex(0x343434),
                              0);
    lv_obj_set_style_border_color(active_brewing->process_step_markers[stage],
                                  active ? lv_color_hex(0xF6B27D) : lv_color_hex(0x4B4741),
                                  0);
    lv_obj_set_style_text_color(active_brewing->process_step_labels[stage],
                                active ? lv_color_hex(0xFFFFFF) : lv_color_hex(0x8C8C8C),
                                0);
}

/****************************************************************************************
 * @brief Format one compact temperature/target value for the active process view.
 ****************************************************************************************/
static void screen_active_brewing_format_temperature(char *buffer,
                                                     size_t buffer_size,
                                                     int16_t temperature_c_x10,
                                                     uint8_t target_c,
                                                     bool valid)
{
    int16_t whole_c;
    int16_t decimal_c;

    if (buffer == NULL || buffer_size == 0U)
    {
        return;
    }

    if (!valid)
    {
        snprintf(buffer, buffer_size, "--.- / %u C", (unsigned int)target_c);
        return;
    }

    whole_c = (int16_t)(temperature_c_x10 / 10);
    decimal_c = (int16_t)(temperature_c_x10 % 10);
    if (decimal_c < 0)
    {
        decimal_c = (int16_t)-decimal_c;
    }

    snprintf(buffer,
             buffer_size,
             "%d.%d / %u C",
             (int)whole_c,
             (int)decimal_c,
             (unsigned int)target_c);
}

static lv_obj_t *screen_active_brewing_create_header(lv_obj_t *parent,
                                                     screen_active_brewing_t *active_brewing)
{
    lv_obj_t *header;
    lv_obj_t *title;

    header = lv_obj_create(parent);
    screen_active_brewing_set_static(header);
    lv_obj_set_width(header, lv_pct(100));
    lv_obj_set_height(header, 64);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_all(header, 0, 0);

    screen_active_brewing_create_nav_button(header,
                                            "<",
                                            LV_ALIGN_LEFT_MID,
                                            &active_brewing->back_button_context);

    title = lv_label_create(header);
    lv_label_set_text(title, "Brewing");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xC8C8C8), 0);
    lv_obj_center(title);

    screen_active_brewing_create_menu_button(header, active_brewing);
    return header;
}

static lv_obj_t *screen_active_brewing_create_nav_button(lv_obj_t *parent,
                                                         const char *text,
                                                         lv_align_t align,
                                                         screen_active_brewing_nav_context_t *context)
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
    lv_obj_add_event_cb(button, screen_active_brewing_nav_event_cb, LV_EVENT_CLICKED, context);

    label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);
    return button;
}

static lv_obj_t *screen_active_brewing_create_menu_button(lv_obj_t *parent,
                                                          screen_active_brewing_t *active_brewing)
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
                        screen_active_brewing_nav_event_cb,
                        LV_EVENT_CLICKED,
                        &active_brewing->menu_button_context);

    for (offset_y = -7; offset_y <= 7; offset_y = (int8_t)(offset_y + 7))
    {
        line = lv_obj_create(button);
        screen_active_brewing_set_static(line);
        lv_obj_set_size(line, 20, 2);
        lv_obj_set_style_bg_color(line, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_bg_opa(line, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(line, 0, 0);
        lv_obj_align(line, LV_ALIGN_CENTER, 0, offset_y);
    }

    return button;
}

static lv_obj_t *screen_active_brewing_create_tab_button(lv_obj_t *parent,
                                                         const char *text,
                                                         screen_active_brewing_tab_context_t *context)
{
    lv_obj_t *button;
    lv_obj_t *label;

    button = lv_button_create(parent);
    lv_obj_set_width(button, lv_pct(50));
    lv_obj_set_height(button, 34);
    lv_obj_set_style_radius(button, 0, 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0xD96F28), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0xF47B32), LV_STATE_CHECKED);
    lv_obj_set_style_border_width(button, 0, 0);
    lv_obj_add_event_cb(button, screen_active_brewing_tab_event_cb, LV_EVENT_CLICKED, context);

    label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_hex(0x111111), 0);
    lv_obj_center(label);
    return button;
}

static lv_obj_t *screen_active_brewing_create_tank(lv_obj_t *parent,
                                                   const char *title,
                                                   const char *temperature_text,
                                                   bool filled,
                                                   lv_obj_t **temperature_label)
{
    lv_obj_t *tank;
    lv_obj_t *fill;
    lv_obj_t *label;

    tank = lv_obj_create(parent);
    screen_active_brewing_set_static(tank);
    lv_obj_set_width(tank, 104);
    lv_obj_set_height(tank, 138);
    lv_obj_set_style_bg_color(tank, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_opa(tank, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(tank, lv_color_hex(0x4B4741), 0);
    lv_obj_set_style_border_width(tank, 2, 0);
    lv_obj_set_style_radius(tank, 4, 0);
    lv_obj_set_style_pad_all(tank, 6, 0);

    fill = lv_obj_create(tank);
    screen_active_brewing_set_static(fill);
    lv_obj_set_width(fill, lv_pct(100));
    lv_obj_set_height(fill, filled ? 44 : 16);
    lv_obj_align(fill, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(fill, filled ? lv_color_hex(0x55B047) : lv_color_hex(0x38342E), 0);
    lv_obj_set_style_bg_opa(fill, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(fill, 0, 0);
    lv_obj_set_style_radius(fill, 0, 0);

    label = lv_label_create(tank);
    lv_label_set_text(label, title);
    lv_obj_set_width(label, lv_pct(100));
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 8);

    label = lv_label_create(tank);
    lv_label_set_text(label, temperature_text);
    lv_obj_set_width(label, lv_pct(100));
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xE67526), 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -6);
    if (temperature_label != NULL)
    {
        *temperature_label = label;
    }

    return tank;
}

static lv_obj_t *screen_active_brewing_create_disabled_control_button(lv_obj_t *parent,
                                                                      const char *text,
                                                                      uint32_t color)
{
    lv_obj_t *button;
    lv_obj_t *label;

    button = lv_button_create(parent);
    lv_obj_set_width(button, lv_pct(50));
    lv_obj_set_height(button, 48);
    lv_obj_set_style_bg_color(button, lv_color_hex(color), 0);
    lv_obj_set_style_radius(button, 5, 0);
    lv_obj_add_state(button, LV_STATE_DISABLED);

    label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);
    return button;
}

/****************************************************************************************
 * @brief Create one compact visual brewing-stage marker.
 *
 * This is only a presentation scaffold today. Later it should be driven by real brewing
 * process state, but the shape can stay: a small marker plus a short fixed label.
 ****************************************************************************************/
static lv_obj_t *screen_active_brewing_create_process_step(lv_obj_t *parent,
                                                           screen_active_brewing_t *active_brewing,
                                                           const char *text,
                                                           brewing_process_stage_t stage)
{
    lv_obj_t *step;
    lv_obj_t *marker;
    lv_obj_t *label;

    step = lv_obj_create(parent);
    screen_active_brewing_set_static(step);
    lv_obj_set_width(step, lv_pct(25));
    lv_obj_set_height(step, 56);
    lv_obj_set_style_bg_opa(step, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(step, 0, 0);
    lv_obj_set_style_pad_all(step, 0, 0);

    marker = lv_obj_create(step);
    screen_active_brewing_set_static(marker);
    lv_obj_set_size(marker, 18, 18);
    lv_obj_set_style_radius(marker, 9, 0);
    lv_obj_set_style_bg_color(marker, lv_color_hex(0x343434), 0);
    lv_obj_set_style_bg_opa(marker, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(marker, lv_color_hex(0x4B4741), 0);
    lv_obj_set_style_border_width(marker, 1, 0);
    lv_obj_align(marker, LV_ALIGN_TOP_MID, 0, 0);

    label = lv_label_create(step);
    lv_label_set_text(label, text);
    lv_obj_set_width(label, lv_pct(100));
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0x8C8C8C), 0);
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, 0);
    active_brewing->process_step_markers[stage] = marker;
    active_brewing->process_step_labels[stage] = label;
    return step;
}

/****************************************************************************************
 * @brief Create a small old-style process overview strip for the Overall tab.
 ****************************************************************************************/
static void screen_active_brewing_create_process_strip(screen_active_brewing_t *active_brewing,
                                                       lv_obj_t *parent)
{
    lv_obj_t *strip;

    strip = lv_obj_create(parent);
    screen_active_brewing_set_static(strip);
    lv_obj_set_width(strip, lv_pct(100));
    lv_obj_set_height(strip, 62);
    lv_obj_align(strip, LV_ALIGN_TOP_MID, 0, 190);
    lv_obj_set_style_bg_color(strip, lv_color_hex(0x151515), 0);
    lv_obj_set_style_bg_opa(strip, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(strip, lv_color_hex(0x343434), 0);
    lv_obj_set_style_border_width(strip, 1, 0);
    lv_obj_set_style_radius(strip, 4, 0);
    lv_obj_set_style_pad_all(strip, 4, 0);
    lv_obj_set_style_pad_column(strip, 0, 0);
    lv_obj_set_layout(strip, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(strip, LV_FLEX_FLOW_ROW);

    screen_active_brewing_create_process_step(strip, active_brewing, "Water", BREWING_PROCESS_STAGE_WATER);
    screen_active_brewing_create_process_step(strip, active_brewing, "Mash", BREWING_PROCESS_STAGE_MASH);
    screen_active_brewing_create_process_step(strip, active_brewing, "Boil", BREWING_PROCESS_STAGE_BOIL);
    screen_active_brewing_create_process_step(strip, active_brewing, "Done", BREWING_PROCESS_STAGE_DONE);
}

static void screen_active_brewing_create_overall_page(screen_active_brewing_t *active_brewing,
                                                      lv_obj_t *parent)
{
    lv_obj_t *progress;
    lv_obj_t *label;
    lv_obj_t *sub_label;
    lv_obj_t *controls;

    progress = lv_obj_create(parent);
    screen_active_brewing_set_static(progress);
    lv_obj_set_size(progress, 116, 116);
    lv_obj_set_style_radius(progress, 58, 0);
    lv_obj_set_style_bg_color(progress, lv_color_hex(0x1F1D1B), 0);
    lv_obj_set_style_bg_opa(progress, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(progress, lv_color_hex(0xF47B32), 0);
    lv_obj_set_style_border_width(progress, 5, 0);
    lv_obj_align(progress, LV_ALIGN_TOP_MID, 0, 4);

    label = lv_label_create(progress);
    lv_label_set_text(label, "0");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);
    active_brewing->progress_value_label = label;

    label = lv_label_create(parent);
    lv_label_set_text(label, "Ready - 0%");
    lv_obj_set_width(label, lv_pct(100));
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 132);
    active_brewing->overall_state_label = label;

    sub_label = lv_label_create(parent);
    lv_label_set_text(sub_label, "Remaining time: --:--");
    lv_obj_set_width(sub_label, lv_pct(100));
    lv_obj_set_style_text_align(sub_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(sub_label, lv_color_hex(0x8C8C8C), 0);
    lv_obj_align(sub_label, LV_ALIGN_TOP_MID, 0, 158);
    active_brewing->overall_detail_label = sub_label;

    screen_active_brewing_create_process_strip(active_brewing, parent);

    controls = lv_obj_create(parent);
    screen_active_brewing_set_static(controls);
    lv_obj_set_width(controls, lv_pct(100));
    lv_obj_set_height(controls, 48);
    lv_obj_align(controls, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_opa(controls, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(controls, 0, 0);
    lv_obj_set_style_pad_all(controls, 0, 0);
    lv_obj_set_style_pad_column(controls, 8, 0);
    lv_obj_set_layout(controls, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(controls, LV_FLEX_FLOW_ROW);

    screen_active_brewing_create_disabled_control_button(controls, "PAUSE", 0xF47B32);
    screen_active_brewing_create_disabled_control_button(controls, "STOP", 0xB64131);
}

static void screen_active_brewing_create_actions_page(screen_active_brewing_t *active_brewing,
                                                      lv_obj_t *parent)
{
    lv_obj_t *machine;
    lv_obj_t *center_panel;
    lv_obj_t *label;

    machine = lv_obj_create(parent);
    screen_active_brewing_set_static(machine);
    lv_obj_set_width(machine, lv_pct(100));
    lv_obj_set_height(machine, 154);
    lv_obj_set_style_bg_color(machine, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(machine, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(machine, 0, 0);
    lv_obj_set_style_pad_all(machine, 0, 0);
    lv_obj_set_style_pad_column(machine, 4, 0);
    lv_obj_set_layout(machine, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(machine, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(machine, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);

    screen_active_brewing_create_tank(machine,
                                      "Boil",
                                      "--.- / -- C",
                                      false,
                                      &active_brewing->boil_temperature_label);

    center_panel = lv_obj_create(machine);
    screen_active_brewing_set_static(center_panel);
    lv_obj_set_width(center_panel, 32);
    lv_obj_set_height(center_panel, 138);
    lv_obj_set_style_bg_color(center_panel, lv_color_hex(0x282828), 0);
    lv_obj_set_style_bg_opa(center_panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(center_panel, lv_color_hex(0x343434), 0);
    lv_obj_set_style_border_width(center_panel, 1, 0);
    lv_obj_set_style_radius(center_panel, 3, 0);

    label = lv_label_create(center_panel);
    lv_label_set_text(label, "P\nH\nV");
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0x8C8C8C), 0);
    lv_obj_center(label);

    screen_active_brewing_create_tank(machine,
                                      "Mash",
                                      "--.- / -- C",
                                      true,
                                      &active_brewing->mash_temperature_label);

    label = lv_label_create(parent);
    lv_label_set_text(label, "Machine overview");
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label, lv_pct(100));
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0x8C8C8C), 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 170);
}

static void screen_active_brewing_select_tab(screen_active_brewing_t *active_brewing,
                                             screen_active_brewing_tab_id_t tab_id)
{
    uint32_t tab_index;

    if (active_brewing == NULL || tab_id >= SCREEN_ACTIVE_BREWING_TAB_COUNT)
    {
        return;
    }

    for (tab_index = 0U; tab_index < SCREEN_ACTIVE_BREWING_TAB_COUNT; ++tab_index)
    {
        if (tab_index == (uint32_t)tab_id)
        {
            lv_obj_add_state(active_brewing->tab_buttons[tab_index], LV_STATE_CHECKED);
            lv_obj_remove_flag(active_brewing->tab_pages[tab_index], LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            lv_obj_remove_state(active_brewing->tab_buttons[tab_index], LV_STATE_CHECKED);
            lv_obj_add_flag(active_brewing->tab_pages[tab_index], LV_OBJ_FLAG_HIDDEN);
        }
    }

    active_brewing->selected_tab_id = tab_id;
}

static void screen_active_brewing_nav_event_cb(lv_event_t *event)
{
    screen_active_brewing_nav_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL || context->handler == NULL)
    {
        return;
    }

    context->handler(context->action, context->value, context->user_data);
}

static void screen_active_brewing_tab_event_cb(lv_event_t *event)
{
    screen_active_brewing_tab_context_t *context;

    context = lv_event_get_user_data(event);
    if (context == NULL || context->active_brewing == NULL)
    {
        return;
    }

    screen_active_brewing_select_tab(context->active_brewing, context->tab_id);
}

void screen_active_brewing_init(screen_active_brewing_t *active_brewing,
                                ui_action_handler_t action_handler,
                                void *user_data)
{
    lv_obj_t *container;
    lv_obj_t *tabs;
    uint32_t tab_index;

    if (active_brewing == NULL)
    {
        return;
    }

    memset(active_brewing, 0, sizeof(*active_brewing));
    active_brewing->back_button_context.action = UI_ACTION_SHOW_BREW_CHECKLIST;
    active_brewing->back_button_context.handler = action_handler;
    active_brewing->back_button_context.user_data = user_data;
    active_brewing->menu_button_context.action = UI_ACTION_SHOW_MENU;
    active_brewing->menu_button_context.handler = action_handler;
    active_brewing->menu_button_context.user_data = user_data;

    active_brewing->screen = lv_obj_create(NULL);
    screen_active_brewing_set_static(active_brewing->screen);
    lv_obj_set_style_bg_color(active_brewing->screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(active_brewing->screen, LV_OPA_COVER, 0);

    container = lv_obj_create(active_brewing->screen);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, SCREEN_ACTIVE_BREWING_PAD, 0);
    lv_obj_set_style_pad_row(container, 8, 0);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);

    screen_active_brewing_create_header(container, active_brewing);

    active_brewing->recipe_label = lv_label_create(container);
    lv_label_set_text(active_brewing->recipe_label, "--");
    lv_label_set_long_mode(active_brewing->recipe_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(active_brewing->recipe_label, lv_pct(100));
    lv_obj_set_style_text_color(active_brewing->recipe_label, lv_color_hex(0xE67526), 0);

    tabs = lv_obj_create(container);
    screen_active_brewing_set_static(tabs);
    lv_obj_set_width(tabs, lv_pct(100));
    lv_obj_set_height(tabs, 34);
    lv_obj_set_style_bg_color(tabs, lv_color_hex(0xD96F28), 0);
    lv_obj_set_style_bg_opa(tabs, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(tabs, 0, 0);
    lv_obj_set_style_pad_all(tabs, 0, 0);
    lv_obj_set_layout(tabs, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(tabs, LV_FLEX_FLOW_ROW);

    active_brewing->tab_contexts[SCREEN_ACTIVE_BREWING_TAB_OVERALL].tab_id =
        SCREEN_ACTIVE_BREWING_TAB_OVERALL;
    active_brewing->tab_contexts[SCREEN_ACTIVE_BREWING_TAB_OVERALL].active_brewing =
        active_brewing;
    active_brewing->tab_contexts[SCREEN_ACTIVE_BREWING_TAB_ACTIONS].tab_id =
        SCREEN_ACTIVE_BREWING_TAB_ACTIONS;
    active_brewing->tab_contexts[SCREEN_ACTIVE_BREWING_TAB_ACTIONS].active_brewing =
        active_brewing;

    active_brewing->tab_buttons[SCREEN_ACTIVE_BREWING_TAB_OVERALL] =
        screen_active_brewing_create_tab_button(tabs,
                                                "Overall",
                                                &active_brewing->tab_contexts[SCREEN_ACTIVE_BREWING_TAB_OVERALL]);
    active_brewing->tab_buttons[SCREEN_ACTIVE_BREWING_TAB_ACTIONS] =
        screen_active_brewing_create_tab_button(tabs,
                                                "Actions",
                                                &active_brewing->tab_contexts[SCREEN_ACTIVE_BREWING_TAB_ACTIONS]);

    for (tab_index = 0U; tab_index < SCREEN_ACTIVE_BREWING_TAB_COUNT; ++tab_index)
    {
        active_brewing->tab_pages[tab_index] = lv_obj_create(container);
        screen_active_brewing_set_static(active_brewing->tab_pages[tab_index]);
        lv_obj_set_width(active_brewing->tab_pages[tab_index], lv_pct(100));
        lv_obj_set_flex_grow(active_brewing->tab_pages[tab_index], 1);
        lv_obj_set_style_bg_color(active_brewing->tab_pages[tab_index], lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_opa(active_brewing->tab_pages[tab_index], LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(active_brewing->tab_pages[tab_index], 0, 0);
        lv_obj_set_style_pad_all(active_brewing->tab_pages[tab_index], 0, 0);
    }

    screen_active_brewing_create_overall_page(active_brewing,
                                              active_brewing->tab_pages[SCREEN_ACTIVE_BREWING_TAB_OVERALL]);
    screen_active_brewing_create_actions_page(active_brewing,
                                              active_brewing->tab_pages[SCREEN_ACTIVE_BREWING_TAB_ACTIONS]);
    screen_active_brewing_select_tab(active_brewing, SCREEN_ACTIVE_BREWING_TAB_OVERALL);
}

void screen_active_brewing_show_recipe(screen_active_brewing_t *active_brewing,
                                       const recipe_t *recipe)
{
    if (active_brewing == NULL || recipe == NULL || active_brewing->shown_recipe_id == recipe->id)
    {
        return;
    }

    lv_label_set_text(active_brewing->recipe_label, recipe->name);
    active_brewing->back_button_context.value = recipe->id;
    active_brewing->shown_recipe_id = recipe->id;
}

/****************************************************************************************
 * @brief Update read-only live values shown on the Active Brewing scaffold.
 *
 * This function never sends commands and never decides whether brewing is allowed. It only
 * renders the latest machine facts that the logic layer already prepared for the UI.
 ****************************************************************************************/
void screen_active_brewing_update(screen_active_brewing_t *active_brewing,
                                  const status_screen_view_model_t *status_view_model,
                                  const brewing_process_view_model_t *process_view_model)
{
    const status_machine_snapshot_t *machine;
    brewing_process_stage_t stage;

    if (active_brewing == NULL || status_view_model == NULL || process_view_model == NULL)
    {
        return;
    }

    if (!active_brewing->has_shown_process ||
        !screen_active_brewing_process_matches(&active_brewing->shown_process, process_view_model))
    {
        snprintf(active_brewing->progress_value_text,
                 sizeof(active_brewing->progress_value_text),
                 "%u",
                 (unsigned int)process_view_model->progress_percent);
        screen_active_brewing_set_label_text(active_brewing->progress_value_label,
                                             active_brewing->progress_value_text);
        screen_active_brewing_set_label_text(active_brewing->overall_state_label,
                                             process_view_model->state_text);
        screen_active_brewing_set_label_text(active_brewing->overall_detail_label,
                                             process_view_model->detail_text);

        for (stage = BREWING_PROCESS_STAGE_WATER;
             stage < BREWING_PROCESS_STAGE_COUNT;
             stage = (brewing_process_stage_t)(stage + 1))
        {
            screen_active_brewing_style_process_step(active_brewing,
                                                     stage,
                                                     stage == process_view_model->current_stage);
        }

        active_brewing->shown_process = *process_view_model;
        active_brewing->has_shown_process = true;
    }

    machine = &status_view_model->machine;
    if (active_brewing->has_shown_machine &&
        screen_active_brewing_machine_matches(&active_brewing->shown_machine, machine))
    {
        return;
    }

    if (!machine->mcu_status_valid)
    {
        screen_active_brewing_set_label_text(active_brewing->mash_temperature_label, "--.- / -- C");
        screen_active_brewing_set_label_text(active_brewing->boil_temperature_label, "--.- / -- C");
    }
    else
    {
        screen_active_brewing_format_temperature(active_brewing->mash_temperature_text,
                                                 sizeof(active_brewing->mash_temperature_text),
                                                 machine->mash_temp_c_x10,
                                                 machine->mash_target_c,
                                                 true);
        screen_active_brewing_format_temperature(active_brewing->boil_temperature_text,
                                                 sizeof(active_brewing->boil_temperature_text),
                                                 machine->boil_temp_c_x10,
                                                 machine->boil_target_c,
                                                 true);

        screen_active_brewing_set_label_text(active_brewing->mash_temperature_label,
                                             active_brewing->mash_temperature_text);
        screen_active_brewing_set_label_text(active_brewing->boil_temperature_label,
                                             active_brewing->boil_temperature_text);
    }

    active_brewing->shown_machine = *machine;
    active_brewing->has_shown_machine = true;
}
