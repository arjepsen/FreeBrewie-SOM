# FreeBrewie SOM Development Environment Consolidated

*Date: 2026-04-06*

## Purpose

This document defines the practical development and bring-up workflow for the FreeBrewie SOM side.

It is meant to keep the development environment, deployment path, runtime users, and manual test procedure clear and stable.

---

## Development host

Development is done on a Debian VM.

Typical workflow:

* edit and build on the VM
* copy the target binary to the SOM
* test manually on the SOM first
* only later move behavior under the appliance service

---

## Build system

The project uses CMake.

Typical target build flow in VS Code:

1. `CMake: Delete Cache and Reconfigure`
2. `CMake: Build`

The current target build directory is:

* `build-target`

The target binary path is:

* `build-target/Apps/BrewieApp/brewie_app`

---

## Target runtime model

The SOM side is intended to be an appliance-style Linux system.

That means:

* maintenance work is done through a maintenance account
* the application itself runs as a dedicated runtime user
* the runtime path and service path should stay stable

---

## Locked bring-up facts

These facts are now considered fixed unless deliberately changed later.

### Users

* Login / maintenance user: `admin`
* Runtime / appliance user: `brewie`

### `admin`

* interactive maintenance account
* has a normal shell
* used for SSH login, file copy, inspection, and maintenance work

### `brewie`

* runtime-only account
* shell: `/usr/sbin/nologin`
* uid/gid are system-style runtime values
* belongs to the runtime-access groups needed by the app:

  * `tty`
  * `dialout`
  * `video`
  * `input`

### Important rule

Do **not** assume:

* SSH login as `brewie`
* a `/home/brewie` directory

Those are not part of the current SOM setup.

---

## Filesystem layout

The intended appliance-side layout is:

* runtime install path: `/opt/brewie`
* runtime state area: `/var/lib/brewie`
* config area: `/etc/brewie`
* log area: `/var/log/brewie`

Important current fact:

* `brewie` does **not** use `/home/brewie`
* the runtime install path to care about is `/opt/brewie`

---

## Service model

The managed service is:

* `brewie.service`

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

* the app starts as `brewie`
* display init is currently bypassed intentionally
* `/dev/ttyS1` opens successfully
* heartbeat frames are sent repeatedly
* the MCU responds with `STATUS_REPORT`
* the SOM receives and prints parsed incoming frame information
* `FAULT_REPORT` can also be received

This means the current known-good baseline is:

* **new source structure builds**
* **manual runtime launch works**
* **serial link works**
* **heartbeat works**
* **MCU response reception works**

---

## Runtime verification checklist

When manually testing the app on the SOM, verify:

* app starts as `brewie`
* `/dev/ttyS1` opens
* heartbeat is sent
* MCU `STATUS_REPORT` frames are received
* no assumptions are made yet about the display path

At the current stage, the serial/comms baseline is the anchor.
Display/UI work should be added beside it, not by disturbing it.

---

## Current practical development order

The intended order is:

1. keep the working serial baseline intact
2. keep communication separate from UI work
3. bring up the smallest possible display path
4. use `Screen_boot` as the first visible startup/debug screen
5. only then grow into fuller UI behavior

---

## Notes for future threads

For this SOM setup, always remember:

* log in as `admin`
* copy files as `admin`
* install into `/opt/brewie`
* run manually as `brewie`
* do **not** assume `/home/brewie`
* do **not** assume SSH login as `brewie`
* `brewie.service` is the managed service path, but manual bring-up comes first
