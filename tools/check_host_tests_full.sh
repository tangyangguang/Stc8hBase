#!/bin/sh
set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD_DIR=${TMPDIR:-/tmp}/stc8hbase-host-full

rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"

sh "${ROOT_DIR}/tools/check_host_tests.sh"

compile_sdcc() {
    label=$1
    source=$2
    output=$3

    echo "== sdcc: ${label}"
    sdcc -mmcs51 --std-sdcc11 \
        -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/hal" -I"${ROOT_DIR}/drivers" \
        -I"${ROOT_DIR}/protocols" -I"${ROOT_DIR}/utils" \
        -c -o "${output}" "${source}"
}

expect_sdcc_fail() {
    label=$1
    source=$2
    output=$3

    echo "== sdcc expect fail: ${label}"
    if sdcc -mmcs51 --std-sdcc11 \
        -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/hal" -I"${ROOT_DIR}/drivers" \
        -I"${ROOT_DIR}/protocols" -I"${ROOT_DIR}/utils" \
        -c -o "${output}" "${source}" > "${output}.log" 2>&1; then
        echo "${label} unexpectedly compiled" >&2
        exit 1
    fi
}

check_no_gptr() {
    asm_file=$1
    label=$2

    if grep -Eq '__gptr(get|put)' "${asm_file}"; then
        echo "generic pointer helper found in ${label}" >&2
        exit 1
    fi
}

check_util_crc_xdata() {
    source="${BUILD_DIR}/util_crc_xdata.c"
    cat > "${source}" <<EOF
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
    compile_sdcc "util crc xdata api" "${source}" "${BUILD_DIR}/util_crc_xdata.rel"
    check_no_gptr "${BUILD_DIR}/util_crc_xdata.asm" "util_crc_xdata"
}

check_nrf24_code_xdata_apis() {
    source="${BUILD_DIR}/nrf24_code_xdata.c"
    cat > "${source}" <<EOF
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
#define STC8H_SYSCLK_HZ 11059200UL
#define DRV_NRF24L01_ENABLE_CHECK_PRESENT 0
#define DRV_NRF24L01_ENABLE_ARG_CHECK 0
#define DRV_NRF24L01_ENABLE_ADDRESS_API 0
#define DRV_NRF24L01_ENABLE_PIPE0_FIXED_API 0
#define DRV_NRF24L01_ENABLE_RAW_API 0
#define DRV_NRF24L01_ENABLE_POWER_DOWN 0
#define DRV_NRF24L01_ENABLE_ENTER_STANDBY 0
#define DRV_NRF24L01_ENABLE_ENTER_RX 0
#define DRV_NRF24L01_ENABLE_READ_PAYLOAD 0
#define DRV_NRF24L01_ENABLE_WRITE_PAYLOAD 0
#define DRV_NRF24L01_ENABLE_FIXED_PAYLOAD_API 0
#define DRV_NRF24L01_ENABLE_XDATA_PAYLOAD_API 1
#define DRV_NRF24L01_ENABLE_CODE_ADDRESS_API 1
#define DRV_NRF24L01_ENABLE_DYNAMIC_PAYLOAD 0
#define DRV_NRF24L01_ENABLE_ACK_PAYLOAD 0
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
static const STC8H_CODE stc8h_u8 addr[5] = {1u, 2u, 3u, 4u, 5u};
void main(void)
{
    (void)drv_nrf24l01_config_pipe0_fixed_code(addr);
}
EOF
    compile_sdcc "nRF24 CODE/XDATA APIs" "${source}" "${BUILD_DIR}/nrf24_code_xdata.rel"
    check_no_gptr "${BUILD_DIR}/nrf24_code_xdata.asm" "nrf24_code_xdata"
}

