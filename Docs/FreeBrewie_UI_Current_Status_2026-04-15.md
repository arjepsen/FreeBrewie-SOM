# FreeBrewie UI Current Status
_Date: 2026-04-15_

## Purpose
This note tracks the **current SOM/UI bring-up status**.

It is a status note only.

For software structure and file responsibilities, use:
- `FreeBrewie_SOM_Architecture_Notes_2026-04-15.md`

For MCU/SOM protocol truth, use:
- `Brewie_SOM_MCU_Protocol_2026-04-01.md`

---

## Current known-good state
The following is currently working on the SOM side:

- the target application builds
- the application runs on the SOM as user `brewie`
- `/dev/ttyS1` opens successfully
- heartbeat transmit works
- `STATUS_REPORT` frames are received from the MCU

This gives a real **headless serial baseline**.

---

## Current unstable area
The display/UI path is still not stable enough yet.

So the current situation is:

- **comms baseline:** working
- **display/UI path:** not yet proven stable

---

## Current direction
The current development direction is:

1. keep the working serial baseline intact
2. keep communication separated from UI code
3. bring up the smallest possible LVGL display output first
4. use `Screen_boot` as the first visible startup/debug screen
5. only then grow toward fuller UI behavior

---

## Immediate next milestone
The next concrete milestone is:

- get a minimal real display output on the SOM
- keep comms independent while doing so
- prove the first visible LVGL path before reintroducing higher UI layers

In practice, this means:
- comms remains usable even if display work fails
- the first screen should be a very small boot/debug screen

---

## Why this matters
The project now has a known-good communication baseline.

That means future work should avoid mixing:
- protocol/comms behavior
with
- display bring-up experiments

The display path should be added beside the comms baseline, not by disturbing it.

---

## Current summary
Short version:

- SOM app runs
- MCU link works
- heartbeat works
- status reception works
- display path still needs first stable proof
