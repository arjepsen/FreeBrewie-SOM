# Admin Deployment Helpers

## Purpose
This folder contains manual admin tools that should be included in a prepared
FreeBrewie SOM SD-card image.

These helpers are not part of the normal `brewie_app` runtime. They are maintenance
tools for the `admin` user.

## MCU flashing from the SOM
`flash_mcu_from_som.sh` flashes the ATmega2560 MCU from the SOM over `/dev/ttyS1`.

The prepared SOM image should include:
- `avrdude`
- `/etc/avrdude.conf`
- `/home/admin/flash_mcu_from_som.sh`
- access to `/dev/ttyS1`
- sysfs GPIO access to A13 `PE9`, exported as Linux `gpio137`

The helper:
- stops `brewie.service` so the app releases `/dev/ttyS1`
- resets the MCU with `gpio137`
- runs `avrdude` with the `wiring` programmer at `115200`
- restarts `brewie.service` if it was running before the flash attempt

## MCU bootloader requirement
SOM-side flashing requires the Brewie-compatible ATmega2560/STK500v2 bootloader.

The tested working MCU recovery state is:
- old Brewie-carried STK500v2 bootloader image
- `lfuse=0xFF`
- `hfuse=0xD8`
- `efuse=0xFD`
- `lock=0x3F`

The final lock byte must remain open. Hardware testing showed that applying the
stock final lock byte `0x0F` made the FreeBrewie app fail to start on this board.

## User-facing upgrade path
For normal users, the intended path is:

1. Write the prepared FreeBrewie SOM image to the SD card.
2. Boot the Brewie from that SD card.
3. Flash the FreeBrewie MCU firmware from the SOM.
4. Keep USBasp/ISP only as a recovery path.

If SOM-side flashing fails with `stk500v2_getsync(): timeout communicating with
programmer`, the MCU probably does not have the required bootloader state. Restore
the bootloader once from the MCU repo with:

```text
mega2560_recovery_usbasp -> Custom -> Restore Bootloader USBasp
```

Then return to SOM-side flashing as the normal workflow.

## Buzzer/audio note
`probe_buzzer_gpio.sh` is kept only as a cautious generic GPIO pulse helper. It should
not be treated as the current buzzer bring-up path.

The old Brewie Linux image exposed:

```text
/dev/brewie-buzzer -> /sys/class/gpio/gpio5_pb2
```

On the current A13 Bullseye image, the original alias does not map safely to a normal GPIO.
The first suspected Linux sysfs GPIO number, `gpio34`, is A13 `PB2`, but current kernel
debug output shows PB2 muxed to the PWM controller and requested by the display backlight.
It must not be used for buzzer probing:
- normal probing produced no buzzer sound
- inverted probing affected the display
- `/sys/kernel/debug/pwm` showed `pwm-0 (backlight)`

Later hardware tracing showed that the black round sounder is driven through an
ST TS4871-style audio amplifier from the SOM `HPOUTL`/`HPCOM` audio pins. The old GUI
also used `aplay /usr/share/beep.wav` for button feedback. Treat sound feedback as an
ALSA/audio feature, not as a direct GPIO output.

Current hardware status:
- the amplifier IC was damaged during live probing and has been removed
- the carrier `+5V` rail no longer appears hard-shorted with the IC removed
- buzzer/audio feedback is deferred until the amplifier/sounder hardware is repaired

Copy the helper to the SOM and run:

```bash
sudo /home/admin/probe_buzzer_gpio.sh --gpio <verified-gpio-number>
```

Only after a new candidate GPIO has been checked against kernel/pinctrl state should
inverted polarity be tried:

```bash
sudo /home/admin/probe_buzzer_gpio.sh --gpio <verified-gpio-number> --active-low
```

Only after the pin and polarity are proven should the normal `brewie_app` grow a real
platform GPIO-output module. Do not use this helper for the buzzer/audio path unless a
new hardware revision proves that the sounder is actually GPIO-driven.
