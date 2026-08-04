# Brewie SOM Platform Notes
_Date: 2026-08-04_

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

Observed 2026-06-25 state:
- `brewie.service` is active and enabled
- `ExecStart` still points at `/opt/brewie/hello.sh`
- `/opt/brewie/brewie_app` is installed and owned by `brewie:brewie`
- `brewie` is in the required `dialout` and `video` groups

Observed 2026-06-26 state:
- the tracked unit in `Deploy/Systemd/brewie.service` has been installed on the SOM
- `brewie.service` starts `/opt/brewie/brewie_app`
- `Main PID` is `brewie_app`, not `hello.sh`
- the target screen comes on through the service-started app
- journal output shows `HEARTBEAT` transmit and `STATUS_REPORT` receive under systemd

Practical rule:
- manual runs remain useful as the first test for a newly copied binary
- the normal appliance startup path is now the managed `brewie.service`

Current service integration note:
- service autostart is proven with the tracked unit in `Deploy/Systemd/brewie.service`

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

## Current Brewie-specific GPIO facts
The current Olimex Bullseye image does not create the original Brewie `/dev/brewie-*`
aliases by itself. The old Brewie Linux installation used these aliases:

| Old alias | Old sysfs target | Current interpretation |
|---|---|---|
| `/dev/brewie-button2` | `/sys/class/gpio/gpio2_pb15` | old drain/second button input |
| `/dev/brewie-buzzer` | `/sys/class/gpio/gpio5_pb2` | old buzzer output |
| `/dev/brewie-hold-power` | `/sys/class/gpio/gpio3_pb4` | old hold-power output |
| `/dev/brewie-lcd-backlight` | `/sys/class/gpio/gpio1_pb3` | LCD backlight, A13 `PB3`, Linux `gpio35` |
| `/dev/brewie-lcd-pwre` | `/sys/class/gpio/gpio7_pb10` | LCD power-enable, A13 `PB10`, Linux `gpio42` |
| `/dev/brewie-led2` | `/sys/class/gpio/gpio4_pb16` | old second LED output |
| `/dev/brewie-mcu-reset` | `/sys/class/gpio/gpio6_pe9` | MCU reset, A13 `PE9`, Linux `gpio137` |
| `/dev/brewie-power` | `/sys/class/gpio/gpio8_pc7` | old power-related GPIO |

Current tested MCU reset line:
- A13 pin: `PE9`
- Linux GPIO number: `137`
- current sysfs path after export: `/sys/class/gpio/gpio137`
- old Brewie alias target: `/sys/class/gpio/gpio6_pe9`

The current kernel pinctrl dump showed `PE9` as unclaimed before export, and exporting
`gpio137` worked:

```text
gpio-137 (                    |sysfs               ) out lo
```

Observed reset behavior on 2026-08-01:
- driving `gpio137` high briefly and then low resets the MCU
- after reset, the MCU returns to `STANDBY`
- the power-button LED turns off
- the SOM Status screen can show `last rx = none` because current MCU firmware only sends
  periodic `STATUS_REPORT` frames after the startup path reaches `ACTIVE`
- pressing the physical Brewie power button moves the MCU through startup again, after
  which `last rx` resumes and sequence numbers increase

Practical rule:
- do not use `/dev/brewie-mcu-reset` on the current image unless we deliberately recreate
  that compatibility alias
- for current manual testing, use `/sys/class/gpio/gpio137` after exporting it
- any future MCU flashing helper must stop `brewie.service`, control `gpio137`, run
  `avrdude` on `/dev/ttyS1`, and then restore the app/service path cleanly

### Buzzer/audio status
The carrier-board buzzer/speaker was originally suspected to be a GPIO-driven buzzer
because the old Linux image exposed this alias:

```text
/dev/brewie-buzzer -> /sys/class/gpio/gpio5_pb2
```

Using the same A13 GPIO numbering rule proven for other pins, `PB2` appears to map to Linux
`gpio34`:

```text
('B' - 'A') * 32 + 2 = 34
```

Current GPIO status:
- `gpio34` / `PB2` is **not** available as a normal buzzer GPIO on the current image
- kernel pinctrl shows `PB2` muxed to `1c20e00.pwm`
- `/sys/kernel/debug/pwm` shows `pwm-0 (backlight)` with a 50 us period and inverse
  polarity
- testing `gpio34` produced no buzzer sound
- testing inverted polarity on `gpio34` affected the display, matching the backlight PWM
  ownership
- the old GUI code did not directly toggle the buzzer alias for button sounds; it called
  `aplay /usr/share/beep.wav`
- helper added: `Deploy/Admin/probe_buzzer_gpio.sh`
- not yet integrated into `brewie_app`

