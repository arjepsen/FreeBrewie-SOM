# FreeBrewie UI Current Status
_Date: 2026-07-18_
_Updated: 2026-07-18_

## Purpose
This document captures the current real status of the SOM-side UI bring-up.

It is meant to stay short and practical:
- what is already proven
- what is still missing
- what the current screen actually is
- what the next UI step should be

It should match the real code and bring-up state, not an older plan.

---

## Current position
The SOM-side application is now past the earlier headless-only baseline.

The following is currently proven on the real SOM target:

- `brewie_app` builds in the `FreeBrewie-SOM` repo
- the new `src/` structure is in place:
  - `src/UI/`
  - `src/Logic/`
  - `src/Comms/`
  - `src/Platform/`
- the application runs on the SOM as user `brewie`
- `/dev/ttyS1` opens successfully
- SOM heartbeat is sent
- MCU `STATUS_REPORT` frames are received
- `FAULT_REPORT` can also be received
- target LVGL display bring-up now initializes through Linux DRM
- a first visible portrait text screen has now been shown on the real target display
- the target display path now uses custom rotated DRM flushing, so LVGL works in logical
  272x480 portrait coordinates even though the physical panel scans out 480x272
- the Goodix touchscreen is visible as `/dev/input/event0`
- touch is now wired into LVGL and mapped into the portrait coordinate system
- a temporary status-screen button proves LVGL receives click events
- normal heartbeat/status report logging has been reduced so the journal stays useful
- Home/menu/status navigation works in the simulator
- Home/menu/status navigation has now also been verified on the real SOM display
- touch has been verified on the real SOM screen after the latest Home/menu cleanup
- MCU RX reports continue increasing sequence numbers while the UI is running
- target CPU use was first observed around 1.4-1.7% during the current Home/menu/status
  baseline, then around 0.7% after the first top-level navigation cleanup
- normal target builds now use `RelWithDebInfo`, Cortex-A8/NEON CPU flags, and LVGL's
  NEON software drawing path for supported RGB565 blend/convert operations
- Home `LET'S BREW` now opens a safe first `Recipes` scaffold inspired by the old recipe
  chooser
- `Screen_status` is structured as a scrollable diagnostics list
- status text formatting is separated into `Logic/Status_view_model.*`
- visible label updates are dirty-checked so unchanged text is not repeatedly pushed into
  LVGL

So the project is no longer only at “headless serial baseline”.
We now have:
- working comms baseline
- working target display initialization
- first visible portrait LVGL output on the real screen
- first target touch integration proof
- first product-shaped navigation shell
- real target confirmation that the current UI shell, touch path, and comms path coexist
  with low CPU use

---

## What is currently visible
The normal UI direction now starts at Home, with Status available through the menu.
The latest hardware pass confirms that the Home/menu/status shell works on the real SOM,
not only in the simulator.

Status is still not the final boot UX. It remains available to prove and inspect that:
- LVGL is alive
- DRM output is alive
- the app can render visible objects while comms are also running
- MCU/link values are still reaching the UI

The status screen remains a high-contrast live diagnostics screen. It is still reachable
from the menu, but it is no longer the normal product landing screen.

This is intentional. It should stay useful for development and service visibility, while
Home becomes the normal user-facing screen.

---

## Current display backend
For the real SOM target, the current display path is:

- LVGL
- custom Linux DRM scanout backend in `Platform/Display.c`
- RGB565 dirty-rectangle rotation in `Platform/Display_rotation.c`
- DRM device path: `/dev/dri/card0`
- logical LVGL resolution: 272x480
- physical DRM scanout: 480x272 RGB565

This replaced the earlier target-side display bypass and the temporary landscape DRM proof.
LVGL's built-in DRM rotation was tested on hardware and was not usable. The app now rotates
dirty rectangles into a double-buffered DRM scanout buffer and page-flips on vblank.

The simulator path still uses SDL.

So the current split is:

- simulator build -> SDL
- target build -> custom rotated DRM

---

## What is already separated correctly
The current code structure is now aligned with the intended SOM architecture split:

### `src/Comms/`
Owns:
- serial transport
- protocol framing / parsing
- heartbeat send path
- incoming MCU frame handling

### `src/Logic/`
Owns:
- application-side logic/state interpretation
- status-screen view-model data in `Status_view_model.c`
- non-UI machine/application reasoning
- reserved future app-level routing in `App_orchestrator.c`

### `src/UI/`
Owns:
- screen creation/update
- UI widgets and screen-level layout
- Home/menu/status screens and placeholder navigation targets
- dirty-checked updates for visible label text

### `src/Platform/`
Owns:
- display backend glue
- timing base
- logging
- platform-specific behavior

### `src/App.c`
Owns:
- application-level orchestration of the above pieces

### `src/main.c`
Owns:
- program entry
- top-level bring-up
- starting the application

