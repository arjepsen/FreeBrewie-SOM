#!/bin/sh
#
# Flash the Brewie ATmega2560 MCU from the SOM.
#
# This is an admin-only helper for controlled manual use. It deliberately does not live
# inside BrewieApp, because flashing must stop the running app before taking over the MCU
# serial port and reset line.

set -eu

MCU_RESET_GPIO=137
MCU_RESET_PATH="/sys/class/gpio/gpio${MCU_RESET_GPIO}"
MCU_SERIAL_DEVICE="/dev/ttyS1"
BREWIE_SERVICE="brewie.service"
AVRDUDE_PART="atmega2560"
AVRDUDE_PROGRAMMER="wiring"
AVRDUDE_BAUD="115200"

usage()
{
    echo "Usage: $0 /path/to/firmware.hex" >&2
}

die()
{
    echo "ERROR: $*" >&2
    exit 1
}

require_root()
{
    if [ "$(id -u)" != "0" ]; then
        die "run as root, for example: sudo $0 /home/admin/firmware.hex"
    fi
}

require_inputs()
{
    if [ "$#" -ne 1 ]; then
        usage
        exit 2
    fi

    HEX_FILE=$1

    if [ ! -r "$HEX_FILE" ]; then
        die "firmware hex file is not readable: $HEX_FILE"
    fi

    command -v avrdude >/dev/null 2>&1 || die "avrdude is not installed"

    if [ ! -c "$MCU_SERIAL_DEVICE" ]; then
        die "MCU serial device is missing: $MCU_SERIAL_DEVICE"
    fi
}

setup_reset_gpio()
{
    if [ ! -d "$MCU_RESET_PATH" ]; then
        echo "$MCU_RESET_GPIO" > /sys/class/gpio/export
    fi

    echo out > "$MCU_RESET_PATH/direction"

    # On the tested Brewie SOM, PE9/gpio137 resets the MCU when driven high. Low is the
    # released state that lets the ATmega run normally.
    echo 0 > "$MCU_RESET_PATH/value"
}

stop_brewie_service()
{
    SERVICE_WAS_ACTIVE=0

    if command -v systemctl >/dev/null 2>&1 &&
       systemctl is-active --quiet "$BREWIE_SERVICE"; then
        SERVICE_WAS_ACTIVE=1
        systemctl stop "$BREWIE_SERVICE"
    fi
}

restart_brewie_service_if_needed()
{
    if [ "${SERVICE_WAS_ACTIVE:-0}" = "1" ] &&
       command -v systemctl >/dev/null 2>&1; then
        systemctl start "$BREWIE_SERVICE"
    fi
}

reset_mcu_into_bootloader()
{
    # Match the proven reset polarity and the old Brewie order: assert reset high, then
    # release low immediately before avrdude tries to talk to the bootloader.
    echo 1 > "$MCU_RESET_PATH/value"
    sleep 0.2
    echo 0 > "$MCU_RESET_PATH/value"
}

run_avrdude()
{
    CONFIG_ARGS=""

    if [ -r /etc/avrdude.conf ]; then
        CONFIG_ARGS="-C /etc/avrdude.conf"
    fi

    # shellcheck disable=SC2086
    avrdude $CONFIG_ARGS \
        -v \
        -p "$AVRDUDE_PART" \
        -c "$AVRDUDE_PROGRAMMER" \
        -P "$MCU_SERIAL_DEVICE" \
        -b "$AVRDUDE_BAUD" \
        -D \
        -U "flash:w:${HEX_FILE}:i"
}

explain_bootloader_requirement()
{
    echo >&2
    echo "MCU UART flashing failed." >&2
    echo "This path requires the Brewie-compatible ATmega2560 bootloader to be installed." >&2
    echo "If the MCU was last flashed with USBasp/ISP, the bootloader may be missing." >&2
    echo "Restore it with the MCU repo's Restore Bootloader USBasp target, then use SOM flashing." >&2
}

main()
{
    require_root
    require_inputs "$@"
    setup_reset_gpio
    stop_brewie_service

    # Make a best effort to restart BrewieApp if flashing fails or is interrupted after the
    # service has been stopped.
    trap 'echo 0 > "$MCU_RESET_PATH/value" 2>/dev/null || true; restart_brewie_service_if_needed' EXIT INT TERM

    echo "Flashing MCU through the ATmega2560 bootloader on ${MCU_SERIAL_DEVICE}..."
    reset_mcu_into_bootloader
    if ! run_avrdude; then
        explain_bootloader_requirement
        exit 1
    fi

    trap - EXIT INT TERM
    echo 0 > "$MCU_RESET_PATH/value"
    restart_brewie_service_if_needed
}

main "$@"
