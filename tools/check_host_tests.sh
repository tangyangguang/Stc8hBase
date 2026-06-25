#!/bin/sh
set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD_DIR=${TMPDIR:-/tmp}/stc8hbase-host-tests

mkdir -p "${BUILD_DIR}"

run_c_test() {
    source_file=$1
    output_file="${BUILD_DIR}/$(basename "${source_file}" .c)"

    echo "== host: ${source_file}"
    cc -std=c89 -Wall -Wextra \
        -DSTC8H_CHIP_STC8H1K08=1 -DSTC8H_CHIP_STC8H8K64U=0 \
        -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/drivers" -I"${ROOT_DIR}/hal" \
        "${ROOT_DIR}/${source_file}" -o "${output_file}"
    "${output_file}"
}

run_c_test_h8k64u() {
    source_file=$1
    output_file="${BUILD_DIR}/$(basename "${source_file}" .c)-h8k64u"

    echo "== host h8k64u: ${source_file}"
    cc -std=c89 -Wall -Wextra \
        -DSTC8H_CHIP_STC8H1K08=0 -DSTC8H_CHIP_STC8H8K64U=1 \
        -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/drivers" -I"${ROOT_DIR}/hal" \
        -I"${ROOT_DIR}/protocols" -I"${ROOT_DIR}/utils" \
        "${ROOT_DIR}/${source_file}" -o "${output_file}"
    "${output_file}"
}

run_util_crc_compile_checks() {
    tmp_dir="${BUILD_DIR}/util-crc-checks"

    rm -rf "${tmp_dir}"
    mkdir -p "${tmp_dir}"

    echo "== trim: util crc default no xdata api"
    cat > "${tmp_dir}/util_crc_default.c" <<EOF
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
#include "${ROOT_DIR}/utils/util_crc.c"
EOF
    cc -std=c89 -Wall -Wextra -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/utils" \
        -c "${tmp_dir}/util_crc_default.c" -o "${tmp_dir}/util_crc_default.o"
    if nm "${tmp_dir}/util_crc_default.o" | grep -q 'util_crc16_modbus_xdata'; then
        echo "util_crc16_modbus_xdata emitted when UTIL_CRC16_MODBUS_ENABLE_XDATA is off" >&2
        exit 1
    fi

    echo "== sdcc: util crc xdata api"
    cat > "${tmp_dir}/util_crc_xdata.c" <<EOF
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
#define UTIL_CRC16_MODBUS_ENABLE_XDATA 1
#include "${ROOT_DIR}/utils/util_crc.c"
static STC8H_XDATA stc8h_u8 bytes[2] = {0x12u, 0x34u};
void main(void)
{
    (void)util_crc16_modbus_xdata(bytes, 2u);
}
EOF
    sdcc -mmcs51 --std-sdcc11 \
        -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/utils" \
        -c -o "${tmp_dir}/util_crc_xdata.rel" "${tmp_dir}/util_crc_xdata.c"
    check_no_gptr_in_body \
        "${tmp_dir}/util_crc_xdata.asm" \
        "util_crc16_modbus_xdata" \
        "${tmp_dir}/util_crc_xdata.body"
}

run_sdcc_compile_h8k64u() {
    source_file=$1
    output_file=${2:-"${BUILD_DIR}/$(basename "${source_file}" .c)-h8k64u.rel"}

    echo "== sdcc h8k64u: ${source_file}"
    sdcc -mmcs51 --std-sdcc11 \
        -DSTC8H_CHIP_STC8H1K08=0 \
        -DSTC8H_CHIP_STC8H8K64U=1 \
        -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/hal" -I"${ROOT_DIR}/drivers" \
        -I"${ROOT_DIR}/protocols" -I"${ROOT_DIR}/utils" \
        -c -o "${output_file}" "${source_file}"
}

extract_sdcc_function_body() {
    asm_file=$1
    function_name=$2
    output_file=$3

    awk -v label="_${function_name}:" '
        $0 == label { in_func = 1; print; next }
        in_func && /^_[A-Za-z0-9_]+:$/ { exit }
        in_func { print }
    ' "${asm_file}" > "${output_file}"

    if [ ! -s "${output_file}" ]; then
        echo "function ${function_name} not found in ${asm_file}" >&2
        exit 1
    fi
}

check_no_gptr_in_body() {
    asm_file=$1
    function_name=$2
    body_file=$3

    extract_sdcc_function_body "${asm_file}" "${function_name}" "${body_file}"
    if grep -Eq '__gptr(get|put)' "${body_file}"; then
        echo "${function_name} emitted generic pointer helpers" >&2
        exit 1
    fi
}

count_sdcc_body_lines() {
    body_file=$1

    grep -Ev '^[[:space:]]*($|;)' "${body_file}" | wc -l | tr -d ' '
}

