# FreeBrewie SOM Development Environment Consolidated
_Date: 2026-06-22_

## Purpose
This document defines the practical development and bring-up workflow for the FreeBrewie SOM side.

It is meant to keep the development environment, deployment path, runtime users, target build requirements, and manual test procedure clear and stable.

---

## Development host
Development is done on a Debian VM.

Typical workflow:
- edit and build on the VM
- copy the target binary to the SOM
- test manually on the SOM first
- only later move behavior under the appliance service

The VM is a host build machine, not the SOM itself.

---

## Build system
The project uses CMake.

Typical target build flow in VS Code:
1. `CMake: Delete Cache and Reconfigure`
2. `CMake: Build`

The current target build directory is:

- `build-target`

The target binary path is:

- `build-target/Apps/BrewieApp/brewie_app`

---

## Target ABI and toolchain
The current target build uses the ARM hard-float target toolchain:

- `cmake/toolchain-armhf-bullseye.cmake`

This means:
- VM host architecture is `amd64`
- target ABI is `armhf`
- target-side development libraries must match `armhf`

This matters for packages like DRM libraries.

---

## Target runtime model
The SOM side is intended to be an appliance-style Linux system.

That means:
- maintenance work is done through a maintenance account
- the application itself runs as a dedicated runtime user
- the runtime path and service path should stay stable

---

## Locked bring-up facts
These facts are now considered fixed unless deliberately changed later.

### Users
- Login / maintenance user: `admin`
- Runtime / appliance user: `brewie`

### `admin`
- interactive maintenance account
- has a normal shell
- used for SSH login, file copy, inspection, and maintenance work

### `brewie`
- runtime-only account
- shell: `/usr/sbin/nologin`
- home/state area: `/var/lib/brewie`
- belongs to the runtime-access groups needed by the app:
  - `tty`
  - `dialout`
  - `video`
  - `input`

### Important rule
Do **not** assume:
- SSH login as `brewie`
- a `/home/brewie` directory

Those are not part of the current SOM setup.

---

## Filesystem layout
The intended appliance-side layout is:

- runtime install path: `/opt/brewie`
- runtime state area: `/var/lib/brewie`
- config area: `/etc/brewie`
- log area: `/var/log/brewie`

Important current fact:
- `brewie` does **not** use `/home/brewie`
- the runtime install path to care about is `/opt/brewie`

---

## Service model
The managed service is:

- `brewie.service`

This is the appliance service that should eventually launch the application automatically.

During early bring-up, manual testing should happen first.
Only after manual runtime behavior is proven should the service path be updated or relied upon.

---

## Manual deployment flow
The correct deployment pattern is:

1. build on the Debian VM
2. copy the binary to the SOM as `admin`
3. install the binary into `/opt/brewie`
4. run the binary manually as `brewie`
5. verify runtime behavior
6. only afterward move back toward the managed service flow

### Copy from VM to SOM
Example:

```bash
scp build-target/Apps/BrewieApp/brewie_app admin@<som-ip>:/home/admin/brewie_app
```

### SSH to the SOM
```bash
ssh admin@<som-ip>
```

### Install the binary
```bash
sudo install -o brewie -g brewie -m 0755 /home/admin/brewie_app /opt/brewie/brewie_app
```

### Manual runtime test
```bash
sudo -u brewie /opt/brewie/brewie_app
```

This is the correct manual bring-up command for the current SOM setup.

---

## Current known-good manual runtime result
The current known-good SOM-side baseline is:

- the app starts as `brewie`
- DRM display init succeeds
- `/dev/ttyS1` opens successfully
- heartbeat frames are sent repeatedly
- the MCU responds with `STATUS_REPORT`
- the SOM receives and prints parsed incoming frame information
- `FAULT_REPORT` can also be received
- first visible boot-screen text can be shown on the SOM display

This means the current known-good baseline is:

- **new source structure builds**
- **manual runtime launch works**
- **serial link works**
- **heartbeat works**
- **MCU response reception works**
- **target LVGL/DRM display init works**
- **first visible text render works**

Touch/input is **not** yet reintegrated into BrewieApp.

---

## Runtime verification checklist
When manually testing the app on the SOM, verify:

