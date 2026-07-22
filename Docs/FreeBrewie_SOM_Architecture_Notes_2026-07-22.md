# FreeBrewie SOM Architecture Notes
_Date: 2026-07-21_

## Purpose
This document defines the current target architecture for the Brewie SOM application.

It is meant to keep file ownership, module boundaries, and subsystem responsibilities clear while the SOM app is still in early bring-up.

It should be read together with:
- `FreeBrewie_UI_Current_Status_2026-07-22.md`
- `FreeBrewie_Recipe_Model_Decisions_2026-07-21.md`
- `FreeBrewie_SOM_Development_Environment_Consolidated_2026-07-21.md`
- `Brewie_SOM_Platform_Notes_2026-07-02.md`
- `Brewie_SOM_MCU_Protocol_2026-04-01.md`

---

## Main architectural split
The SOM application should be understood in four main groups under `Apps/BrewieApp/src/`:

- `UI/`
- `Logic/`
- `Comms/`
- `Platform/`

And two thin top-level files:

- `main.c`
- `App.c`

This is the current intended shape of the SOM application.

The point of this split is:
- keep communication separate from UI
- keep platform glue separate from application logic
- keep `main.c` small
- keep the Brewie app understandable as it grows

---

## Current source tree shape

```text
Apps/BrewieApp/src/
    App.c
    App.h
    main.c
    main_sim.c

    UI/
    Logic/
    Comms/
    Platform/
```

This is now the active structure for `BrewieApp`.

### Important note about old structure
The current Brewie SOM app should **not** be treated as using an old split `.c` / `.h` folder layout.

It should also **not** be treated as depending on a generic `Shared/` application structure for its main code shape.

The active app code now lives under the grouped `src/` structure above.

---

## Top-level ownership

### `main.c`
`main.c` should stay thin.

Owns:
- process entry
- top-level initialization call into the app layer
- top-level app loop call
- shutdown/exit path if needed

Must not own:
- serial protocol details
- screen construction
- platform driver logic
- application state logic

Rule:
`main.c` starts the app. It should not become the app.

### `App.c`
`App.c` is the SOM app coordinator.

Owns:
- high-level bring-up of app subsystems
- holding the main app context
- calling subsystem update functions in the correct order
- keeping the current app runnable in both target and simulator builds

At the current stage, this means `App.c` coordinates:
- platform init
- comms init/update
- status view-model init/update
- UI init/update
- display update

Must not become:
- raw serial transport code
- protocol parser code
- detailed widget construction file
- Linux DRM implementation file

Rule:
`App.c` coordinates the app, but does not replace the subsystem modules.

---

## Current subsystem responsibilities

## 1. `Comms/`
Purpose:
Own the SOM-side communication path to the MCU.

Current file set:
- `Comms.c`
- `Comms.h`
- `Comms_link.c`
- `Comms_link.h`
- `Protocol.c`
- `Protocol.h`
- `Transport_serial.c`
- `Transport_serial.h`

### `Transport_serial`
Owns:
- opening and configuring the Linux serial device
- raw byte send/receive
- device path handling
- serial-port lifetime

At the current bring-up stage this means:
- opening `/dev/ttyS1`
- reading and writing raw framed data

Must not own:
- message semantics
- UI state
- application decisions

### `Protocol`
Owns:
- frame format knowledge
- encode/decode of the SOM-MCU protocol
- message type recognition
- checksum/CRC handling
- payload packing/unpacking

Must not own:
- Linux serial device handling
- widget updates
- business logic decisions

### `Comms`
Owns:
- higher-level SOM comms progression
- heartbeat sending
- periodic comms update flow
- receiving and interpreting `STATUS_REPORT` / `FAULT_REPORT` into compact app-facing facts

Must not own:
- screen rendering
- direct widget creation
- display backend code

### `Comms_link`
Owns:
- compact current link status / latest received data representation passed upward toward logic/UI

