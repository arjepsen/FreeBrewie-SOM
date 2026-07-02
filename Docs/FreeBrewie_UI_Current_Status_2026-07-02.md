# FreeBrewie UI Current Status
_Date: 2026-07-02_
_Updated: 2026-07-02_

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

So the project is no longer only at “headless serial baseline”.
We now have:
- working comms baseline
- working target display initialization
- first visible portrait LVGL output on the real screen
- first target touch integration proof

---

## What is currently visible
The currently visible target screen is still only a bring-up/debug screen.

It is not yet the final boot UX.
It is being used to prove that:
- LVGL is alive
- DRM output is alive
- the app can render visible objects while comms are also running

The current visible proof is a high-contrast live status/debug text render.

This is intentional.
At this stage, obvious visible output is more important than polished screen design.

---

## Current display backend
For the real SOM target, the current display path is:

- LVGL
- custom Linux DRM scanout backend in `Platform/Display.c`
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
- status-screen view-model data
- non-UI machine/application reasoning

### `src/UI/`
Owns:
- screen creation/update
- UI widgets and screen-level layout
- current live status/debug screen

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
The SOM-side UI is still early bring-up work.

The following are **not** yet finished:

- stable final status/home/fault screen design
- real screen flow between startup / status / home / fault screens
- binding real machine state to a fuller UI
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
3. keep using the current status/debug screen as the first real screen
4. keep touch/input owned by `Platform/`, not directly inside screen code
5. move toward `Screen_home` and fuller UI behavior
6. keep the status/debug screen available as a service/developer screen

So the next goal is **not** “build the whole UI”.
The next goal is:
- add the first product-shaped home/navigation shell
- keep the current live status/debug screen as a diagnostic destination
- show real bring-up status through the existing architecture

Naming/orientation note:
- `Screen_status` is currently the first live status/debug screen, not finished product UI
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

The UI is therefore no longer “not working”.
A more accurate description is:

**The first target display and touch milestones are achieved, but the real UI is still in early bring-up.**