- app starts as `brewie`
- target display init succeeds
- `/dev/ttyS1` opens
- heartbeat is sent
- MCU `STATUS_REPORT` frames are received
- first visible boot text appears on screen

At the current stage, the serial/comms baseline remains the anchor.
Display/UI work should continue beside it, not by disturbing it.

---

## Current practical development order
The intended order is:

1. keep the working serial baseline intact
2. keep communication separate from UI work
3. bring up the smallest possible display path
4. use `Screen_boot` as the first visible startup/debug screen
5. only then grow into fuller UI behavior

---

## DRM / target display build requirements on the VM
The current target display path uses LVGL Linux DRM on ARM.

This introduced extra build requirements on the Debian VM.

### Foreign architecture
The VM needed `armhf` foreign architecture enabled:

```bash
sudo dpkg --add-architecture armhf
sudo apt update
```

### Target development package
The VM needed the ARM target DRM development package:

```bash
sudo apt install libdrm-dev:armhf
```

### Important note
A plain host-side `libdrm-dev` package is not enough for target linking.
The build is for `armhf`, not for the VM host architecture.

---

## Sysroot reality
The cross build uses a Bullseye armhf sysroot.

This matters because:
- headers and libraries visible on the host VM are not automatically sufficient
- target-side libraries may also need to exist inside the sysroot used by the toolchain

In practice, the DRM bring-up exposed this explicitly.

---

## Current DRM bring-up technical debt / workaround notes
These are not ideal final solutions, but they are part of the current working bring-up knowledge and should not be forgotten.

### LVGL target config
The target LVGL config currently enables:
- `LV_USE_LINUX_DRM 1`
- `LV_USE_LINUX_FBDEV 1`
- `LV_USE_EVDEV 1`

The current active target display path is **DRM**, not fbdev.

### Header visibility workaround
During target DRM bring-up, the build needed these symlinks so the active include path could see the top-level DRM headers:

```bash
sudo ln -sf /usr/include/xf86drm.h /usr/include/libdrm/xf86drm.h
sudo ln -sf /usr/include/xf86drmMode.h /usr/include/libdrm/xf86drmMode.h
```

### Sysroot library workaround
Because the linker uses the Bullseye armhf sysroot, DRM libraries had to exist there as well.

The working workaround was:

```bash
sudo mkdir -p /opt/sysroots/bullseye-armhf/usr/lib/arm-linux-gnueabihf
sudo cp -av /usr/lib/arm-linux-gnueabihf/libdrm.so* /opt/sysroots/bullseye-armhf/usr/lib/arm-linux-gnueabihf/
```

### Important note
This is current working knowledge, not necessarily the final elegant solution.
But it is part of the real bring-up state and must be remembered if the environment is rebuilt.

---

## SOM runtime package requirement for DRM
After enabling DRM in the app, the SOM itself needed the DRM runtime package:

```bash
sudo apt update
sudo apt install libdrm2
```

Without this, the app failed at runtime with missing `libdrm.so.2`.

---

## Upgrade caution on the VM
For the current project phase:

- `apt update` is fine when needed to refresh package metadata
- broad `apt upgrade` is not recommended just to solve a single dependency issue

Reason:
- the VM acts as a controlled build environment
- broad upgrades can introduce unnecessary churn
- targeted package installs are preferred during bring-up

---

## Current UI/display bring-up state
The target display backend now initializes through DRM on the SOM and can coexist with the working serial link.

Current state:
- target DRM display init works
- first visible text render has been achieved
- this is still a minimal bring-up path
- touch/input integration is still pending
- full UI behavior is not yet considered stable

So the current stage is:
- **display bring-up has started**
- **full UI integration is not finished**

---

## Notes for future threads
For this SOM setup, always remember:

- log in as `admin`
- copy files as `admin`
- install into `/opt/brewie`
- run manually as `brewie`
- do **not** assume `/home/brewie`
- do **not** assume SSH login as `brewie`
- `brewie.service` is the managed service path, but manual bring-up comes first
- target builds are `armhf`
- DRM support added real VM/sysroot/runtime dependencies
- current target display backend is DRM
- current visible display proof exists, but touch is still pending
