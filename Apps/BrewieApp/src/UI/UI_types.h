#ifndef FREEBREWIE_UI_TYPES_H
#define FREEBREWIE_UI_TYPES_H

/****************************************************************************************
 * @file UI_types.h
 * @brief Shared UI navigation and action types.
 *
 * Responsibility: define UI-level navigation types.
 * Owns: screen IDs, UI action IDs, and screen action callback type.
 * Does not own: machine-control commands.
 ****************************************************************************************/

/**
 * Screen identifiers used by the first FreeBrewie navigation shell.
 *
 * These are intentionally UI-level destinations, not machine-control commands. A button
 * can ask the app to show a screen without directly touching pumps, heaters, valves, or
 * protocol messages. Keeping that split now makes the later manual-service UI much safer
 * to grow.
 */
typedef enum
{
    UI_SCREEN_HOME = 0,
    UI_SCREEN_MENU,
    UI_SCREEN_STATUS,
    UI_SCREEN_RECIPES,
    UI_SCREEN_MANUAL,
    UI_SCREEN_SETTINGS,
    UI_SCREEN_FAULT
} ui_screen_id_t;

/**
 * User actions emitted by screen widgets.
 *
 * At this stage the actions are only navigation requests. Later, real brewing, cleaning,
 * and service actions should still pass through app logic before they can become MCU
 * protocol requests.
 */
typedef enum
{
    UI_ACTION_SHOW_HOME = 0,
    UI_ACTION_SHOW_MENU,
    UI_ACTION_SHOW_STATUS,
    UI_ACTION_SHOW_RECIPES,
    UI_ACTION_SHOW_MANUAL,
    UI_ACTION_SHOW_SETTINGS
} ui_action_t;

typedef void (*ui_action_handler_t)(ui_action_t action, void *user_data);

#endif
