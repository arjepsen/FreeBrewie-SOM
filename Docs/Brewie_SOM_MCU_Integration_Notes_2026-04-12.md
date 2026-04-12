# Brewie SOM-MCU Integration Notes

## Purpose
This document defines the **practical integration approach** between the Brewie SOM and the ATmega2560 MCU.

This file is meant to sit beside:
- the MCU-side protocol document
- the SOM platform notes
- the SOM development-setup notes

It answers:
- what is the real SOM↔MCU link path?
- how should first bring-up be approached?
- what should be tested first?
- what is expected from each side?
- how should the SOM software be split internally?

---

## High-level ownership split
The integration path is based on a deliberate role split:

### SOM owns
- UI
- recipes
- manual-service workflows
- cleaning logic
- high-level timing
- when target behavior changes

### MCU owns
- hardware execution
- direct measurements
- interlocks
- faults
- startup gating after the user power-button request
- controlled shutdown behavior

So the SOM is the **high-level intent side**, while the MCU is the **executor/safety side**.

---

## Physical / OS-side endpoint
Known SOM-side serial endpoint:
- **`/dev/ttyS1`**

That is the intended Linux device for the Brewie SOM↔MCU serial protocol.

This should be treated as the real integration path.
A PC serial test tool is useful for smoke testing, but it is not the final intended architecture.

---

## Protocol direction summary
The shared framed protocol uses:

```text
SYNC1   0xAA
SYNC2   0x55
TYPE
SEQ
LEN
DATA
CRC8
```

CRC choice:
- **CRC-8 Dallas/Maxim**

Sequence-counter rule:
- each sender maintains its own rolling `SEQ`
- valid range `1..255`
- `0` reserved
- wrap from `255` to `1`

---

## Message ownership

### SOM → MCU
- `HEARTBEAT`
- `CONTROL_SNAPSHOT`
- `SHUTDOWN_REQUEST`
- `FAULT_CLEAR_REQUEST`

### MCU → SOM
- `STATUS_REPORT`
- `FAULT_REPORT`

### Either direction
- `ACK`
- `NACK`

---

## Expected startup interaction
The intended first real interaction is now:

1. mains power is applied
2. the dedicated MCU-side supply powers the MCU
3. MCU boots and enters `STANDBY`
4. MCU waits for the user to press the machine `POWER_BUTTON`
5. user presses `POWER_BUTTON`
6. MCU enters `BOOT`
7. MCU powers the rest of the required rails and powers the SOM
8. SOM boots Linux and starts the Brewie-side service or app
9. SOM opens `/dev/ttyS1`
10. SOM sends valid `HEARTBEAT`
11. MCU receives first valid `HEARTBEAT`
12. MCU transitions from `BOOT` to `ACTIVE`
13. MCU begins normal reporting
14. SOM may now send `CONTROL_SNAPSHOT`

This is the current first integration milestone.

---

## Important SOM-side structure rule
The SOM should be **one program with internal separation of concerns**.

That means the integration work should not drift into a structure where:
- UI owns serial transport
- raw protocol experiments are mixed into widget code
- `main.c` grows into the whole app

Instead, the intended split should be:

### `main.c`
Owns only:
- process entry
- minimal top-level bootstrap
- calling into the app layer

`main.c` should stay thin.

### `App.c`
Owns:
- application lifecycle
- orchestration of major subsystems
- mode selection and app-level coordination
- shared app state used across modules

`App.c` is the coordinator.
It is not the low-level serial backend and not the UI widget layer.

### Comms module
Owns:
- opening `/dev/ttyS1`
- protocol framing
- CRC
- `SEQ`
- heartbeat send/receive behavior
- `STATUS_REPORT` receive path
- later snapshot transmit/ack handling

### Display/backend module
Owns:
- target LVGL/display backend bring-up
- framebuffer/DRM/input/display glue
- the target-specific display path below the UI layer

### UI layer
Owns:
- screens
- widgets
- visual updates from app state

The UI layer should not own serial transport.
The comms layer should not own widget construction.

---

## First bring-up rule for current phase
At the current project stage, the right approach is:

### keep the known-good serial baseline intact
The current headless serial path is valuable because it already proves:
- target process runs
- `/dev/ttyS1` opens
- heartbeat transmit works
- `STATUS_REPORT` reception works

That baseline should not be casually destabilized while display bring-up is still uncertain.

### bring up display separately first
The next step should be a **display-only bring-up path** through the real application structure.

Meaning:
- do not make disposable test programs if avoidable
- do not integrate comms into the UI layer
- do not force the first unstable display experiments into the same runtime path as the known-good serial baseline

---

## What should be tested first
The project is now at the point where the right next step is narrow and controlled.

Recommended order:

### 1. Preserve the headless serial baseline
Confirm:
- target app still runs
- `/dev/ttyS1` still opens
- heartbeat still sends
- `STATUS_REPORT` frames are still received

### 2. Minimal target display bring-up
Confirm:
- target-side display backend can initialize through the real app structure
- LVGL can render the smallest possible visible output
- this works without involving the UI layer beyond the minimum needed

### 3. Stable display-only loop
Confirm:
- the minimal display output remains stable
- no obvious crash/hang/tearing issues appear immediately

### 4. Reintroduce the UI layer above the minimal display path
Confirm:
- `App.c` can initialize the display/backend module
- `App.c` can then initialize the UI layer
- widget/screen creation works without mixing in comms responsibilities

### 5. Recombine display path with the proven comms path
Confirm:
- the display path and the serial path can coexist
- UI still does not own protocol transport
- the app still preserves the intended separation of concerns

---

## Practical first-test philosophy
The first SOM↔MCU tests should remain **small and easy to localize**.

Do not begin with broad mixed behavior.
Begin with:
- one known-good serial baseline
- one minimal display-only baseline
- recombination only after both are independently believable

This makes it much easier to understand whether a failure belongs to:
- target display/backend bring-up
- UI layer behavior
- serial/protocol behavior
- cross-module interaction

---

## Immediate next step
The immediate next step should therefore be:

1. leave the current headless serial baseline intact
2. add a very small display-only bring-up path through the real app structure
3. keep `main.c` thin
4. let `App.c` own orchestration
5. keep comms outside the UI layer

That is the cleanest path forward from the current known-good state.
