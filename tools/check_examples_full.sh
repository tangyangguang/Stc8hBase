#!/bin/sh
set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)

run_platformio_example() {
    example_dir=$1
    echo "== pio: ${example_dir}"
    (cd "${ROOT_DIR}/${example_dir}" && pio run)
}

run_platformio_env() {
    example_dir=$1
    env_name=$2
    echo "== pio: ${example_dir} -e ${env_name}"
    (cd "${ROOT_DIR}/${example_dir}" && pio run -e "${env_name}")
}

run_make_example() {
    example_dir=$1
    echo "== make: ${example_dir}"
    (cd "${ROOT_DIR}/${example_dir}" && make clean && make)
}

check_map_absent() {
    map_file=$1
    shift

    for pattern in "$@"; do
        if grep -q "${pattern}" "${ROOT_DIR}/${map_file}"; then
            echo "forbidden symbol '${pattern}' found in ${map_file}" >&2
            exit 1
        fi
    done
}

check_ota_app_base() {
    map_file=$1

    if ! python3 - "${ROOT_DIR}/${map_file}" <<'PY'
import sys
found = False
with open(sys.argv[1], "r", encoding="utf-8", errors="ignore") as fh:
    for line in fh:
        parts = line.split()
        if len(parts) >= 3 and parts[0] == "C:" and parts[1] == "00000200" and parts[2] == "s_HOME":
            found = True
            break
raise SystemExit(0 if found else 1)
PY
    then
        echo "OTA app HOME area is not linked at 0x0200 in ${map_file}" >&2
        exit 1
    fi

    if python3 - "${ROOT_DIR}/${map_file}" <<'PY'
import sys
bad = False
with open(sys.argv[1], "r", encoding="utf-8", errors="ignore") as fh:
    for line in fh:
        parts = line.split()
        if len(parts) >= 3 and parts[0] == "C:" and parts[2] == "_main":
            try:
                bad = int(parts[1], 16) < 0x0200
            except ValueError:
                bad = False
            break
raise SystemExit(0 if bad else 1)
PY
    then
        echo "OTA app main is linked below 0x0200 in ${map_file}" >&2
        exit 1
    fi
}

check_ota_bootloader_layout() {
    map_file=$1
    hex_file=$2

    if ! python3 - "${ROOT_DIR}/${map_file}" <<'PY'
import sys
found = False
with open(sys.argv[1], "r", encoding="utf-8", errors="ignore") as fh:
    for line in fh:
        parts = line.split()
        if len(parts) >= 3 and parts[0] == "C:" and parts[1] == "00000000" and parts[2] == "_h8k64u_ota_reset_stub":
            found = True
            break
raise SystemExit(0 if found else 1)
PY
    then
        echo "OTA bootloader reset stub is not linked at 0x0000 in ${map_file}" >&2
        exit 1
    fi

    if ! grep -qx ':0300000002B40047' "${ROOT_DIR}/${hex_file}"; then
        echo "OTA bootloader reset stub is not present in ${hex_file}" >&2
        exit 1
    fi

    if ! python3 - "${ROOT_DIR}/${map_file}" <<'PY'
import sys
found = False
with open(sys.argv[1], "r", encoding="utf-8", errors="ignore") as fh:
    for line in fh:
        parts = line.split()
        if len(parts) >= 3 and parts[0] == "C:" and parts[1] == "0000B400" and parts[2] == "s_HOME":
            found = True
            break
raise SystemExit(0 if found else 1)
PY
    then
        echo "OTA bootloader HOME area is not linked at 0xB400 in ${map_file}" >&2
        exit 1
    fi

    if python3 - "${ROOT_DIR}/${map_file}" <<'PY'
import sys
bad = False
with open(sys.argv[1], "r", encoding="utf-8", errors="ignore") as fh:
    for line in fh:
        parts = line.split()
        if len(parts) >= 2 and parts[0] == "C:":
            try:
                addr = int(parts[1], 16)
            except ValueError:
                continue
            if 0xFC00 <= addr < 0x10000:
                bad = True
                break
raise SystemExit(0 if bad else 1)
PY
    then
        echo "OTA bootloader code overlaps parameter sectors in ${map_file}" >&2
        exit 1
    fi
}

sh "${ROOT_DIR}/tools/check_host_tests_full.sh"

for ini in "${ROOT_DIR}"/examples/platformio/*/platformio.ini; do
    run_platformio_example "examples/platformio/$(basename "$(dirname "${ini}")")"
done

run_platformio_env "examples/platformio/h8k64u_ota_min_app" "STC8H8K64U_mark_valid_iap"
run_platformio_env "examples/platformio/eeprom_rw" "STC8H1K08_write_test"
run_platformio_env "examples/platformio/h8k64u_uart2_hello" "STC8H8K64U_uart1_pin_group1"

sh "${ROOT_DIR}/tools/check_nrf24_examples.sh"

run_make_example "examples/make/gpio_blink"
run_make_example "examples/make/i2c_scan"
run_make_example "examples/make/milestone1_demo"

check_map_absent \
    "examples/platformio/gpio_blink/.pio/build/STC8H1K08/firmware.map" \
    "_stc8h_uart" "_stc8h_i2c" "_drv_lcd1602" "_drv_button" "_drv_ec11" \
    "_drv_ir" "_drv_tm1637" "_stc8h_spi" "_stc8h_adc" "_stc8h_eeprom" "_util_" \
    "_stc8h_wdt" "_stc8h_power" "_stc8h_exti" "_drv_nrf24l01" "_proto_rf_link"

check_ota_app_base \
    "examples/platformio/h8k64u_ota_min_app/.pio/build/STC8H8K64U/firmware.map"
check_ota_app_base \
    "examples/platformio/h8k64u_ota_min_app/.pio/build/STC8H8K64U_mark_valid_iap/firmware.map"

check_ota_bootloader_layout \
    "examples/platformio/h8k64u_rs485_ota_bootloader/.pio/build/STC8H8K64U/firmware.map" \
    "examples/platformio/h8k64u_rs485_ota_bootloader/.pio/build/STC8H8K64U/firmware.hex"
check_ota_bootloader_layout \
    "examples/platformio/h8k64u_uart1_ota_bootloader/.pio/build/STC8H8K64U/firmware.map" \
    "examples/platformio/h8k64u_uart1_ota_bootloader/.pio/build/STC8H8K64U/firmware.hex"

if grep -Eq '\(stc8h_u32\)1u? *<< *rx->bit_index' "${ROOT_DIR}/drivers/drv_ir_rx.c"; then
    echo "forbidden variable u32 shift found in drivers/drv_ir_rx.c" >&2
    exit 1
fi

echo "full example checks passed"
