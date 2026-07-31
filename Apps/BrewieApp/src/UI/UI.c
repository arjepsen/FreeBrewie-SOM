#include "UI.h"

#include <stddef.h>
#include <string.h>

#include "Logic/Recipe_catalog.h"
#include "Logic/Recipe_draft.h"
#include "UI_theme.h"

/**
 * One row in the event-to-screen navigation table.
 *
 * `recipe_section_id` is only meaningful for `UI_SCREEN_RECIPE_SECTION`.
 * Other rows keep the default details value so the table can stay uniform.
 */
typedef struct
{
    ui_action_t action;
    ui_screen_id_t screen_id;
    recipe_section_id_t recipe_section_id;
} ui_action_route_t;

static bool ui_find_action_route(ui_action_t action,
                                 ui_screen_id_t *screen_id,
                                 recipe_section_id_t *recipe_section_id);
static bool ui_request_workflow_permission(ui_t *ui, ui_action_t action, uint32_t value);
static void ui_handle_action(ui_action_t action, uint32_t value, void *user_data);
static lv_obj_t *ui_show_recipe_detail_screen(ui_t *ui, uint32_t recipe_id);
static lv_obj_t *ui_show_recipe_builder_screen(ui_t *ui);
static lv_obj_t *ui_show_recipe_draft_brewing_screen(ui_t *ui);
static lv_obj_t *ui_show_recipe_draft_details_screen(ui_t *ui);
static lv_obj_t *ui_show_recipe_draft_ingredients_screen(ui_t *ui);
static lv_obj_t *ui_show_recipe_draft_fermentation_screen(ui_t *ui);
static lv_obj_t *ui_show_recipe_draft_menu_screen(ui_t *ui);
static lv_obj_t *ui_show_recipe_section_screen(ui_t *ui,
                                               uint32_t recipe_id,
                                               recipe_section_id_t recipe_section_id);
static lv_obj_t *ui_show_brew_setup_screen(ui_t *ui, uint32_t recipe_id);
static lv_obj_t *ui_show_brew_checklist_screen(ui_t *ui, uint32_t recipe_id);
static lv_obj_t *ui_show_active_brewing_screen(ui_t *ui, uint32_t recipe_id);
static lv_obj_t *ui_show_manual_screen(ui_t *ui);
static lv_obj_t *ui_show_settings_screen(ui_t *ui);
static void ui_show_screen(ui_t *ui,
                           ui_screen_id_t screen_id,
                           uint32_t value,
                           recipe_section_id_t recipe_section_id);

static const ui_action_route_t ui_action_routes[] = {
    {UI_ACTION_SHOW_HOME, UI_SCREEN_HOME, RECIPE_SECTION_DETAILS},
    {UI_ACTION_SHOW_MENU, UI_SCREEN_MENU, RECIPE_SECTION_DETAILS},
    {UI_ACTION_SHOW_STATUS, UI_SCREEN_STATUS, RECIPE_SECTION_DETAILS},
    {UI_ACTION_SHOW_RECIPES, UI_SCREEN_RECIPES, RECIPE_SECTION_DETAILS},
    {UI_ACTION_SHOW_RECIPE_DETAIL, UI_SCREEN_RECIPE_DETAIL, RECIPE_SECTION_DETAILS},
    {UI_ACTION_SHOW_RECIPE_BUILDER, UI_SCREEN_RECIPE_BUILDER, RECIPE_SECTION_DETAILS},
    {UI_ACTION_SHOW_RECIPE_DRAFT_BREWING, UI_SCREEN_RECIPE_DRAFT_BREWING, RECIPE_SECTION_DETAILS},
    {UI_ACTION_SHOW_RECIPE_DRAFT_DETAILS, UI_SCREEN_RECIPE_DRAFT_DETAILS, RECIPE_SECTION_DETAILS},
    {UI_ACTION_SHOW_RECIPE_DRAFT_FERMENTATION,
     UI_SCREEN_RECIPE_DRAFT_FERMENTATION,
     RECIPE_SECTION_DETAILS},
    {UI_ACTION_SHOW_RECIPE_DRAFT_INGREDIENTS,
     UI_SCREEN_RECIPE_DRAFT_INGREDIENTS,
     RECIPE_SECTION_DETAILS},
    {UI_ACTION_SHOW_RECIPE_DRAFT_MENU, UI_SCREEN_RECIPE_DRAFT_MENU, RECIPE_SECTION_DETAILS},
    {UI_ACTION_SHOW_RECIPE_DETAILS_SECTION, UI_SCREEN_RECIPE_SECTION, RECIPE_SECTION_DETAILS},
    {UI_ACTION_SHOW_RECIPE_INGREDIENTS_SECTION, UI_SCREEN_RECIPE_SECTION, RECIPE_SECTION_INGREDIENTS},
    {UI_ACTION_SHOW_RECIPE_BREWING_SECTION, UI_SCREEN_RECIPE_SECTION, RECIPE_SECTION_BREWING},
    {UI_ACTION_SHOW_RECIPE_FERMENTATION_SECTION,
     UI_SCREEN_RECIPE_SECTION,
     RECIPE_SECTION_FERMENTATION},
    {UI_ACTION_SHOW_BREW_SETUP, UI_SCREEN_BREW_SETUP, RECIPE_SECTION_DETAILS},
    {UI_ACTION_SHOW_BREW_CHECKLIST, UI_SCREEN_BREW_CHECKLIST, RECIPE_SECTION_DETAILS},
    {UI_ACTION_SHOW_ACTIVE_BREWING, UI_SCREEN_ACTIVE_BREWING, RECIPE_SECTION_DETAILS},
    {UI_ACTION_SHOW_MANUAL, UI_SCREEN_MANUAL, RECIPE_SECTION_DETAILS},
    {UI_ACTION_SHOW_SETTINGS, UI_SCREEN_SETTINGS, RECIPE_SECTION_DETAILS}};

