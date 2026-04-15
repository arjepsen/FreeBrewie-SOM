# FreeBrewie SOM Architecture Notes
_Date: 2026-04-15_

## Purpose
This note defines the intended **SOM-side software structure** at a high level, before more code is written.

Main idea:
- the **MCU** owns hardware execution, measurements, interlocks, and faults
- the **SOM** owns UI, business logic, data handling, and communication with the MCU

The SOM side should be **one main application**, with clear internal separation.

---

## Top-level groups

### 1. `main` / `App`
Top-level process startup and coordination.

**Purpose**
- start the application
- initialize major modules
- run the main loop
- coordinate, not implement, lower-level details

**Files**
- `main.c`
- `App.h/.c`

---

### 2. `UI`
Everything the user sees and touches.

**Purpose**
- screens
- widgets
- navigation
- visual updates
- forwarding user actions upward

**Must not own**
- serial/protocol details
- recipe timing
- machine control decisions

**Core files**
- `UI.h/.c`
- `UI_types.h`
- `UI_theme.h/.c`

**Screen files**
- `Screen_boot.h/.c`
- `Screen_home.h/.c`
- `Screen_fault.h/.c`

**Later screen files**
- `Screen_recipe_list.h/.c`
- `Screen_recipe_run.h/.c`
- `Screen_manual.h/.c`
- `Screen_cleaning.h/.c`
- `Screen_settings.h/.c`
- `Screen_shutdown.h/.c`

**Shared UI components**
- `UI_status_bar.h/.c`
- `UI_dialog.h/.c`

**Special note**
- `Screen_boot` is the correct place for the first visible LVGL bring-up and simple startup/debug status text

---

### 3. `Logic`
SOM-side business logic.

**Purpose**
- startup flow
- shutdown flow
- recipe logic
- manual-service logic
- cleaning logic
- fault handling at user/application level
- interpreting machine state
- producing desired machine targets

**Must not own**
- raw serial I/O
- LVGL widget code

**Core files**
- `App_logic.h/.c`
- `App_types.h`

**Machine state / targets**
- `Machine_state.h/.c`
- `Machine_targets.h/.c`

**Early logic files**
- `Startup_logic.h/.c`
- `Fault_logic.h/.c`
- `User_actions.h/.c`

**Later logic files**
- `Shutdown_logic.h/.c`
- `Recipe_logic.h/.c`
- `Manual_logic.h/.c`
- `Cleaning_logic.h/.c`
- `Recipes.h/.c`
- `Settings.h/.c`

---

### 4. `Comms`
Communication with the MCU.

**Purpose**
- own `/dev/ttyS1`
- own the Brewie serial protocol
- send heartbeat
- receive machine reports
- send control snapshots and requests
- maintain link state

**Important split**
Communication should be split into:
- a **transport** layer
- a **protocol/comms** layer

Do **not** bury comms inside UI or high-level logic.

**Core files**
- `Comms.h/.c`
- `Comms_types.h`

**Transport**
- `Transport_serial.h/.c`

**Protocol**
- `Protocol.h/.c`

**Link state**
- `Comms_link.h/.c`

**Later split if needed**
- `Comms_rx.h/.c`
- `Comms_tx.h/.c`
- `Protocol_crc.h/.c`

---

### 5. `Platform`
Linux/target-side plumbing.

**Purpose**
- bring up the real display backend
- input backend
- timing helpers
- logging
- system/path helpers

This is **not** the UI itself.

**Core files**
- `Platform.h/.c`
- `Display.h/.c`
- `Time_base.h/.c`
- `Logging.h/.c`

**Later platform files**
- `Input.h/.c`
- `System_paths.h/.c`

**Special note**
- the first minimal display-only bring-up belongs in `Display.c`
- this must stay separate from protocol/comms

---

## Recommended minimal first structure

```text
src/
    main.c
    App.h
    App.c

    UI/
        UI.h
        UI.c
        UI_types.h
        UI_theme.h
        UI_theme.c
        Screen_boot.h
        Screen_boot.c
        Screen_home.h
        Screen_home.c
        Screen_fault.h
        Screen_fault.c
        UI_status_bar.h
        UI_status_bar.c
        UI_dialog.h
        UI_dialog.c

    Logic/
        App_logic.h
        App_logic.c
        App_types.h
        Machine_state.h
        Machine_state.c
        Machine_targets.h
        Machine_targets.c
        Startup_logic.h
        Startup_logic.c
        Fault_logic.h
        Fault_logic.c
        User_actions.h
        User_actions.c

    Comms/
        Comms.h
        Comms.c
        Comms_types.h
        Transport_serial.h
        Transport_serial.c
        Protocol.h
        Protocol.c
        Comms_link.h
        Comms_link.c

    Platform/
        Platform.h
        Platform.c
        Display.h
        Display.c
        Time_base.h
        Time_base.c
        Logging.h
        Logging.c
```

---

## Main responsibility flow

Keep this direction:

- **UI** shows state and emits user actions
- **Logic** decides what the application wants
- **Comms** exchanges state and requests with the MCU
- **Platform** provides Linux/display/input/time plumbing
- **App** coordinates the groups

More concretely:

- UI -> Logic
- Logic -> Machine targets
- Comms -> Machine state
- Platform -> supports UI and Comms
- App -> ties everything together

---

## Boundary rules

### `App`
coordinates modules  
does not become raw serial code or widget code

### `UI`
owns screens  
does not parse frames or decide brewing policy

### `Logic`
owns decisions/workflows  
does not own raw transport or LVGL widgets

### `Comms`
owns transport + protocol  
does not own UI or recipe logic

### `Platform`
owns target/Linux plumbing  
does not own business meaning

---

## Immediate practical use
This structure should guide the next coding steps.

Immediate goals:
1. keep the working serial baseline in the **Comms** group
2. add the first minimal LVGL display output through **Platform/Display**
3. use **UI/Screen_boot** as the first real visible screen
4. keep communication independent of display work