run_trim_compile_checks() {
    tmp_dir="${BUILD_DIR}/trim-checks"

    rm -rf "${tmp_dir}"
    mkdir -p "${tmp_dir}"

    echo "== trim: proto fixed fast path"
    cat > "${tmp_dir}/proto_fixed_fast.c" <<EOF
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
#define PROTO_RF_LINK_ENABLE_RESET 0
#define PROTO_RF_LINK_ENABLE_TICK 0
#define PROTO_RF_LINK_ENABLE_CONNECT 0
#define PROTO_RF_LINK_ENABLE_SEND_DATA 0
#define PROTO_RF_LINK_ENABLE_SEND_DATA_FIXED 1
#define PROTO_RF_LINK_ENABLE_SEND_DATA_FIXED_FAST_PATH 1
#define PROTO_RF_LINK_ENABLE_PACKET_ARG_CHECK 0
#define PROTO_RF_LINK_ENABLE_INIT_TIMEOUT_FIELDS 0
#define PROTO_RF_LINK_INCLUDE_TIMEOUT_FIELDS 0
#define PROTO_RF_LINK_TRACK_STATE 0
#define PROTO_RF_LINK_TRACK_SEQ_RX 0
#define PROTO_RF_LINK_TRACK_ACK_PENDING 0
#define PROTO_RF_LINK_ENABLE_SEND_STATUS 0
#define PROTO_RF_LINK_ENABLE_SEND_HEARTBEAT 0
#define PROTO_RF_LINK_ENABLE_POLL 0
#define PROTO_RF_LINK_ENABLE_GET_STATE 0
#include "${ROOT_DIR}/protocols/proto_rf_link.c"
EOF
    cc -std=c89 -Wall -Wextra -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/protocols" \
        -c "${tmp_dir}/proto_fixed_fast.c" -o "${tmp_dir}/proto_fixed_fast.o"
    if nm "${tmp_dir}/proto_fixed_fast.o" | grep -Eq 'proto_rf_link_(clear_packet|make_packet)'; then
        echo "proto fixed fast path still emits generic packet helpers" >&2
        exit 1
    fi

    echo "== trim: ADC channel check off"
    cat > "${tmp_dir}/adc_no_channel_check.c" <<EOF
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
#define STC8H_ADC_ENABLE_CHANNEL_CHECK 0
#include "${ROOT_DIR}/hal/stc8h_adc.c"
EOF
    sdcc -mmcs51 --std-sdcc11 -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/hal" \
        -c -o "${tmp_dir}/adc_no_channel_check.rel" "${tmp_dir}/adc_no_channel_check.c"
    if grep -q '_stc8h_adc_channel_valid' "${tmp_dir}/adc_no_channel_check.sym"; then
        echo "ADC channel validator emitted when STC8H_ADC_ENABLE_CHANNEL_CHECK=0" >&2
        exit 1
    fi

    echo "== trim: PWM fixed-only API"
    cat > "${tmp_dir}/pwm_fixed_only.c" <<EOF
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
#define STC8H_PWM_ENABLE_GENERIC_API 0
#define STC8H_PWM_ENABLE_FIXED_CHANNEL_API 1
#define STC8H_PWM_GROUP_MASK 0x03u
#define STC8H_PWM_A_CHANNEL_MASK 0x01u
#define STC8H_PWM_B_CHANNEL_MASK 0x0Eu
#define STC8H_PWM_ENABLE_DISABLE 0
#define STC8H_PWM_ENABLE_SET_DUTY_CHANNEL_CHECK 0
#define STC8H_PWM_ENABLE_SET_DUTY_CLAMP 0
#define STC8H_PWM_TRACK_PERIOD_PRESCALER 0
#include "${ROOT_DIR}/hal/stc8h_pwm.c"
void main(void)
{
    stc8h_pwm_set_prescaler_a(0u);
    stc8h_pwm_set_period_a(4095u);
    stc8h_pwm_init_a1(STC8H_PWM_PIN_PWM1_P10);
    stc8h_pwm_set_duty_a1(300u);
    stc8h_pwm_enable_a1();
    stc8h_pwm_set_prescaler_b(0u);
    stc8h_pwm_set_period_b(255u);
    stc8h_pwm_init_b6(STC8H_PWM_PIN_PWM6_P54);
    stc8h_pwm_init_b7(STC8H_PWM_PIN_PWM7_P33);
    stc8h_pwm_init_b8(STC8H_PWM_PIN_PWM8_P34);
    stc8h_pwm_set_duty_b6(0u);
    stc8h_pwm_set_duty_b7(100u);
    stc8h_pwm_set_duty_b8(100u);
    stc8h_pwm_enable_b6();
    stc8h_pwm_enable_b7();
    stc8h_pwm_enable_b8();
}
EOF
    sdcc -mmcs51 --std-sdcc11 -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/hal" \
        -c -o "${tmp_dir}/pwm_fixed_only.rel" "${tmp_dir}/pwm_fixed_only.c"
    if grep -Eq '_stc8h_pwm_(set_prescaler|set_period|init_channel|set_duty|enable)($|[[:space:]])' "${tmp_dir}/pwm_fixed_only.sym"; then
        echo "generic PWM API emitted in fixed-only trim check" >&2
        exit 1
    fi
    check_no_gptr_in_body "${tmp_dir}/pwm_fixed_only.asm" \
        "stc8h_pwm_write16" "${tmp_dir}/pwm_write16.body"

    echo "== trim: nRF24 fixed payload and direct FEATURE enable"
    cat > "${tmp_dir}/nrf24_fixed_direct.c" <<'EOF'
#include <stdio.h>

#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
#define STC8H_SYSCLK_HZ 11059200UL
#define DRV_NRF24L01_ENABLE_ARG_CHECK 0
#define DRV_NRF24L01_ENABLE_READ_STATUS 0
#define DRV_NRF24L01_ENABLE_READ_FIFO_STATUS 0
#define DRV_NRF24L01_ENABLE_READ_OBSERVE_TX 0
#define DRV_NRF24L01_ENABLE_CHECK_PRESENT 0
#define DRV_NRF24L01_ENABLE_ADDRESS_API 0
#define DRV_NRF24L01_ENABLE_RAW_API 1
#define DRV_NRF24L01_ENABLE_RX_PIPE_API 0
#define DRV_NRF24L01_ENABLE_POWER_DOWN 0
#define DRV_NRF24L01_ENABLE_ENTER_STANDBY 0
#define DRV_NRF24L01_ENABLE_ENTER_RX 0
#define DRV_NRF24L01_ENABLE_READ_PAYLOAD 0
#define DRV_NRF24L01_ENABLE_WRITE_PAYLOAD 0
#define DRV_NRF24L01_ENABLE_FIXED_PAYLOAD_API 1
#define DRV_NRF24L01_FIXED_PAYLOAD_SIZE 11u
#define DRV_NRF24L01_ENABLE_DYNAMIC_PAYLOAD 0
#define DRV_NRF24L01_ENABLE_ACK_PAYLOAD 1
#define DRV_NRF24L01_ENABLE_WRITE_ACK_PAYLOAD 0
#define DRV_NRF24L01_ENABLE_DISABLE_ACK_PAYLOAD 0
#define DRV_NRF24L01_ENABLE_READ_DYNAMIC_PAYLOAD_SIZE 0
#define DRV_NRF24L01_ENABLE_TX_RESULT_API 0
#define DRV_NRF24L01_ENABLE_RX_PACKET_API 0
#define DRV_NRF24L01_ENABLE_ACK_PRELOAD_API 0
#define DRV_NRF24L01_ENABLE_RECOVER 0
#define DRV_NRF24L01_FEATURE_ENABLE_DIRECT_WRITE 1
#define DRV_NRF24L01_CE_HIGH() do { } while (0)
#define DRV_NRF24L01_CE_LOW() do { } while (0)
#define DRV_NRF24L01_CSN_HIGH() do { } while (0)
#define DRV_NRF24L01_CSN_LOW() do { last_len = 0u; } while (0)
#define DRV_NRF24L01_CONFIGURE_PINS() do { } while (0)
#define DRV_NRF24L01_POWER_UP_DELAY() do { } while (0)
#define DRV_NRF24L01_CE_PULSE_DELAY() do { } while (0)

static unsigned char last_cmd;
static unsigned char last_len;
static unsigned char feature_value;
static unsigned char dynpd_value;

unsigned char stc8h_spi_transfer(unsigned char value)
{
    if (last_len == 0u) {
        last_cmd = value;
        last_len = 1u;
        return 0x0Eu;
    }
    ++last_len;
    if (last_cmd == 0x3Du) {
        feature_value = value;
    }
    if (last_cmd == 0x3Cu) {
        dynpd_value = value;
    }
    return 0x0Eu;
}

#include "drv_nrf24l01.c"

int main(void)
{
    unsigned char payload[11] = {0};

    last_len = 0u;
    if (drv_nrf24l01_enable_ack_payload(DRV_NRF24L01_PIPE0) != STC8H_OK) {
        puts("enable_ack_payload must succeed in direct-write mode");
        return 1;
    }
    if ((feature_value != 0x06u) || (dynpd_value != DRV_NRF24L01_PIPE0)) {
        puts("direct FEATURE/DYNPD writes did not use expected values");
        return 1;
    }

    last_len = 0u;
    (void)drv_nrf24l01_write_payload_fixed(payload);
    if ((last_cmd != 0xA0u) || (last_len != 12u)) {
        puts("fixed payload write must send W_TX_PAYLOAD plus fixed bytes");
        return 1;
    }
    return 0;
}
EOF
    cc -std=c89 -Wall -Wextra -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/drivers" -I"${ROOT_DIR}/hal" \
        "${tmp_dir}/nrf24_fixed_direct.c" -o "${tmp_dir}/nrf24_fixed_direct"
    "${tmp_dir}/nrf24_fixed_direct"

    echo "== codegen: proto XDATA fixed APIs"
    cat > "${tmp_dir}/proto_xdata_only.c" <<EOF
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
#define PROTO_RF_LINK_ENABLE_INIT 0
#define PROTO_RF_LINK_ENABLE_SET_IDS 0
#define PROTO_RF_LINK_ENABLE_RESET 0
#define PROTO_RF_LINK_ENABLE_TICK 0
#define PROTO_RF_LINK_ENABLE_CONNECT 0
#define PROTO_RF_LINK_ENABLE_SEND_DATA 0
#define PROTO_RF_LINK_ENABLE_SEND_DATA_FIXED 0
#define PROTO_RF_LINK_ENABLE_SEND_DATA_FIXED_FAST_PATH 1
#define PROTO_RF_LINK_ENABLE_XDATA_FIXED_API 1
#define PROTO_RF_LINK_ENABLE_PACKET_ARG_CHECK 0
#define PROTO_RF_LINK_ENABLE_INIT_TIMEOUT_FIELDS 0
#define PROTO_RF_LINK_INCLUDE_TIMEOUT_FIELDS 0
#define PROTO_RF_LINK_TRACK_STATE 0
#define PROTO_RF_LINK_TRACK_SEQ_RX 0
#define PROTO_RF_LINK_TRACK_ACK_PENDING 0
#define PROTO_RF_LINK_ENABLE_SEND_STATUS 0
#define PROTO_RF_LINK_ENABLE_SEND_HEARTBEAT 0
#define PROTO_RF_LINK_ENABLE_POLL 0
#define PROTO_RF_LINK_ENABLE_POLL_DATA_FIXED 0
#define PROTO_RF_LINK_ENABLE_GET_STATE 0
#include "${ROOT_DIR}/protocols/proto_rf_link.c"
EOF
    sdcc -mmcs51 --std-sdcc11 -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/protocols" \
        -c -o "${tmp_dir}/proto_xdata_only.rel" "${tmp_dir}/proto_xdata_only.c"
    if grep -Eq '__gptr(get|put)' "${tmp_dir}/proto_xdata_only.asm"; then
        echo "proto XDATA-only fixed wrapper emitted generic pointer helpers" >&2
        exit 1
    fi

    cat > "${tmp_dir}/proto_xdata_compare.c" <<EOF
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
#define PROTO_RF_LINK_ENABLE_INIT 0
#define PROTO_RF_LINK_ENABLE_SET_IDS 0
#define PROTO_RF_LINK_ENABLE_RESET 0
#define PROTO_RF_LINK_ENABLE_TICK 0
#define PROTO_RF_LINK_ENABLE_CONNECT 0
#define PROTO_RF_LINK_ENABLE_SEND_DATA 0
#define PROTO_RF_LINK_ENABLE_SEND_DATA_FIXED 1
#define PROTO_RF_LINK_ENABLE_SEND_DATA_FIXED_FAST_PATH 1
#define PROTO_RF_LINK_ENABLE_XDATA_FIXED_API 1
#define PROTO_RF_LINK_ENABLE_PACKET_ARG_CHECK 0
#define PROTO_RF_LINK_ENABLE_INIT_TIMEOUT_FIELDS 0
#define PROTO_RF_LINK_INCLUDE_TIMEOUT_FIELDS 0
#define PROTO_RF_LINK_TRACK_STATE 0
#define PROTO_RF_LINK_TRACK_SEQ_RX 0
#define PROTO_RF_LINK_TRACK_ACK_PENDING 0
#define PROTO_RF_LINK_ENABLE_SEND_STATUS 0
#define PROTO_RF_LINK_ENABLE_SEND_HEARTBEAT 0
#define PROTO_RF_LINK_ENABLE_POLL 0
#define PROTO_RF_LINK_ENABLE_POLL_DATA_FIXED 1
#define PROTO_RF_LINK_ENABLE_GET_STATE 0
#include "${ROOT_DIR}/protocols/proto_rf_link.c"
EOF
    sdcc -mmcs51 --std-sdcc11 -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/protocols" \
        -c -o "${tmp_dir}/proto_xdata_compare.rel" "${tmp_dir}/proto_xdata_compare.c"
    check_no_gptr_in_body "${tmp_dir}/proto_xdata_compare.asm" \
        "proto_rf_link_init_xdata" "${tmp_dir}/proto_init_xdata.body"
    check_no_gptr_in_body "${tmp_dir}/proto_xdata_compare.asm" \
        "proto_rf_link_set_ids_xdata" "${tmp_dir}/proto_set_ids_xdata.body"
    check_no_gptr_in_body "${tmp_dir}/proto_xdata_compare.asm" \
        "proto_rf_link_send_data_fixed_xdata" "${tmp_dir}/proto_send_xdata.body"
    check_no_gptr_in_body "${tmp_dir}/proto_xdata_compare.asm" \
        "proto_rf_link_poll_data_fixed_xdata" "${tmp_dir}/proto_poll_xdata.body"
    extract_sdcc_function_body "${tmp_dir}/proto_xdata_compare.asm" \
        "proto_rf_link_send_data_fixed" "${tmp_dir}/proto_send_generic.body"
    generic_lines=$(count_sdcc_body_lines "${tmp_dir}/proto_send_generic.body")
    xdata_lines=$(count_sdcc_body_lines "${tmp_dir}/proto_send_xdata.body")
    if [ "${xdata_lines}" -ge "${generic_lines}" ]; then
        echo "proto XDATA fixed sender is not smaller than generic fixed sender (${xdata_lines} >= ${generic_lines})" >&2
        exit 1
    fi

    echo "== codegen: nRF24 XDATA/CODE APIs"
    cat > "${tmp_dir}/nrf24_xdata_code.c" <<EOF
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
#define STC8H_SYSCLK_HZ 11059200UL
#define DRV_NRF24L01_ENABLE_CHECK_PRESENT 0
#define DRV_NRF24L01_ENABLE_ARG_CHECK 0
#define DRV_NRF24L01_ENABLE_ADDRESS_API 0
#define DRV_NRF24L01_ENABLE_PIPE0_FIXED_API 0
#define DRV_NRF24L01_ENABLE_READ_FIFO_STATUS 0
#define DRV_NRF24L01_ENABLE_READ_OBSERVE_TX 0
#define DRV_NRF24L01_ENABLE_READ_STATUS 0
#define DRV_NRF24L01_ENABLE_RAW_API 0
#define DRV_NRF24L01_ENABLE_POWER_DOWN 0
#define DRV_NRF24L01_ENABLE_ENTER_STANDBY 0
#define DRV_NRF24L01_ENABLE_ENTER_RX 0
#define DRV_NRF24L01_ENABLE_READ_PAYLOAD 0
#define DRV_NRF24L01_ENABLE_WRITE_PAYLOAD 0
#define DRV_NRF24L01_ENABLE_FIXED_PAYLOAD_API 0
#define DRV_NRF24L01_ENABLE_XDATA_PAYLOAD_API 1
#define DRV_NRF24L01_ENABLE_CODE_ADDRESS_API 1
#define DRV_NRF24L01_FIXED_PAYLOAD_SIZE 11u
#define DRV_NRF24L01_ENABLE_DYNAMIC_PAYLOAD 0
#define DRV_NRF24L01_ENABLE_ACK_PAYLOAD 0
#define DRV_NRF24L01_ENABLE_WRITE_ACK_PAYLOAD 0
#define DRV_NRF24L01_ENABLE_DISABLE_ACK_PAYLOAD 0
#define DRV_NRF24L01_ENABLE_READ_DYNAMIC_PAYLOAD_SIZE 0
#define DRV_NRF24L01_ENABLE_TX_RESULT_API 0
#define DRV_NRF24L01_ENABLE_RX_PACKET_API 0
#define DRV_NRF24L01_ENABLE_ACK_PRELOAD_API 0
#define DRV_NRF24L01_ENABLE_RECOVER 0
#define DRV_NRF24L01_CE_HIGH() do { } while (0)
#define DRV_NRF24L01_CE_LOW() do { } while (0)
#define DRV_NRF24L01_CSN_HIGH() do { } while (0)
#define DRV_NRF24L01_CSN_LOW() do { } while (0)
#define DRV_NRF24L01_CONFIGURE_PINS() do { } while (0)
#define DRV_NRF24L01_POWER_UP_DELAY() do { } while (0)
#define DRV_NRF24L01_CE_PULSE_DELAY() do { } while (0)
#include "${ROOT_DIR}/drivers/drv_nrf24l01.c"
static const STC8H_CODE stc8h_u8 code_addr[5] = { 1u, 2u, 3u, 4u, 5u };
void main(void)
{
    (void)drv_nrf24l01_config_pipe0_fixed_code(code_addr);
}
EOF
    sdcc -mmcs51 --std-sdcc11 --Werror -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/drivers" -I"${ROOT_DIR}/hal" \
        -c -o "${tmp_dir}/nrf24_xdata_code.rel" "${tmp_dir}/nrf24_xdata_code.c"
    check_no_gptr_in_body "${tmp_dir}/nrf24_xdata_code.asm" \
        "drv_nrf24l01_write_payload_fixed_xdata" "${tmp_dir}/nrf24_write_payload_xdata.body"
    check_no_gptr_in_body "${tmp_dir}/nrf24_xdata_code.asm" \
        "drv_nrf24l01_read_payload_fixed_xdata" "${tmp_dir}/nrf24_read_payload_xdata.body"
    check_no_gptr_in_body "${tmp_dir}/nrf24_xdata_code.asm" \
        "drv_nrf24l01_config_pipe0_fixed_code" "${tmp_dir}/nrf24_config_pipe0_code.body"
    check_no_gptr_in_body "${tmp_dir}/nrf24_xdata_code.asm" \
        "drv_nrf24l01_write_buf_xdata" "${tmp_dir}/nrf24_write_buf_xdata.body"
    check_no_gptr_in_body "${tmp_dir}/nrf24_xdata_code.asm" \
        "drv_nrf24l01_read_buf_xdata" "${tmp_dir}/nrf24_read_buf_xdata.body"
    check_no_gptr_in_body "${tmp_dir}/nrf24_xdata_code.asm" \
        "drv_nrf24l01_write_buf_code" "${tmp_dir}/nrf24_write_buf_code.body"

    echo "== codegen: nRF24 check_present DATA/CODE path"
    cat > "${tmp_dir}/nrf24_check_present_spaces.c" <<EOF
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
#define STC8H_SYSCLK_HZ 11059200UL
#define DRV_NRF24L01_ENABLE_CHECK_PRESENT 1
#define DRV_NRF24L01_ENABLE_ARG_CHECK 0
#define DRV_NRF24L01_ENABLE_ADDRESS_API 0
#define DRV_NRF24L01_ENABLE_PIPE0_FIXED_API 0
#define DRV_NRF24L01_ENABLE_READ_FIFO_STATUS 0
#define DRV_NRF24L01_ENABLE_READ_OBSERVE_TX 0
#define DRV_NRF24L01_ENABLE_READ_STATUS 0
#define DRV_NRF24L01_ENABLE_RAW_API 0
#define DRV_NRF24L01_ENABLE_POWER_DOWN 0
#define DRV_NRF24L01_ENABLE_ENTER_STANDBY 0
#define DRV_NRF24L01_ENABLE_ENTER_RX 0
#define DRV_NRF24L01_ENABLE_READ_PAYLOAD 0
#define DRV_NRF24L01_ENABLE_WRITE_PAYLOAD 0
#define DRV_NRF24L01_ENABLE_FIXED_PAYLOAD_API 0
#define DRV_NRF24L01_ENABLE_XDATA_PAYLOAD_API 0
#define DRV_NRF24L01_ENABLE_CODE_ADDRESS_API 0
#define DRV_NRF24L01_ENABLE_DYNAMIC_PAYLOAD 0
#define DRV_NRF24L01_ENABLE_ACK_PAYLOAD 0
#define DRV_NRF24L01_ENABLE_WRITE_ACK_PAYLOAD 0
#define DRV_NRF24L01_ENABLE_DISABLE_ACK_PAYLOAD 0
#define DRV_NRF24L01_ENABLE_READ_DYNAMIC_PAYLOAD_SIZE 0
#define DRV_NRF24L01_ENABLE_TX_RESULT_API 0
#define DRV_NRF24L01_ENABLE_RX_PACKET_API 0
#define DRV_NRF24L01_ENABLE_ACK_PRELOAD_API 0
#define DRV_NRF24L01_ENABLE_RECOVER 0
#define DRV_NRF24L01_CE_HIGH() do { } while (0)
#define DRV_NRF24L01_CE_LOW() do { } while (0)
#define DRV_NRF24L01_CSN_HIGH() do { } while (0)
#define DRV_NRF24L01_CSN_LOW() do { } while (0)
#define DRV_NRF24L01_CONFIGURE_PINS() do { } while (0)
#define DRV_NRF24L01_POWER_UP_DELAY() do { } while (0)
#define DRV_NRF24L01_CE_PULSE_DELAY() do { } while (0)
#include "${ROOT_DIR}/drivers/drv_nrf24l01.c"
EOF
    sdcc -mmcs51 --std-sdcc11 -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/drivers" -I"${ROOT_DIR}/hal" \
        -c -o "${tmp_dir}/nrf24_check_present_spaces.rel" "${tmp_dir}/nrf24_check_present_spaces.c"
    if grep -Eq '__gptr(get|put)' "${tmp_dir}/nrf24_check_present_spaces.asm"; then
        echo "nRF24 check_present DATA/CODE path emitted generic pointer helpers" >&2
        exit 1
    fi

    echo "== codegen: TM1637 DATA raw4 API"
    cat > "${tmp_dir}/tm1637_data_raw4.c" <<EOF
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
#define DRV_TM1637_ENABLE_DISPLAY_DIGITS 0
#define DRV_TM1637_ENABLE_DISPLAY_RAW4 0
#define DRV_TM1637_ENABLE_DISPLAY_RAW4_DATA 1
#define DRV_TM1637_ENABLE_DISPLAY_RAW 0
#define DRV_TM1637_ENABLE_SET_DISPLAY 0
#define DRV_TM1637_ENABLE_BRIGHTNESS_STATE 0
#define DRV_TM1637_ENABLE_RAW_LEN_CHECK 0
#define DRV_TM1637_ENABLE_DISPLAY_NUMBER 0
#define DRV_TM1637_ENABLE_ENCODE_DIGIT 0
#define DRV_TM1637_ENABLE_CLEAR 0
static volatile unsigned char tm1637_pin_sink;
static unsigned char tm1637_dio_read(void)
{
    return tm1637_pin_sink;
}
#define BOARD_TM1637_CLK_HIGH() do { ++tm1637_pin_sink; } while (0)
#define BOARD_TM1637_CLK_LOW() do { ++tm1637_pin_sink; } while (0)
#define BOARD_TM1637_DIO_HIGH() do { ++tm1637_pin_sink; } while (0)
#define BOARD_TM1637_DIO_LOW() do { ++tm1637_pin_sink; } while (0)
#define BOARD_TM1637_DIO_READ() tm1637_dio_read()
#include "${ROOT_DIR}/drivers/drv_tm1637.c"
EOF
    sdcc -mmcs51 --std-sdcc11 -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/drivers" -I"${ROOT_DIR}/hal" \
        -c -o "${tmp_dir}/tm1637_data_raw4.rel" "${tmp_dir}/tm1637_data_raw4.c"
    check_no_gptr_in_body "${tmp_dir}/tm1637_data_raw4.asm" \
        "drv_tm1637_display_raw4_data" "${tmp_dir}/tm1637_raw4_data.body"
}