Rule:
`Comms/` owns the MCU link, but does not own the UI.

---

## 2. `Logic/`
Purpose:
Own the SOM-side application logic and compact app state that sits above raw communications and below widgets.

Current file set:
- `App_orchestrator.c`
- `App_orchestrator.h`
- `Brewing_process_view_model.c`
- `Brewing_process_view_model.h`
- `Fault_logic.c`
- `Fault_logic.h`
- `Machine_state.c`
- `Machine_state.h`
- `Machine_targets.c`
- `Machine_targets.h`
- `Recipe_catalog.c`
- `Recipe_catalog.h`
- `Recipe_draft.c`
- `Recipe_draft.h`
- `Recipe_types.h`
- `Startup_logic.c`
- `Startup_logic.h`
- `Status_view_model.c`
- `Status_view_model.h`
- `Style_catalog.c`
- `Style_catalog.h`
- `User_actions.c`
- `User_actions.h`

### `App_orchestrator`
Owns:
- future high-level app state coherence
- future routing of MCU facts and user requests through the right logic modules
- future workflow and allowed-action coordination

At the current stage, `App_orchestrator` is deliberately reserved and has no active public
state. The current UI only performs screen navigation, and the diagnostic status screen has
its own `Status_view_model` module. Do not put presentation-only data in `App_orchestrator`
just to make it active.

The first real use of `App_orchestrator` should be a user request that needs to be checked
against fault state, startup state, machine state, or workflow permissions.

### `Brewing_process_view_model`
Owns:
- read-only Active Brewing presentation state
- current displayed process stage
- displayed progress percentage
- displayed Pause/Stop availability flags

Important current fact:
`Brewing_process_view_model` is not real brewing workflow logic yet. It provides a small
stable shape for the Active Brewing screen to render while the real process/orchestrator
model is still being built.

### `Status_view_model`
Owns:
- the current diagnostic/status screen view model
- stable backing storage for formatted diagnostic strings
- cached comms snapshots so unchanged status text is not rebuilt every UI refresh
- a compact read-only machine snapshot for product-shaped screens that need raw current
  values instead of diagnostic strings

This module exists because the status screen is a dense diagnostics view. It is acceptable
for that screen to show compact text strings, but production brewing screens should move
toward raw values and screen-specific dirty widget updates.

### `Recipe_types`
Owns:
- plain recipe-domain data types and stable recipe IDs
- current read-only recipe metadata fields rendered by the recipe screens

This is intentionally not tied to LVGL. The embedded UI, future recipe storage, and future
web/API interface should be able to share these plain recipe data shapes.

The fuller recipe-model direction is documented in
`FreeBrewie_Recipe_Model_Decisions_2026-07-21.md`. That document should be checked before
adding recipe storage, draft editing, import/export, or brewing-plan conversion.

### `Recipe_catalog`
Owns:
- the current read-only static recipe list
- lookup by index and recipe ID
- realistic sample recipe fields for Details, Ingredients, Brewing, and Fermentation screens

This is a temporary catalog until real persistence exists, but it establishes the right
boundary: recipe data comes from `Logic/`, while screens only render it and emit user
actions.

### `Recipe_draft`
Owns:
- the current in-memory unsaved recipe draft edited by the Recipe Builder flow
- draft dirty/completion flags
- bounded future draft field ownership

Current fact:
`Recipe_draft` owns the temporary draft recipe name, first Details values, and first
Fermentables/Hops arrays, establishing the correct boundary before storage, keyboard input,
BeerXML/BeerJSON mapping, or Brewfather API sync is added. It stores the selected style
values, but does not own the available style catalog. Recipe Builder and draft screens
should render/edit this model instead of owning recipe values themselves.

### `Style_catalog`
Owns:
- bounded in-memory cache of selectable beer style records
- loading starter style records from `Data/styles.json`
- small fallback starter list for bring-up when the data file is not installed yet

