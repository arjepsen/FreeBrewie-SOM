#ifndef FREEBREWIE_UI_H
#define FREEBREWIE_UI_H

/****************************************************************************************
 * @file UI.h
 * @brief Top-level LVGL UI shell and navigation owner.
 *
 * Responsibility: initialize screens and apply UI navigation.
 * Owns: screen instances, menu targets, and deferred screen changes.
 * Does not own: comms, protocol parsing, or machine safety decisions.
 ****************************************************************************************/

#include <stdbool.h>

#include "Logic/Recipe_draft.h"
#include "Screen_active_brewing.h"
#include "Screen_brew_checklist.h"
#include "Screen_brew_setup.h"
#include "Screen_home.h"
#include "Screen_manual.h"
#include "Screen_menu.h"
#include "Screen_recipe_builder.h"
#include "Screen_recipe_detail.h"
#include "Screen_recipe_draft_brewing.h"
#include "Screen_recipe_draft_details.h"
#include "Screen_recipe_draft_fermentation.h"
#include "Screen_recipe_draft_ingredients.h"
#include "Screen_recipe_draft_menu.h"
#include "Screen_recipe_section.h"
#include "Screen_recipes.h"
#include "Screen_settings.h"
#include "Screen_status.h"
#include "UI_types.h"

typedef struct ui_t ui_t;

/**
 * Top-level UI state owned by the app.
 *
 * Common screens are created during `ui_init()` for snappy boot-to-home navigation. Less
 * common workflow screens are stored here too, but created lazily the first time the user
 * visits them. That keeps event callback contexts stable without allocating every LVGL
 * object tree during startup.
 */
struct ui_t
{
    screen_home_t home;
    screen_status_t status;
    screen_recipes_t recipes;
    recipe_draft_t recipe_draft;
    screen_recipe_builder_t recipe_builder;
    bool recipe_builder_created;
    screen_recipe_draft_menu_t recipe_draft_menu;
    bool recipe_draft_menu_created;
    screen_recipe_draft_brewing_t recipe_draft_brewing;
    bool recipe_draft_brewing_created;
    screen_recipe_draft_details_t recipe_draft_details;
    bool recipe_draft_details_created;
    screen_recipe_draft_fermentation_t recipe_draft_fermentation;
    bool recipe_draft_fermentation_created;
    screen_recipe_draft_ingredients_t recipe_draft_ingredients;
    bool recipe_draft_ingredients_created;
    screen_recipe_detail_t recipe_detail;
    bool recipe_detail_created;
    screen_recipe_section_t recipe_section;
    bool recipe_section_created;
    screen_brew_setup_t brew_setup;
    bool brew_setup_created;
    screen_brew_checklist_t brew_checklist;
    bool brew_checklist_created;
    screen_active_brewing_t active_brewing;
    bool active_brewing_created;
    screen_menu_t menu;
    screen_manual_t manual;
    bool manual_created;
    screen_settings_t settings;
    bool settings_created;
    ui_screen_id_t current_screen;  // Screen currently loaded into LVGL.
    ui_screen_id_t pending_screen;  // Deferred navigation request.
    uint32_t pending_value;
    recipe_section_id_t pending_recipe_section;
    bool has_pending_screen;
};

void ui_init(ui_t *ui);
void ui_update(ui_t *ui,
               const status_screen_view_model_t *status_view_model,
               const brewing_process_view_model_t *process_view_model);

#endif
