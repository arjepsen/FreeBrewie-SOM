# Brewie SOM Platform Notes

## Purpose
This document captures the **platform truth** for the Brewie SOM side of the project.

It is the SOM-side equivalent of the MCU pin map / structure notes:
- what hardware the SOM is
- what software base it runs
- what important board-level enable steps were needed
- how it is expected to connect to the MCU

This file is meant to answer:
- what is the SOM platform?
- what Linux image are we targeting?
- what major low-level platform details are already known?

---

## Project context
The Brewie UI rebuild project is based on a separate Linux-capable SOM and is not running on the ATmega2560.

High-level split:
- **SOM** = UI, recipes, user logic, cleaning/manual-service flows, protocol master behavior
- **MCU** = hardware execution, measurements, interlocks, fault handling, controlled shutdown behavior

This matches the MCU-side documentation and protocol direction:
- the SOM sends `HEARTBEAT` and `CONTROL_SNAPSHOT`
- the MCU sends `STATUS_REPORT` and `FAULT_REPORT`

---

## SOM hardware
Known platform:
- **Allwinner A13 SOM**

This is the Linux host side of the Brewie rebuild project.

The intended serial link toward the MCU is:
- **`/dev/ttyS1`**

That is the expected SOM-side endpoint for the Brewie SOM↔MCU protocol path.

---

## Linux image
Known image used for the project:
- **`A13-SOM-bullseye-minimal-20241007-115325.img`**

This image was used as the target base system for the SOM work.

Target userspace direction:
- Debian Bullseye based
- lightweight appliance-style system
- intended to boot straight into the Brewie software stack

---

## Display / touch bring-up
The Brewie display and touch path required explicit enable/setup work.

### Display enable
A custom device-tree overlay was used:

- **`brewie-display-enable.dts`**
- compiled and installed as:
  - **`brewie-display-enable.dtbo`**

Installed overlay location:
- **`/usr/lib/olinuxino-overlays/sun5i-a13/`**

Referenced from:
- **`uEnv`**
- using:
  - `fdtoverlays=.../brewie-display-enable.dtbo`

Important remembered detail:
- display enable GPIO was configured as:
  - **`PB3` active high**
- represented in the overlay as:
  - `enable-gpios = <&pio 1 3 0>;`

### Touch
Touch bring-up logs indicated a Goodix touchscreen.

Observed Linux log detail:
- **`Goodix-TS 2-0014: ID 911, version: 1060`**

There were also fallback/firmware-related log references involving:
- **`goodix_911_cfg.bin`**

So the touch path was not just theoretical; it had already been brought up far enough to identify the controller in Linux.

---

## Brewie filesystem/service layout
A number of Brewie-specific directories were established on the SOM image:

- `/opt/brewie`
- `/etc/brewie`
- `/var/lib/brewie`
- `/var/log/brewie`

A service scaffold was also created:

- **`brewie.service`**

That means the intended SOM software shape is not “run manually forever from a shell,” but rather:
- appliance-style Linux system
- project files under `/opt/brewie`
- configuration under `/etc/brewie`
- state under `/var/lib/brewie`
- logs under `/var/log/brewie`
- managed startup via `systemd`

---

## SOM ↔ MCU link expectations
The SOM-side serial/protocol role is:

### SOM sends
- `HEARTBEAT`
- `CONTROL_SNAPSHOT`
- `SHUTDOWN_REQUEST`
- `FAULT_CLEAR_REQUEST`

### SOM receives
- `STATUS_REPORT`
- `FAULT_REPORT`
- `ACK`
- `NACK`

The MCU boots first, powers the SOM, and waits in `BOOT` until it receives the first valid `HEARTBEAT`.

That means the SOM software stack must reach the point where it can:
1. open `/dev/ttyS1`
2. speak the framed protocol correctly
3. begin sending `HEARTBEAT`
4. continue sending regular `HEARTBEAT` frames
5. later send `CONTROL_SNAPSHOT`

---

## Practical interpretation
At this stage, the SOM platform is already far enough along that it should be treated as a **real target environment**, not just a future idea.

Important implication:
- MCU↔SOM integration should increasingly be tested against the real SOM Linux environment
- PC-side serial smoke tools are still useful, but they are secondary to a real SOM-side test utility or service

---

## What this file does not try to define
This document does **not** fully define:
- the LVGL application structure
- the build workflow
- the development VM/toolchain
- the SOM-side protocol test workflow

Those belong in separate SOM documents.
