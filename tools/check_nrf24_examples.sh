#!/bin/sh
set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)

check_early_init_order() {
    label=$1
    rst_file=$2

    if [ ! -f "$rst_file" ]; then
        echo "$label main build artifact not found: $rst_file" >&2
        exit 1
    fi

    nrf_line=$(awk '/lcall[[:space:]]+_drv_nrf24l01_init_pins/{ print NR; exit }' "$rst_file")
    uart_line=$(awk '/lcall[[:space:]]+_stc8h_uart_init/{ print NR; exit }' "$rst_file")
    spi_line=$(awk '/lcall[[:space:]]+_stc8h_spi_init/{ print NR; exit }' "$rst_file")

    if [ -z "$nrf_line" ]; then
        echo "$label must drive nRF24 CE/CSN idle levels directly at boot" >&2
        exit 1
    fi
    if [ -z "$spi_line" ]; then
        echo "$label does not call stc8h_spi_init()" >&2
        exit 1
    fi
    if [ -n "$uart_line" ] && [ "$nrf_line" -gt "$uart_line" ]; then
        echo "$label initializes UART before forcing nRF24 CE/CSN idle levels" >&2
        exit 1
    fi
    if [ "$nrf_line" -gt "$spi_line" ]; then
        echo "$label initializes SPI before forcing nRF24 CE/CSN idle levels" >&2
        exit 1
    fi
}

check_pin_codegen() {
    label=$1
    rst_file=$2

    if [ ! -f "$rst_file" ]; then
        echo "$label nRF24 build artifact not found: $rst_file" >&2
        exit 1
    fi
    if ! grep -Eq 'anl[[:space:]]+_P1M0,#0xbb' "$rst_file"; then
        echo "$label does not configure P1.2/P1.6 as quasi-bidirectional outputs" >&2
        exit 1
    fi
    if ! grep -Eq 'anl[[:space:]]+_P1M1,#0xbb' "$rst_file"; then
        echo "$label does not clear P1.2/P1.6 high-impedance mode bits" >&2
        exit 1
    fi
    if ! grep -Eq 'orl[[:space:]]+_P1,#0x44' "$rst_file"; then
        echo "$label does not release nRF24 CE/CSN latches before driving idle levels" >&2
        exit 1
    fi
    if ! grep -Eq 'anl[[:space:]]+_P1,#0xbf' "$rst_file"; then
        echo "$label does not drive nRF24 CE low during pin init" >&2
        exit 1
    fi
}

check_example() {
    example_name=$1
    example_dir="$ROOT_DIR/examples/platformio/$example_name"

    if [ ! -d "$example_dir" ]; then
        echo "missing nRF24 example: $example_dir" >&2
        exit 1
    fi

    if [ "${CHECK_NRF24_EXAMPLES_SKIP_BUILD:-0}" != "1" ]; then
        (cd "$example_dir" && pio run -t clean && pio run)
    fi
    check_early_init_order "$example_name" "$example_dir/.pio/build/STC8H1K08/src/main.rst"
    check_pin_codegen "$example_name" "$example_dir/.pio/build/STC8H1K08/src/drv_nrf24l01_wrap.rst"
}

check_example nrf24_fixed_ping
check_example nrf24_ack_payload
check_example rf_link_nrf24_small
check_example nrf24_uart_diag
