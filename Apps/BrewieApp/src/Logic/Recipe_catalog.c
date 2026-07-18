#include "Recipe_catalog.h"

static const recipe_t recipe_catalog[] = {
    {
        1U,
        "Hopus Pocus",
        "American IPA",
        "Old UI sample recipe used while the real storage layer is still absent.",
        "Single infusion mash target placeholder.",
        "Hop-forward boil schedule placeholder.",
        "Fermentation plan placeholder.",
        0xE67526U,
    },
    {
        2U,
        "Summer Ale",
        "Blonde Ale",
        "Light sample recipe for testing list and detail navigation.",
        "Gentle mash profile placeholder.",
        "Balanced boil schedule placeholder.",
        "Clean ale fermentation placeholder.",
        0xD39C35U,
    },
    {
        3U,
        "Dark Starter",
        "Porter",
        "Dark sample recipe for testing longer recipe detail content.",
        "Roasty mash profile placeholder.",
        "Moderate boil schedule placeholder.",
        "Cool fermentation placeholder.",
        0x70442BU,
    },
    {
        4U,
        "Wheat Lift",
        "Hefeweizen",
        "Wheat-focused sample recipe for scroll and detail testing.",
        "Wheat mash profile placeholder.",
        "Low bitterness boil placeholder.",
        "Warm fermentation placeholder.",
        0xD8B15DU,
    },
    {
        5U,
        "Red Signal",
        "Amber Ale",
        "Amber sample recipe for scroll and detail testing.",
        "Balanced malt mash placeholder.",
        "Medium boil schedule placeholder.",
        "Clean fermentation placeholder.",
        0xB0472CU,
    },
    {
        6U,
        "Nordic Pale",
        "Pale Ale",
        "Pale sample recipe for scroll and detail testing.",
        "Simple infusion mash placeholder.",
        "Fresh hop boil placeholder.",
        "Neutral fermentation placeholder.",
        0xC78A2BU,
    },
    {
        7U,
        "Night Mash",
        "Stout",
        "Stout sample recipe for scroll and detail testing.",
        "Dark malt mash placeholder.",
        "Roast-balanced boil placeholder.",
        "Cool fermentation placeholder.",
        0x50322CU,
    },
    {
        8U,
        "Table Light",
        "Session Ale",
        "Low-gravity sample recipe for scroll and detail testing.",
        "Light body mash placeholder.",
        "Short boil placeholder.",
        "Fast fermentation placeholder.",
        0xE0A13BU,
    },
};

size_t recipe_catalog_count()
{
    return sizeof(recipe_catalog) / sizeof(recipe_catalog[0]);
}

const recipe_t *recipe_catalog_get_by_index(size_t index)
{
    if (index >= recipe_catalog_count())
    {
        return NULL;
    }

    return &recipe_catalog[index];
}

const recipe_t *recipe_catalog_find_by_id(recipe_id_t recipe_id)
{
    size_t index;

    for (index = 0U; index < recipe_catalog_count(); ++index)
    {
        if (recipe_catalog[index].id == recipe_id)
        {
            return &recipe_catalog[index];
        }
    }

    return NULL;
}
