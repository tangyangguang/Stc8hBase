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
        if len(parts) >= 3 and parts[0] == "C:" and parts[1] == "00006C00" and parts[2] == "s_HOME":
            found = True
            break
raise SystemExit(0 if found else 1)
PY
    then
        echo "OTA app HOME area is not linked at 0x6C00 in ${map_file}" >&2
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
                bad = int(parts[1], 16) < 0x6C00
            except ValueError:
                bad = False
            break
raise SystemExit(0 if bad else 1)
PY
    then
        echo "OTA app main is linked below 0x6C00 in ${map_file}" >&2
        exit 1
    fi
}

check_ota_bootloader_layout() {
    map_file=$1
    hex_file=$2

    python3 - "${ROOT_DIR}/${map_file}" "${ROOT_DIR}/${hex_file}" <<'PY'
import sys

map_path, hex_path = sys.argv[1:]
home_ok = False
with open(map_path, "r", encoding="utf-8", errors="ignore") as fh:
    for line in fh:
        parts = line.split()
        if len(parts) >= 3 and parts[0] == "C:" and parts[1] == "00000200" and parts[2] == "s_HOME":
            home_ok = True
if not home_ok:
    raise SystemExit("bootloader HOME is not linked at protected address 0x0200")

memory = {}
upper = 0
with open(hex_path, "r", encoding="ascii") as fh:
    for raw in fh:
        line = raw.strip()
        if not line or not line.startswith(":"):
            continue
        count = int(line[1:3], 16)
        addr = int(line[3:7], 16)
        kind = int(line[7:9], 16)
        data = bytes.fromhex(line[9:9 + count * 2])
        if kind == 0:
            for index, value in enumerate(data):
                memory[upper + addr + index] = value
        elif kind == 4:
            upper = int.from_bytes(data, "big") << 16
        elif kind == 1:
            break
if bytes(memory.get(i, 0xFF) for i in range(3)) != bytes((0x02, 0x02, 0x00)):
    raise SystemExit("reset vector does not jump to protected bootloader 0x0200")
if bytes(memory.get(0x43 + i, 0xFF) for i in range(3)) != bytes((0x02, 0x6C, 0x43)):
    raise SystemExit("UART2 vector does not forward to application 0x6C43")
if any(addr >= 0x6C00 for addr in memory):
    raise SystemExit("bootloader image overlaps IAP-writable application region")
PY
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
    "_stc8h_wdt" "_stc8h_power" "_stc8h_exti" "_stc8h_qei" \
    "_drv_nrf24l01" "_proto_rf_link"

check_ota_app_base \
    "examples/platformio/h8k64u_ota_min_app/.pio/build/STC8H8K64U/firmware.map"
check_ota_app_base \
    "examples/platformio/h8k64u_ota_min_app/.pio/build/STC8H8K64U_mark_valid_iap/firmware.map"

check_ota_bootloader_layout \
    "examples/platformio/h8k64u_rs485_ota_bootloader/.pio/build/STC8H8K64U/firmware.map" \
    "examples/platformio/h8k64u_rs485_ota_bootloader/.pio/build/STC8H8K64U/firmware.hex"

ota_tmp=$(mktemp -d)
trap 'rm -rf "${ota_tmp}"' EXIT HUP INT TERM
python3 "${ROOT_DIR}/tools/stc8h_ota.py" pack \
    --hex "${ROOT_DIR}/examples/platformio/h8k64u_ota_min_app/.pio/build/STC8H8K64U_mark_valid_iap/firmware.hex" \
    --output "${ota_tmp}/min-app.stcota" \
    --board-id 0 --hardware-revision 0 --app-id 0 \
    --version 1.0.0 --build 1
python3 "${ROOT_DIR}/tools/stc8h_ota.py" factory \
    --boot-hex "${ROOT_DIR}/examples/platformio/h8k64u_rs485_ota_bootloader/.pio/build/STC8H8K64U/firmware.hex" \
    --package "${ota_tmp}/min-app.stcota" \
    --output "${ota_tmp}/factory.hex"
python3 "${ROOT_DIR}/tools/stc8h_ota.py" inspect "${ota_tmp}/min-app.stcota"
rm -rf "${ota_tmp}"
trap - EXIT HUP INT TERM

if grep -Eq '\(stc8h_u32\)1u? *<< *rx->bit_index' "${ROOT_DIR}/drivers/drv_ir_rx.c"; then
    echo "forbidden variable u32 shift found in drivers/drv_ir_rx.c" >&2
    exit 1
fi

echo "full example checks passed"
