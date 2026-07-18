#include "UI.h"

#include <string.h>

#include "Logic/Recipe_catalog.h"
#include "UI_theme.h"

static void ui_handle_action(ui_action_t action, uint32_t value, void *user_data);
static void ui_show_screen(ui_t *ui,
                           ui_screen_id_t screen_id,
                           uint32_t value,
                           recipe_section_id_t recipe_section_id);

static void ui_handle_action(ui_action_t action, uint32_t value, void *user_data)
{
    ui_t *ui;
    ui_screen_id_t screen_id;
    recipe_section_id_t recipe_section_id;

    ui = user_data;
    if (ui == NULL)
    {
        return;
    }

    screen_id = UI_SCREEN_HOME;
    recipe_section_id = RECIPE_SECTION_DETAILS;
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
    else if (action == UI_ACTION_SHOW_RECIPES)
    {
        screen_id = UI_SCREEN_RECIPES;
    }
    else if (action == UI_ACTION_SHOW_RECIPE_DETAIL)
    {
        screen_id = UI_SCREEN_RECIPE_DETAIL;
    }
    else if (action == UI_ACTION_SHOW_RECIPE_DETAILS_SECTION)
    {
        screen_id = UI_SCREEN_RECIPE_SECTION;
        recipe_section_id = RECIPE_SECTION_DETAILS;
    }
    else if (action == UI_ACTION_SHOW_RECIPE_INGREDIENTS_SECTION)
    {
        screen_id = UI_SCREEN_RECIPE_SECTION;
        recipe_section_id = RECIPE_SECTION_INGREDIENTS;
    }
    else if (action == UI_ACTION_SHOW_RECIPE_BREWING_SECTION)
    {
        screen_id = UI_SCREEN_RECIPE_SECTION;
        recipe_section_id = RECIPE_SECTION_BREWING;
    }
    else if (action == UI_ACTION_SHOW_RECIPE_FERMENTATION_SECTION)
    {
        screen_id = UI_SCREEN_RECIPE_SECTION;
        recipe_section_id = RECIPE_SECTION_FERMENTATION;
    }
    else if (action == UI_ACTION_SHOW_BREW_SETUP)
    {
        screen_id = UI_SCREEN_BREW_SETUP;
    }
    else if (action == UI_ACTION_SHOW_MANUAL)
    {
        screen_id = UI_SCREEN_MANUAL;
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
    ui->pending_value = value;
    ui->pending_recipe_section = recipe_section_id;
    ui->has_pending_screen = true;
}

static void ui_show_screen(ui_t *ui,
                           ui_screen_id_t screen_id,
                           uint32_t value,
                           recipe_section_id_t recipe_section_id)
{
    lv_obj_t *screen;
    const recipe_t *recipe;

    if (ui == NULL)
    {
        return;
    }

    screen = ui->home.screen;
    if (screen_id == UI_SCREEN_MENU)
    {
        screen = ui->menu.screen;
    }
    else if (screen_id == UI_SCREEN_STATUS)
    {
        screen = ui->status.screen;
    }
    else if (screen_id == UI_SCREEN_RECIPES)
    {
        screen = ui->recipes.screen;
    }
    else if (screen_id == UI_SCREEN_RECIPE_DETAIL)
    {
        recipe = recipe_catalog_find_by_id(value);
        if (recipe == NULL)
        {
            return;
        }

        screen_recipe_detail_show_recipe(&ui->recipe_detail, recipe);
        screen = ui->recipe_detail.screen;
    }
    else if (screen_id == UI_SCREEN_RECIPE_SECTION)
    {
        recipe = recipe_catalog_find_by_id(value);
        if (recipe == NULL)
        {
            return;
        }

        screen_recipe_section_show(&ui->recipe_section, recipe, recipe_section_id);
        screen = ui->recipe_section.screen;
    }
    else if (screen_id == UI_SCREEN_BREW_SETUP)
    {
        recipe = recipe_catalog_find_by_id(value);
        if (recipe == NULL)
        {
            return;
        }

        if (!ui->brew_setup_created)
        {
            screen_brew_setup_init(&ui->brew_setup, ui_handle_action, ui);
            ui->brew_setup_created = true;
        }

        screen_brew_setup_show_recipe(&ui->brew_setup, recipe);
        screen = ui->brew_setup.screen;
    }
    else if (screen_id == UI_SCREEN_MANUAL)
    {
        if (!ui->manual_created)
        {
            screen_manual_init(&ui->manual, ui_handle_action, ui);
            ui->manual_created = true;
        }

        screen = ui->manual.screen;
    }
    else if (screen_id == UI_SCREEN_SETTINGS)
    {
        if (!ui->settings_created)
        {
            screen_settings_init(&ui->settings, ui_handle_action, ui);
            ui->settings_created = true;
        }

        screen = ui->settings.screen;
    }

    if (screen == NULL)
    {
        return;
    }

    ui->current_screen = screen_id;
    lv_screen_load(screen);
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
    screen_recipes_init(&ui->recipes, ui_handle_action, ui);
    screen_recipe_detail_init(&ui->recipe_detail, ui_handle_action, ui);
    screen_recipe_section_init(&ui->recipe_section, ui_handle_action, ui);
    screen_status_init(&ui->status, ui_handle_action, ui);
    screen_menu_init(&ui->menu, ui_handle_action, ui);
    ui_show_screen(ui, UI_SCREEN_HOME, 0U, RECIPE_SECTION_DETAILS);
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
        ui_show_screen(ui, ui->pending_screen, ui->pending_value, ui->pending_recipe_section);
    }

    screen_home_update(&ui->home, view_model);
    screen_status_update(&ui->status, view_model);
}