run_iap_program_compile_checks() {
    tmp_dir="${BUILD_DIR}/iap-program-checks"

    rm -rf "${tmp_dir}"
    mkdir -p "${tmp_dir}"

    echo "== sdcc h8k64u: IAP program backend"
    cat > "${tmp_dir}/iap_program_h8k64u.c" <<EOF
#define STC8H_CHIP_STC8H1K08 0
#define STC8H_CHIP_STC8H8K64U 1
#define STC8H_IAP_PROGRAM_ENABLE 1
#include "${ROOT_DIR}/hal/stc8h_iap_program.c"
EOF
    sdcc -mmcs51 --std-sdcc11 \
        -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/hal" -I"${ROOT_DIR}/drivers" \
        -I"${ROOT_DIR}/protocols" -I"${ROOT_DIR}/utils" \
        -c -o "${tmp_dir}/iap_program_h8k64u.rel" "${tmp_dir}/iap_program_h8k64u.c"

    echo "== sdcc expect fail: IAP program rejects STC8H1K08"
    cat > "${tmp_dir}/iap_program_wrong_chip.c" <<EOF
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
#define STC8H_IAP_PROGRAM_ENABLE 1
#include "${ROOT_DIR}/hal/stc8h_iap_program.c"
EOF
    if sdcc -mmcs51 --std-sdcc11 \
        -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/hal" -I"${ROOT_DIR}/drivers" \
        -I"${ROOT_DIR}/protocols" -I"${ROOT_DIR}/utils" \
        -c -o "${tmp_dir}/iap_program_wrong_chip.rel" "${tmp_dir}/iap_program_wrong_chip.c" \
        > "${tmp_dir}/iap_program_wrong_chip.log" 2>&1; then
        echo "IAP program backend compiled for STC8H1K08" >&2
        exit 1
    fi

    echo "== sdcc expect fail: IAP program rejects low app base"
    cat > "${tmp_dir}/iap_program_low_base.c" <<EOF
#define STC8H_CHIP_STC8H1K08 0
#define STC8H_CHIP_STC8H8K64U 1
#define STC8H_IAP_PROGRAM_ENABLE 1
#define STC8H_IAP_PROGRAM_APP_BASE 0x0100u
#include "${ROOT_DIR}/hal/stc8h_iap_program.c"
EOF
    if sdcc -mmcs51 --std-sdcc11 \
        -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/hal" -I"${ROOT_DIR}/drivers" \
        -I"${ROOT_DIR}/protocols" -I"${ROOT_DIR}/utils" \
        -c -o "${tmp_dir}/iap_program_low_base.rel" "${tmp_dir}/iap_program_low_base.c" \
        > "${tmp_dir}/iap_program_low_base.log" 2>&1; then
        echo "IAP program backend accepted low app base" >&2
        exit 1
    fi

    echo "== sdcc expect fail: IAP program rejects bootloader overlap"
    cat > "${tmp_dir}/iap_program_overlap.c" <<EOF
#define STC8H_CHIP_STC8H1K08 0
#define STC8H_CHIP_STC8H8K64U 1
#define STC8H_IAP_PROGRAM_ENABLE 1
#define STC8H_IAP_PROGRAM_APP_LIMIT 0xF000u
#include "${ROOT_DIR}/hal/stc8h_iap_program.c"
EOF
    if sdcc -mmcs51 --std-sdcc11 \
        -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/hal" -I"${ROOT_DIR}/drivers" \
        -I"${ROOT_DIR}/protocols" -I"${ROOT_DIR}/utils" \
        -c -o "${tmp_dir}/iap_program_overlap.rel" "${tmp_dir}/iap_program_overlap.c" \
        > "${tmp_dir}/iap_program_overlap.log" 2>&1; then
        echo "IAP program backend accepted bootloader overlap" >&2
        exit 1
    fi

    echo "== sdcc expect fail: IAP program rejects flash base above app base"
    cat > "${tmp_dir}/iap_program_bad_flash_base.c" <<EOF
#define STC8H_CHIP_STC8H1K08 0
#define STC8H_CHIP_STC8H8K64U 1
#define STC8H_IAP_PROGRAM_ENABLE 1
#define STC8H_IAP_PROGRAM_FLASH_BASE 0x0400u
#include "${ROOT_DIR}/hal/stc8h_iap_program.c"
EOF
    if sdcc -mmcs51 --std-sdcc11 \
        -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/hal" -I"${ROOT_DIR}/drivers" \
        -I"${ROOT_DIR}/protocols" -I"${ROOT_DIR}/utils" \
        -c -o "${tmp_dir}/iap_program_bad_flash_base.rel" "${tmp_dir}/iap_program_bad_flash_base.c" \
        > "${tmp_dir}/iap_program_bad_flash_base.log" 2>&1; then
        echo "IAP program backend accepted flash base above app base" >&2
        exit 1
    fi
}

