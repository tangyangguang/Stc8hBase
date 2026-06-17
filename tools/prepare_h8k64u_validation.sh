#!/usr/bin/env sh
set -eu

usage() {
    cat <<'EOF'
Usage:
  ./tools/prepare_h8k64u_validation.sh [download_port]

Purpose:
  Build all STC8H8K64U-LQFP48 validation examples without requiring hardware.
  The script also prints serial-device discovery output and manual upload/monitor
  command templates for the later hardware validation step.

Notes:
  - This script does not upload firmware.
  - UART2/UART3 validation may need separate monitor adapters wired to those pins.
  - EEPROM destructive write/erase validation is intentionally not included.
EOF
}

if [ "$#" -gt 1 ]; then
    usage
    exit 2
fi

case "${1:-}" in
    -h|--help)
        usage
        exit 0
        ;;
esac

download_port="${1:-}"
script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
root_dir=$(CDPATH= cd -- "${script_dir}/.." && pwd)

build_example() {
    example_dir="$1"
    echo "== build: ${example_dir}"
    (cd "${root_dir}/${example_dir}" && pio run)
}

list_serial_devices() {
    echo
    echo "== serial devices reported by PlatformIO"
    if ! pio device list; then
        echo "pio device list failed; check PlatformIO installation before hardware validation." >&2
    fi

    echo
    echo "== likely local serial devices"
    found=0
    for pattern in \
        /dev/cu.usbserial* \
        /dev/cu.wchusbserial* \
        /dev/cu.SLAB_USBtoUART* \
        /dev/cu.usbmodem*
    do
        for device in $pattern; do
            if [ -e "$device" ]; then
                found=1
                echo "$device"
            fi
        done
    done
    if [ "$found" -eq 0 ]; then
        echo "(none)"
    fi
}

print_next_steps() {
    echo
    echo "== manual hardware validation commands"
    if [ -n "$download_port" ]; then
        echo "DOWNLOAD_PORT=${download_port}"
    else
        echo "DOWNLOAD_PORT=/dev/cu.usbserial-xxx"
    fi
    cat <<'EOF'
UART1_MONITOR_PORT=${DOWNLOAD_PORT}
UART2_MONITOR_PORT=/dev/cu.usbserial-uart2
UART3_MONITOR_PORT=/dev/cu.usbserial-uart3

(cd examples/platformio/h8k64u_uart2_hello && pio run -t upload --upload-port "${DOWNLOAD_PORT}")
pio device monitor --port "${UART1_MONITOR_PORT}" --baud 115200
# Optional real UART2 TX monitor if wired:
# pio device monitor --port "${UART2_MONITOR_PORT}" --baud 9600

(cd examples/platformio/h8k64u_uart3_hello && pio run -t upload --upload-port "${DOWNLOAD_PORT}")
pio device monitor --port "${UART1_MONITOR_PORT}" --baud 115200
# Optional real UART3 TX monitor if wired:
# pio device monitor --port "${UART3_MONITOR_PORT}" --baud 9600

(cd examples/platformio/h8k64u_gpio_blink && pio run -t upload --upload-port "${DOWNLOAD_PORT}")
pio device monitor --port "${UART1_MONITOR_PORT}" --baud 115200
# Confirm P1.3 toggles externally only if a probe or LED is wired.

(cd examples/platformio/h8k64u_adc_read && pio run -t upload --upload-port "${DOWNLOAD_PORT}")
pio device monitor --port "${UART1_MONITOR_PORT}" --baud 115200

(cd examples/platformio/h8k64u_eeprom_safe && pio run -t upload --upload-port "${DOWNLOAD_PORT}")
pio device monitor --port "${UART1_MONITOR_PORT}" --baud 115200

(cd examples/platformio/h8k64u_wdt_feed && pio run -t upload --upload-port "${DOWNLOAD_PORT}")
pio device monitor --port "${UART1_MONITOR_PORT}" --baud 115200
EOF
}

build_example examples/platformio/h8k64u_uart2_hello
build_example examples/platformio/h8k64u_uart3_hello
build_example examples/platformio/h8k64u_gpio_blink
build_example examples/platformio/h8k64u_adc_read
build_example examples/platformio/h8k64u_eeprom_safe
build_example examples/platformio/h8k64u_wdt_feed

list_serial_devices
print_next_steps

echo
echo "H8K64U no-hardware preparation passed."
