# FreeBrewie Recipe Model Decisions
_Date: 2026-07-21_

## Purpose
This document records the current direction for how FreeBrewie should model recipes.

The goal is to imitate the original Brewie UI flow at first, while building a cleaner,
faster, smaller, and more future-proof recipe system underneath it.

This document should guide the next recipe-builder, storage, web UI, and SOM-MCU workflow
work.

---

## Core decision
FreeBrewie should treat a recipe as a **domain object**, not as a UI screen and not as an MCU
command script.

That means:
- recipe data lives in `Logic/`
- LVGL screens render and edit recipe data, but do not own recipe meaning
- the future web UI should use the same recipe structures as the embedded UI
- storage should save/load recipe data, but should not decide UI flow
- the SOM converts a selected recipe into current machine intent during brewing
- the MCU does not know what a recipe is

The MCU should continue to receive compact target-state snapshots and report compact actual
machine state. Recipe interpretation remains a SOM responsibility.

---

## Lessons from the old Brewie code
The old code is useful for understanding user-facing behavior:
- recipe creation starts by naming a recipe
- the recipe menu is split into Details, Ingredients, Brewing, and Fermentation
- Ingredients are split into Fermentables and Hops
- Brewing includes mash/water/boil/cooling concepts
- the old UI presents recipes as something the user can browse, edit, and then brew

However, the old implementation should not be copied.

The old `Recipe` class mixes too many responsibilities:
- raw recipe data
- file path and saving
- validation
- calculated values
- Qt/QML notifications
- UI-friendly reshaping of nested lists
- brewing-process values such as mash water, sparging, delayed hopping, and cooling

That kind of large mixed object is flexible at first, but it makes performance, testing,
storage changes, web UI reuse, and MCU command generation harder later.

FreeBrewie should keep those responsibilities separate from the start.

---

## Lessons from other brewing projects
External references checked while writing this:

- BeerXML: https://www.beerxml.com/beerxml.htm
- BeerJSON: https://beerjson.github.io/beerjson/
- BeerJSON object schema: https://beerjson.github.io/beerjson/beer.json.html
- Brewfather recipe API/docs: https://docs.brewfather.app/api
- Brewfather recipe designer guide: https://docs.brewfather.app/getting-started/creating-a-new-recipe
- Brewfather BeerXML import from BeerSmith: https://docs.brewfather.app/getting-started/import-recipes
- BeerSmith import/export docs: https://beersmith.com/help2/importing_and_exporting_files.htm
- CraftBeerPi 4 mash profile docs: https://openbrewing.gitbook.io/craftbeerpi4_support/readme/craftbeerpi-4-server/mash-profile
- Breww recipe file docs: https://breww.com/docs/breww-recipe-file-import-export/

Useful ideas from those systems:
- separate recipe identity/details from ingredients and process stages
- keep ingredient lists structured, not preformatted display strings
- model mash, boil, and fermentation as ordered steps/stages
- keep calculated values separate from user-entered values
- make import/export formats versioned
- allow recipe creation to start simple, then fill sections over time
- treat BeerXML as the first likely interchange path for BeerSmith, Brewfather, and other
  brewing tools
- treat the Brewfather API as a future sync/integration path, not only as a file import path

Ideas to treat carefully:
- general brewing formats can be too broad for an appliance
- XML-style wrapper structures are not ideal for lean embedded runtime data
- cloud/API recipe objects may include fields FreeBrewie does not need on-device
- process-stage models are useful, but the MCU protocol should stay compact and hardware-focused
- BeerSmith's native `.bsmx` format exists, but BeerXML is the better first compatibility
  target because it is the intended interchange format between many brewing programs

---

## Compatibility direction
FreeBrewie should use a native internal model first, then map that model to external formats
and services.

Compatibility goals:
- BeerXML import/export for BeerSmith, Brewfather, Brewer's Friend, Brewtarget, and similar
  tools
- BeerJSON awareness so the model does not fight a more modern structured recipe format
- Brewfather API compatibility later, likely for recipe sync, batch/brew-session data, and
  measured-value updates

The internal model should not become "BeerXML in C" or "Brewfather JSON in C".

Instead:
- keep FreeBrewie structs clean and appliance-focused
- keep units explicit and consistent internally
- preserve enough fields to round-trip common recipes later
- keep importer/exporter/API mapping code separate from UI and brewing runtime logic
- document any external fields FreeBrewie ignores or cannot represent

---

## Proposed model layers

### 1. Recipe domain data
Plain recipe data should represent what the user means to brew.

Likely fields:
- recipe id
- name
- style
- author/source later if useful
- batch size
- efficiency or equipment assumptions later if useful
- fermentables
- hops
- miscellaneous additions
- yeast/culture
- water profile or water targets
- mash schedule
- boil schedule
- cooling target
- fermentation schedule
- notes
- created/updated metadata if storage needs it