/****************************************************************************************
 * @brief Find the screen route for a user action.
 *
 * A small table keeps navigation easy to audit and avoids a long chain of branches. This
 * runs only when the user taps a control, so readability matters more than microseconds.
 ****************************************************************************************/
static bool ui_find_action_route(ui_action_t action,
                                 ui_screen_id_t *screen_id,
                                 recipe_section_id_t *recipe_section_id)
{
    size_t index;

    if (screen_id == NULL || recipe_section_id == NULL)
    {
        return false;
    }

    for (index = 0U; index < (sizeof(ui_action_routes) / sizeof(ui_action_routes[0])); ++index)
    {
        if (ui_action_routes[index].action == action)
        {
            *screen_id = ui_action_routes[index].screen_id;
            *recipe_section_id = ui_action_routes[index].recipe_section_id;
            return true;
        }
    }

    return false;
}

/****************************************************************************************
 * @brief Ask app logic before navigation enters a workflow-sensitive screen.
 *
 * Screen widgets still only emit UI actions. This is the narrow bridge where the UI shell
 * lets the app/orchestrator prepare or start recipe workflow state before the visible screen
 * changes. A false result simply leaves the user on the current screen for now; later we can
 * show a proper validation dialog through the same boundary.
 ****************************************************************************************/
static bool ui_request_workflow_permission(ui_t *ui, ui_action_t action, uint32_t value)
{
    if (ui == NULL || ui->workflow_handler == NULL)
    {
        return true;
    }

    if (action == UI_ACTION_SHOW_BREW_CHECKLIST || action == UI_ACTION_SHOW_ACTIVE_BREWING)
    {
        return ui->workflow_handler(action, (recipe_id_t)value, ui->workflow_user_data);
    }

    return true;
}

/****************************************************************************************
 * @brief Queue navigation requested by a screen widget.
 *
 * LVGL allows many operations from event callbacks, but changing screens while the current
 * click/release event is still bubbling can be fragile. Queue navigation and apply it
 * during the normal UI update instead; that keeps button callbacks tiny and avoids
 * re-entering the refresh/event path.
 ****************************************************************************************/
static void ui_handle_action(ui_action_t action, uint32_t value, void *user_data)
{
    ui_t *ui;
    ui_screen_id_t screen_id;
    recipe_section_id_t recipe_section_id;

    ui = user_data;
    if (ui == NULL || !ui_find_action_route(action, &screen_id, &recipe_section_id))
    {
        return;
    }

    if (!ui_request_workflow_permission(ui, action, value))
    {
        return;
    }

    ui->pending_screen = screen_id;
    ui->pending_value = value;
    ui->pending_recipe_section = recipe_section_id;
    ui->has_pending_screen = true;
}

