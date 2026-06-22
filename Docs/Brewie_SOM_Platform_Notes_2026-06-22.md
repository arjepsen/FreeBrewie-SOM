# Brewie SOM Platform Notes
_Date: 2026-06-22_

## Purpose
This document captures the current practical SOM platform facts for the FreeBrewie project.

It is meant to keep machine-specific setup facts stable across future work and future threads.

---

## Current SOM identity
Current target machine:
- A13 SOM platform (Olimex)
- appliance-style Linux system
- maintenance and bring-up work done from the `admin` account
- appliance/runtime app executed as `brewie`

---

## Runtime users

### `admin`
Purpose:
- interactive maintenance account
- SSH login
- file copy
- system inspection
- package installation
- service control
- manual bring-up support

Observed current setup:
- shell: `/bin/bash`
- home: `/home/admin`

### `brewie`
Purpose:
- runtime/appliance account for the Brewie app

Observed current setup:
- shell: `/usr/sbin/nologin`
- home/state area: `/var/lib/brewie`

Current groups:
- `brewie`
- `tty`
- `dialout`
- `video`
- `input`

Important rule:
- do not assume SSH login as `brewie`
- do not assume `/home/brewie` exists

---

## Current filesystem/runtime layout
Current relevant paths:
- app install path: `/opt/brewie`
- runtime state area: `/var/lib/brewie`
- service file: `/etc/systemd/system/brewie.service`

Current observed fact:
- `/opt/brewie` exists and is owned by `brewie:brewie`

---

## Current service model
Managed service:
- `brewie.service`

Observed previous state:
- the service existed and was enabled
- it was still launching an old placeholder script rather than the new target app during bring-up

Practical rule:
- early bring-up is still done manually first
- service integration comes after manual behavior is proven

---

## Current serial device fact
Current working MCU serial path:
- `/dev/ttyS1`

Current proven behavior:
- app opens `/dev/ttyS1`
- heartbeat frames are sent
- MCU responds with `STATUS_REPORT`
- `FAULT_REPORT` can also be received

This serial baseline has been re-proven after the current SOM code restructuring.

---

## Current display stack fact
Current target display path in BrewieApp:
- LVGL on Linux DRM
- DRM device path: `/dev/dri/card0`

This is now the chosen target display direction.
Framebuffer-first bring-up was considered, but DRM was selected because the SOM already exposes `/dev/dri/card0` and LVGL includes a Linux DRM backend.

Observed target device nodes:
- `/dev/fb0`
- `/dev/dri/card0`
- `/dev/dri/renderD128`

Current result:
- DRM display init succeeds on the SOM
- the app can initialize display and serial together
- a first visible boot-screen text has been shown on the real screen

This is the first successful visible LVGL-on-target milestone.

---

## Current display state
Current status is **not** “full UI working”.

What is currently true:
- target DRM display init works
- first visible text output works
- screen is no longer just black
- serial/comms continue working while display is initialized

What is **not** yet done:
- touch/input reintegration into BrewieApp (although early tests using lvgl did show touch was working)
- final boot-screen layout logic
- full home/fault screen flow on target
- production-polished display startup behavior

So the current display milestone is:
- **first visible LVGL proof on the real SOM target**

---

## Current runtime dependency on the SOM
Because BrewieApp now uses DRM on target, the SOM needs the DRM runtime library present.

Current required runtime package:
- `libdrm2`

Observed failure before installing it:
- `/opt/brewie/brewie_app: error while loading shared libraries: libdrm.so.2: cannot open shared object file`

Observed resolution:
- install `libdrm2`
- app then starts and initializes DRM correctly

This dependency should now be considered part of the SOM runtime platform requirements.

---

## Current manual deployment / run procedure

### Copy from VM to SOM
Typical copy target:
- `/home/admin/brewie_app`

Example:
```bash
scp build-target/Apps/BrewieApp/brewie_app admin@<som-ip>:/home/admin/brewie_app
```

### Install into runtime location
```bash
sudo install -o brewie -g brewie -m 0755 /home/admin/brewie_app /opt/brewie/brewie_app
```

### Manual run for bring-up
```bash
sudo -u brewie /opt/brewie/brewie_app
```

This is the correct manual bring-up command for the current SOM setup.

---

## Current proven bring-up state
The following has now been proven together on the SOM:

- app launches as `brewie`
- DRM display init succeeds
- `/dev/ttyS1` opens
- heartbeat frames are sent
- `STATUS_REPORT` frames are received
- first visible boot text can be shown on the screen

This is the current known-good combined baseline.

---

## Current bring-up warning
The current state still includes some bring-up-style technical debt.

Examples:
- display bring-up was forced through explicit DRM enablement in LVGL config
- VM/sysroot handling for DRM needed extra work
- first on-screen output is still a strong visual proof step, not final polished UI behavior

So:
- do not treat the current state as production-clean
- do treat it as the first real integrated SOM baseline

---

## Immediate practical interpretation
At this point the SOM side has reached:

- correct source structure under `Apps/BrewieApp/src/`
- working serial baseline
- working DRM init
- first visible screen output

So the next SOM-side work should build on that baseline rather than re-opening structure debates.

---

## Notes for future work
When continuing SOM work, remember:

- log in as `admin`
- install and inspect as `admin`
- run the app as `brewie`
- use `/opt/brewie/brewie_app` as the runtime path
- `/dev/ttyS1` is the current MCU serial path
- `/dev/dri/card0` is the current display backend target
- `libdrm2` is now part of the SOM runtime requirement
- touch is still pending