This layer should use compact C structs and fixed practical limits. Avoid heap allocation
unless a clear need appears.

### 2. Recipe draft
A draft is the editable in-progress version of a recipe.

The draft should own:
- current editable values
- dirty flags
- validation state
- which fields/sections are complete enough to continue

The draft should not own:
- LVGL widgets
- file paths
- MCU commands
- active brewing runtime state

Current implementation:
`Logic/Recipe_draft` owns the temporary recipe-builder name, the first draft Details fields,
and the first fixed-size Fermentables/Hops ingredient arrays. The module is intentionally
small for now, but it is the right place to add bounded draft fields as the builder grows.

### 3. Recipe catalog
The catalog is a list/index of saved or built-in recipes.

Current state:
- `Recipe_catalog` is a static read-only sample list
- this is acceptable as a UI scaffold

Future state:
- catalog indexes saved recipes
- catalog exposes lightweight summaries for lists
- full recipe data is loaded only when needed

This keeps the Recipes screen fast and memory-light.

### 4. Recipe view models
View models should contain only what a screen needs to show efficiently.

Examples:
- compact recipe row summaries for the recipe list
- draft Details screen rows
- draft Ingredients tab rows
- calculated-value display rows

View models may contain formatted strings, but the core recipe/draft model should keep raw
values.

### 5. Brewing plan/runtime
Starting a recipe should produce a runtime brewing plan owned by SOM logic.

The runtime plan is not the same as the recipe file:
- recipe = user intent and brewing instructions
- runtime plan = current active execution state derived from the selected recipe
- control snapshot = compact current target state sent to the MCU

The SOM should step through the runtime plan and send only the current target state to the
MCU.

---

## SOM-MCU protocol implications
The current protocol direction remains correct:

- the SOM owns recipes, timing, UI, manual-service flow, and high-level intent
- the MCU owns measurements, interlocks, faults, and safe hardware execution
- the MCU receives `CONTROL_SNAPSHOT`, not a full recipe
- the MCU reports `STATUS_REPORT` and `FAULT_REPORT`, not recipe progress semantics

This is efficient because:
- recipe data can be large and does not cross the serial link
- the MCU does not need parsing/storage for recipe structures
- repeated serial traffic stays small
- unchanged recipe data does not create communication load
- safety remains close to hardware on the MCU

Future protocol additions should continue this pattern. If brewing needs more detail, prefer
small current-intent fields or explicit command/status messages over sending whole recipe
sections.

---

## Initial FreeBrewie recipe shape
The first real editable model should be smaller than a full brewing-standard recipe.

Suggested first version:

```text
recipe
    id
    name
    style
    batch_size_l

    details
        estimated_abv
        estimated_ibu
        estimated_og
        estimated_fg
        estimated_color

    ingredients
        fermentable additions
        hop additions

    brewing
        mash_in_water_l
        mash_in_temperature_c
        mash steps
        sparge water_l
        sparge temperature_c
        sparge time_min
        boil time_min
        delayed hopping time_min
        cooling target_c

    fermentation
        fermentation steps
```

The model can grow later, but this first shape matches the old Brewie recipe screens and the
machine's likely near-term needs.

---

## Practical constraints for C implementation
Because the SOM is memory-limited and the UI must stay responsive:

- prefer fixed maximum counts for early implementation
- keep list summaries separate from full recipe data
- keep raw numbers as integers where practical, for example deciliters, grams, minutes, and degrees C
- avoid repeated `snprintf` in hot UI updates
- format display strings only when visible values change
- avoid per-frame allocation
- keep recipe editing local until the user explicitly saves
- validate draft sections incrementally

Example early fixed limits:
- fermentables: 8 additions
- hops: 8 additions
- mash steps: 6 steps
- fermentation steps: 6 steps
- notes: short fixed text, or defer notes until keyboard/storage is better understood

These limits can change after real recipes are tested.

---

## Open questions
These should be answered before full storage is implemented:

- What exact minimum fields are required to brew safely?
- Which values are user-entered and which are calculated?
- How closely should the saved file format resemble old `.json` recipe files?
- Should saved recipes use a FreeBrewie-native versioned JSON format first?
- Do we need BeerXML/BeerJSON import/export soon, or later?
- Which recipe values should be sent into the active brewing runtime plan?
- Which values must never be sent to the MCU because they are only UI/display metadata?

---

## Current implementation step
Do not implement filesystem recipe storage yet.

Implemented first:
- added `Logic/Recipe_draft.c/.h`
- moved the temporary draft name out of `Screen_recipe_builder`
- made Recipe Builder, draft menu, draft Details, and draft Ingredients read from the draft model
- added the first RAM-only draft Details fields for style and calculated values
- added the first RAM-only Fermentables/Hops arrays for the Ingredients screen
- kept the draft in memory only
- kept all MCU output disabled

Next code steps:
- add brewing-process draft fields next: mash, water, boil, and cooling
- add validation
- then design storage with versioning
