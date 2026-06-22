# FreeBrewie SOM Architecture Notes
_Date: 2026-06-22_

## Purpose
This document defines the current target architecture for the Brewie SOM application.

It is meant to keep file ownership, module boundaries, and subsystem responsibilities clear while the SOM app is still in early bring-up.

It should be read together with:
- `FreeBrewie_UI_Current_Status_2026-06-22.md`
- `FreeBrewie_SOM_Development_Environment_Consolidated_2026-06-22.md`
- `Brewie_SOM_Platform_Notes_2026-06-22.md`
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
- logic init/update
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
- `App_logic.c`
- `App_logic.h`
- `Fault_logic.c`
- `Fault_logic.h`
- `Machine_state.c`
- `Machine_state.h`
- `Machine_targets.c`
- `Machine_targets.h`
- `Startup_logic.c`
- `Startup_logic.h`
- `User_actions.c`
- `User_actions.h`

### `App_logic`
Owns:
- app-level state progression
- assembling compact screen-facing data from comms/platform facts
- current boot-screen view model

At the current stage, this is important because:
- the boot screen is now driven through logic-owned text/status data
- communication facts are not supposed to be stuffed directly into widgets everywhere

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
- `Screen_boot.c`
- `Screen_boot.h`
- `Screen_fault.c`
- `Screen_fault.h`
- `Screen_home.c`
- `Screen_home.h`
- `UI.c`
- `UI.h`
- `UI_dialog.c`
- `UI_dialog.h`
- `UI_status_bar.c`
- `UI_status_bar.h`
- `UI_theme.c`
- `UI_theme.h`

### `UI.c`
Owns:
- top-level UI init/update
- selecting which screens/components are active
- keeping the UI layer together

Must not own:
- serial I/O
- Linux DRM setup
- raw protocol parsing

### `Screen_boot`
Owns:
- the current first visible boot/debug screen
- early visible bring-up path
- current proof that target LVGL output works

Important current fact:
`Screen_boot` is intentionally the first visible screen used for bring-up and debugging.
It is not yet the finished product UI.

At the current stage it is the correct place for:
- unmistakable visible text
- compact boot/debug status
- simple proof that display + UI + logic are connected

### `Screen_home`
Owns:
- eventual main/home screen for the product UI
- not yet the current bring-up focus

### `Screen_fault`
Owns:
- eventual fault presentation screen
- not yet the current bring-up focus

### `UI_dialog`
Owns:
- reusable dialog/popup behavior

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
- target DRM init succeeds on the SOM
- first visible text has now been shown on the real screen

This is a major bring-up milestone.

`Display.c` must own:
- SDL-vs-target backend split
- LVGL display backend creation
- LVGL periodic handler progression

It must not own:
- screen-specific content
- serial data interpretation
- app state decisions

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
4. `Screen_boot` becomes the first visible screen
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
- `/dev/ttyS1` opens
- heartbeat is sent
- MCU `STATUS_REPORT` frames are received
- target DRM display init succeeds
- first visible text has been shown on screen

This means:
- the restructured app is alive
- comms and display can coexist
- the current architecture split is not merely theoretical anymore

---

## Current known limitations
The current SOM app is still in early bring-up.

Known limitations:
- `Screen_boot` is still a bring-up/debug screen, not finished product UI
- touch/input path is not yet reintegrated into BrewieApp
- DRM build path currently includes some build-environment friction/workarounds described in the development-environment doc
- display output is proven, but the UI layer is not yet feature-complete
- `Screen_home` / `Screen_fault` are not yet the active focus

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
3. keep `Screen_boot` as the current visible bring-up screen
4. improve the boot screen only enough to prove the data path cleanly
5. only then grow toward fuller home/fault screens and later touch/input integration

This avoids mixing first-display bring-up with broader UI redesign.
