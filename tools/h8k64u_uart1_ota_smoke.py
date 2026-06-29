#!/usr/bin/env python3
import argparse
import binascii
import pathlib
import struct
import subprocess
import sys
import time

import serial


ROOT = pathlib.Path(__file__).resolve().parents[1]
APP_DIR = ROOT / "examples/platformio/h8k64u_ota_min_app"
BOOT_DIR = ROOT / "examples/platformio/h8k64u_uart1_ota_bootloader"

APP_BASE = 0x0200
APP_LIMIT = 0xB3FF
LOCAL_ADDR = 0x22
HOST_ADDR = 0xA5

CMD_BEGIN = 1
CMD_WRITE_BLOCK = 2
CMD_VERIFY = 3
CMD_COMMIT = 4
CMD_STATUS = 6

STATUS_OK = 0

FAIL_REASON_NAMES = {
    0: "NONE",
    1: "ARG",
    2: "MANIFEST",
    3: "ERASE",
    4: "STATE",
    5: "OFFSET",
    6: "RANGE",
    7: "DUPLICATE",
    8: "WRITE",
    9: "READ",
    10: "INCOMPLETE",
    11: "CRC",
    12: "PARAMS",
}

SOF = b"OT"
FRAME_VERSION = 1
FRAME_OVERHEAD = 16
FRAME_PAYLOAD_MAX = 128


def run(cmd, cwd):
    print("+", " ".join(cmd))
    subprocess.run(cmd, cwd=str(cwd), check=True)


def crc16_modbus(data):
    crc = 0xFFFF
    for value in data:
        crc ^= value
        for _ in range(8):
            if crc & 1:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
            crc &= 0xFFFF
    return crc


def read_ihex(path):
    memory = {}
    upper = 0
    for raw in path.read_text().splitlines():
        line = raw.strip()
        if not line:
            continue
        if not line.startswith(":"):
            raise ValueError("bad Intel HEX line")
        count = int(line[1:3], 16)
        addr = int(line[3:7], 16)
        rectype = int(line[7:9], 16)
        data = bytes.fromhex(line[9:9 + count * 2])
        got_crc = int(line[9 + count * 2:11 + count * 2], 16)
        total = count + (addr >> 8) + (addr & 0xFF) + rectype + sum(data) + got_crc
        if (total & 0xFF) != 0:
            raise ValueError("bad Intel HEX checksum")
        if rectype == 0x00:
            base = upper + addr
            for i, value in enumerate(data):
                memory[base + i] = value
        elif rectype == 0x01:
            break
        elif rectype == 0x04:
            upper = int.from_bytes(data, "big") << 16
        else:
            continue
    return memory


def load_app_image(path):
    memory = read_ihex(path)
    app_addrs = [addr for addr in memory if APP_BASE <= addr <= APP_LIMIT]
    if not app_addrs:
        raise ValueError("app image has no bytes in OTA app range")
    min_addr = min(app_addrs)
    max_addr = max(app_addrs)
    if min_addr < APP_BASE:
        raise ValueError("app image starts below OTA app base")
    image = bytearray([0xFF] * (max_addr - APP_BASE + 1))
    for addr in app_addrs:
        image[addr - APP_BASE] = memory[addr]
    return bytes(image)


def encode_manifest(image, version=(1, 0, 0)):
    body = bytearray(31)
    struct.pack_into("<IBHHHHHII", body, 0,
                     0x4F544131,
                     1,
                     0x0864,
                     0,
                     0,
                     0,
                     APP_BASE,
                     len(image),
                     binascii.crc32(image) & 0xFFFFFFFF)
    body[23] = version[0]
    body[24] = version[1]
    body[25] = version[2]
    body[26] = 1
    struct.pack_into("<H", body, 27, 0)
    struct.pack_into("<H", body, 29, crc16_modbus(body[:-2]))
    return bytes(body)


def build_frame(dst, src, cmd, seq, offset, payload=b""):
    if len(payload) > FRAME_PAYLOAD_MAX:
        raise ValueError("payload too large")
    frame = bytearray(FRAME_OVERHEAD + len(payload))
    frame[0:2] = SOF
    frame[2] = FRAME_VERSION
    frame[3] = dst
    frame[4] = src
    frame[5] = cmd
    struct.pack_into("<H", frame, 6, seq)
    struct.pack_into("<I", frame, 8, offset)
    struct.pack_into("<H", frame, 12, len(payload))
    frame[14:14 + len(payload)] = payload
    struct.pack_into("<H", frame, len(frame) - 2, crc16_modbus(frame[:-2]))
    return bytes(frame)


def parse_frame(frame, local_addr):
    if len(frame) < FRAME_OVERHEAD:
        return None
    if frame[0:2] != SOF or frame[2] != FRAME_VERSION:
        return None
    payload_len = struct.unpack_from("<H", frame, 12)[0]
    if len(frame) != FRAME_OVERHEAD + payload_len:
        return None
    if crc16_modbus(frame[:-2]) != struct.unpack_from("<H", frame, len(frame) - 2)[0]:
        return None
    if frame[3] != local_addr:
        return None
    return {
        "dst": frame[3],
        "src": frame[4],
        "cmd": frame[5],
        "seq": struct.unpack_from("<H", frame, 6)[0],
        "offset": struct.unpack_from("<I", frame, 8)[0],
        "payload": frame[14:-2],
    }


