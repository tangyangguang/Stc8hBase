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

# Asserts the .mem ROM line reports at most $2 bytes used.
# Catches accidental regressions when trim macros stop working.
check_mem_rom_at_most() {
    mem_file=$1
    max_bytes=$2
    used=$(python3 - "${ROOT_DIR}/${mem_file}" <<'PY'
import sys
with open(sys.argv[1], "r", encoding="utf-8", errors="ignore") as fh:
    for line in fh:
        if "ROM/EPROM/FLASH" in line:
            print(int(line.split()[3]))
            break
    else:
        raise SystemExit("ROM/EPROM/FLASH line not found")
PY
)
    if [ "${used}" -gt "${max_bytes}" ]; then
        echo "ROM ${used} exceeds guard ${max_bytes} in ${mem_file}" >&2
        exit 1
    fi
}

check_sdcc_interrupt_using() {
    tmp_dir=$(mktemp -d)
    trap 'rm -rf "${tmp_dir}"' EXIT
    cat > "${tmp_dir}/test_using.c" <<'EOF'
#include "stc8h_compiler.h"
STC8H_INTERRUPT_USING(test_isr, 0, 1)
{
}
void main(void) {}
EOF
    sdcc -mmcs51 --std-sdcc11 -I"${ROOT_DIR}/core" -c -o "${tmp_dir}/test_using.rel" "${tmp_dir}/test_using.c"
}

check_eeprom_api_trim() {
    tmp_dir=$(mktemp -d)
    trap 'rm -rf "${tmp_dir}"' EXIT

    cat > "${tmp_dir}/eeprom_read_only.c" <<EOF
#define STC8H_EEPROM_ENABLE_READ 1
#define STC8H_EEPROM_ENABLE_WRITE 0
#define STC8H_EEPROM_ENABLE_ERASE 0
#include "${ROOT_DIR}/hal/stc8h_eeprom.c"
EOF
    sdcc -mmcs51 --std-sdcc11 -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/hal" -c -o "${tmp_dir}/eeprom_read_only.rel" "${tmp_dir}/eeprom_read_only.c"
    if grep -Eq '_stc8h_eeprom_(write|erase_sector)' "${tmp_dir}/eeprom_read_only.sym"; then
        echo "disabled EEPROM write/erase symbols found in read-only trim check" >&2
        exit 1
    fi

    cat > "${tmp_dir}/eeprom_fixed.c" <<EOF
#define STC8H_EEPROM_ENABLE_READ 0
#define STC8H_EEPROM_ENABLE_WRITE 0
#define STC8H_EEPROM_ENABLE_ERASE 0
#define STC8H_EEPROM_ENABLE_FIXED_BLOCK 1
#define STC8H_EEPROM_FIXED_ADDR 0x0E00u
#define STC8H_EEPROM_FIXED_SIZE 8u
#include "${ROOT_DIR}/hal/stc8h_eeprom.c"
EOF
    sdcc -mmcs51 --std-sdcc11 -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/hal" -c -o "${tmp_dir}/eeprom_fixed.rel" "${tmp_dir}/eeprom_fixed.c"
    if grep -Eq '_stc8h_eeprom_(read|write|erase_sector)($|[[:space:]])' "${tmp_dir}/eeprom_fixed.sym"; then
        echo "generic EEPROM symbols found in fixed-block trim check" >&2
        exit 1
    fi
    if ! grep -q '_stc8h_eeprom_read_fixed' "${tmp_dir}/eeprom_fixed.sym"; then
        echo "fixed EEPROM read symbol missing" >&2
        exit 1
    fi
    if ! grep -q '_stc8h_eeprom_save_fixed' "${tmp_dir}/eeprom_fixed.sym"; then
        echo "fixed EEPROM save symbol missing" >&2
        exit 1
    fi
}

check_sdcc_interrupt_using
check_eeprom_api_trim

