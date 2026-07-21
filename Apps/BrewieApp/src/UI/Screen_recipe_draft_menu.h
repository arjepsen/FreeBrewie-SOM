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

#include "UI_dialog.h"
#include "UI_types.h"
#include "lvgl.h"

typedef struct
{
    /** Navigation action emitted when this button is clicked. */
    ui_action_t action;
    /** Optional action value, unused for the local draft menu. */
    uint32_t value;
    /** Callback owned by the UI shell. */
    ui_action_handler_t handler;
    /** Opaque pointer passed back to the callback, normally the ui_t instance. */
    void *user_data;
} screen_recipe_draft_menu_nav_context_t;

typedef struct
{
    /** Title shown in the local-only preview dialog. */
    const char *title;
    /** Body shown in the local-only preview dialog. */
    const char *body;
    /** Optional navigation action for sections that already have a real local screen. */
    ui_action_t action;
    /** Callback owned by the UI shell. */
    ui_action_handler_t handler;
    /** Opaque pointer passed back to the callback, normally the ui_t instance. */
    void *user_data;
    /** Screen instance updated by this section row. */
    struct screen_recipe_draft_menu_t *draft_menu;
} screen_recipe_draft_menu_section_context_t;

typedef struct screen_recipe_draft_menu_t
{
    /** Root LVGL screen object for the draft recipe menu. */
    lv_obj_t *screen;
    /** Draft recipe name shown in the old-style recipe bar. */
    lv_obj_t *name_label;
    /** Draft style/status label shown below the recipe name. */
    lv_obj_t *style_label;
    /** Event callback context for returning to the create-name step. */
    screen_recipe_draft_menu_nav_context_t back_button_context;
    /** Event callback context for opening the top-level menu. */
    screen_recipe_draft_menu_nav_context_t menu_button_context;
    /** Local-only section contexts for Details/Ingredients/Brewing/Fermentation. */
    screen_recipe_draft_menu_section_context_t section_contexts[4];
    /** Local-only section preview dialog. */
    ui_dialog_t section_dialog;
    /** Last shown draft name, used to avoid unchanged label writes. */
    const char *shown_name;
} screen_recipe_draft_menu_t;

void screen_recipe_draft_menu_init(screen_recipe_draft_menu_t *draft_menu,
                                   ui_action_handler_t action_handler,
                                   void *user_data);
void screen_recipe_draft_menu_show(screen_recipe_draft_menu_t *draft_menu,
                                   const char *draft_name);

#endif
