#!/bin/sh
set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)

run_platformio_example() {
    example_dir=$1
    echo "== pio: ${example_dir}"
    (cd "${ROOT_DIR}/${example_dir}" && pio run)
}

run_make_example() {
    example_dir=$1
    echo "== make: ${example_dir}"
    (cd "${ROOT_DIR}/${example_dir}" && make clean && make)
}

sh "${ROOT_DIR}/tools/check_host_tests.sh"

# Daily representative builds. Full release and specialty checks live in
# tools/check_examples_full.sh.
run_platformio_example "examples/platformio/gpio_blink"
run_platformio_example "examples/platformio/uart_hello"
run_platformio_example "examples/platformio/i2c_scan"
run_platformio_example "examples/platformio/spi_loopback"
run_platformio_example "examples/platformio/pwm_output"
run_platformio_example "examples/platformio/adc_pot"
run_platformio_example "examples/platformio/eeprom_rw"
run_platformio_example "examples/platformio/wdt_feed"
run_platformio_example "examples/platformio/nrf24_fixed_ping"
run_platformio_example "examples/platformio/h8k64u_gpio_blink"
run_platformio_example "examples/platformio/h8k64u_uart2_hello"
run_platformio_example "examples/platformio/h8k64u_ota_min_app"

run_make_example "examples/make/gpio_blink"
run_make_example "examples/make/i2c_scan"

echo "daily example checks passed"
