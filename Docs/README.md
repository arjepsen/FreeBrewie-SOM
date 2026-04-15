# SOM docs bundle

This folder contains the current **SOM-side** document set for the FreeBrewie project.

Use these documents as follows:

- **`Brewie_SOM_Platform_Notes.md`**  
  Hardware/platform facts for the SOM target.

- **`FreeBrewie_SOM_Development_Environment_Consolidated_2026-04-06.md`**  
  Development host, toolchain, build environment, and workflow notes.

- **`Brewie_SOM_MCU_Integration_Notes_2026-04-12.md`**  
  Practical SOM↔MCU integration notes, serial link path, and current protocol integration direction.

- **`FreeBrewie_SOM_Architecture_Notes_2026-04-15.md`**  
  SOM-side software structure, top-level groups, and intended file responsibilities.

- **`FreeBrewie_UI_Current_Status_2026-04-15.md`**  
  Current SOM/UI bring-up status and immediate next milestone.

## Notes
- The SOM side is intended to be **one main application** with clear internal separation.
- The MCU remains responsible for hardware execution, measurements, interlocks, and faults.
- The SOM remains responsible for UI, business logic, data handling, and communication with the MCU.
- For the shared SOM↔MCU protocol truth, see the project protocol document in the main project docs.
