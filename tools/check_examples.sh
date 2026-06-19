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

check_sym_absent() {
    sym_file=$1
    shift
    for symbol in "$@"; do
        if awk -v symbol="${symbol}" '$2 == symbol { found = 1 } END { exit found ? 0 : 1 }' \
            "${ROOT_DIR}/${sym_file}"; then
            echo "forbidden symbol '${symbol}' found in ${sym_file}" >&2
            exit 1
        fi
    done
}

check_global_sym_absent() {
    sym_file=$1
    shift
    for symbol in "$@"; do
        if awk -v symbol="${symbol}" '$2 == symbol && $4 ~ /G/ { found = 1 } END { exit found ? 0 : 1 }' \
            "${ROOT_DIR}/${sym_file}"; then
            echo "forbidden global symbol '${symbol}' found in ${sym_file}" >&2
            exit 1
        fi
    done
}

check_no_gptr_in_tree() {
    tree_path=$1
    if grep -R -Eq '__gptr(get|put)' "${ROOT_DIR}/${tree_path}"; then
        echo "generic pointer helper found in ${tree_path}" >&2
        exit 1
    fi
}

check_ota_app_base() {
    map_file=$1

    if ! awk '$3 == "s_HOME" && $1 == "C:" && $2 == "00000200" { found = 1 } END { exit found ? 0 : 1 }' \
        "${ROOT_DIR}/${map_file}"; then
        echo "OTA app HOME area is not linked at 0x0200 in ${map_file}" >&2
        exit 1
    fi

    if awk '$3 == "_main" && $1 == "C:" && ("0x" $2) + 0 < 0x0200 { bad = 1 } END { exit bad ? 0 : 1 }' \
        "${ROOT_DIR}/${map_file}"; then
        echo "OTA app main is linked below 0x0200 in ${map_file}" >&2
        exit 1
    fi
}

check_ota_bootloader_layout() {
    map_file=$1

    if ! awk '$3 == "_h8k64u_ota_reset_stub" && $1 == "C:" && $2 == "00000000" { found = 1 } END { exit found ? 0 : 1 }' \
        "${ROOT_DIR}/${map_file}"; then
        echo "OTA bootloader reset stub is not linked at 0x0000 in ${map_file}" >&2
        exit 1
    fi

    if ! awk '$3 == "s_HOME" && $1 == "C:" && $2 == "0000B800" { found = 1 } END { exit found ? 0 : 1 }' \
        "${ROOT_DIR}/${map_file}"; then
        echo "OTA bootloader HOME area is not linked at 0xB800 in ${map_file}" >&2
        exit 1
    fi

    if awk '$1 == "C:" && $2 ~ /^[0-9A-F]+$/ && ("0x" $2) + 0 >= 0xFC00 && ("0x" $2) + 0 < 0x10000 { bad = 1 } END { exit bad ? 0 : 1 }' \
        "${ROOT_DIR}/${map_file}"; then
        echo "OTA bootloader code overlaps parameter sectors in ${map_file}" >&2
        exit 1
    fi
}