Must not own:
- draft recipe state
- LVGL widgets
- full BeerXML/BeerJSON import
- recipe persistence

Current fact:
`Style_catalog` uses fixed-size buffers and no heap allocation. The first JSON reader is a
small parser for the project-owned `Data/styles.json` shape, not a general JSON engine.
This keeps the style data editable outside C while keeping the SOM runtime predictable.

### `Fault_logic`
Owns:
- SOM-side interpretation of current fault-state information for the UI/app layer

### `Machine_state`
Owns:
- compact current machine-state representation on the SOM side

### `Machine_targets`
Owns:
- current target-state representation that the SOM wants to hold/send

### `Startup_logic`
Owns:
- SOM-side startup progression/context

### `User_actions`
Owns:
- SOM-side meaning of user commands/actions before they become protocol actions or UI events

Rule:
`Logic/` should answer:
- what does the app currently know?
- what does the app currently want?
- what should the UI currently show?

It should not own the raw transport or the raw display backend.

---

## 3. `UI/`
Purpose:
Own LVGL screen construction and screen updating.

Current file set:
- `Screen_active_brewing.c`
- `Screen_active_brewing.h`
- `Screen_brew_checklist.c`
- `Screen_brew_checklist.h`
- `Screen_brew_setup.c`
- `Screen_brew_setup.h`
- `Screen_status.c`
- `Screen_status.h`
- `Screen_fault.c`
- `Screen_fault.h`
- `Screen_home.c`
- `Screen_home.h`
- `Screen_manual.c`
- `Screen_manual.h`
- `Screen_menu.c`
- `Screen_menu.h`
- `Screen_recipe_builder.c`
- `Screen_recipe_builder.h`
- `Screen_recipe_draft_brewing.c`
- `Screen_recipe_draft_brewing.h`
- `Screen_recipe_draft_details.c`
- `Screen_recipe_draft_details.h`
- `Screen_recipe_draft_ingredients.c`
- `Screen_recipe_draft_ingredients.h`
- `Screen_recipe_draft_menu.c`
- `Screen_recipe_draft_menu.h`
- `Screen_recipes.c`
- `Screen_recipes.h`
- `Screen_settings.c`
- `Screen_settings.h`
- `UI.c`
- `UI.h`
- `UI_dialog.c`
- `UI_dialog.h`
- `UI_number_editor.c`
- `UI_number_editor.h`
- `UI_scroll.c`
- `UI_scroll.h`
- `UI_status_bar.c`
- `UI_status_bar.h`
- `UI_text_editor.c`
- `UI_text_editor.h`
- `UI_theme.c`
- `UI_theme.h`

### `UI.c`
Owns:
- top-level UI init/update
- selecting which screens/components are active
- keeping the UI layer together
- owning button callback contexts for UI navigation
- deferring navigation requested from LVGL event callbacks until normal `ui_update()`
- lazy-creating optional placeholder screens that are not needed at startup

Must not own:
- serial I/O
- Linux DRM setup
- raw protocol parsing

Important current fact:
The target LVGL build uses the built-in allocator with `LV_MEM_SIZE` set to 256 KB. The
old-style navigation shell exceeded LVGL's default 64 KB heap when too many screens were
created during boot. New optional screens should therefore be created lazily unless they
are required for the first visible Home path.

### `Screen_status`
Owns:
- the scrollable live diagnostics/status screen
- status rows and temporary touch/click proof rows
- dirty-checked label updates from `status_screen_view_model_t`

Important current fact:
`Screen_status` is no longer intended to be the normal Home screen. It remains available
from the menu as a service/developer diagnostics destination.

Terminology/orientation note:
- the current `Screen_status` role is really a live status/debug screen
- a true animated boot/splash screen is wanted later and should be treated as a separate
  startup phase rather than mixed into the long-lived status screen
- the current target render is now portrait through the platform display layer; UI code
  should treat 272x480 as the target shape
