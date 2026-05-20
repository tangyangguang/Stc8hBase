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

run_c_test "tests/host/test_drv_ec11_small.c"
run_c_test "tests/host/test_drv_ec11_small_full_detent.c"
run_c_test "tests/host/test_drv_ec11_small_isr.c"
run_c_test "tests/host/test_drv_nrf24l01_timing.c"

echo "host tests passed"
