#!/usr/bin/env sh
set -eu

usage() {
    cat <<'EOF'
Usage:
  ./tools/upload_delay_probe.sh <562|1687|2250|4500|9000> [port]

Examples:
  ./tools/upload_delay_probe.sh 562
  ./tools/upload_delay_probe.sh 2250 /dev/cu.usbserial-130

Default port:
  /dev/cu.usbserial-110

When stcgal says it is waiting for MCU power cycle, power off/on or reset the STC8H board.
EOF
}

if [ "$#" -lt 1 ] || [ "$#" -gt 2 ]; then
    usage
    exit 2
fi

width_us="$1"
port="${2:-/dev/cu.usbserial-110}"

case "$width_us" in
    562|1687|2250|4500|9000)
        env_name="pulse_${width_us}"
        ;;
    *)
        usage
        exit 2
        ;;
esac

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
root_dir=$(CDPATH= cd -- "${script_dir}/.." && pwd)
probe_dir="${root_dir}/examples/platformio/delay_us_probe"
stcgal_pkg="${HOME}/.platformio/packages/tool-stcgal"
python_bin="${PYTHON_BIN:-/opt/homebrew/bin/python3}"

if [ ! -x "$python_bin" ]; then
    python_bin="python3"
fi

if [ ! -d "$stcgal_pkg" ]; then
    echo "stcgal package not found: ${stcgal_pkg}" >&2
    echo "Install PlatformIO packages first, for example: pio run -d ${probe_dir}" >&2
    exit 1
fi

cd "$probe_dir"

echo "Building delay_us_probe ${env_name}..."
pio run -e "$env_name"

hex_file=".pio/build/${env_name}/firmware.hex"
if [ ! -f "$hex_file" ]; then
    echo "Firmware not found: ${probe_dir}/${hex_file}" >&2
    exit 1
fi

echo "Uploading ${hex_file} to ${port}..."
PYTHONPATH="$stcgal_pkg" "$python_bin" -m stcgal \
    -P stc8g \
    -p "$port" \
    -t 11059.2 \
    -a \
    -b 38400 \
    "$hex_file"