This is important:
the comms path is no longer being treated as “part of the UI”.
The display bring-up was added beside the working comms path, not by mixing the two together.

---

## What is not yet finished
The SOM-side UI is still early bring-up work, but the baseline is now strong enough to grow
the Home/menu shell in small steps.

The following are **not** yet finished:

- stable final home/status/fault screen design
- final real screen flow between startup / home / status / fault screens
- binding real machine state to a fuller UI
- real recipe storage and recipe selection/detail screens
- routing hardware-affecting user actions through `App_orchestrator`
- polished redraw/update behavior
- true animated boot/splash screen during SOM startup

Also important:
the current visible text render proves that display output works,
but it does **not** yet prove that the full UI layer is complete or stable.

---

## Current touch discovery
The target touchscreen has been identified:

- device: `/dev/input/event0`
- name: `Goodix Capacitive TouchScreen`
- event types include `BTN_TOUCH`, `ABS_X`, `ABS_Y`, and multitouch fields
- ABS ranges observed through `evtest`:
  - X 0..799
  - Y 0..479

The runtime `brewie` user is already in the `input` group. The app now wires this input
device into LVGL through the platform display layer. Corner testing on the physical screen
showed plausible portrait coordinates, and the temporary `Touch OK` button proves normal
LVGL button click events.

---

## Current known-good bring-up baseline
The safest currently proven baseline is:

1. app starts on SOM as `brewie`
2. DRM display initializes successfully
3. `/dev/ttyS1` opens
4. heartbeat is sent
5. MCU reports are received
6. a visible portrait LVGL live status/debug screen is shown
7. touch input reaches LVGL in portrait coordinates
8. `brewie.service` starts `/opt/brewie/brewie_app` automatically
9. simulator Home/menu/status navigation works after the latest cleanup
10. real SOM Home/menu/status navigation works after the latest cleanup
11. real SOM touch still works after the latest cleanup
12. MCU RX reports keep increasing sequence numbers while the UI is running
13. target CPU was roughly 1.4-1.7% in the first verified baseline
14. target CPU was roughly 0.7% after the first top-level navigation cleanup

This is the current anchor state.

Any further UI work should preserve this baseline.

---

## Current practical interpretation
At this point, the SOM-side application has reached:

- first real target display output
- correct target portrait orientation
- while preserving the already-working MCU serial link

That is a meaningful milestone because it proves:
- structure cleanup did not destroy the comms baseline
- target display output can coexist with the live MCU link
- the project can now move from “headless proof” to “real screen bring-up”

---

## Next recommended UI step
The next UI step should stay small and controlled.

Recommended next step:

1. keep the comms path unchanged
2. keep the rotated DRM display path unchanged
3. keep Home as the normal first screen
4. keep touch/input owned by `Platform/`, not directly inside screen code
5. continue polishing `Screen_home` and the top-level menu
6. keep the status/debug screen available as a service/developer screen
7. keep diagnostic values in a scrollable list, since the list will grow over time
8. add safe placeholder destinations before implementing hardware-affecting workflows
9. grow `Screen_recipes` from a static chooser scaffold into the old-style recipe browsing
   and details flow

So the next goal is **not** “build the whole UI”.
The next goal is:
- continue refining the first product-shaped home/navigation shell
- keep the current live status/debug screen as a diagnostic destination
- keep Recipes, Extras, and Settings as safe navigation targets until their workflows are
  designed and routed through app logic
- use the old UI as the screen/functionality reference, but keep the new implementation
  modular, readable, safe, and optimized

Naming/orientation note:
- `Screen_status` is currently the live diagnostics screen, not finished product UI
- a true animated boot/splash screen should be a separate future startup phase
- the target render is now portrait
- continuous full-screen animation is visibly choppy on the A13 SOM; keep the final UI
  pleasant by using partial redraws, short transitions, and targeted motion

---

## Current caution
The current display/build path still includes bring-up-era technical debt.

In particular:
- DRM enablement introduced extra VM/sysroot dependency handling
- runtime `libdrm2` is required on the SOM
- current visible screen content is still a deliberate test-oriented render
- touch mapping has only been proven through simple corner/button testing, not through a
  finished production UI
- NEON/optimized target flags build successfully, but still need a target-device run to
  measure real-world CPU and responsiveness

So Claude or any later assistant should not assume:
- the display stack is fully polished
- the build environment is friction-free
- the screen flow is already final

---

## Short summary
Current real status is now:

- code structure reorganized and building
- serial link proven
- heartbeat proven
- MCU reports proven
- target portrait DRM display init proven
- first visible portrait LVGL target output proven
- touch device integrated and simple LVGL click events proven
- simulator Home/menu/status navigation proven

The UI is therefore no longer “not working”.
A more accurate description is:

**The first target display and touch milestones are achieved, but the real UI is still in early bring-up.**