run_iap_ota_params_compile_checks() {
    tmp_dir="${BUILD_DIR}/iap-ota-params-checks"

    rm -rf "${tmp_dir}"
    mkdir -p "${tmp_dir}"

    echo "== sdcc h8k64u: OTA parameter IAP backend"
    cat > "${tmp_dir}/iap_ota_params_h8k64u.c" <<EOF
#define STC8H_CHIP_STC8H1K08 0
#define STC8H_CHIP_STC8H8K64U 1
#define STC8H_IAP_OTA_PARAMS_ENABLE 1
#include "${ROOT_DIR}/hal/stc8h_iap_ota_params.c"
EOF
    sdcc -mmcs51 --std-sdcc11 \
        -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/hal" -I"${ROOT_DIR}/drivers" \
        -I"${ROOT_DIR}/protocols" -I"${ROOT_DIR}/utils" \
        -c -o "${tmp_dir}/iap_ota_params_h8k64u.rel" "${tmp_dir}/iap_ota_params_h8k64u.c"

    echo "== sdcc expect fail: OTA parameter IAP rejects STC8H1K08"
    cat > "${tmp_dir}/iap_ota_params_wrong_chip.c" <<EOF
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
#define STC8H_IAP_OTA_PARAMS_ENABLE 1
#include "${ROOT_DIR}/hal/stc8h_iap_ota_params.c"
EOF
    if sdcc -mmcs51 --std-sdcc11 \
        -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/hal" -I"${ROOT_DIR}/drivers" \
        -I"${ROOT_DIR}/protocols" -I"${ROOT_DIR}/utils" \
        -c -o "${tmp_dir}/iap_ota_params_wrong_chip.rel" "${tmp_dir}/iap_ota_params_wrong_chip.c" \
        > "${tmp_dir}/iap_ota_params_wrong_chip.log" 2>&1; then
        echo "OTA parameter IAP backend compiled for STC8H1K08" >&2
        exit 1
    fi

    echo "== sdcc expect fail: OTA parameter IAP rejects moved record A"
    cat > "${tmp_dir}/iap_ota_params_moved_a.c" <<EOF
#define STC8H_CHIP_STC8H1K08 0
#define STC8H_CHIP_STC8H8K64U 1
#define STC8H_IAP_OTA_PARAMS_ENABLE 1
#define STC8H_IAP_OTA_PARAM_A_BASE 0xFA00u
#include "${ROOT_DIR}/hal/stc8h_iap_ota_params.c"
EOF
    if sdcc -mmcs51 --std-sdcc11 \
        -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/hal" -I"${ROOT_DIR}/drivers" \
        -I"${ROOT_DIR}/protocols" -I"${ROOT_DIR}/utils" \
        -c -o "${tmp_dir}/iap_ota_params_moved_a.rel" "${tmp_dir}/iap_ota_params_moved_a.c" \
        > "${tmp_dir}/iap_ota_params_moved_a.log" 2>&1; then
        echo "OTA parameter IAP backend accepted moved record A" >&2
        exit 1
    fi

    echo "== sdcc expect fail: OTA parameter IAP rejects flash base above record A"
    cat > "${tmp_dir}/iap_ota_params_bad_flash_base.c" <<EOF
#define STC8H_CHIP_STC8H1K08 0
#define STC8H_CHIP_STC8H8K64U 1
#define STC8H_IAP_OTA_PARAMS_ENABLE 1
#define STC8H_IAP_OTA_PARAMS_FLASH_BASE 0xFE00u
#include "${ROOT_DIR}/hal/stc8h_iap_ota_params.c"
EOF
    if sdcc -mmcs51 --std-sdcc11 \
        -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/hal" -I"${ROOT_DIR}/drivers" \
        -I"${ROOT_DIR}/protocols" -I"${ROOT_DIR}/utils" \
        -c -o "${tmp_dir}/iap_ota_params_bad_flash_base.rel" "${tmp_dir}/iap_ota_params_bad_flash_base.c" \
        > "${tmp_dir}/iap_ota_params_bad_flash_base.log" 2>&1; then
        echo "OTA parameter IAP backend accepted flash base above record A" >&2
        exit 1
    fi
}

