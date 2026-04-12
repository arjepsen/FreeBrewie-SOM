# SOM architecture

## Purpose

This document describes the current intended structure of the SOM-side software.

The SOM is the higher-level Linux side of the machine. It is responsible for application logic, UI, and communication with the MCU. The MCU remains the fast hardware executor.

## Design principles

- one repository for the SOM stack
- one main application under active bring-up
- shared code separated from app code
- non-blocking runtime paths from the start
- keep the weak SOM in mind: avoid unnecessary work, unnecessary processes, and unnecessary build targets

## Current structure

### `Apps/BrewieApp`

The main appliance application.

This should eventually own:

- startup sequencing
- link supervision to the MCU
- status presentation
- user interaction
- higher-level machine logic

Right now it is still in bring-up mode.

### `Shared/Platform`

Low-level Linux/platform helpers shared by applications.

Current examples:

- serial port handling
- monotonic time access

This is the right place for wrappers around Linux system interfaces that multiple apps may need.

### `Shared/Protocol`

MCU link protocol support.

Current examples:

- sender state
- RX state machine
- heartbeat frame construction
- frame parsing

This should stay independent from UI code.

### `Tools/`

Standalone utilities.

Useful for diagnostics and development, but not part of the core application runtime path.

## Runtime layering

Preferred layering:

1. platform
2. protocol
3. application runtime
4. UI

The important rule is:

- protocol must not depend on UI
- platform must not depend on application logic
- the application can depend on all lower layers

## Bring-up lesson from current work

The serial/heartbeat path was verified successfully only after the display/UI path was temporarily bypassed.

That means the runtime should continue to be developed in this order:

1. serial link
2. protocol correctness
3. status decoding
4. minimal display output
5. proper UI layer
6. touch/input
7. service/autostart integration

## Naming note

Avoid using names like `pump_*` for generic data-flow helpers in the SOM codebase, because `pump` already has a real Brewie hardware meaning.

For example, serial RX helpers should use names like:

- `app_process_serial_rx`
- `app_handle_serial_rx`

not names that can be mistaken for pump control code.

## Current practical conclusion

For now, one application is enough.

Do not split heartbeat and debug/status display into separate processes yet. Keep them inside `BrewieApp`, but keep them as separate internal modules/functions.

That gives:

- one runtime process
- one clear control path
- less overhead
- easier bring-up
- cleaner later refactoring if a split ever becomes justified