- continuous full-screen animation is visibly choppy on the A13 SOM, so final screen
  transitions should favor partial redraws and targeted motion
- final screen components should avoid assuming long horizontal text fields will fit

At the current stage it is the correct place for:
- compact status/debug information
- a growing scrollable list of live values
- simple proof that display + UI + logic are connected

### `Screen_home`
Owns:
- current first product-shaped Home screen
- normal landing view for the user
- menu entry point and simple status summaries

### `Screen_menu`
Owns:
- old-Brewie-inspired top-level navigation menu
- menu rows for Home, Recipes, Manual/Cleaning, Settings, and Status
- menu row callback contexts

Must not own:
- workflow state
- machine-control decisions
- hardware-affecting actions

### `Screen_manual`
Owns:
- old-Brewie-inspired Manual/Cleaning scaffold
- safe selectable rows for Short Clean, Sanitizing Clean, Full Clean, Drain After Brew,
  Full Drain, and Unclogging
- local selected-mode explanation labels

Must not own:
- active clean/drain workflow state
- interlock decisions
- MCU commands
- direct hardware control

Important current fact:
`Screen_manual` is intentionally presentation-only. Selecting a row updates local
explanation text. Later, starting a real clean, drain, or unclogging workflow must route
through app-level logic and safety checks before becoming protocol traffic.

### `Screen_settings`
Owns:
- old-Brewie-inspired Settings scaffold
- safe selectable rows for WiFi, Units, Time, Water Settings, Calibration, Language,
  and About
- local selected-category explanation labels

Must not own:
- persistent settings storage
- network configuration writes
- calibration writes
- MCU commands
- direct system mutation

Important current fact:
`Screen_settings` is intentionally presentation-only. Selecting a row updates local
explanation text. Later, changing settings must route through app/system services and
validation rather than being performed directly from LVGL callbacks.

### `Screen_recipes`
Owns:
- safe first recipe chooser scaffold
- old-Brewie-inspired recipe-list presentation
- Recipes screen back/menu navigation callbacks
- recipe-row navigation requests using stable recipe IDs
- create-recipe navigation request to the safe Recipe Builder scaffold

Must not own yet:
- recipe persistence
- recipe selection side effects
- brewing start logic
- direct hardware control

### `Screen_recipe_builder`
Owns:
- safe old-Brewie-inspired first create-recipe step
- local non-persistent draft recipe name
- local-only draft-name dialog through `UI_dialog`
- bottom Cancel/Done presentation

Must not own yet:
- text entry overlays
- recipe persistence
- recipe validation
- save/delete behavior
- section editing for details, ingredients, brewing, or fermentation
- brewing start logic
- direct hardware control

Important current fact:
`Screen_recipe_builder` is intentionally presentation-only. It now follows the old UI
create-recipe shape more closely: create starts by naming the recipe, then the local
`DONE` path opens a separate draft recipe menu. The Name row now uses `UI_text_editor` to
commit bounded text into `Logic/Recipe_draft`, while real recipe storage and validation are
still absent.

### `Screen_recipe_draft_menu`
Owns:
- safe old-Brewie-inspired menu for a newly named local draft recipe
- local display of the draft recipe name
- old-style section buttons for Details, Ingredients, Brewing, and Fermentation
- local-only section explanation dialog for sections that do not have screens yet
- disabled `BREW LATER` presentation

Must not own yet:
- recipe persistence
- recipe validation
- real section editing forms
- brewing preflight or start behavior
- direct hardware control

Important current fact:
`Screen_recipe_draft_menu` is intentionally local-only. It is the next visual/navigation
step after naming a recipe, but it does not insert a recipe into `Recipe_catalog`, save a
file, or route any brewing action.

### `Screen_recipe_draft_brewing`
Owns:
- safe old-Brewie-inspired read-only Brewing section for a local draft recipe
- display of the draft recipe name
- local Water, Mash, Boil, and Cooling panels
- disabled `MODIFY LATER` presentation

