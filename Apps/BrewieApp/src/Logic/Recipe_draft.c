#include "Recipe_draft.h"

#include <stddef.h>

#define RECIPE_DRAFT_PLACEHOLDER_NAME "Tap to name"

static const char *recipe_draft_clean_name(const char *name);

/****************************************************************************************
 * @brief Return either a usable name pointer or the draft placeholder.
 *
 * This first draft model stores stable string pointers only. A real keyboard/storage pass
 * will replace this with fixed character buffers or another carefully bounded string
 * strategy, but the UI should already talk to this logic model instead of owning recipe
 * values itself.
 ****************************************************************************************/
static const char *recipe_draft_clean_name(const char *name)
{
    if (name == NULL || name[0] == '\0')
    {
        return RECIPE_DRAFT_PLACEHOLDER_NAME;
    }

    return name;
}

/****************************************************************************************
 * @brief Initialize a RAM-only recipe draft.
 ****************************************************************************************/
void recipe_draft_init(recipe_draft_t *draft)
{
    recipe_draft_reset(draft);
}

/****************************************************************************************
 * @brief Reset the draft to the first recipe-builder state.
 ****************************************************************************************/
void recipe_draft_reset(recipe_draft_t *draft)
{
    if (draft == NULL)
    {
        return;
    }

    draft->name = RECIPE_DRAFT_PLACEHOLDER_NAME;
    draft->has_name = false;
    draft->dirty = false;
}

/****************************************************************************************
 * @brief Store the current draft recipe name.
 *
 * The current UI only offers stable built-in sample strings, so this function stores the
 * pointer directly. When real text entry arrives, this module should become the place that
 * copies into bounded draft-owned storage.
 ****************************************************************************************/
void recipe_draft_set_name(recipe_draft_t *draft, const char *name)
{
    if (draft == NULL)
    {
        return;
    }

    draft->name = recipe_draft_clean_name(name);
    draft->has_name = (draft->name != RECIPE_DRAFT_PLACEHOLDER_NAME);
    draft->dirty = true;
}

/****************************************************************************************
 * @brief Return the current visible draft recipe name.
 ****************************************************************************************/
const char *recipe_draft_get_name(const recipe_draft_t *draft)
{
    if (draft == NULL)
    {
        return "";
    }

    return recipe_draft_clean_name(draft->name);
}

/****************************************************************************************
 * @brief Return true once the draft has a real recipe name.
 ****************************************************************************************/
bool recipe_draft_has_name(const recipe_draft_t *draft)
{
    if (draft == NULL)
    {
        return false;
    }

    return draft->has_name;
}

/****************************************************************************************
 * @brief Return true when the user has changed draft data since reset.
 ****************************************************************************************/
bool recipe_draft_is_dirty(const recipe_draft_t *draft)
{
    if (draft == NULL)
    {
        return false;
    }

    return draft->dirty;
}
