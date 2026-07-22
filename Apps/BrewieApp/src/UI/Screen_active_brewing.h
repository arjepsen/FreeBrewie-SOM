#ifndef FREEBREWIE_SCREEN_ACTIVE_BREWING_H
#define FREEBREWIE_SCREEN_ACTIVE_BREWING_H

/****************************************************************************************
 * @file Screen_active_brewing.h
 * @brief Safe old-Brewie-inspired active brewing scaffold.
 *
 * Responsibility: show the first UI-only active brewing presentation.
 * Owns: Active brewing LVGL objects, local tab state, and navigation callback contexts.
 * Must not own: process control, pause/stop authority, heaters, pumps, valves, or MCU commands.
 ****************************************************************************************/

#include <stdbool.h>

#include "Logic/Brewing_process_view_model.h"
#include "Logic/Recipe_types.h"
#include "Logic/Status_view_model.h"
#include "UI_types.h"
#include "lvgl.h"

typedef enum
{
    SCREEN_ACTIVE_BREWING_TAB_OVERALL = 0,
    SCREEN_ACTIVE_BREWING_TAB_ACTIONS,
    SCREEN_ACTIVE_BREWING_TAB_COUNT
} screen_active_brewing_tab_id_t;

typedef struct
{
    ui_action_t action;
    uint32_t value;
    ui_action_handler_t handler;
    void *user_data;
} screen_active_brewing_nav_context_t;

typedef struct
{
    screen_active_brewing_tab_id_t tab_id;
    struct screen_active_brewing_t *active_brewing;
} screen_active_brewing_tab_context_t;

/**
 * Active brewing presentation state.
 *
 * This screen is still read-only presentation. It caches the last machine/process values
 * it rendered so periodic updates can skip unchanged labels and styles.
 */
typedef struct screen_active_brewing_t
{
    lv_obj_t *screen;
    lv_obj_t *recipe_label;
    lv_obj_t *progress_value_label;
    lv_obj_t *overall_state_label;
    lv_obj_t *overall_detail_label;
    lv_obj_t *mash_temperature_label;
    lv_obj_t *boil_temperature_label;
    lv_obj_t *process_step_markers[BREWING_PROCESS_STAGE_COUNT];
    lv_obj_t *process_step_labels[BREWING_PROCESS_STAGE_COUNT];
    lv_obj_t *tab_buttons[SCREEN_ACTIVE_BREWING_TAB_COUNT];
    lv_obj_t *tab_pages[SCREEN_ACTIVE_BREWING_TAB_COUNT];
    screen_active_brewing_nav_context_t back_button_context;
    screen_active_brewing_nav_context_t menu_button_context;
    screen_active_brewing_tab_context_t tab_contexts[SCREEN_ACTIVE_BREWING_TAB_COUNT];
    recipe_id_t shown_recipe_id;  // Recipe currently shown, used to avoid unchanged label updates.
    screen_active_brewing_tab_id_t selected_tab_id;
    status_machine_snapshot_t shown_machine;
    brewing_process_view_model_t shown_process;
    bool has_shown_machine;
    bool has_shown_process;
    char progress_value_text[8];
    char mash_temperature_text[24];
    char boil_temperature_text[24];
} screen_active_brewing_t;

void screen_active_brewing_init(screen_active_brewing_t *active_brewing,
                                ui_action_handler_t action_handler,
                                void *user_data);
void screen_active_brewing_show_recipe(screen_active_brewing_t *active_brewing, const recipe_t *recipe);
void screen_active_brewing_update(screen_active_brewing_t *active_brewing,
                                  const status_screen_view_model_t *status_view_model,
                                  const brewing_process_view_model_t *process_view_model);

#endif
