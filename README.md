# FreeBrewie SOM stack

Fresh reorganized scaffold for the SOM-side software stack.

## Structure
- `Apps/` = actual applications
- `Shared/` = shared libraries/modules used by applications
- `Tools/` = standalone developer/service tools
- `external/` = third-party dependencies (LVGL submodule lives here)
- `cmake/` = toolchain files and shared CMake helpers

## Notes
This scaffold is intentionally reorganized for clarity:
- one repository
- one `src/` per application
- shared code separated from app code
- tools kept separate

## Important honesty note
This package was built from the readable public repo state and project docs.
A few files that were not directly readable from the repo view are included as
explicit placeholders or fresh scaffold files rather than copied originals.
In particular:
- `external/lvgl/` is represented only by a placeholder README in this zip
- `cmake/toolchain-armhf-bullseye.cmake` is a practical scaffold, not a verified copy

## Intended next step
Use this as the new clean layout baseline, then fold in the real LVGL submodule
and any verified local toolchain details from the VM.
