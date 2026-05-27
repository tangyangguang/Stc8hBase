#!/bin/sh
set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD_DIR=${TMPDIR:-/tmp}/stc8hbase-host-tests

mkdir -p "${BUILD_DIR}"

run_c_test() {
    source_file=$1
    output_file="${BUILD_DIR}/$(basename "${source_file}" .c)"

    echo "== host: ${source_file}"
    cc -std=c89 -Wall -Wextra -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/drivers" -I"${ROOT_DIR}/hal" \
        "${ROOT_DIR}/${source_file}" -o "${output_file}"
    "${output_file}"
}

run_trim_compile_checks() {
    tmp_dir="${BUILD_DIR}/trim-checks"

    rm -rf "${tmp_dir}"
    mkdir -p "${tmp_dir}"

    echo "== trim: proto fixed fast path"
    cat > "${tmp_dir}/proto_fixed_fast.c" <<EOF
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

    echo "== trim: nRF24 fixed payload and direct FEATURE enable"
    cat > "${tmp_dir}/nrf24_fixed_direct.c" <<'EOF'
#include <stdio.h>

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
}

run_c_test "tests/host/test_drv_ec11_small.c"
run_c_test "tests/host/test_drv_ec11_small_full_detent.c"
run_c_test "tests/host/test_drv_ec11_small_isr.c"
run_c_test "tests/host/test_drv_nrf24l01_core.c"
run_c_test "tests/host/test_drv_nrf24l01_timing.c"
run_c_test "tests/host/test_nrf24_pair_diag_logic.c"
run_trim_compile_checks

echo "host tests passed"