Must not own yet:
- mash/water/boil/cooling editing forms
- brewing validation
- recipe persistence
- brewing preflight or hardware actions

Important current fact:
`Screen_recipe_draft_brewing` mirrors the old Brewing view shape before implementing the
old edit forms. Its values render from `Logic/Recipe_draft` and remain local-only.

### `Screen_recipe_draft_details`
Owns:
- safe old-Brewie-inspired Details section for a local draft recipe
- display of the draft recipe name
- `BEER STYLE` and `CALCULATED VALUES` panels
- local `SELECT STYLE` picker overlay

Must not own yet:
- full BJCP/style database selection
- calculated recipe values
- recipe persistence
- validation
- brewing preflight or hardware actions

Important current fact:
`Screen_recipe_draft_details` mirrors the old Details view shape before implementing the
old full Details edit form. Its visible values render from `Logic/Recipe_draft`; style
choices come from `Logic/Style_catalog`. Selecting a style updates the RAM-only draft but
does not save, calculate, import, or contact the MCU.

### `Screen_recipe_draft_ingredients`
Owns:
- safe old-Brewie-inspired read-only Ingredients section for a local draft recipe
- display of the draft recipe name
- local Fermentables/Hops tab selection
- fermentable bag and hop cage rows rendered from `Logic/Recipe_draft`
- disabled `MODIFY LATER` presentation

Must not own yet:
- fermentable/hop editing forms
- unit conversion
- recipe persistence
- validation
- brewing preflight or hardware actions

Important current fact:
`Screen_recipe_draft_ingredients` mirrors the old Ingredients view shape before
implementing the old Fermentables/Hops edit forms. Its values render from `Logic/Recipe_draft`
and remain local-only.

### `UI_text_editor`
Owns:
- reusable bounded text-entry modal
- textarea and on-screen keyboard widgets
- OK/Cancel callbacks

Must not own:
- recipe values
- search/filter state
- validation
- persistence
- hardware actions

Important current fact:
`UI_text_editor` is UI infrastructure. It commits text through a caller callback; the model
or screen that opened it decides what to do with that text. The first user is Recipe Builder,
which stores the recipe name in `Logic/Recipe_draft`. The keyboard uses custom equal-width
alphabetical LVGL keyboard maps and old-Brewie-inspired colors, because the default LVGL
QWERTY layout is too dense for reliable touch on the Brewie panel. The recipe-name editor
is shaped like the old full-screen dark/orange keyboard view: title and input above, alphabet
grid below, orange action icons for symbol/shift/backspace controls, command row at the
bottom, and Nordic characters such as `æ`, `ø`, and `å` on the symbol page. A tiny custom
keyboard event adapter handles the visual shift arrow and `Done` label while leaving normal
typing and backspace behavior on LVGL's keyboard implementation.

### `Screen_recipe_detail`
Owns:
- safe selected-recipe landing presentation
- old-style section navigation rows for Details, Ingredients, Brewing, and Fermentation
- Brew button navigation to the safe Brew Setup scaffold
- detail-screen back/menu navigation callbacks

Must not own yet:
- editing forms
- recipe save/delete behavior
- brewing preflight or start behavior
- direct hardware control

### `Screen_recipe_section`
Owns:
- safe read-only presentation for one selected recipe section
- selected recipe/section labels and fixed reusable row widgets backed by `Recipe_types`
- back/menu navigation callbacks

Must not own yet:
- recipe editing forms
- recipe persistence
- brewing preflight or start behavior
- direct hardware control

### `Screen_brew_setup`
Owns:
- safe old-Brewie-inspired pre-brew parameter scaffold
- selected recipe label
- local display-only Automatic Water Inlet and Automatic Cooling toggles
- Start navigation to the safe checklist scaffold

Must not own:
- brewing start permission
- recipe validation
- checklist completion
- active brewing state
- MCU commands
- direct hardware control