Current hardware/audio finding:
- PCB tracing indicates the black round sounder is driven through an ST TS4871-style
  audio amplifier, not directly from a SOM GPIO
- the amplifier input traces go to SOM audio pins `HPOUTL` and `HPCOM`
- the amplifier supply is on the carrier `+5V` rail
- the amplifier standby pin is pulled low through 10k, so the amp is normally enabled
- ALSA playback on the current image can drive an AC signal into the amplifier input
- amplifier output was also observed during audio testing
- the original sounder measured like a piezo/capacitive transducer rather than a normal
  low-resistance speaker
- the amplifier IC was damaged during live probing and has been removed; the `+5V` rail
  no longer appears hard-shorted with the IC removed

Repair note:
- replacement amplifier candidate: STMicroelectronics `TS4871IST` in MiniSO-8 / MSOP-8
- replacement sounder should be a passive speaker or passive magnetic transducer, not an
  active DC buzzer
- audio feedback is deferred until the carrier-board amplifier/sounder hardware is
  repaired

Practical rule:
- do not probe `gpio34` / `PB2` again for buzzer on this image
- treat user-interface beep feedback as an ALSA/audio feature, not as a direct GPIO output
- do not add normal app audio feedback until the carrier hardware has been repaired and
  audio output is re-proven
- if a future GPIO candidate is ever investigated for another purpose, verify it against
  kernel/pinctrl state first, then use:

```bash
sudo /home/admin/probe_buzzer_gpio.sh --gpio <verified-gpio-number>
```

Only after the GPIO has been checked should inverted polarity be tried:

```bash
sudo /home/admin/probe_buzzer_gpio.sh --gpio <verified-gpio-number> --active-low
```

Do not add normal app audio feedback until the carrier-board amplifier/sounder hardware
has been repaired and the ALSA output path has been re-tested.

---

## Current display stack fact
Current target display path in BrewieApp:
- LVGL on Linux DRM
- DRM device path: `/dev/dri/card0`

This is now the chosen target display direction. The current product path is a custom
rotated DRM backend: LVGL sees a logical 272x480 portrait display, while the real DRM
scanout stays at the physical 480x272 RGB565 panel mode. Dirty LVGL rectangles are rotated
into a non-visible DRM buffer and page-flipped on vblank.

Framebuffer-first bring-up was considered, but DRM was selected because the SOM already
exposes `/dev/dri/card0`. LVGL's built-in DRM rotation paths were tested and failed on this
SOM image; the fbdev rotation path used unacceptable CPU.

Observed target device nodes:
- `/dev/fb0`
- `/dev/dri/card0`
- `/dev/dri/renderD128`

Current result:
- DRM display init succeeds on the SOM
- the app can initialize display and serial together
- a live portrait status/debug screen has been shown on the real screen

This is the first successful visible portrait LVGL-on-target milestone.

---

## Current display state
Current status is **not** “full UI working”.

What is currently true:
- target DRM display init works
- first visible portrait text output works
- screen is no longer just black
- serial/comms continue working while display is initialized
- manually started BrewieApp shows updating MCU status information on the target display
- service-started BrewieApp shows updating MCU status information on the target display
- partial redraw animation is acceptable
- continuous full-screen redraw animation is visibly choppy and should not be the default
  UI transition style

What is **not** yet done:
- final portrait-oriented status/home/fault layout logic
- true animated boot/splash screen during SOM startup
- full home/fault screen flow on target
- production-polished display startup behavior

So the current display milestone is:
- **first visible portrait LVGL proof on the real SOM target**

---

## Current touch/input fact
Current detected touch device:
- `/dev/input/event0`
- name: `Goodix Capacitive TouchScreen`
- sysfs path: `/devices/platform/soc/1c2b400.i2c/i2c-2/2-0014/input/input0`
- event capabilities include `BTN_TOUCH`, `ABS_X`, `ABS_Y`, and multitouch fields
- ABS ranges observed through `evtest`:
  - `ABS_X`: 0..799
  - `ABS_Y`: 0..479
  - `ABS_MT_POSITION_X`: 0..799
  - `ABS_MT_POSITION_Y`: 0..479

Practical note:
- the runtime `brewie` user is already in the `input` group, so the app should be able to
  open `/dev/input/event0`
- `BrewieApp` now opens the Goodix event device through LVGL evdev support
- the target display layer maps the touchscreen into the 272x480 portrait coordinate system
- physical corner taps and a temporary LVGL status-screen button have proven basic touch
  mapping and button click events
- this is still a bring-up proof, not a finished touch calibration/settings UI

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
- live status/debug information can be shown on the screen
- touch input reaches LVGL and a button click can be detected
- `brewie.service` can start the app automatically

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
- touch is integrated through LVGL evdev on `/dev/input/event0`
