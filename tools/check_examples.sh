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

check_sdcc_interrupt_using

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

if grep -Eq '\(stc8h_u32\)1u? *<< *rx->bit_index' "${ROOT_DIR}/drivers/drv_ir_rx.c"; then
    echo "forbidden variable u32 shift found in drivers/drv_ir_rx.c" >&2
    exit 1
fi

echo "example build and symbol checks passed"
