#!/bin/sh

set -eu

GPIO_NUMBER=""
PULSE_COUNT=3
ON_SECONDS=0.10
OFF_SECONDS=0.15
ACTIVE_VALUE=1
IDLE_VALUE=0

usage()
{
    cat <<EOF
Usage: $0 --gpio N [--count N] [--on SECONDS] [--off SECONDS] [--active-low]

Pulse a verified Brewie SOM carrier GPIO for cautious manual probing.

Defaults:
  --gpio N         Linux sysfs GPIO number to probe
  --count 3        number of short beeps
  --on 0.10        active time for each beep
  --off 0.15       idle time between beeps

This helper only uses sysfs GPIO and leaves the line idle before exiting.

Important:
  Do not use gpio34 as the default buzzer guess. Testing showed no buzzer sound, and
  inverted probing affected the display on the current Olimex Bullseye image.
  The current buzzer/sounder path appears to be ALSA audio through an amplifier, not GPIO.
EOF
}

while [ "$#" -gt 0 ]; do
    case "$1" in
        --gpio)
            GPIO_NUMBER="$2"
            shift 2
            ;;
        --count)
            PULSE_COUNT="$2"
            shift 2
            ;;
        --on)
            ON_SECONDS="$2"
            shift 2
            ;;
        --off)
            OFF_SECONDS="$2"
            shift 2
            ;;
        --active-low)
            ACTIVE_VALUE=0
            IDLE_VALUE=1
            shift
            ;;
        --help|-h)
            usage
            exit 0
            ;;
        *)
            usage >&2
            exit 2
            ;;
    esac
done

if [ -z "$GPIO_NUMBER" ]; then
    usage >&2
    exit 2
fi

if [ "$GPIO_NUMBER" = "34" ]; then
    echo "Refusing gpio34: it affected the display during previous buzzer probing." >&2
    echo "Use a newly verified GPIO number instead." >&2
    exit 2
fi

GPIO_PATH="/sys/class/gpio/gpio${GPIO_NUMBER}"

if [ ! -d "$GPIO_PATH" ]; then
    echo "$GPIO_NUMBER" > /sys/class/gpio/export
fi

echo out > "${GPIO_PATH}/direction"
echo "$IDLE_VALUE" > "${GPIO_PATH}/value"

echo "Pulsing gpio${GPIO_NUMBER}: active=${ACTIVE_VALUE}, idle=${IDLE_VALUE}"

INDEX=0
while [ "$INDEX" -lt "$PULSE_COUNT" ]; do
    echo "$ACTIVE_VALUE" > "${GPIO_PATH}/value"
    sleep "$ON_SECONDS"
    echo "$IDLE_VALUE" > "${GPIO_PATH}/value"
    sleep "$OFF_SECONDS"
    INDEX=$((INDEX + 1))
done

echo "$IDLE_VALUE" > "${GPIO_PATH}/value"
echo "Done. Only retry with --active-low after the pin has been checked against pinctrl/debugfs."
