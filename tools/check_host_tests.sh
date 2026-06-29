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

echo "host tests passed"
