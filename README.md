# FreeBrewie SOM stack

SOM-side software for the FreeBrewie rebuild.

This repository is the Linux-side stack that runs on the Olimex A13 SOM. Its first job is to bring up a reliable MCU link, then gradually grow into the actual appliance-side application stack.

## Current status

Working now:
- Target build completes in VS Code.
- LVGL examples and demos are disabled for normal project builds.
- `brewie_app` starts on the SOM when run as the `brewie` user.
- The app can open `/dev/ttyS1`.
- The app sends heartbeat frames to the MCU.
- The MCU replies with `STATUS_REPORT` frames, and the SOM-side RX parser can decode them.
- The current display/UI path is not yet stable. A temporary headless bring-up path was used to verify the serial protocol path first.

## Repository structure

- `Apps/`  
  Actual applications that run on the SOM.

- `Shared/`  
  Shared modules used by one or more applications.

- `Tools/`  
  Standalone developer and service tools.

- `external/`  
  Third-party dependencies, including LVGL.

- `cmake/`  
  Toolchain and shared CMake support files.

## Applications

### `Apps/BrewieApp`

Main SOM application.

Current responsibility during bring-up:

- initialize the runtime
- open the MCU serial link
- send heartbeat periodically
- receive and decode MCU frames
- provide the base application loop for later UI and control work

Other SOM applications can be added later, but for now the focus should stay on `BrewieApp` until the main runtime path is solid.

## Configuration

The target build currently copies `lv_conf_target.h` over `lv_conf.h` during the target configure step.

That means:

- simulator-oriented LVGL config belongs in `lv_conf.h`
- target-oriented LVGL config belongs in `lv_conf_target.h`

If target display or font options appear to "revert", check `lv_conf_target.h` first.

## Build policy

Normal project builds should stay lean.

Current policy:

- do not build LVGL examples by default
- do not build LVGL demos by default
- do not build extra SOM apps by default during early bring-up
- focus on one known-good application path at a time

## Runtime users

- `admin` is the interactive maintenance account.
- `brewie` is the appliance/runtime account.

The `brewie` user is the correct runtime identity for the application because it has the required device access groups such as `video`, `dialout`, `input`, and `tty`.

## Bring-up sequence used so far

1. Build `brewie_app` on the VM.
2. Copy the binary to the SOM.
3. Run the binary manually as `brewie`.
4. Verify serial open on `/dev/ttyS1`.
5. Verify heartbeat TX.
6. Verify MCU `STATUS_REPORT` RX.
7. Keep the app headless until the serial/runtime side is stable.
8. Reintroduce display and UI in very small steps afterward.

## Immediate next steps

1. Keep the current working serial/heartbeat path as the known-good baseline.
2. Clean up the temporary debug prints and bypasses without losing the working behavior.
3. Log or decode a little more useful information from `STATUS_REPORT`.
4. Reintroduce display bring-up with a minimal LVGL test:
   - display only
   - one label
   - no touch first
   - no full UI layer first
5. Only after that, reconnect the proper UI module.

## Documentation policy

At this stage, keep documentation compact and practical.

Use the repo docs as follows:

- `README.md`  
  Repository overview, current status, and near-term direction.

- `Docs/README.md`  
  Index of the SOM-side document set.

- `Docs/FreeBrewie_SOM_Architecture_Notes_2026-04-15.md`  
  SOM-side software structure, top-level groups, and intended file responsibilities.

- `Docs/FreeBrewie_UI_Current_Status_2026-04-15.md`  
  Current SOM/UI bring-up status and immediate next milestone.

- `Docs/Brewie_SOM_MCU_Integration_Notes_2026-04-12.md`  
  Practical SOM↔MCU integration notes, serial link path, and current protocol integration direction.

Anything beyond that should be added only when it serves an active need.

## About

No description, website, or topics provided.
