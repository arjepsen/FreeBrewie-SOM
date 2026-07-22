#ifndef FREEBREWIE_SCREEN_RECIPE_DRAFT_MENU_H
#define FREEBREWIE_SCREEN_RECIPE_DRAFT_MENU_H

/****************************************************************************************
 * @file Screen_recipe_draft_menu.h
 * @brief Local-only old-Brewie-style menu for a newly named draft recipe.
 *
 * Responsibility: show the next recipe-creation step after the draft has a name.
 * Owns: draft recipe menu LVGL objects, local section-preview dialog, and navigation callbacks.
 * Must not own: recipe persistence, recipe validation, brewing start logic, or MCU commands.
 ****************************************************************************************/

#include <stdint.h>

#include "Logic/Recipe_draft.h"
#include "UI_dialog.h"
#include "UI_types.h"
#include "lvgl.h"

typedef struct
{
    ui_action_t action;
    uint32_t value;
    ui_action_handler_t handler;
    void *user_data;
} screen_recipe_draft_menu_nav_context_t;

/**
 * Context kept alive for each draft section row.
 *
 * Some rows open a real editor screen; others still open a local preview dialog. Keeping
 * both options in this small context lets the row callback stay generic and cheap.
 */
typedef struct
{
    const char *title;
    const char *body;
    ui_action_t action;
    ui_action_handler_t handler;
    void *user_data;
    struct screen_recipe_draft_menu_t *draft_menu;
} screen_recipe_draft_menu_section_context_t;

/**
 * Long-lived LVGL objects and callback contexts for the draft section menu.
 *
 * The UI shell keeps this struct after first creation, so event callbacks can safely point
 * at the stored contexts instead of temporary stack data.
 */
typedef struct screen_recipe_draft_menu_t
{
    lv_obj_t *screen;
    lv_obj_t *name_label;
    lv_obj_t *style_label;
    screen_recipe_draft_menu_nav_context_t back_button_context;
    screen_recipe_draft_menu_nav_context_t menu_button_context;
    screen_recipe_draft_menu_section_context_t section_contexts[4];
    ui_dialog_t section_dialog;
    const char *shown_name;  // Last shown draft name, used to avoid unchanged label writes.
} screen_recipe_draft_menu_t;

void screen_recipe_draft_menu_init(screen_recipe_draft_menu_t *draft_menu,
                                   ui_action_handler_t action_handler,
                                   void *user_data);
void screen_recipe_draft_menu_show(screen_recipe_draft_menu_t *draft_menu,
                                   const recipe_draft_t *draft);

#endif
