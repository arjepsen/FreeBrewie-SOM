# Brewie SOM-MCU Integration Notes
_Date: 2026-08-01_

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

Known SOM-side MCU reset endpoint:
- old Brewie Linux alias: `/dev/brewie-mcu-reset`
- old alias target: `/sys/class/gpio/gpio6_pe9`
- current Olimex Bullseye sysfs GPIO after export: `/sys/class/gpio/gpio137`
- A13 pin name: `PE9`

The old alias is not present on the current Olimex image. The current tested path is
`gpio137`.

Observed 2026-08-01 reset behavior:
- setting `gpio137` high briefly and then low resets the ATmega2560
- after reset the MCU returns to `STANDBY`
- the power-button LED turns off
- current MCU firmware does not emit normal periodic `STATUS_REPORT` frames in `STANDBY`
- pressing the physical Brewie power button moves startup forward and `STATUS_REPORT`
  reception resumes on the SOM

This means `last rx = none` after an MCU reset is not automatically a serial failure. It can
also mean the MCU is alive but waiting in `STANDBY`.

---

## Manual MCU flashing path
The original Brewie software flashed the ATmega2560 from the SOM using `avrdude`, the MCU
UART on `/dev/ttyS1`, and the SOM-controlled MCU reset line. The current project keeps this
as an explicit admin operation first, not an in-app feature.

This path requires the Brewie-compatible ATmega2560/STK500v2 bootloader. If the MCU
was last flashed through USBasp/ISP and the bootloader was erased or bypassed, UART
flashing from the SOM will fail because there is no bootloader listening on `/dev/ttyS1`
after reset.

MCU-side recovery testing on 2026-08-02 showed the working state is the old
Brewie-carried STK500v2 bootloader image, `hfuse=0xD8`, and `lock=0x3F`. The stock
final lock byte `0x0F` made the FreeBrewie app fail to start on this board.

Tracked helper:

```text
Deploy/Admin/flash_mcu_from_som.sh
```

Install `avrdude` on the SOM before first use:

```bash
sudo apt update
sudo apt install avrdude
```

Copy the helper and the MCU firmware hex to the SOM:

```bash
scp Deploy/Admin/flash_mcu_from_som.sh admin@192.168.1.130:/home/admin/flash_mcu_from_som.sh
scp ../FreeBrewie-MCU/.pio/build/mega2560_bare/firmware.hex admin@192.168.1.130:/home/admin/freebrewie_mcu.hex
```

Run the flash from the SOM:

```bash
chmod +x /home/admin/flash_mcu_from_som.sh
sudo /home/admin/flash_mcu_from_som.sh /home/admin/freebrewie_mcu.hex
```

The helper does the following:
- requires root
- checks that the hex file, `avrdude`, and `/dev/ttyS1` exist
- exports and prepares `gpio137`
- stops `brewie.service` if it is running, so BrewieApp releases `/dev/ttyS1`
- pulses MCU reset high then low
- runs:

```bash
avrdude -v -p atmega2560 -c wiring -P /dev/ttyS1 -b 115200 -D -U flash:w:<hex-file>:i
```

- leaves `gpio137` low, the observed released state
- restarts `brewie.service` if it was running before the flash attempt

If `avrdude` reports `stk500v2_getsync(): timeout communicating with programmer`, treat that
as a real error. The most likely cause is a missing ATmega2560 bootloader after USBasp/ISP
flashing. Restore the Brewie-compatible bootloader with USBasp, then return to SOM-side flashing as
the normal workflow.

After a successful flash, remember that the MCU may be in `STANDBY`. If the SOM status
screen shows `last rx = none`, press the physical Brewie power button and confirm that
`STATUS_REPORT` reception resumes.

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

## Current bring-up rule
At the current project stage, the right approach is:

### keep the known-good service/comms/display baseline intact
The current integrated path is valuable because it proves:
- `brewie.service` starts `/opt/brewie/brewie_app`
- target process runs as `brewie`
- `/dev/ttyS1` opens
- heartbeat transmit works
- `STATUS_REPORT` reception works
- `FAULT_REPORT` reception works
- target DRM display init works
- live status/debug information appears on the target screen

That baseline should not be casually destabilized while touch, fuller UI, and actuator
control paths are still immature.

### keep subsystems separated as the UI grows
Meaning:
- do not integrate comms into the UI layer
- do not put widget code into the raw protocol path
- do not treat the current status/debug screen as the final boot animation
- keep future boot/splash animation separate from the long-lived status/home/fault flow

---

## What should be tested first
The project is now past the first serial/display/service proof. The next steps should still
be narrow and controlled.

Recommended order:

### 1. Preserve the integrated baseline
Confirm:
- service-started target app still runs
- `/dev/ttyS1` still opens
- heartbeat still sends
- `STATUS_REPORT` frames are still received
- target display still updates

### 2. Keep the current status screen focused
Confirm:
- the current `Screen_status` role is treated as live status/debug
- a true future boot/splash screen remains a separate startup phase
- the final portrait orientation is kept in mind

### 3. Bring touch/input back carefully
Confirm:
- input events reach the app without disturbing comms
- input handling stays in `Platform/` or another dedicated input boundary
- screen code receives user intent, not raw device details

### 4. Build the first manual-service path
Confirm:
- UI can request one controlled target change
- SOM sends a `CONTROL_SNAPSHOT`
- MCU ACK/NACK behavior is visible
- status feedback returns to the UI

### 5. Expand only one actuator family at a time
Confirm:
- MCU safety clamps remain final
- actuator behavior is observed through status/fault feedback
- broad recipe/cleaning flows wait until primitives are believable

---

## Practical first-test philosophy
The first SOM↔MCU tests should remain **small and easy to localize**.

Do not begin with broad mixed behavior.
Begin with:
- one known-good service/comms/display baseline
- one small UI or control change at a time
- target verification after every hardware-facing change

This makes it much easier to understand whether a failure belongs to:
- target display/backend bring-up
- UI layer behavior
- serial/protocol behavior
- cross-module interaction

---

## Immediate next step
The immediate next step should therefore be:

1. leave the current service/comms/display baseline intact
2. keep the current live status/debug screen focused
3. keep `main.c` thin
4. let `App.c` own orchestration
5. keep comms outside the UI layer
6. keep touch/input in the platform layer
7. build the first home/navigation shell on top of the proven touch/display path

That is the cleanest path forward from the current known-good state.