/****************************************************************************************
 * @brief Prepare the selected recipe overview screen.
 ****************************************************************************************/
static lv_obj_t *ui_show_recipe_detail_screen(ui_t *ui, uint32_t recipe_id)
{
    const recipe_catalog_entry_t *recipe;

    recipe = recipe_catalog_find_by_id(recipe_id);
    if (recipe == NULL)
    {
        return NULL;
    }

    if (!ui->recipe_detail_created)
    {
        screen_recipe_detail_init(&ui->recipe_detail, ui_handle_action, ui);
        ui->recipe_detail_created = true;
    }

    screen_recipe_detail_show_recipe(&ui->recipe_detail, recipe);
    return ui->recipe_detail.screen;
}

/****************************************************************************************
 * @brief Prepare the recipe-builder start screen.
 ****************************************************************************************/
static lv_obj_t *ui_show_recipe_builder_screen(ui_t *ui)
{
    if (!ui->recipe_builder_created)
    {
        screen_recipe_builder_init(&ui->recipe_builder, &ui->recipe_draft, ui_handle_action, ui);
        ui->recipe_builder_created = true;
    }

    screen_recipe_builder_show(&ui->recipe_builder);
    return ui->recipe_builder.screen;
}

/****************************************************************************************
 * @brief Prepare the draft Brewing editor.
 ****************************************************************************************/
static lv_obj_t *ui_show_recipe_draft_brewing_screen(ui_t *ui)
{
    if (!ui->recipe_builder_created)
    {
        return NULL;
    }

    if (!ui->recipe_draft_brewing_created)
    {
        screen_recipe_draft_brewing_init(&ui->recipe_draft_brewing,
                                         &ui->recipe_draft,
                                         ui_handle_action,
                                         ui);
        ui->recipe_draft_brewing_created = true;
    }

    screen_recipe_draft_brewing_show(&ui->recipe_draft_brewing, &ui->recipe_draft);
    return ui->recipe_draft_brewing.screen;
}

/****************************************************************************************
 * @brief Prepare the draft Details editor.
 ****************************************************************************************/
static lv_obj_t *ui_show_recipe_draft_details_screen(ui_t *ui)
{
    if (!ui->recipe_builder_created)
    {
        return NULL;
    }

    if (!ui->recipe_draft_details_created)
    {
        screen_recipe_draft_details_init(&ui->recipe_draft_details,
                                         &ui->recipe_draft,
                                         ui_handle_action,
                                         ui);
        ui->recipe_draft_details_created = true;
    }

    screen_recipe_draft_details_show(&ui->recipe_draft_details, &ui->recipe_draft);
    return ui->recipe_draft_details.screen;
}

/****************************************************************************************
 * @brief Prepare the draft Ingredients editor.
 ****************************************************************************************/
static lv_obj_t *ui_show_recipe_draft_ingredients_screen(ui_t *ui)
{
    if (!ui->recipe_builder_created)
    {
        return NULL;
    }

    if (!ui->recipe_draft_ingredients_created)
    {
        screen_recipe_draft_ingredients_init(&ui->recipe_draft_ingredients, ui_handle_action, ui);
        ui->recipe_draft_ingredients_created = true;
    }

    screen_recipe_draft_ingredients_show(&ui->recipe_draft_ingredients, &ui->recipe_draft);
    return ui->recipe_draft_ingredients.screen;
}

/****************************************************************************************
 * @brief Prepare the draft Fermentation editor.
 ****************************************************************************************/
static lv_obj_t *ui_show_recipe_draft_fermentation_screen(ui_t *ui)
{
    if (!ui->recipe_builder_created)
    {
        return NULL;
    }

    if (!ui->recipe_draft_fermentation_created)
    {
        screen_recipe_draft_fermentation_init(&ui->recipe_draft_fermentation,
                                              &ui->recipe_draft,
                                              ui_handle_action,
                                              ui);
        ui->recipe_draft_fermentation_created = true;
    }

    screen_recipe_draft_fermentation_show(&ui->recipe_draft_fermentation, &ui->recipe_draft);
    return ui->recipe_draft_fermentation.screen;
}

