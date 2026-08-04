# FreeBrewie Process Plan Design
_Date: 2026-08-04_

## Purpose
This document defines the direction for FreeBrewie's process plan model.

The process plan matters because it becomes the common executable shape beneath:
- the old-Brewie-style basic recipe editor
- a future advanced brewing editor
- a future expert/web-only editor with much more direct machine control

The goal is to avoid hard-coding the current basic recipe fields into the execution model.

---

## Core decision
A process plan is the SOM-side ordered instruction set that the runtime/session runner walks.

It is not an LVGL screen, not an import/export format, and not raw serial protocol bytes.
The SOM runtime converts the current process-plan state into machine targets and then into
compact MCU control snapshots.

The same process plan shape should be usable no matter how it was created:
- basic editor: simple fields generate a process plan
- advanced editor: richer brewing actions generate a process plan
- expert editor: low-level expert actions map very closely to a process plan

---

## Basic, advanced, and expert editing

### Basic editor
The basic editor imitates the original Brewie recipe flow:
- mash water
- mash temperature and time
- sparge water/time
- boil time
- cooling target
- fermentation notes

The user sees simple brewing fields. The SOM fills in default hidden behavior and generates
process-plan steps.

Example:

```text
Basic input:
    mash water = 12.0 L
    mash rest = 66 C for 60 min

Generated process plan:
    set mash fill/temperature targets
    wait until fill/temperature conditions are satisfied
    hold mash temperature for 60 min
```

### Advanced editor
The advanced editor should still be brewing-oriented, but more flexible:
- multiple mash/boil side actions
- transfers between sides
- heater duty limits
- pump behavior
- overlapping operations
- user prompts and ingredient additions

It should still produce the same process-plan format.

### Expert editor
The expert editor is intended for the future web UI, not the small embedded screen.

It should allow much more direct machine intent:
- set valves
- set pumps
- set heater targets or duty limits
- wait for time/temperature/volume/user input
- combine actions that run at the same time

Expert mode should restrict brewing creativity as little as practical, but it must not be
able to bypass hard safety limits. Fault handling, watchdog handling, dry-heater prevention,
electrical limits, and hard interlocks live below recipe semantics.

---

## Process steps are state segments
Process-plan steps should be treated primarily as **state segments**, not only as one-off
commands.

A step may:
- apply one or more target changes
- keep all unchanged targets from the previous step
- stay active until an exit condition is met

This is important because a `WAIT` step does not mean "turn everything off." It means:
"keep the currently requested state steady until the wait condition is complete."

Example:

```text
Step 1:
    set mash heater target = 66 C
    set boil heater duty limit = 35%
    set pump = on
    exit when mash temperature reaches 66 C

Step 2:
    no target changes
    exit after 10 min

Step 2 keeps the heater, duty, and pump requests from Step 1 unless it explicitly changes
them.
```

---

## First process-plan instruction categories
The first C scaffold should be small but should point in the right direction.

Suggested concepts:

- target segment
  - apply target changes, then hold until an exit condition
- prompt
  - show a user prompt or ingredient-addition instruction
- complete
  - marks normal end of the process plan

Exit conditions:
- immediate
- duration elapsed
- temperature reached
- volume reached
- user confirmed

Target changes:
- vessel fill/volume target
- vessel temperature target
- heater duty limit
- pump state/duty
- valve state
- cooling target/state

The early implementation does not need every target field yet. The important rule is that
the structure should grow by adding target fields and exit conditions, not by adding only
basic-recipe-specific step names.

---

## What belongs below the process plan
Some behavior must not be stored as permanent recipe/process data unless a very explicit
expert/service mode later requires it.

Examples:
- how to pulse an inlet valve to avoid overshoot
- how long to wait for a noisy sensor to settle
- how to recover from a small fill error
- how to derate a heater because of current machine state
- how to stop safely after a fault

Those decisions depend on calibration, hardware condition, sensor readings, firmware
version, and safety policy. They belong in the runtime controller, machine-target layer, or
MCU firmware.

The process plan may state:

```text
fill mash side to 12.0 L with tolerance/default correction allowed
```

The runtime decides the exact valve pulses needed today.

---

## SOM-MCU relationship
The process plan is walked by the SOM.

The MCU should continue to receive compact current target snapshots, not the full recipe or
full process plan, unless the protocol is deliberately redesigned later.

This keeps:
- recipe data on the SOM
- execution timing and workflow on the SOM
- hard hardware safety close to the MCU
- serial traffic small

---

## Current code direction
`Logic/Process_plan` should become the canonical process instruction scaffold.

The current old-Brewie-like recipe fields should be converted into generic process-plan
segments. The current converter may still live in `Process_plan.c` for a short time, but as
the logic grows it should likely move into a dedicated builder module, for example:

```text
Recipe_model -> Process_plan_builder -> Process_plan
```

Future expert editing should produce the same `process_plan_t`, either directly or through a
very thin validation/conversion layer.

`Logic/Process_runner` is the first runtime holder for a prepared process plan. It carries
the current step index and the current `Machine_targets` image. It is intentionally passive
today: it can start a plan and apply target segments, but it does not yet advance from real
time/sensor conditions or send MCU snapshots.

The first app-level wiring now runs through `Logic/App_orchestrator`:

- entering the selected recipe checklist prepares a catalog recipe into `Recipe_model` and
  `Process_plan`
- the orchestrator marks that workflow state as preflight
- pressing START starts the passive `Process_runner`
- the orchestrator builds a 16-byte `CONTROL_SNAPSHOT` preview from the current
  `Machine_targets`
- top-level `App` sends that first snapshot once through `Comms`
- `Brewing_process_view_model` turns the orchestrator/runner state into Active Brewing
  screen text

This is still early bring-up, not a full hardware-control loop. The Status screen shows a
short read-only `ctrl` diagnostic row for the snapshot bytes and an `ack` row for the latest
MCU ACK/NACK summary. Repeated snapshot updates, ACK/NACK timeout/retry policy, and stricter
safety gates are still future work.

`Logic/Machine_targets` is the current bridge from process-plan target changes to the MCU's
16-byte `CONTROL_SNAPSHOT` payload. Some planned target fields, such as cooling target and
heater duty limit, are held SOM-side only until the shared SOM-MCU protocol grows matching
fields.

The shared protocol can now represent valve targets without ambiguity:

```text
0 = no requested valve target
1 = open
2 = close
3 = close hard
4 = sparge open
5 = sparge close
```

The SOM target layer still does not map process-plan valve masks into snapshot bytes yet.
That is now a process ownership decision, not a wire-format limitation: we still need to
define exactly which process-plan fields are allowed to command valves before expert valve
control is enabled.