Important current fact:
`Screen_brew_setup` is intentionally a UI scaffold. Option rows toggle local labels only.
The real transition from selected recipe to active brewing must later route through
`App_orchestrator`, recipe validation, fault/startup state, and MCU safety commands.

### `Screen_brew_checklist`
Owns:
- safe old-Brewie-inspired pre-brew checklist scaffold
- selected recipe label
- local display-only checklist toggles
- Start navigation to the safe Active Brewing scaffold

Must not own:
- checklist validation authority
- brewing start permission
- active brewing state
- MCU commands
- direct hardware control

Important current fact:
`Screen_brew_checklist` is intentionally a UI scaffold. Checklist rows are local UI state
only. The real final Start action must later be controlled by app-level validation and MCU
state, not by the screen layer.

### `Screen_active_brewing`
Owns:
- safe old-Brewie-inspired Active Brewing presentation scaffold
- selected recipe label
- local Overall/Actions tab switching
- read-only live mash/boil temperature and pump-state presentation from the logic-layer
  machine snapshot
- compact visual process-stage strip driven by `Brewing_process_view_model`
- process/progress/tank presentation using simple LVGL objects rather than bitmap assets
- inert Pause/Stop process-control presentation

Must not own:
- active brewing process state
- pause/continue/stop authority
- heaters, pumps, valves, or sensors
- MCU commands
- direct hardware control

Important current fact:
`Screen_active_brewing` is intentionally UI-only. It now renders read-only facts from the
logic-layer machine snapshot and `Brewing_process_view_model`, but it is not the source of
truth for real brewing. Later this screen should render full process state from
app/process logic and emit user requests to `App_orchestrator`, not drive hardware directly.

### `Screen_fault`
Owns:
- eventual fault presentation screen
- not yet the current bring-up focus

### `UI_dialog`
Owns:
- reusable lightweight modal dialog/popup behavior
- modal overlay/panel widgets and simple show/hide behavior

Must not own:
- workflow decisions
- persistence
- hardware actions
- screen navigation

### `UI_number_editor`
Owns:
- reusable bounded numeric editor modal
- caller-provided min/max/step handling
- integer-only value display, including deciliters shown as liters without floating point

Must not own:
- recipe fields
- validation policy beyond caller-provided bounds
- persistence
- hardware actions

Important current fact:
`UI_number_editor` is first used by the draft Details `Batch size` row. It commits a
bounded `uint16_t` through a caller callback, so future water-volume, temperature, and time
fields can reuse the same interaction pattern without duplicating LVGL modal code.

### `UI_scroll`
Owns:
- shared scrollbar gutter styling for scrollable LVGL containers

Important current fact:
LVGL draws scrollbars over the scrollable object. On the portrait display, full-width rows
need a small content gutter so the scrollbar does not visually overlap row rectangles or
text.

### `UI_status_bar`
Owns:
- reusable status-bar behavior

### `UI_theme`
Owns:
- shared theme/style setup for the Brewie UI layer

Rule:
`UI/` owns what is shown and how LVGL objects are arranged.
It should not own transport, protocol, or Linux device bring-up.

---

## 4. `Platform/`
Purpose:
Own target/simulator platform glue that is not business logic and not UI semantics.

Current file set:
- `Display.c`
- `Display.h`
- `Display_rotation.c`
- `Display_rotation.h`
- `Logging.c`
- `Logging.h`
- `Platform.c`
- `Platform.h`
- `Time_base.c`
- `Time_base.h`

### `Display`
Owns:
- LVGL initialization path for the current platform
- simulator display creation for non-ARM builds
- target display backend setup for ARM builds
- periodic LVGL handler progression

Important current fact:
- simulator path uses SDL
- target path now uses Linux DRM
- DRM rotation math is isolated in `Display_rotation`
- target DRM init succeeds on the SOM
- first visible text has now been shown on the real screen

This is a major bring-up milestone.

