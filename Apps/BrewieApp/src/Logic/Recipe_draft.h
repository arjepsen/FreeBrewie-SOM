#ifndef FREEBREWIE_RECIPE_DRAFT_H
#define FREEBREWIE_RECIPE_DRAFT_H

/****************************************************************************************
 * @file Recipe_draft.h
 * @brief In-memory editable recipe draft model.
 *
 * Responsibility: own the current unsaved recipe values while the user is building or
 * editing a recipe.
 * Owns: RAM-only draft fields, dirty flags, and completion/validation helpers.
 * Must not own: LVGL widgets, recipe files, import/export mapping, web transport, brewing
 * runtime state, or MCU commands.
 ****************************************************************************************/

#include <stdbool.h>

typedef struct
{
    /** Current draft recipe name. Points to stable string storage in this first scaffold. */
    const char *name;
    /** True once the user has entered or selected a real name instead of the placeholder. */
    bool has_name;
    /** True when draft data has changed since it was created or reset. */
    bool dirty;
} recipe_draft_t;

void recipe_draft_init(recipe_draft_t *draft);
void recipe_draft_reset(recipe_draft_t *draft);
void recipe_draft_set_name(recipe_draft_t *draft, const char *name);
const char *recipe_draft_get_name(const recipe_draft_t *draft);
bool recipe_draft_has_name(const recipe_draft_t *draft);
bool recipe_draft_is_dirty(const recipe_draft_t *draft);

#endif