/****************************************************************************************
 * @brief Prepare the draft recipe menu.
 ****************************************************************************************/
static lv_obj_t *ui_show_recipe_draft_menu_screen(ui_t *ui)
{
    if (!ui->recipe_builder_created)
    {
        return NULL;
    }

    if (!ui->recipe_draft_menu_created)
    {
        screen_recipe_draft_menu_init(&ui->recipe_draft_menu, ui_handle_action, ui);
        ui->recipe_draft_menu_created = true;
    }

    screen_recipe_draft_menu_show(&ui->recipe_draft_menu, &ui->recipe_draft);
    return ui->recipe_draft_menu.screen;
}

/****************************************************************************************
 * @brief Prepare one selected-recipe section screen.
 ****************************************************************************************/
static lv_obj_t *ui_show_recipe_section_screen(ui_t *ui,
                                               uint32_t recipe_id,
                                               recipe_section_id_t recipe_section_id)
{
    const recipe_catalog_entry_t *recipe;

    recipe = recipe_catalog_find_by_id(recipe_id);
    if (recipe == NULL)
    {
        return NULL;
    }

    if (!ui->recipe_section_created)
    {
        screen_recipe_section_init(&ui->recipe_section, ui_handle_action, ui);
        ui->recipe_section_created = true;
    }

    screen_recipe_section_show(&ui->recipe_section, recipe, recipe_section_id);
    return ui->recipe_section.screen;
}

/****************************************************************************************
 * @brief Prepare the brew setup screen for one selected recipe.
 ****************************************************************************************/
static lv_obj_t *ui_show_brew_setup_screen(ui_t *ui, uint32_t recipe_id)
{
    const recipe_catalog_entry_t *recipe;

    recipe = recipe_catalog_find_by_id(recipe_id);
    if (recipe == NULL)
    {
        return NULL;
    }

    if (!ui->brew_setup_created)
    {
        screen_brew_setup_init(&ui->brew_setup, ui_handle_action, ui);
        ui->brew_setup_created = true;
    }

    screen_brew_setup_show_recipe(&ui->brew_setup, recipe);
    return ui->brew_setup.screen;
}

/****************************************************************************************
 * @brief Prepare the brew checklist screen for one selected recipe.
 ****************************************************************************************/
static lv_obj_t *ui_show_brew_checklist_screen(ui_t *ui, uint32_t recipe_id)
{
    const recipe_catalog_entry_t *recipe;

    recipe = recipe_catalog_find_by_id(recipe_id);
    if (recipe == NULL)
    {
        return NULL;
    }

    if (!ui->brew_checklist_created)
    {
        screen_brew_checklist_init(&ui->brew_checklist, ui_handle_action, ui);
        ui->brew_checklist_created = true;
    }

    screen_brew_checklist_show_recipe(&ui->brew_checklist, recipe);
    return ui->brew_checklist.screen;
}

/****************************************************************************************
 * @brief Prepare the active brewing screen for one selected recipe.
 ****************************************************************************************/
static lv_obj_t *ui_show_active_brewing_screen(ui_t *ui, uint32_t recipe_id)
{
    const recipe_catalog_entry_t *recipe;

    recipe = recipe_catalog_find_by_id(recipe_id);
    if (recipe == NULL)
    {
        return NULL;
    }

    if (!ui->active_brewing_created)
    {
        screen_active_brewing_init(&ui->active_brewing, ui_handle_action, ui);
        ui->active_brewing_created = true;
    }

    screen_active_brewing_show_recipe(&ui->active_brewing, recipe);
    return ui->active_brewing.screen;
}

/****************************************************************************************
 * @brief Prepare the Manual/Cleaning screen.
 ****************************************************************************************/
static lv_obj_t *ui_show_manual_screen(ui_t *ui)
{
    if (!ui->manual_created)
    {
        screen_manual_init(&ui->manual, ui_handle_action, ui);
        ui->manual_created = true;
    }

    return ui->manual.screen;
}

/****************************************************************************************
 * @brief Prepare the Settings screen.
 ****************************************************************************************/
static lv_obj_t *ui_show_settings_screen(ui_t *ui)
{
    if (!ui->settings_created)
    {
        screen_settings_init(&ui->settings, ui_handle_action, ui);
        ui->settings_created = true;
    }

    return ui->settings.screen;
}

