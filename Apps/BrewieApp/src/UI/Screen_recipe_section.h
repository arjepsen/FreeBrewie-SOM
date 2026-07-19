#ifndef FREEBREWIE_SCREEN_RECIPE_SECTION_H
#define FREEBREWIE_SCREEN_RECIPE_SECTION_H

/****************************************************************************************
 * @file Screen_recipe_section.h
 * @brief Safe read-only screen for one recipe section.
 *
 * Responsibility: show selected recipe Details, Ingredients, Brewing, or Fermentation
 * data without editing or hardware side effects.
 * Owns: Recipe-section LVGL objects and back/menu callbacks.
 * Must not own: recipe editing, saving, brewing start logic, or MCU commands.
 ****************************************************************************************/

#include "Logic/Recipe_types.h"
#include "UI_types.h"
#include "lvgl.h"

#define SCREEN_RECIPE_SECTION_MAX_ROWS 8U

typedef enum
{
    RECIPE_SECTION_DETAILS = 0,
    RECIPE_SECTION_INGREDIENTS,
    RECIPE_SECTION_BREWING,
    RECIPE_SECTION_FERMENTATION
} recipe_section_id_t;

typedef struct
{
    ui_action_t action;
    uint32_t value;
    ui_action_handler_t handler;
    void *user_data;
} screen_recipe_section_button_context_t;

typedef struct
{
    lv_obj_t *screen;
    lv_obj_t *title_label;
    lv_obj_t *recipe_label;
    lv_obj_t *row_objects[SCREEN_RECIPE_SECTION_MAX_ROWS];
    lv_obj_t *row_title_labels[SCREEN_RECIPE_SECTION_MAX_ROWS];
    lv_obj_t *row_value_labels[SCREEN_RECIPE_SECTION_MAX_ROWS];
    recipe_id_t shown_recipe_id;
    recipe_section_id_t shown_section_id;
    screen_recipe_section_button_context_t back_button_context;
    screen_recipe_section_button_context_t menu_button_context;
} screen_recipe_section_t;

void screen_recipe_section_init(screen_recipe_section_t *section,
                                ui_action_handler_t action_handler,
                                void *user_data);
void screen_recipe_section_show(screen_recipe_section_t *section,
                                const recipe_t *recipe,
                                recipe_section_id_t section_id);

#endif
