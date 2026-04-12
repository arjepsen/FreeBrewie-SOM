# SOM bring-up status

## Confirmed working

### Build and structure

- The new repository structure is usable.
- The target build completes.
- LVGL examples and demos are disabled for normal builds.
- The build focus is currently limited to `BrewieApp`.

### Runtime identity and permissions

- The `brewie` user is the correct runtime user.
- `admin` does not have the device access needed for normal appliance execution.
- The `brewie` user has the required groups for:
  - framebuffer access
  - serial access
  - input access

### Serial link

Headless bring-up confirmed:

- `brewie_app` opens `/dev/ttyS1`
- heartbeat frames are transmitted
- `STATUS_REPORT` frames are received back from the MCU
- the RX parser is decoding message type/sequence/length

Observed example behavior during test:

- heartbeat counter increments continuously
- MCU sends repeated `STATUS_REPORT` messages in response / alongside the live link

## Confirmed not yet stable

### Display/UI path

The original attempt to bring up display + UI + serial together was not stable.

Findings:

- framebuffer access works when running as `brewie`
- the application can open `/dev/fb0`
- the serial path itself is good
- instability came from the display/UI side, not the serial/protocol side
- UI-related calls in the headless path caused crashes or bad behavior

### Touch path

Not verified yet in the current application bring-up.

The code currently assumes `/dev/input/event0` as the touch device path, but this should be rechecked on the SOM before relying on it.

## Current known-good baseline

The current known-good baseline is:

- run `brewie_app` manually as `brewie`
- bypass display/UI
- verify serial open
- verify heartbeat TX
- verify `STATUS_REPORT` RX

This baseline must be preserved while the display path is reintroduced.

## Recommended next technical step

Reintroduce graphics in the smallest possible way:

1. create the LVGL display only
2. render a single static label
3. do not enable touch yet
4. do not use the larger UI helper layer yet
5. verify the app still keeps heartbeat and RX alive

Only after that should the real UI module be brought back in.
