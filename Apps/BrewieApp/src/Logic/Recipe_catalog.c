#include "Recipe_catalog.h"

/*
 * Temporary read-only recipe catalog.
 *
 * These are not yet user recipes and are not persisted anywhere. They give the LVGL UI a
 * realistic data shape while we rebuild the old Brewie recipe screens. Keeping the data in
 * Logic instead of UI also points toward the future browser/web-UI path: screens should
 * render recipe data, not own it.
 */
static const recipe_catalog_entry_t recipe_catalog[] = {
    {
        .id = 1U,
        .name = "Hopus Pocus",
        .style = "American IPA",
        .summary = "Old UI sample recipe used while the real storage layer is still absent.",
        .batch_size = "20 L",
        .abv = "6.2%",
        .ibu = "58",
        .og = "1.058",
        .fermentables = "Pale malt, light crystal, small wheat addition",
        .hops = "Cascade at 60 min, Centennial at 15 min, Citra late",
        .additions = "Irish moss near end of boil",
        .yeast = "Clean American ale yeast",
        .mash = "Single infusion at 66 C for 60 min",
        .boil = "60 min boil with late aroma hops",
        .cooling = "Cool to fermentation temperature after boil",
        .water = "Automatic water inlet with balanced pale ale profile",
        .fermentation = "Clean ale fermentation with optional dry hop after primary",
        .fermentation_temperature = "19 C",
        .fermentation_duration = "14 days",
        .accent_color = 0xE67526U,
    },
    {
        .id = 2U,
        .name = "Summer Ale",
        .style = "Blonde Ale",
        .summary = "Light sample recipe for testing list and detail navigation.",
        .batch_size = "20 L",
        .abv = "4.8%",
        .ibu = "22",
        .og = "1.046",
        .fermentables = "Pilsner malt, pale malt, small dextrin malt addition",
        .hops = "Hallertau at 60 min, Saaz at 10 min",
        .additions = "Optional orange peel late in boil",
        .yeast = "Neutral ale yeast",
        .mash = "Gentle mash at 65 C for 60 min",
        .boil = "Balanced 60 min boil",
        .cooling = "Cool quickly for a clean finish",
        .water = "Medium mineral profile for a crisp blonde ale",
        .fermentation = "Clean ale fermentation for a light, bright finish",
        .fermentation_temperature = "18 C",
        .fermentation_duration = "10 days",
        .accent_color = 0xD39C35U,
    },
    {
        .id = 3U,
        .name = "Dark Starter",
        .style = "Porter",
        .summary = "Dark sample recipe for testing longer recipe detail content.",
        .batch_size = "20 L",
        .abv = "5.4%",
        .ibu = "32",
        .og = "1.052",
        .fermentables = "Pale malt, brown malt, chocolate malt, crystal malt",
        .hops = "Fuggles bittering and light late addition",
        .additions = "None",
        .yeast = "English ale yeast",
        .mash = "Roasty mash profile at 67 C for 60 min",
        .boil = "Moderate 60 min boil",
        .cooling = "Cool steadily to preserve malt profile",
        .water = "Dark beer water profile with rounded malt balance",
        .fermentation = "Cool fermentation for restrained ester character",
        .fermentation_temperature = "18 C",
        .fermentation_duration = "14 days",
        .accent_color = 0x70442BU,
    },
    {
        .id = 4U,
        .name = "Wheat Lift",
        .style = "Hefeweizen",
        .summary = "Wheat-focused sample recipe for scroll and detail testing.",
        .batch_size = "20 L",
        .abv = "5.0%",
        .ibu = "14",
        .og = "1.049",
        .fermentables = "Wheat malt, pilsner malt",
        .hops = "Single low bitterness noble hop addition",
        .additions = "None",
        .yeast = "Hefeweizen yeast",
        .mash = "Wheat mash profile at 65 C for 60 min",
        .boil = "Low bitterness 60 min boil",
        .cooling = "Cool to warm fermentation range",
        .water = "Soft water profile for a gentle wheat beer finish",
        .fermentation = "Warm fermentation for classic wheat yeast character",
        .fermentation_temperature = "20 C",
        .fermentation_duration = "10 days",
        .accent_color = 0xD8B15DU,
    },
    {
        .id = 5U,
        .name = "Red Signal",
        .style = "Amber Ale",
        .summary = "Amber sample recipe for scroll and detail testing.",
        .batch_size = "20 L",
        .abv = "5.5%",
        .ibu = "34",
        .og = "1.053",
        .fermentables = "Pale malt, Munich malt, medium crystal malt",
        .hops = "Magnum bittering, Cascade late",
        .additions = "None",
        .yeast = "American ale yeast",
        .mash = "Balanced malt mash at 66 C for 60 min",
        .boil = "Medium 60 min boil schedule",
        .cooling = "Cool to ale fermentation range",
        .water = "Balanced water profile for malt and hop structure",
        .fermentation = "Clean fermentation for amber malt clarity",
        .fermentation_temperature = "19 C",
        .fermentation_duration = "12 days",
        .accent_color = 0xB0472CU,
    },
    {
        .id = 6U,
        .name = "Nordic Pale",
        .style = "Pale Ale",
        .summary = "Pale sample recipe for scroll and detail testing.",
        .batch_size = "20 L",
        .abv = "5.1%",
        .ibu = "40",
        .og = "1.050",
        .fermentables = "Pale malt, oats, light crystal malt",
        .hops = "Fresh hop bittering and late hop addition",
        .additions = "None",
        .yeast = "Neutral ale yeast",
        .mash = "Simple infusion mash at 66 C for 60 min",
        .boil = "Fresh hop 60 min boil",
        .cooling = "Cool quickly after the late hop stand",
        .water = "Pale ale water profile with moderate sulfate",
        .fermentation = "Neutral fermentation to keep hops forward",
        .fermentation_temperature = "18 C",
        .fermentation_duration = "12 days",
        .accent_color = 0xC78A2BU,
    },
    {
        .id = 7U,
        .name = "Night Mash",
        .style = "Stout",
        .summary = "Stout sample recipe for scroll and detail testing.",
        .batch_size = "20 L",
        .abv = "5.8%",
        .ibu = "36",
        .og = "1.056",
        .fermentables = "Pale malt, roasted barley, chocolate malt, oats",
        .hops = "English bittering hop addition",
        .additions = "Optional lactose for a softer finish",
        .yeast = "English ale yeast",
        .mash = "Dark malt mash at 67 C for 60 min",
        .boil = "Roast-balanced 60 min boil",
        .cooling = "Cool steadily for malt balance",
        .water = "Dark water profile for roast balance",
        .fermentation = "Cool fermentation for smooth stout character",
        .fermentation_temperature = "18 C",
        .fermentation_duration = "14 days",
        .accent_color = 0x50322CU,
    },
    {
        .id = 8U,
        .name = "Table Light",
        .style = "Session Ale",
        .summary = "Low-gravity sample recipe for scroll and detail testing.",
        .batch_size = "20 L",
        .abv = "3.6%",
        .ibu = "24",
        .og = "1.036",
        .fermentables = "Pale malt, oats, small crystal malt addition",
        .hops = "Moderate bittering and light late hop addition",
        .additions = "None",
        .yeast = "Clean ale yeast",
        .mash = "Light body mash at 65 C for 45 min",
        .boil = "Short 45 min boil for a lighter session beer",
        .cooling = "Cool quickly for a bright finish",
        .water = "Light session water profile",
        .fermentation = "Fast clean fermentation for quick turnaround",
        .fermentation_temperature = "19 C",
        .fermentation_duration = "8 days",
        .accent_color = 0xE0A13BU,
    },
};

size_t recipe_catalog_count()
{
    return sizeof(recipe_catalog) / sizeof(recipe_catalog[0]);
}

const recipe_catalog_entry_t *recipe_catalog_get_by_index(size_t index)
{
    if (index >= recipe_catalog_count())
    {
        return NULL;
    }

    return &recipe_catalog[index];
}

const recipe_catalog_entry_t *recipe_catalog_find_by_id(recipe_id_t recipe_id)
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