for ini in "${ROOT_DIR}"/examples/platformio/*/platformio.ini; do
    run_platformio_example "examples/platformio/$(basename "$(dirname "${ini}")")"
done

run_platformio_env "examples/platformio/eeprom_rw" "STC8H1K08_write_test"

run_make_example "examples/make/gpio_blink"
run_make_example "examples/make/i2c_scan"
run_make_example "examples/make/milestone1_demo"

check_map_absent \
    "examples/platformio/gpio_blink/.pio/build/STC8H1K08/firmware.map" \
    "_stc8h_uart" "_stc8h_i2c" "_drv_lcd1602" "_drv_button" "_drv_ec11" \
    "_drv_ir" "_drv_tm1637" "_stc8h_spi" "_stc8h_adc" "_stc8h_eeprom" "_util_" \
    "_stc8h_wdt" "_stc8h_power" "_stc8h_exti" "_stc8h_gpio_toggle"

check_map_absent \
    "examples/platformio/ir_nec_rx_int_sleep/.pio/build/STC8H1K08/firmware.map" \
    "_stc8h_i2c" "_drv_lcd1602" "_drv_button" "_drv_ec11" "_stc8h_adc" \
    "_stc8h_spi" "_stc8h_eeprom" "_drv_tm1637" "_drv_ir_tx" "_stc8h_pwm" "_util_" \
    "_stc8h_wdt" "__div" "__mod" "__mullong" "_stc8h_exti_disable" \
    "_stc8h_exti_clear_flags" "_stc8h_power_idle"

check_map_absent \
    "examples/platformio/gpio_blink/.pio/build/STC8H1K08/firmware.map" \
    "_drv_nrf24l01" "_proto_rf_link"

check_map_absent \
    "examples/platformio/pwm_pwma_pwmb_small/.pio/build/STC8H1K08/firmware.map" \
    "_drv_ec11_init " "_drv_ec11_scan " "_drv_ec11_get_delta " \
    "_drv_ec11_set_fast" "_drv_ec11_set_reverse" "_drv_ec11_set_steps_per_detent"

check_map_absent \
    "examples/platformio/rf_link_nrf24_small/.pio/build/STC8H1K08/firmware.map" \
    "_drv_nrf24l01_read_fifo_status" "_drv_nrf24l01_read_observe_tx" \
    "_drv_nrf24l01_read_reg" "_drv_nrf24l01_write_reg" "_drv_nrf24l01_read_buf" \
    "_drv_nrf24l01_write_buf" "_drv_nrf24l01_command" \
    "_drv_nrf24l01_set_address_width" "_drv_nrf24l01_set_tx_address" \
    "_drv_nrf24l01_set_rx_address" "_drv_nrf24l01_set_payload_size" \
    "_drv_nrf24l01_enable_dynamic_payload" "_drv_nrf24l01_disable_dynamic_payload" \
    "_drv_nrf24l01_enable_ack_payload" "_drv_nrf24l01_disable_ack_payload" \
    "_drv_nrf24l01_read_dynamic_payload_size" "_drv_nrf24l01_write_ack_payload"

check_map_absent \
    "examples/platformio/rf_link_nrf24_small/.pio/build/STC8H1K08/firmware.map" \
    "_proto_rf_link_connect " "_proto_rf_link_send_data " "_proto_rf_link_poll " \
    "_proto_rf_link_reset " "_proto_rf_link_tick " "_proto_rf_link_get_state "

check_map_absent \
    "examples/platformio/tm1637_number/.pio/build/STC8H1K08/firmware.map" \
    "_drv_tm1637_set_display" "_drv_tm1637_display_raw " "_drv_tm1637_display_digits" \
    "_drv_tm1637_display_number" "_drv_tm1637_clear"

check_map_absent \
    "examples/platformio/pwm_pwma_pwmb_small/.pio/build/STC8H1K08/firmware.map" \
    "_stc8h_pwm_disable"

check_map_absent \
    "examples/platformio/pwm_pwma_pwmb_small/.pio/build/STC8H1K08/firmware.mem" \
    "Could not get" "DSEG.*overflow" "OSEG.*overflow"

# Hard ROM ceilings for the "small" fixed-path examples. The guards
# catch regressions in:
#   - STC8H_PWM_TRACK_PERIOD_PRESCALER / SET_DUTY_CHANNEL_CHECK / CLAMP
#   - PROTO_RF_LINK_INCLUDE_TIMEOUT_FIELDS
#   - PROTO_RF_LINK_ENABLE_SEND_DATA_FIXED_TRACK_ACK / POLL_DATA_FIXED_TRACK_LINK
#   - DRV_EC11_ENABLE_NULL_CHECK
# Raise the ceiling deliberately if a real feature has been added.
check_mem_rom_at_most \
    "examples/platformio/pwm_pwma_pwmb_small/.pio/build/STC8H1K08/firmware.mem" \
    2350
check_mem_rom_at_most \
    "examples/platformio/rf_link_nrf24_small/.pio/build/STC8H1K08/firmware.mem" \
    2080

if grep -Eq '\(stc8h_u32\)1u? *<< *rx->bit_index' "${ROOT_DIR}/drivers/drv_ir_rx.c"; then
    echo "forbidden variable u32 shift found in drivers/drv_ir_rx.c" >&2
    exit 1
fi

echo "example build and symbol checks passed"