`Display.c` must own:
- SDL-vs-target backend split
- LVGL display backend creation
- LVGL periodic handler progression

It must not own:
- low-level pixel rotation loops
- screen-specific content
- serial data interpretation
- app state decisions

### `Display_rotation`
Owns:
- RGB565 pixel mapping from logical portrait LVGL dirty rectangles into the physical
  landscape DRM framebuffer
- the production rotation path, which uses a NEON 8x8 tiled core on the A13 target and
  scalar edge handling/fallback elsewhere

Related tool:
- `Tools/DisplayRotationBenchmark` measures the explicit scalar fallback against the
  production best-available rotation path on the SOM.

### `Logging`
Owns:
- app logging helpers and current debug prints

### `Time_base`
Owns:
- app timing source / monotonic time access

### `Platform`
Owns:
- top-level platform-side helpers that do not belong in the display or timing module specifically

Rule:
`Platform/` owns system glue, not application meaning.

---

## Current boot/update flow
The intended current SOM-side flow is:

1. `main.c` enters the app
2. `App.c` initializes platform, comms, logic, and UI
3. `Display.c` initializes LVGL backend
4. `Screen_home` becomes the first visible screen
5. runtime loop continues:
   - comms update
   - logic update
   - UI update
   - display/LVGL handler update

This ordering is important:
- comms facts should be gathered first
- logic should convert those facts into app-facing state
- UI should update from that app-facing state
- display backend should then flush/refresh

---

## Current known-good bring-up baseline
The following is now proven on the SOM target:

- target build succeeds
- app runs as runtime user `brewie`
- `brewie.service` starts `/opt/brewie/brewie_app`
- `/dev/ttyS1` opens
- heartbeat is sent
- MCU `STATUS_REPORT` frames are received
- target DRM display init succeeds
- Home/menu/status navigation works on the real portrait display
- live status/debug text remains available from the menu

This means:
- the restructured app is alive
- comms and display can coexist
- the current architecture split is not merely theoretical anymore

---

## Current known limitations
The current SOM app is still in early bring-up.

Known limitations:
- `Screen_status` is still a bring-up/debug screen, not finished product UI
- DRM build path currently includes some build-environment friction/workarounds described in the development-environment doc
- display, touch, Home/menu/status, and comms are proven together on the real SOM
- Cortex-A8/NEON target optimization flags and LVGL NEON drawing are enabled and build,
  but still need target-side measurement after deployment
- the UI layer is not yet feature-complete
- `Screen_recipes` is currently a static safe scaffold, not real recipe storage or recipe
  selection
- `Screen_fault` is not yet the active focus

---

## Architecture rules to preserve
These are the important rules to keep from drifting:

### 1. Keep comms separate from UI
The serial/protocol path must continue working regardless of screen work.

### 2. Keep `main.c` small
Do not move subsystem code into `main.c`.

### 3. Keep `App.c` as coordinator, not dumping ground
Do not let `App.c` become raw transport, raw DRM, or widget-detail code.

### 4. Keep platform glue in `Platform/`
Linux/SDL/DRM details belong there.

### 5. Keep screen-specific code in `UI/`
Visible screen construction belongs in screen files, not in comms/platform files.

### 6. Keep SOM-side meaning in `Logic/`
The SOM should translate machine/comms facts into app/UI meaning in the logic layer.

---

## Practical next-step interpretation
Given the current code and bring-up state, the most sensible next UI-side direction is:

1. keep current comms baseline intact
2. keep DRM target display path intact
3. keep Home as the normal first screen
4. keep a true animated boot/splash screen as a separate future startup phase
5. keep touch/input owned by the platform layer
6. use the proven touch path to grow the old-style top-level navigation shell
7. keep `Screen_status` available as a diagnostic destination
8. grow `Screen_recipes` from static chooser scaffold toward recipe browsing/details
   before implementing any brewing action

This avoids mixing live status, future boot animation, and broader UI redesign.
