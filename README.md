# FreeBrewie SOM stack

SOM-side software for the FreeBrewie rebuild.

This repository is the Linux-side stack that runs on the Olimex A13 SOM. Its first job is to bring up a reliable MCU link, then gradually grow into the actual appliance-side application stack.

## Current status

Working now:
- Target build completes.
- LVGL examples and demos are disabled for normal project builds.
- `brewie_app` starts on the SOM when run as the `brewie` user.
- `brewie.service` starts `/opt/brewie/brewie_app` automatically on the SOM.
- The app can open `/dev/ttyS1`.
- The app sends heartbeat frames to the MCU.
- The MCU replies with `STATUS_REPORT` frames, and the SOM-side RX parser can decode them.
- The app decodes `STATUS_REPORT` and `FAULT_REPORT` into compact app-facing status.
- The target display initializes through LVGL on Linux DRM.
- The current screen shows live status/debug information on the real target display.
- The physical LCD mode is 480x272, but the appliance uses the panel in portrait. The
  simulator opens a portrait 272x480 window; target-side rotation is still blocked.

Still pending:
- touch/input integration in `BrewieApp`
- final portrait-oriented status/home/fault UI
- simulator/local UI build cleanup
- real manual-service, cleaning, and brewing workflows

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
- drive the first live status/debug screen
- provide the base application loop for later UI and control work

Other SOM applications can be added later, but for now the focus should stay on `BrewieApp` until the main runtime path is solid.

## Configuration

The build selects the LVGL configuration through CMake before LVGL itself is added.
This avoids one build mode silently overwriting the configuration used by the other mode.

That means:
- simulator-oriented LVGL config belongs in `lv_conf_sim.h`
- target-oriented LVGL config belongs in `lv_conf_target.h`
- `lv_conf.h` is only a safe local-development fallback for tools that expect that filename

If display or font options appear to "revert", check which config path CMake printed during
configure:

- simulator build should print `lv_conf_sim.h`
- target build should print `lv_conf_target.h`

Orientation is handled in the display platform layer:
- the simulator uses a 272x480 SDL window, matching the user-facing portrait layout
- the target currently opens the real 480x272 DRM mode without rotation
- the current SOM DRM driver does not expose a hardware plane rotation property
- LVGL target rotation on the current direct-buffer DRM backend needs more work before it
  can be used safely

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
7. Reintroduce DRM display and UI in small steps.
8. Install the tracked `brewie.service`.
9. Verify service autostart of `/opt/brewie/brewie_app`.

## Immediate next steps

1. Keep the current service/comms/display path as the known-good baseline.
2. Keep `Screen_status` focused as the live status/debug screen.
3. Keep any future animated boot/splash screen separate from the long-lived status screen.
4. Bring touch/input back into `BrewieApp`.
5. Grow toward the first manual-service UI while preserving MCU safety boundaries.

## Documentation policy

At this stage, keep documentation compact and practical.

The current useful SOM doc set is:

- `README.md`
- `Docs/README_2026-06-22.md`
- `Docs/Brewie_SOM_Platform_Notes_2026-06-22.md`
- `Docs/Brewie_SOM_Service_Autostart_2026-06-25.md`
- `Docs/FreeBrewie_SOM_Development_Environment_Consolidated_2026-06-22.md`
- `Docs/Brewie_SOM_MCU_Integration_Notes_2026-04-12.md`
- `Docs/FreeBrewie_SOM_Architecture_Notes_2026-06-22.md`
- `Docs/FreeBrewie_UI_Current_Status_2026-06-22.md`

Use them as follows:

- `Docs/README_2026-06-22.md`  
  Short index of the SOM-side document set.
- `Docs/Brewie_SOM_Platform_Notes_2026-06-22.md`  
  Hardware/platform facts for the SOM target.
- `Docs/Brewie_SOM_Service_Autostart_2026-06-25.md`
  Current systemd service install and autostart status.
- `Docs/FreeBrewie_SOM_Development_Environment_Consolidated_2026-06-22.md`  
  Development host, toolchain, build environment, and workflow notes.
- `Docs/Brewie_SOM_MCU_Integration_Notes_2026-04-12.md`  
  Practical SOM↔MCU integration notes and serial/protocol direction.
- `Docs/FreeBrewie_SOM_Architecture_Notes_2026-06-22.md`  
  SOM-side software structure, top-level groups, and intended file responsibilities.
- `Docs/FreeBrewie_UI_Current_Status_2026-06-22.md`  
  Current SOM/UI bring-up status and immediate next milestone.

For the shared SOM-MCU protocol truth, use
`FreeBrewie-MCU/Documentation/Brewie_SOM_MCU_Protocol_2026-04-01.md`.

Anything beyond this should be added only when it serves an active need.