run_rs485_uart_compile_checks() {
    tmp_dir="${BUILD_DIR}/rs485-uart-checks"

    rm -rf "${tmp_dir}"
    mkdir -p "${tmp_dir}"

    echo "== sdcc: RS485 UART wrapper"
    cat > "${tmp_dir}/rs485_uart.c" <<EOF
#define STC8H_CHIP_STC8H1K08 0
#define STC8H_CHIP_STC8H8K64U 1
#define STC8H_UART_ENABLE_UART2 1
#define STC8H_UART2_BAUD 115200UL
#define BOARD_RS485_TX_ENABLE() do { } while (0)
#define BOARD_RS485_RX_ENABLE() do { } while (0)
#include "${ROOT_DIR}/drivers/drv_rs485_uart.c"
void main(void)
{
    stc8h_u8 byte;

    byte = 0x55u;
    (void)drv_rs485_uart_init(STC8H_UART2);
    (void)drv_rs485_uart_write(STC8H_UART2, &byte, 1u);
}
EOF
    sdcc -mmcs51 --std-sdcc11 \
        -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/hal" -I"${ROOT_DIR}/drivers" \
        -I"${ROOT_DIR}/protocols" -I"${ROOT_DIR}/utils" \
        -c -o "${tmp_dir}/rs485_uart.rel" "${tmp_dir}/rs485_uart.c"
}

run_c_test "tests/host/test_drv_ec11_small.c"
run_c_test "tests/host/test_drv_ec11_small_full_detent.c"
run_c_test "tests/host/test_drv_ec11_small_isr.c"
run_c_test "tests/host/test_drv_nrf24l01_core.c"
run_c_test "tests/host/test_drv_nrf24l01_timing.c"
run_c_test "tests/host/test_drv_tm1637_address_space.c"
run_c_test "tests/host/test_drv_rs485_uart.c"
run_c_test "tests/host/test_nrf24_pair_diag_logic.c"
run_c_test "tests/host/test_proto_ota_frame.c"
run_c_test "tests/host/test_proto_rf_link_address_space.c"
run_c_test_h8k64u "tests/host/test_stc8h_ota_core.c"
run_c_test_h8k64u "tests/host/test_stc8h_ota_format.c"
run_c_test_h8k64u "tests/host/test_stc8h_ota_params.c"
run_c_test "tests/host/test_util_crc.c"
run_c_test "tests/host/test_util_crc32.c"
run_util_crc_compile_checks
run_trim_compile_checks
run_iap_program_compile_checks
run_iap_ota_params_compile_checks
run_rs485_uart_compile_checks

echo "host tests passed"
