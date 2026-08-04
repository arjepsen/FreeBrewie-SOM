#!/bin/sh

set -eu

GPIO_NUMBER=34
PULSE_COUNT=3
ON_SECONDS=0.10
OFF_SECONDS=0.15
ACTIVE_VALUE=1
IDLE_VALUE=0

usage()
{
    cat <<EOF
Usage: $0 [--gpio N] [--count N] [--on SECONDS] [--off SECONDS] [--active-low]

Probe the suspected Brewie SOM carrier buzzer GPIO.

Defaults:
  --gpio 34        A13 PB2, from the old /dev/brewie-buzzer -> gpio5_pb2 mapping
  --count 3        number of short beeps
  --on 0.10        active time for each beep
  --off 0.15       idle time between beeps

This helper only uses sysfs GPIO and leaves the line idle before exiting.
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

GPIO_PATH="/sys/class/gpio/gpio${GPIO_NUMBER}"

if [ ! -d "$GPIO_PATH" ]; then
    echo "$GPIO_NUMBER" > /sys/class/gpio/export
fi

echo out > "${GPIO_PATH}/direction"
echo "$IDLE_VALUE" > "${GPIO_PATH}/value"

echo "Probing buzzer on gpio${GPIO_NUMBER}: active=${ACTIVE_VALUE}, idle=${IDLE_VALUE}"

INDEX=0
while [ "$INDEX" -lt "$PULSE_COUNT" ]; do
    echo "$ACTIVE_VALUE" > "${GPIO_PATH}/value"
    sleep "$ON_SECONDS"
    echo "$IDLE_VALUE" > "${GPIO_PATH}/value"
    sleep "$OFF_SECONDS"
    INDEX=$((INDEX + 1))
done

echo "$IDLE_VALUE" > "${GPIO_PATH}/value"
echo "Done. If there was no sound, retry with --active-low before assuming the pin is wrong."