# Asserts the .mem ROM line reports at most $2 bytes used.
# Catches accidental regressions when trim macros stop working.
check_mem_rom_at_most() {
    mem_file=$1
    max_bytes=$2
    label=$3
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
    echo "ROM guard ${label}: ${used}/${max_bytes} bytes"
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
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
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
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
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

check_ec11_small_isr_api() {
    tmp_dir=$(mktemp -d)
    trap 'rm -rf "${tmp_dir}"' EXIT

    cat > "${tmp_dir}/ec11_small_isr.c" <<EOF
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
#define DRV_EC11_ENABLE_FULL_API 0
#define DRV_EC11_ENABLE_SMALL_API 0
#define DRV_EC11_ENABLE_SMALL_ISR_API 1
#define DRV_EC11_ENABLE_NULL_CHECK 0
#include "${ROOT_DIR}/drivers/drv_ec11.h"
static STC8H_DATA drv_ec11_small_t ec11;
void main(void)
{
    drv_ec11_small_init_isr(&ec11);
    (void)drv_ec11_scan_delta_small_isr(&ec11, 1u, 1u);
}
#include "${ROOT_DIR}/drivers/drv_ec11.c"
EOF

    sdcc -mmcs51 --std-sdcc11 -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/drivers" \
        --out-fmt-ihx --code-size 8192 --iram-size 256 \
        -o "${tmp_dir}/ec11_small_isr.ihx" "${tmp_dir}/ec11_small_isr.c"

    if grep -Eq '__gptr(get|put)|_drv_ec11_transition_PARM_2' \
        "${tmp_dir}/ec11_small_isr.map" "${tmp_dir}/ec11_small_isr.rst"; then
        echo "EC11 small ISR API emitted generic pointer helpers or ordinary transition overlay parameter" >&2
        exit 1
    fi

    if grep -Eq '^OSEG[[:space:]]+[[:xdigit:]]+[[:space:]]+[[:xdigit:]]*[1-9A-Fa-f]' \
        "${tmp_dir}/ec11_small_isr.map"; then
        echo "EC11 small ISR API generated OSEG usage" >&2
        exit 1
    fi
}

check_spi_miso_input_codegen() {
    tmp_dir=$(mktemp -d)
    trap 'rm -rf "${tmp_dir}"' EXIT

    for group in 0 1 2 3; do
        case "${group}" in
            0) ie_addr='0xfe31'; port_sfr='_P1'; miso_mask='0x10' ;;
            1) ie_addr='0xfe32'; port_sfr='_P2'; miso_mask='0x10' ;;
            2) ie_addr='0xfe34'; port_sfr='_P4'; miso_mask='0x02' ;;
            3) ie_addr='0xfe33'; port_sfr='_P3'; miso_mask='0x08' ;;
        esac

        cat > "${tmp_dir}/spi_group_${group}.c" <<EOF
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
#define STC8H_SPI_PIN_GROUP ${group}u
#include "${ROOT_DIR}/hal/stc8h_spi.c"
EOF
        sdcc -mmcs51 --std-sdcc11 -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/hal" \
            -c -o "${tmp_dir}/spi_group_${group}.rel" "${tmp_dir}/spi_group_${group}.c"

        if ! grep -Eq 'orl[[:space:]]+_P_SW2,#0x80' "${tmp_dir}/spi_group_${group}.asm"; then
            echo "SPI group ${group} init does not enable XFR access before MISO PxIE" >&2
            exit 1
        fi
        if ! grep -Eq "orl[[:space:]]+${port_sfr},#${miso_mask}" "${tmp_dir}/spi_group_${group}.asm"; then
            echo "SPI group ${group} init does not latch MISO high before quasi-bidirectional reads" >&2
            exit 1
        fi
        if ! grep -Eiq "#0x0*${ie_addr#0x}" "${tmp_dir}/spi_group_${group}.asm"; then
            echo "SPI group ${group} init does not touch expected MISO input-enable register ${ie_addr}" >&2
            exit 1
        fi
    done
}

check_pwm_xfr_codegen() {
    tmp_dir=$(mktemp -d)
    trap 'rm -rf "${tmp_dir}"' EXIT

    cat > "${tmp_dir}/pwm_xfr.c" <<EOF
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
#include "${ROOT_DIR}/hal/stc8h_pwm.c"
EOF
    sdcc -mmcs51 --std-sdcc11 -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/hal" \
        -c -o "${tmp_dir}/pwm_xfr.rel" "${tmp_dir}/pwm_xfr.c"

    awk '/^_stc8h_pwm_set_duty:/{in_func=1; next} /^_stc8h_pwm_enable:/{in_func=0} in_func {print}' \
        "${tmp_dir}/pwm_xfr.asm" > "${tmp_dir}/pwm_set_duty.asm"
    if ! grep -Eq 'orl[[:space:]]+_P_SW2,#0x80' "${tmp_dir}/pwm_set_duty.asm"; then
        echo "PWM set_duty does not enable XFR access before CCR writes" >&2
        exit 1
    fi

    awk '/^_stc8h_pwm_enable:/{in_func=1; next} /^_stc8h_pwm_disable:/{in_func=0} in_func {print}' \
        "${tmp_dir}/pwm_xfr.asm" > "${tmp_dir}/pwm_enable.asm"
    if ! grep -Eq 'orl[[:space:]]+_P_SW2,#0x80' "${tmp_dir}/pwm_enable.asm"; then
        echo "PWM enable does not enable XFR access before ENO/CCER/CR1 writes" >&2
        exit 1
    fi

    awk '/^_stc8h_pwm_disable:/{in_func=1; next} in_func {print}' \
        "${tmp_dir}/pwm_xfr.asm" > "${tmp_dir}/pwm_disable.asm"
    if ! grep -Eq 'orl[[:space:]]+_P_SW2,#0x80' "${tmp_dir}/pwm_disable.asm"; then
        echo "PWM disable does not enable XFR access before ENO/CCER/CR1 writes" >&2
        exit 1
    fi
}

