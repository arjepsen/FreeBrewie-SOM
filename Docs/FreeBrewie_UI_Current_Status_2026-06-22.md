# FreeBrewie UI Current Status
_Date: 2026-06-22_
_Updated: 2026-06-22_

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
- a first visible text screen has now been shown on the real target display

So the project is no longer only at “headless serial baseline”.
We now have:
- working comms baseline
- working target display initialization
- first visible LVGL output on the real screen

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
- Linux DRM backend
- DRM device path: `/dev/dri/card0`

This replaced the earlier target-side display bypass.

The simulator path still uses SDL.

So the current split is:

- simulator build -> SDL
- target build -> DRM

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

- proper touch/input integration in `brewie_app`
- stable final status/home/fault screen design
- real screen flow between startup / status / home / fault screens
- binding real machine state to a fuller UI
- polished redraw/update behavior
- true animated boot/splash screen during SOM startup

Also important:
the current visible text render proves that display output works,
but it does **not** yet prove that the full UI layer is complete or stable.

---

## Current known-good bring-up baseline
The safest currently proven baseline is:

1. app starts on SOM as `brewie`
2. DRM display initializes successfully
3. `/dev/ttyS1` opens
4. heartbeat is sent
5. MCU reports are received
6. a visible LVGL live status/debug screen is shown
7. `brewie.service` starts `/opt/brewie/brewie_app` automatically

This is the current anchor state.

Any further UI work should preserve this baseline.

---

## Current practical interpretation
At this point, the SOM-side application has reached:

- first real target display output
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
2. keep the DRM display path unchanged
3. keep using the current status/debug screen as the first real screen
4. replace forced debug-only text gradually with proper status data
5. only after that, move toward `Screen_home` and fuller UI behavior

So the next goal is **not** “build the whole UI”.
The next goal is:
- make the current live status/debug screen clean and intentional
- show real bring-up status through the existing architecture

Naming/orientation note:
- `Screen_boot` is not really a final boot screen; it is currently the first live
  status/debug screen
- a true animated boot/splash screen should be a separate future startup phase
- the current target render is landscape, but the finished appliance UI should be portrait
  relative to the current view, rotated 90 degrees clockwise
- keep text lengths and layout density in mind when evolving this screen, because the final
  portrait layout will have less horizontal room

---

## Current caution
The current display/build path still includes bring-up-era technical debt.

In particular:
- DRM enablement introduced extra VM/sysroot dependency handling
- runtime `libdrm2` is required on the SOM
- current visible screen content is still a deliberate test-oriented render

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
- target DRM display init proven
- first visible LVGL target output proven

The UI is therefore no longer “not working”.
A more accurate description is:

**The first target display milestone is achieved, but the real UI is still in early bring-up.**
