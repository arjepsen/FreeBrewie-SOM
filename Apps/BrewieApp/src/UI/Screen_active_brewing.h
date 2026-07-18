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

#include "Logic/Recipe_types.h"
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
    /** Navigation action emitted when the back/menu button is clicked. */
    ui_action_t action;
    /** Optional action value, currently the selected recipe ID. */
    uint32_t value;
    /** Callback owned by the UI shell. */
    ui_action_handler_t handler;
    /** Opaque pointer passed back to the callback, normally the ui_t instance. */
    void *user_data;
} screen_active_brewing_nav_context_t;

typedef struct
{
    /** Tab selected when this tab button is clicked. */
    screen_active_brewing_tab_id_t tab_id;
    /** Screen instance that receives the local tab selection. */
    struct screen_active_brewing_t *active_brewing;
} screen_active_brewing_tab_context_t;

typedef struct screen_active_brewing_t
{
    /** Root LVGL screen object for Active Brewing. */
    lv_obj_t *screen;
    /** Selected recipe name shown in the active brewing shell. */
    lv_obj_t *recipe_label;
    /** Tab button objects styled when the local tab changes. */
    lv_obj_t *tab_buttons[SCREEN_ACTIVE_BREWING_TAB_COUNT];
    /** Tab content containers shown/hidden when the local tab changes. */
    lv_obj_t *tab_pages[SCREEN_ACTIVE_BREWING_TAB_COUNT];
    /** Event callback context for returning to the checklist scaffold. */
    screen_active_brewing_nav_context_t back_button_context;
    /** Event callback context for opening the top-level menu. */
    screen_active_brewing_nav_context_t menu_button_context;
    /** Event callback contexts for local tab switching. */
    screen_active_brewing_tab_context_t tab_contexts[SCREEN_ACTIVE_BREWING_TAB_COUNT];
    /** Recipe currently shown, used to avoid unchanged label updates. */
    recipe_id_t shown_recipe_id;
    /** Currently selected local tab. */
    screen_active_brewing_tab_id_t selected_tab_id;
} screen_active_brewing_t;

void screen_active_brewing_init(screen_active_brewing_t *active_brewing,
                                ui_action_handler_t action_handler,
                                void *user_data);
void screen_active_brewing_show_recipe(screen_active_brewing_t *active_brewing, const recipe_t *recipe);

#endif
