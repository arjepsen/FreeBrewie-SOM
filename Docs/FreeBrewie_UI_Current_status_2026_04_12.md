# FreeBrewie UI Current Status

## Purpose
This document captures the **current software status** of the FreeBrewie SOM/UI project.

It is meant to answer:
- where are we right now?
- what is working?
- what is only scaffolding?
- what is the next sensible step?

This file is intentionally more changeable than the environment/setup document.

---

## Project identity reminder
The active project naming should now be treated as:

- **FreeBrewie**

Older names still appear in:
- repo naming
- some file naming
- build messages
- historical notes

Those older names should not be taken as the intended product/project name going forward.

---

## High-level project split
Current intended architecture remains:

### SOM owns
- UI
- workflows
- recipes
- timing
- manual-service logic
- cleaning logic
- high-level protocol/master behavior

### MCU owns
- hardware execution
- measurements
- interlocks
- faults
- startup gating
- shutdown behavior

So the SOM is the high-level control/UI side.
The MCU is the direct hardware/safety side.

---

## Current SOM platform state
The SOM platform is no longer hypothetical.
It is proven alive enough for real work.

Confirmed state:
- SOM boots after MCU startup request
- SSH access works
- admin login works
- display stack is at least partially alive
- touch controller is detected
- `/dev/ttyS1` exists
- `brewie.service` exists

That means the SOM side is at:
- **platform brought up**
- but **application still at early bootstrap stage**

---

## Current repo state
Working repo folder on VM:
- `~/brewie-ui-rebuild`

Git state during this check:
- branch: `main`
- working tree clean
- remote still points to:
  - `git@github.com:arjepsen/ReBrewie-LVGL.git`

This means the repo is not in a broken or unknown state.
It is simply still carrying older naming.

---

## Current source structure
Current known top-level application/source split:

- `src/main.c`
- `src/main_sim.c`
- `src/App.c`
- `src/App.h`
- `UI/UI.c`
- `UI/UI.h`

Current interpretation:
- `src/main.c` = real target process entry only
- `src/main_sim.c` = simulator process entry only
- `src/App.c` = application orchestration and lifecycle ownership
- `UI/UI.c` = shared UI layer

This is now the correct mental model.

---

## Current structural clarification
A key clarification from recent bring-up work is:

### `main.c` must stay thin
`src/main.c` should not become the place where the application logic grows.

It should only do very top-level process startup such as:
- minimal early process/bootstrap work
- calling `app_init()`
- calling `app_run()` or equivalent

It should **not** become the place that directly owns:
- serial protocol behavior
- UI logic
- LVGL widget construction
- target display backend details

### `App.c` owns application orchestration
`App.c` should own the application-level bring-up sequence and coordination.

That means:
- deciding which subsystems the app starts
- owning the app lifecycle
- coordinating display path, UI path, and comms path
- owning the shared app state used by the UI and integration layers

But `App.c` should still not absorb low-level details that belong in dedicated modules.

### Dedicated modules below `App.c`
The intended separation should be:

- **Display/backend module**
  - target LVGL display backend bring-up
  - framebuffer/DRM/input/display driver glue
- **Comms module**
  - `/dev/ttyS1`
  - protocol framing
  - heartbeat send/receive handling
  - `STATUS_REPORT` intake
- **UI layer**
  - screen/widget creation
  - visual updates from app state

So `App.c` coordinates these parts, but does not replace them.

---

## Current target build status
A key milestone has already been reached:

### Now proven
- `brewie_app` builds as a real target executable
- target preset works
- target executable links successfully
- target executable runs on the SOM as user `brewie`

This is important because the target path is no longer only theoretical.

---

## Current target runtime status
Current known-good baseline is now:
- app runs on the SOM
- `/dev/ttyS1` opens successfully
- heartbeat is sent
- `STATUS_REPORT` frames are received from the MCU

This is the current **known-good headless serial baseline**.

That baseline is important and should be kept intact while display bring-up is worked on separately.

---

## Current display status
System-level display facts observed on the SOM:
- framebuffer present
- DRM handoff occurs
- backlight/display path is at least partly alive
- a blinking cursor is visible during boot and disappears later

This strongly suggests:
- Linux console / framebuffer / DRM path exists
- but the application display path is not yet stable enough to treat as proven

So the display path should still be treated as:
- **partially alive at platform level**
- **not yet proven at app level**

---

## Current UI status
The UI layer exists, but should still be treated as early scaffolding.

What is true:
- shared UI files exist
- simple LVGL content has been used as a sanity-check direction

What is **not** yet true:
- stable target-side display bring-up through the intended application structure
- stable end-to-end separation between display backend, UI, and comms during target runtime

---

## Current integration status
The SOM↔MCU path is now past the first purely hypothetical stage.

Already proven:
- target process starts
- serial device opens
- heartbeat transmit works
- MCU `STATUS_REPORT` reception works

Still intentionally held back from the display path for now:
- combining unstable display bring-up with the known-good serial baseline
- mixing comms experiments directly into UI ownership

This is deliberate.
The current goal is to keep one stable baseline while the next layer is brought up separately.

---

## Current service/startup status
The SOM still has the appliance/service scaffold in place, but the real app structure is still being refined.

So the project is still in a development-stage runtime shape rather than a finished appliance boot flow.

That is acceptable at this stage.

---

## Immediate next step
The next step should be:

### Add a very small display-only bring-up path
This should be done **without disturbing the known-good headless serial baseline**.

Goal:
- prove the smallest possible real target-side LVGL display output
- do it through the real application structure
- avoid introducing disposable test files

### Structural rule for that step
The display-only bring-up should follow this shape:

- `main.c` stays thin
- `App.c` owns application bring-up
- `App.c` requests display initialization through a dedicated display/backend module
- UI stays separate from comms
- comms is not integrated into the UI layer

### Practical first target for display bring-up
The first display milestone should be extremely small:
- initialize the target display/LVGL backend
- create the smallest possible visible LVGL output
- prove stable on-screen rendering before reintroducing the UI layer above it

So the next display step is **not** “full UI”.
It is:
- **stable minimal target display output first**
- **UI layer second**
- **recombine with comms only after that**

---

## Current development priority
The immediate development priority is therefore:

1. keep the headless serial baseline intact
2. prove a minimal display-only path
3. keep comms and UI separate while doing so
4. only then begin reintroducing more of the shared UI layer on target

This should reduce confusion and make failures much easier to localize.