check_uart2_uart3_trim() {
    tmp_dir=$(mktemp -d)
    trap 'rm -rf "${tmp_dir}"' EXIT

    cat > "${tmp_dir}/uart1_only.c" <<EOF
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
#define STC8H_UART_ASSUME_UART1 1
#include "${ROOT_DIR}/hal/stc8h_uart.c"
void main(void)
{
    (void)stc8h_uart_init(STC8H_UART1);
}
EOF
    sdcc -mmcs51 --std-sdcc11 -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/hal" \
        -c -o "${tmp_dir}/uart1_only.rel" "${tmp_dir}/uart1_only.c"
    if grep -E "S2CON|S3CON|_stc8h_uart2|_stc8h_uart3" \
        "${tmp_dir}/uart1_only.asm" "${tmp_dir}/uart1_only.sym"; then
        echo "UART2/UART3 symbols leaked into UART1-only build" >&2
        exit 1
    fi
}

check_sdcc_interrupt_using
check_eeprom_api_trim
check_ec11_small_isr_api
check_spi_miso_input_codegen
check_pwm_xfr_codegen
check_uart2_uart3_trim
sh "${ROOT_DIR}/tools/check_host_tests.sh"

for ini in "${ROOT_DIR}"/examples/platformio/*/platformio.ini; do
    run_platformio_example "examples/platformio/$(basename "$(dirname "${ini}")")"
done

sh "${ROOT_DIR}/tools/check_nrf24_examples.sh"

run_platformio_env "examples/platformio/delay_us_probe" "pulse_1687"
run_platformio_env "examples/platformio/delay_us_probe" "pulse_2250"
run_platformio_env "examples/platformio/delay_us_probe" "pulse_4500"
run_platformio_env "examples/platformio/delay_us_probe" "pulse_9000"

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

check_global_sym_absent \
    "examples/platformio/rf_link_nrf24_small/.pio/build/STC8H1K08/src/drv_nrf24l01_wrap.sym" \
    "_drv_nrf24l01_read_fifo_status" "_drv_nrf24l01_read_observe_tx" \
    "_drv_nrf24l01_read_status" "_drv_nrf24l01_enter_rx" \
    "_drv_nrf24l01_enter_standby" "_drv_nrf24l01_read_payload" \
    "_drv_nrf24l01_write_payload" "_drv_nrf24l01_read_payload_fixed" \
    "_drv_nrf24l01_write_payload_fixed" \
    "_drv_nrf24l01_read_reg" "_drv_nrf24l01_write_reg" "_drv_nrf24l01_command" \
    "_drv_nrf24l01_set_address_width" "_drv_nrf24l01_set_tx_address" \
    "_drv_nrf24l01_set_rx_address" "_drv_nrf24l01_set_payload_size" \
    "_drv_nrf24l01_enable_dynamic_payload" "_drv_nrf24l01_disable_dynamic_payload" \
    "_drv_nrf24l01_enable_ack_payload" "_drv_nrf24l01_disable_ack_payload" \
    "_drv_nrf24l01_read_dynamic_payload_size" "_drv_nrf24l01_write_ack_payload"

check_sym_absent \
    "examples/platformio/rf_link_nrf24_small/.pio/build/STC8H1K08/src/drv_nrf24l01_wrap.sym" \
    "_drv_nrf24l01_read_buf" "_drv_nrf24l01_write_buf"

check_global_sym_absent \
    "examples/platformio/rf_link_nrf24_small/.pio/build/STC8H1K08/src/proto_rf_link_wrap.sym" \
    "_proto_rf_link_init" "_proto_rf_link_set_ids" "_proto_rf_link_connect" \
    "_proto_rf_link_send_data" "_proto_rf_link_send_data_fixed" \
    "_proto_rf_link_poll" "_proto_rf_link_poll_data_fixed" \
    "_proto_rf_link_reset" "_proto_rf_link_tick" "_proto_rf_link_get_state"

check_map_absent \
    "examples/platformio/rf_link_nrf24_small/.pio/build/STC8H1K08/firmware.map" \
    "_stc8h_spi_write"
check_no_gptr_in_tree \
    "examples/platformio/rf_link_nrf24_small/.pio/build/STC8H1K08/src"

check_map_absent \
    "examples/platformio/tm1637_number/.pio/build/STC8H1K08/firmware.map" \
    "_stc8h_uart_write " "_drv_tm1637_set_display" "_drv_tm1637_display_raw " "_drv_tm1637_display_digits" \
    "_drv_tm1637_display_number" "_drv_tm1637_clear"
check_no_gptr_in_tree \
    "examples/platformio/tm1637_number/.pio/build/STC8H1K08/src"

check_map_absent \
    "examples/platformio/pwm_pwma_pwmb_small/.pio/build/STC8H1K08/firmware.map" \
    "_stc8h_pwm_disable"

check_map_absent \
    "examples/platformio/pwm_pwma_pwmb_small/.pio/build/STC8H1K08/firmware.mem" \
    "Could not get" "DSEG.*overflow" "OSEG.*overflow"

check_ota_app_base \
    "examples/platformio/h8k64u_ota_min_app/.pio/build/STC8H8K64U/firmware.map"

check_ota_bootloader_layout \
    "examples/platformio/h8k64u_rs485_ota_bootloader/.pio/build/STC8H8K64U/firmware.map"

# Hard ROM ceilings for the "small" fixed-path examples. The guards
# catch regressions in:
#   - STC8H_PWM_TRACK_PERIOD_PRESCALER / SET_DUTY_CHANNEL_CHECK / CLAMP
#   - PROTO_RF_LINK_INCLUDE_TIMEOUT_FIELDS
#   - PROTO_RF_LINK_ENABLE_SEND_DATA_FIXED_TRACK_ACK / POLL_DATA_FIXED_TRACK_LINK
#   - DRV_EC11_ENABLE_NULL_CHECK
#   - nRF24 matrix diagnostics drifting back near the 8KB STC8H1K08 limit
# Raise the ceiling deliberately if a real feature has been added.
check_mem_rom_at_most \
    "examples/platformio/pwm_pwma_pwmb_small/.pio/build/STC8H1K08/firmware.mem" \
    2450 \
    "pwm_pwma_pwmb_small"
check_mem_rom_at_most \
    "examples/platformio/rf_link_nrf24_small/.pio/build/STC8H1K08/firmware.mem" \
    2080 \
    "rf_link_nrf24_small"
check_mem_rom_at_most \
    "examples/platformio/nrf24_pair_diag/.pio/build/ptx_matrix_fast/firmware.mem" \
    7900 \
    "nrf24_pair_diag:ptx_matrix_fast"
check_mem_rom_at_most \
    "examples/platformio/nrf24_pair_diag/.pio/build/prx_matrix_fast/firmware.mem" \
    7600 \
    "nrf24_pair_diag:prx_matrix_fast"

if grep -Eq '\(stc8h_u32\)1u? *<< *rx->bit_index' "${ROOT_DIR}/drivers/drv_ir_rx.c"; then
    echo "forbidden variable u32 shift found in drivers/drv_ir_rx.c" >&2
    exit 1
fi

echo "example build and symbol checks passed"