/****************************************************************************************
 * @brief Load the requested screen, creating rare screens only when needed.
 ****************************************************************************************/
static void ui_show_screen(ui_t *ui,
                           ui_screen_id_t screen_id,
                           uint32_t value,
                           recipe_section_id_t recipe_section_id)
{
    lv_obj_t *screen;

    if (ui == NULL)
    {
        return;
    }

    screen = NULL;
    switch (screen_id)
    {
        case UI_SCREEN_HOME:
            screen = ui->home.screen;
            break;

        case UI_SCREEN_MENU:
            screen = ui->menu.screen;
            break;

        case UI_SCREEN_STATUS:
            screen = ui->status.screen;
            break;

        case UI_SCREEN_RECIPES:
            screen = ui->recipes.screen;
            break;

        case UI_SCREEN_RECIPE_DETAIL:
            screen = ui_show_recipe_detail_screen(ui, value);
            break;

        case UI_SCREEN_RECIPE_BUILDER:
            screen = ui_show_recipe_builder_screen(ui);
            break;

        case UI_SCREEN_RECIPE_DRAFT_BREWING:
            screen = ui_show_recipe_draft_brewing_screen(ui);
            break;

        case UI_SCREEN_RECIPE_DRAFT_DETAILS:
            screen = ui_show_recipe_draft_details_screen(ui);
            break;

        case UI_SCREEN_RECIPE_DRAFT_FERMENTATION:
            screen = ui_show_recipe_draft_fermentation_screen(ui);
            break;

        case UI_SCREEN_RECIPE_DRAFT_INGREDIENTS:
            screen = ui_show_recipe_draft_ingredients_screen(ui);
            break;

        case UI_SCREEN_RECIPE_DRAFT_MENU:
            screen = ui_show_recipe_draft_menu_screen(ui);
            break;

        case UI_SCREEN_RECIPE_SECTION:
            screen = ui_show_recipe_section_screen(ui, value, recipe_section_id);
            break;

        case UI_SCREEN_BREW_SETUP:
            screen = ui_show_brew_setup_screen(ui, value);
            break;

        case UI_SCREEN_BREW_CHECKLIST:
            screen = ui_show_brew_checklist_screen(ui, value);
            break;

        case UI_SCREEN_ACTIVE_BREWING:
            screen = ui_show_active_brewing_screen(ui, value);
            break;

        case UI_SCREEN_MANUAL:
            screen = ui_show_manual_screen(ui);
            break;

        case UI_SCREEN_SETTINGS:
            screen = ui_show_settings_screen(ui);
            break;

        case UI_SCREEN_FAULT:
        default:
            break;
    }

    if (screen == NULL)
    {
        return;
    }

    ui->current_screen = screen_id;
    lv_screen_load(screen);
}

void ui_init(ui_t *ui, ui_workflow_handler_t workflow_handler, void *workflow_user_data)
{
    if (ui == NULL)
    {
        return;
    }

    memset(ui, 0, sizeof(*ui));
    ui->workflow_handler = workflow_handler;
    ui->workflow_user_data = workflow_user_data;
    recipe_draft_init(&ui->recipe_draft);
    ui_theme_init();
    screen_home_init(&ui->home, ui_handle_action, ui);
    screen_recipes_init(&ui->recipes, ui_handle_action, ui);
    screen_status_init(&ui->status, ui_handle_action, ui);
    screen_menu_init(&ui->menu, ui_handle_action, ui);
    ui_show_screen(ui, UI_SCREEN_HOME, 0U, RECIPE_SECTION_DETAILS);
}

void ui_update(ui_t *ui,
               const status_screen_view_model_t *status_view_model,
               const brewing_process_view_model_t *process_view_model)
{
    if (ui == NULL)
    {
        return;
    }

    if (ui->has_pending_screen)
    {
        ui->has_pending_screen = false;
        ui_show_screen(ui, ui->pending_screen, ui->pending_value, ui->pending_recipe_section);
    }

    screen_home_update(&ui->home, status_view_model);
    screen_status_update(&ui->status, status_view_model);
    if (ui->current_screen == UI_SCREEN_ACTIVE_BREWING && ui->active_brewing_created)
    {
        screen_active_brewing_update(&ui->active_brewing, status_view_model, process_view_model);
    }
}