def read_status(ser, seq, request_cmd, timeout, rx_buf):
    deadline = time.time() + timeout
    while time.time() < deadline:
        data = ser.read(256)
        if data:
            rx_buf.extend(data)
        while len(rx_buf) >= FRAME_OVERHEAD:
            if rx_buf[0] != SOF[0]:
                del rx_buf[0]
                continue
            if len(rx_buf) >= 2 and rx_buf[1] != SOF[1]:
                del rx_buf[0]
                continue
            if len(rx_buf) < 14:
                break
            payload_len = struct.unpack_from("<H", rx_buf, 12)[0]
            total_len = FRAME_OVERHEAD + payload_len
            if payload_len > FRAME_PAYLOAD_MAX:
                del rx_buf[0]
                continue
            if len(rx_buf) < total_len:
                break
            raw = bytes(rx_buf[:total_len])
            del rx_buf[:total_len]
            frame = parse_frame(raw, HOST_ADDR)
            if frame is None:
                continue
            if frame["cmd"] != CMD_STATUS or frame["seq"] != seq:
                continue
            payload = frame["payload"]
            if len(payload) < 4 or payload[0] != request_cmd:
                raise RuntimeError(f"bad status payload for seq {seq}: {payload.hex()}")
            return payload
    raise TimeoutError(f"timeout waiting status for seq {seq}")


def send_command(ser, seq, cmd, offset=0, payload=b"", timeout=3.0, rx_buf=None):
    if rx_buf is None:
        rx_buf = bytearray()
    ser.write(build_frame(LOCAL_ADDR, HOST_ADDR, cmd, seq, offset, payload))
    ser.flush()
    status = read_status(ser, seq, cmd, timeout, rx_buf)
    if status[1] != STATUS_OK:
        reason_name = FAIL_REASON_NAMES.get(status[3], "UNKNOWN")
        extra = ""
        if len(status) >= 8:
            app_size_low = status[4] | (status[5] << 8)
            request_len = status[6] | (status[7] << 8)
            extra = f" app_size_low16={app_size_low} request_len={request_len}"
        raise RuntimeError(
            f"command {cmd} failed: status={status[1]} ota_state={status[2]} "
            f"reason={status[3]}({reason_name}){extra}"
        )
    return status


def wait_boot_banner(ser, timeout=5.0):
    deadline = time.time() + timeout
    buf = bytearray()
    while time.time() < deadline:
        data = ser.read(256)
        if data:
            buf.extend(data)
            if b"BOOT" in buf:
                print("bootloader banner detected")
                return
    if buf:
        print(buf.decode("ascii", errors="replace"))
    raise TimeoutError("timeout waiting UART1 OTA bootloader banner")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", default="/dev/cu.usbserial-110")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--chunk", type=int, default=64)
    parser.add_argument("--app-env", default="STC8H8K64U_mark_valid_iap")
    parser.add_argument("--skip-upload", action="store_true")
    args = parser.parse_args()

    if args.chunk <= 0 or args.chunk > FRAME_PAYLOAD_MAX:
        raise SystemExit("--chunk must be 1..128")

    app_hex = APP_DIR / f".pio/build/{args.app_env}/firmware.hex"

    run(["pio", "run", "-e", args.app_env], APP_DIR)
    run(["pio", "run"], BOOT_DIR)
    image = load_app_image(app_hex)
    manifest = encode_manifest(image)
    print(f"app image: {len(image)} bytes, crc32=0x{binascii.crc32(image) & 0xFFFFFFFF:08X}")

    if not args.skip_upload:
        run(["pio", "run", "-t", "upload", "--upload-port", args.port], BOOT_DIR)

    time.sleep(0.5)
    with serial.Serial(args.port, args.baud, timeout=0.05, write_timeout=2) as ser:
        wait_boot_banner(ser)
        ser.reset_input_buffer()
        rx_buf = bytearray()
        seq = 1
        print("ota begin")
        send_command(ser, seq, CMD_BEGIN, payload=manifest, timeout=8.0, rx_buf=rx_buf)
        seq += 1

        offset = 0
        while offset < len(image):
            chunk = image[offset:offset + args.chunk]
            send_command(ser, seq, CMD_WRITE_BLOCK, offset=offset, payload=chunk, timeout=3.0, rx_buf=rx_buf)
            offset += len(chunk)
            seq += 1
            if offset % 1024 == 0 or offset == len(image):
                print(f"wrote {offset}/{len(image)}")

        print("ota verify")
        send_command(ser, seq, CMD_VERIFY, timeout=8.0, rx_buf=rx_buf)
        seq += 1

        print("ota commit")
        send_command(ser, seq, CMD_COMMIT, timeout=8.0, rx_buf=rx_buf)

        end = time.time() + 6.0
        text = bytearray(rx_buf)
        rx_buf.clear()
        while time.time() < end:
            data = ser.read(256)
            if data:
                text.extend(data)
                if b"H8K64U OTA app" in text:
                    break
        decoded = text.decode("ascii", errors="replace")
        print(decoded)
        if "H8K64U OTA app" not in decoded:
            raise RuntimeError("app did not print expected validation banner")

    print("UART1 OTA smoke passed")


if __name__ == "__main__":
    main()