check_iap_program_guards() {
    valid="${BUILD_DIR}/iap_program_h8k64u.c"
    wrong_chip="${BUILD_DIR}/iap_program_wrong_chip.c"
    low_base="${BUILD_DIR}/iap_program_low_base.c"
    overlap="${BUILD_DIR}/iap_program_overlap.c"

    cat > "${valid}" <<EOF
#define STC8H_CHIP_STC8H1K08 0
#define STC8H_CHIP_STC8H8K64U 1
#define STC8H_IAP_PROGRAM_ENABLE 1
#include "${ROOT_DIR}/hal/stc8h_iap_program.c"
EOF
    compile_sdcc "IAP program backend" "${valid}" "${BUILD_DIR}/iap_program_h8k64u.rel"

    cat > "${wrong_chip}" <<EOF
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
#define STC8H_IAP_PROGRAM_ENABLE 1
#include "${ROOT_DIR}/hal/stc8h_iap_program.c"
EOF
    expect_sdcc_fail "IAP program rejects STC8H1K08" "${wrong_chip}" "${BUILD_DIR}/iap_program_wrong_chip.rel"

    cat > "${low_base}" <<EOF
#define STC8H_CHIP_STC8H1K08 0
#define STC8H_CHIP_STC8H8K64U 1
#define STC8H_IAP_PROGRAM_ENABLE 1
#define STC8H_IAP_PROGRAM_APP_BASE 0x0100u
#include "${ROOT_DIR}/hal/stc8h_iap_program.c"
EOF
    expect_sdcc_fail "IAP program rejects low app base" "${low_base}" "${BUILD_DIR}/iap_program_low_base.rel"

    cat > "${overlap}" <<EOF
#define STC8H_CHIP_STC8H1K08 0
#define STC8H_CHIP_STC8H8K64U 1
#define STC8H_IAP_PROGRAM_ENABLE 1
#define STC8H_IAP_PROGRAM_APP_LIMIT 0xF000u
#include "${ROOT_DIR}/hal/stc8h_iap_program.c"
EOF
    expect_sdcc_fail "IAP program rejects bootloader overlap" "${overlap}" "${BUILD_DIR}/iap_program_overlap.rel"
}

check_iap_ota_params_guards() {
    valid="${BUILD_DIR}/iap_ota_params_h8k64u.c"
    wrong_chip="${BUILD_DIR}/iap_ota_params_wrong_chip.c"
    moved_a="${BUILD_DIR}/iap_ota_params_moved_a.c"

    cat > "${valid}" <<EOF
#define STC8H_CHIP_STC8H1K08 0
#define STC8H_CHIP_STC8H8K64U 1
#define STC8H_IAP_OTA_PARAMS_ENABLE 1
#include "${ROOT_DIR}/hal/stc8h_iap_ota_params.c"
EOF
    compile_sdcc "OTA parameter IAP backend" "${valid}" "${BUILD_DIR}/iap_ota_params_h8k64u.rel"

    cat > "${wrong_chip}" <<EOF
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
#define STC8H_IAP_OTA_PARAMS_ENABLE 1
#include "${ROOT_DIR}/hal/stc8h_iap_ota_params.c"
EOF
    expect_sdcc_fail "OTA parameter IAP rejects STC8H1K08" "${wrong_chip}" "${BUILD_DIR}/iap_ota_params_wrong_chip.rel"

    cat > "${moved_a}" <<EOF
#define STC8H_CHIP_STC8H1K08 0
#define STC8H_CHIP_STC8H8K64U 1
#define STC8H_IAP_OTA_PARAMS_ENABLE 1
#define STC8H_IAP_OTA_PARAM_A_BASE 0xFA00u
#include "${ROOT_DIR}/hal/stc8h_iap_ota_params.c"
EOF
    expect_sdcc_fail "OTA parameter IAP rejects moved record A" "${moved_a}" "${BUILD_DIR}/iap_ota_params_moved_a.rel"
}

check_util_crc_xdata
check_nrf24_code_xdata_apis
check_iap_program_guards
check_iap_ota_params_guards

echo "full host checks passed"
