# FreeBrewie SOM Development Environment

## Purpose
This document is the **canonical development-environment and workflow document** for the FreeBrewie SOM/UI side.

It is meant to replace overlapping older SOM setup/environment notes by collecting the still-relevant facts into one place.

This file should answer:

- where is SOM-side software developed?
- how is it built?
- how do simulator and target builds differ?
- what is the current repo/build workflow?
- what easy-to-forget details should be written down?

This is an environment/workflow document.
It is **not** the platform-truth document and **not** the current-software-status document.

---

## Scope and relation to other docs

This document should be used together with:

- **SOM platform notes**  
  Hardware, Linux image, display/touch bring-up, `/dev/ttyS1`, service/layout on target.

- **FreeBrewie UI current status**  
  What is working right now, what is still scaffolding, and what the next development step is.

- **SOM↔MCU integration/protocol docs**  
  Message model, startup interaction, heartbeat, snapshots, and link expectations.

So the split should now be:

- **Platform doc** = what the SOM target is
- **Environment doc** = how we develop/build/deploy for it
- **Current-status doc** = where the software stands right now

---

## High-level development model

The FreeBrewie SOM software is **not primarily developed directly on the SOM**.

The intended workflow is:

1. develop on the Debian VM
2. use VS Code + CMake there
3. build either:
   - simulator build for host testing
   - target build for the real SOM
4. copy the built target binary to the SOM
5. run and test it there
6. later install/run it through `brewie.service`

So:

- the **VM** is the real development/build machine
- the **SOM** is the runtime target

---

## Development host

Known working development host:

- **Debian GNU/Linux 11 (Bullseye) amd64 VM**

Why the VM matters:

- stable toolchain environment
- stable dependency set
- controlled sysroot/cross-build setup
- avoids host-machine drift

This VM is the primary development environment for FreeBrewie SOM work.

---

## Repository

Current working repo folder on the VM:

- `~/brewie-ui-rebuild`

Current git remote:

- `git@github.com:arjepsen/ReBrewie-LVGL.git`

Important naming note:

- repo and folder naming still reflect older project history
- the active project naming should now be treated as **FreeBrewie**
- repository renaming can be handled later
- avoid letting old repo naming redefine the current product/project identity

---

## Build system

Build system:

- **CMake**

Primary project files:

- `CMakeLists.txt`
- `CMakePresets.json`

Toolchain file:

- `cmake/toolchain-armhf-bullseye.cmake`

The project is built through CMake both for:

- host simulator
- cross-compiled SOM target

---

## CMake presets

Known important configure presets:

- `sim-debug`
- `target-debug`

Matching build presets:

- `build-sim-debug`
- `build-target-debug`

Meaning:

### `sim-debug`
Use for:

- LVGL simulator work on the VM
- fast UI iteration
- non-hardware-specific experimentation

### `target-debug`
Use for:

- generating the real ARM executable for the SOM
- validating target-side code paths
- preparing binaries to copy/run on the SOM

---

## Very important VS Code distinction

A key confusion rediscovered during recent work:

VS Code/CMake Tools has **two separate choices** that must not be mixed up.

### 1. Configure preset
Examples:

- `sim-debug`
- `target-debug`

This decides:

- host vs cross-compiling mode
- build directory
- toolchain-file usage

### 2. Build target
Examples:

- `brewie_ui`
- `mcu_probe`

This decides:

- which actual executable/target is built when Build is pressed

### Practical consequence

Selecting `target-debug` does **not** automatically mean the UI app is what gets built.

A real issue encountered was:

- VS Code was configured for `target-debug`
- but Build still targeted `mcu_probe`
- so it looked like target UI behavior/build behavior was more confusing than it really was

This distinction is important enough to remember explicitly:

**preset selection alone is not enough — the intended build target must also be selected**

For FreeBrewie UI work, the intended build target is normally:

- **`brewie_ui`**

---

## Editor / IDE

Known editor/tooling direction:

- **VS Code**
- **CMake Tools**
- **C/C++ extension**

Known useful behavior:

- Configure via Command Palette:
  - `Ctrl+Shift+P` → `CMake: Select Configure Preset`
  - `Ctrl+Shift+P` → `CMake: Configure`
- Build target can be switched via:
  - `Ctrl+Shift+P` → `CMake: Set Build Target`

Useful settings/history:

- `compile_commands.json` export enabled
- IntelliSense configured through CMake Tools configuration provider
- `cmake.configureOnOpen` intentionally not forced on every open

---

## VS Code crash note

A previously confirmed issue existed on the VM:

Symptom:

- VS Code crashed when using Git UI paths such as commit/sync

Known practical workaround:

- launch VS Code using:
  - `code --ozone-platform=x11`

Interpretation:

- likely Wayland/Ozone-related UI-path crash in the VM
- X11 launch mode avoided the crash

This matters only for the VM editor workflow, not for the target application.

---

## Toolchain and sysroot

Known target ABI direction:

- **armhf**
- Debian Bullseye compatible userspace
- Linux ARM hard-float target

Known cross compiler:

- **`arm-linux-gnueabihf-gcc`** 10.2.1
- corresponding C++ compiler also available

Known sysroot:

- **`/opt/sysroots/bullseye-armhf`**

This remains one of the most important environment truths because it affects:

- compiler selection
- linker behavior
- dependency compatibility
- generated binary compatibility with the SOM image

---

## Cross-compilation proof

The target build path is no longer theoretical.

It has been verified that the VM can produce a real ARM Linux executable for the SOM.

Observed valid target binary shape:

- `ELF 32-bit LSB pie executable, ARM, EABI5 ...`

This confirms:
- cross-build path works
- generated binary is suitable for the SOM platform

---

## LVGL

Known UI framework direction:

- **LVGL 9 series**

LVGL is included as a git submodule.

Current submodule path:

- `external/lvgl`

The important thing for this document is:
- LVGL is vendorized as a submodule
- it is part of the main repo
- both sim and target builds depend on it

---

## Simulator backend

Simulator dependency on the VM:

- **SDL2**

Installed package used during setup:

- `libsdl2-dev`

Important architectural reminder:

- **SDL is for the simulator path only**
- it is **not** the intended normal runtime display path on the SOM

So the mental model is:
- **VM simulator** = LVGL + SDL
- **real SOM target** = LVGL + real Linux display backend

This distinction should remain explicit to avoid later confusion.

---

## LVGL config files

A key build/config detail rediscovered during recent work:

The project originally had:
- `lv_conf.h`

That file had simulator-oriented settings, including:
- `#define LV_USE_SDL 1`

That caused the target build to try to compile SDL-related LVGL pieces, which is wrong for the real SOM target.

To separate simulator and target behavior, a second config file was introduced:
- `lv_conf_target.h`

Current intent:
- simulator build uses SDL-enabled LVGL config
- target build uses SDL-disabled LVGL config

Known important target-side setting:
- `#define LV_USE_SDL 0`

This is one of the important recent clarifications and should remain documented.

---

## Current source layout

Known important current project files include:
- `src/main.c`
- `src/main_sim.c`
- `UI/UI.c`
- `UI/UI.h`
- `Tools/McuProbe/CMakeLists.txt`
- `Tools/McuProbe/McuProbe.c`

Current interpretation:

### `src/main.c`
- real SOM target entry point

### `src/main_sim.c`
- simulator entry point

### `UI/UI.c`
- shared UI layer

This is the correct mental model.

The goal is **not** to maintain two separate codebases.
The goal is:
- platform-specific entry code stays separate
- shared UI/application code stays shared

---

## Why there are two mains

This is intentional.

The simulator and the real SOM target need different platform bring-up code.

That does **not** mean:
- duplicated application logic
- duplicated UI logic
- different product behavior

It means:
- one entry is optimized for desktop simulation
- one entry is optimized for the real Linux appliance target

That distinction should remain.

---

## Target-app design guardrails
The current target app is being built for a weak Linux SOM, not a roomy desktop machine.

So the target-side runtime should follow these guardrails from the beginning:

- prefer a single process for the first real target app
- separate responsibilities with modules, not immediate process-splitting
- keep steady-state runtime logic non-blocking
- avoid long sleeps in active logic
- avoid CPU-burning busy loops
- avoid unnecessary heap allocation after startup
- keep ownership simple and data structures compact
- avoid bringing simulator convenience assumptions into the target architecture

The purpose of these rules is:
- predictable runtime behavior
- modest CPU use
- easier debugging
- less later rework

---

## Non-blocking runtime direction
For the current target app, the intended shape is:
- timestamp-driven cadence
- non-blocking serial I/O handling
- quick work slices
- incremental UI updates
- lightweight wait strategy between work slices

So the target-side design goal is:
- responsive behavior without turning the app into a constant hot spin loop

This means “non-blocking” should be understood as:
- short bounded work
- explicit cadence/state handling
- modest waiting
- not full-CPU polling

---

## Build-and-run direction for the next milestone
The next real target-side milestone should be approached like this:

1. build `brewie_ui` for target
2. copy it to the SOM
3. run it manually first
4. verify target-side LVGL startup
5. verify `/dev/ttyS1` open
6. verify periodic heartbeat send
7. verify minimal on-screen debug/status output
8. only then replace the older placeholder service command with the real app

This keeps deployment conservative while bring-up is still early.

---

## Practical workflow recommendation
For the immediate phase, it is better to keep the work narrow:

- define the first target app clearly
- implement it as one program with internal modules
- prove it manually on the SOM
- then move it under `brewie.service`

That is better than immediately:
- broad repo cleanup
- process-splitting
- introducing IPC
- designing the full later appliance software stack in detail

---

## Short truthful recap
The development environment is now in a good place for the next real target step:

- VM-side build workflow is known
- simulator vs target distinction is clear
- target executable can be produced reliably
- the next step is controlled target-app bring-up, not more setup archaeology
