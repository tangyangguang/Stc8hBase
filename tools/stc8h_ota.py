#!/usr/bin/env python3
"""Explicit host tool for STC8H8K64U Remote OTA over a serial/RS485 port."""

import argparse
import binascii
import hashlib
import pathlib
import secrets
import struct
import sys
import time

APP_BASE = 0x6C00
APP_LIMIT = 0xEFFF
FLASH_LIMIT = 0x10000
BOOTLOADER_VERSION = 2
TARGET_CHIP = 0x0864

PACKAGE_MAGIC = b"STCO"
PACKAGE_VERSION = 1
PACKAGE_HEADER = struct.Struct("<4sBBHH32s")
MANIFEST_SIZE = 31

SOF = b"OT"
FRAME_VERSION = 1
FRAME_HEADER_SIZE = 18
FRAME_OVERHEAD = 20
FRAME_PAYLOAD_MAX = 128
HOST_ADDR = 0xA5

CMD_INFO = 1
CMD_BEGIN = 2
CMD_DATA = 3
CMD_VERIFY = 4
CMD_ACTIVATE = 5
CMD_ABORT = 6
CMD_STATUS = 7
FLAG_RESTART = 1
STATUS_OK = 0
STATUS_DUPLICATE = 2

STATE_NAMES = {
    0: "EMPTY",
    1: "APP_VALID",
    2: "UPDATE_REQUESTED",
    3: "PREPARING",
    4: "RECEIVING",
    5: "VERIFIED",
    6: "TRIAL_PENDING",
    7: "TRIAL_STARTED",
    8: "RECOVERY",
    9: "FAILED",
}

FAIL_NAMES = {
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
    13: "SESSION",
    14: "NOT_REQUESTED",
    15: "READBACK",
}


def crc16_modbus(data):
    crc = 0xFFFF
    for value in data:
        crc ^= value
        for _ in range(8):
            crc = ((crc >> 1) ^ 0xA001) if crc & 1 else (crc >> 1)
            crc &= 0xFFFF
    return crc


def read_ihex(path):
    memory = {}
    upper = 0
    for raw in path.read_text(encoding="ascii").splitlines():
        line = raw.strip()
        if not line:
            continue
        if not line.startswith(":"):
            raise ValueError("invalid Intel HEX line")
        count = int(line[1:3], 16)
        addr = int(line[3:7], 16)
        record_type = int(line[7:9], 16)
        data = bytes.fromhex(line[9:9 + count * 2])
        checksum = int(line[9 + count * 2:11 + count * 2], 16)
        if (count + (addr >> 8) + (addr & 0xFF) + record_type + sum(data) + checksum) & 0xFF:
            raise ValueError("Intel HEX checksum mismatch")
        if record_type == 0:
            absolute = upper + addr
            for index, value in enumerate(data):
                memory[absolute + index] = value
        elif record_type == 1:
            break
        elif record_type == 4:
            upper = int.from_bytes(data, "big") << 16
    return memory


def image_from_hex(path):
    memory = read_ihex(path)
    outside = sorted(addr for addr in memory if not APP_BASE <= addr <= APP_LIMIT)
    if outside:
        raise ValueError(f"application HEX contains byte outside 0x{APP_BASE:04X}..0x{APP_LIMIT:04X}: 0x{outside[0]:04X}")
    if not memory:
        raise ValueError("application HEX is empty")
    if APP_BASE not in memory:
        raise ValueError(f"application HEX has no entry at 0x{APP_BASE:04X}")
    highest = max(memory)
    image = bytearray([0xFF] * (highest - APP_BASE + 1))
    for addr, value in memory.items():
        image[addr - APP_BASE] = value
    if len(image) > APP_LIMIT - APP_BASE + 1:
        raise ValueError("application exceeds OTA partition")
    return bytes(image)


def make_manifest(image, board_id, hw_revision, app_id, version, build):
    major, minor, patch = (int(part) for part in version.split("."))
    for label, value in (("board ID", board_id),
                         ("hardware revision", hw_revision),
                         ("application ID", app_id), ("build", build)):
        if not 0 <= value <= 0xFFFF:
            raise ValueError(f"{label} must be 0..65535")
    for value in (major, minor, patch):
        if not 0 <= value <= 255:
            raise ValueError("semantic version components must be 0..255")
    body = bytearray(MANIFEST_SIZE)
    struct.pack_into("<IBHHHHHHI", body, 0,
                     0x4F544131, 1, TARGET_CHIP, board_id, hw_revision,
                     app_id, APP_BASE, len(image),
                     binascii.crc32(image) & 0xFFFFFFFF)
    body[21:25] = bytes((major, minor, patch, BOOTLOADER_VERSION))
    struct.pack_into("<HH", body, 25, build, 0)
    struct.pack_into("<H", body, 29, crc16_modbus(body[:29]))
    return bytes(body)


def parse_manifest(data):
    if len(data) != MANIFEST_SIZE or crc16_modbus(data[:29]) != struct.unpack_from("<H", data, 29)[0]:
        raise ValueError("manifest CRC mismatch")
    fields = struct.unpack_from("<IBHHHHHHI", data, 0)
    return {
        "magic": fields[0], "format": fields[1], "chip": fields[2],
        "board": fields[3], "hardware": fields[4], "app": fields[5],
        "base": fields[6], "size": fields[7], "crc32": fields[8],
        "version": tuple(data[21:24]), "min_boot": data[24],
        "build": struct.unpack_from("<H", data, 25)[0],
        "flags": struct.unpack_from("<H", data, 27)[0],
        "crc16": struct.unpack_from("<H", data, 29)[0],
    }


def write_package(path, manifest, image):
    digest = hashlib.sha256(image).digest()
    header = PACKAGE_HEADER.pack(PACKAGE_MAGIC, PACKAGE_VERSION, 0,
                                 len(manifest), len(image), digest)
    path.write_bytes(header + manifest + image)


def make_initial_params(info):
    data = bytearray(36)
    struct.pack_into("<IBBBBHIHHI", data, 0,
                     0x4F545032, 2, 1, 1, 0, 1, 0,
                     info["base"], info["size"], info["crc32"])
    data[22:26] = bytes((*info["version"], info["min_boot"]))
    struct.pack_into("<HHH", data, 26, info["build"], info["size"],
                     info["crc16"])
    struct.pack_into("<HH", data, 32, crc16_modbus(data[:32]), 0xA55A)
    return bytes(data)


def write_ihex(path, memory):
    lines = []
    addresses = sorted(memory)
    index = 0
    while index < len(addresses):
        start = addresses[index]
        chunk = bytearray((memory[start],))
        index += 1
        while (index < len(addresses) and len(chunk) < 16 and
               addresses[index] == start + len(chunk)):
            chunk.append(memory[addresses[index]])
            index += 1
        record = bytearray((len(chunk), (start >> 8) & 0xFF, start & 0xFF, 0)) + chunk
        checksum = (-sum(record)) & 0xFF
        lines.append(":" + (record + bytes((checksum,))).hex().upper())
    lines.append(":00000001FF")
    path.write_text("\n".join(lines) + "\n", encoding="ascii")


def read_package(path):
    data = path.read_bytes()
    if len(data) < PACKAGE_HEADER.size + MANIFEST_SIZE:
        raise ValueError("package is truncated")
    magic, version, reserved, manifest_len, image_len, digest = PACKAGE_HEADER.unpack_from(data)
    if magic != PACKAGE_MAGIC or version != PACKAGE_VERSION or reserved != 0:
        raise ValueError("unsupported package header")
    if manifest_len != MANIFEST_SIZE or len(data) != PACKAGE_HEADER.size + manifest_len + image_len:
        raise ValueError("package length mismatch")
    manifest = data[PACKAGE_HEADER.size:PACKAGE_HEADER.size + manifest_len]
    image = data[PACKAGE_HEADER.size + manifest_len:]
    info = parse_manifest(manifest)
    if info["magic"] != 0x4F544131 or info["format"] != 1 or info["chip"] != TARGET_CHIP:
        raise ValueError("package target is not STC8H8K64U OTA v1")
    if info["base"] != APP_BASE or info["size"] != len(image):
        raise ValueError("manifest application range mismatch")
    if hashlib.sha256(image).digest() != digest:
        raise ValueError("package SHA-256 mismatch")
    if binascii.crc32(image) & 0xFFFFFFFF != info["crc32"]:
        raise ValueError("application CRC32 mismatch")
    return manifest, image, info, digest.hex()


def build_frame(dst, cmd, flags, session, seq, offset=0, payload=b""):
    if len(payload) > FRAME_PAYLOAD_MAX:
        raise ValueError("frame payload too large")
    frame = bytearray(FRAME_OVERHEAD + len(payload))
    frame[:2] = SOF
    frame[2:8] = bytes((FRAME_VERSION, cmd, flags, dst, HOST_ADDR, 0))
    struct.pack_into("<IHHH", frame, 8, session, seq, offset, len(payload))
    frame[FRAME_HEADER_SIZE:FRAME_HEADER_SIZE + len(payload)] = payload
    struct.pack_into("<H", frame, len(frame) - 2, crc16_modbus(frame[:-2]))
    return bytes(frame)


def parse_frame(data, local_addr):
    if len(data) < FRAME_OVERHEAD or data[:2] != SOF or data[2] != FRAME_VERSION:
        return None
    length = struct.unpack_from("<H", data, 16)[0]
    if length > FRAME_PAYLOAD_MAX or len(data) != FRAME_OVERHEAD + length:
        return None
    if data[5] != local_addr or crc16_modbus(data[:-2]) != struct.unpack_from("<H", data, len(data) - 2)[0]:
        return None
    return {
        "cmd": data[3], "flags": data[4], "src": data[6],
        "session": struct.unpack_from("<I", data, 8)[0],
        "seq": struct.unpack_from("<H", data, 12)[0],
        "offset": struct.unpack_from("<H", data, 14)[0],
        "payload": data[FRAME_HEADER_SIZE:-2],
    }


def read_frame(port, timeout, rx_buffer):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        chunk = port.read(256)
        if chunk:
            rx_buffer.extend(chunk)
        while len(rx_buffer) >= FRAME_OVERHEAD:
            start = rx_buffer.find(SOF)
            if start < 0:
                rx_buffer.clear()
                break
            if start:
                del rx_buffer[:start]
            if len(rx_buffer) < FRAME_HEADER_SIZE:
                break
            length = struct.unpack_from("<H", rx_buffer, 16)[0]
            if length > FRAME_PAYLOAD_MAX:
                del rx_buffer[0]
                continue
            total = FRAME_OVERHEAD + length
            if len(rx_buffer) < total:
                break
            raw = bytes(rx_buffer[:total])
            del rx_buffer[:total]
            frame = parse_frame(raw, HOST_ADDR)
            if frame is not None:
                return frame
    raise TimeoutError("timeout waiting for OTA bootloader response")


def decode_status(frame, request_cmd, seq):
    if frame["cmd"] != CMD_STATUS or frame["seq"] != seq:
        raise RuntimeError("unexpected OTA response")
    payload = frame["payload"]
    expected_len = 36 if request_cmd == CMD_INFO else 20
    if len(payload) != expected_len or payload[0] != request_cmd:
        raise RuntimeError("invalid OTA status payload")
    return {
        "status": payload[1], "state": payload[2], "failure": payload[3],
        "bootloader": payload[4], "protocol": payload[5], "address": payload[6],
        "offset": struct.unpack_from("<H", payload, 8)[0],
        "image_size": struct.unpack_from("<H", payload, 10)[0],
        "manifest_crc": struct.unpack_from("<H", payload, 12)[0],
        "build": struct.unpack_from("<H", payload, 14)[0],
        "session": struct.unpack_from("<I", payload, 16)[0],
        "uid": bytes(payload[20:36]) if len(payload) == 36 else b"",
    }


def send_command(port, rx_buffer, address, cmd, session, seq,
                 offset=0, payload=b"", flags=0, timeout=3.0):
    wire = build_frame(address, cmd, flags, session, seq, offset, payload)
    for attempt in range(3):
        port.write(wire)
        port.flush()
        try:
            status = decode_status(read_frame(port, timeout, rx_buffer), cmd, seq)
            if status["status"] not in (STATUS_OK, STATUS_DUPLICATE):
                reason = FAIL_NAMES.get(status["failure"], "UNKNOWN")
                raise RuntimeError(f"{cmd=} failed: state={STATE_NAMES.get(status['state'])} failure={reason}")
            return status
        except TimeoutError:
            if attempt == 2:
                raise
    raise AssertionError("unreachable")


def open_serial(args):
    try:
        import serial
    except ImportError as error:
        raise SystemExit("pyserial is required for serial OTA commands") from error
    return serial.Serial(args.port, args.baud, timeout=0.05, write_timeout=2)


def print_device(status):
    uid = status.get("uid", b"").hex().upper() or "unavailable"
    print(f"address={status['address']} uid={uid} bootloader={status['bootloader']} "
          f"state={STATE_NAMES.get(status['state'], status['state'])} "
          f"session=0x{status['session']:08X} offset={status['offset']}/"
          f"{status['image_size']} build={status['build']} "
          f"failure={FAIL_NAMES.get(status['failure'], status['failure'])}")


def command_pack(args):
    image = image_from_hex(pathlib.Path(args.hex))
    manifest = make_manifest(image, args.board_id, args.hardware_revision,
                             args.app_id, args.version, args.build)
    output = pathlib.Path(args.output)
    write_package(output, manifest, image)
    _, _, info, digest = read_package(output)
    print(f"wrote {output}: size={info['size']} crc32=0x{info['crc32']:08X} sha256={digest}")


def command_inspect(args):
    _, _, info, digest = read_package(pathlib.Path(args.file))
    print(f"target=STC8H8K64U board={info['board']} hw={info['hardware']} app={info['app']}")
    print(f"version={'.'.join(map(str, info['version']))}+{info['build']} base=0x{info['base']:04X} size={info['size']}")
    print(f"crc32=0x{info['crc32']:08X} manifest_crc=0x{info['crc16']:04X} sha256={digest}")


def command_factory(args):
    boot = read_ihex(pathlib.Path(args.boot_hex))
    outside = sorted(addr for addr in boot if not 0 <= addr < APP_BASE)
    if outside:
        raise ValueError(f"bootloader HEX crosses protected split: 0x{outside[0]:04X}")
    if bytes(boot.get(index, 0xFF) for index in range(3)) != bytes((2, 2, 0)):
        raise ValueError("bootloader reset vector is not LJMP 0x0200")
    for vector in range(45):
        offset = 3 + vector * 8
        target = APP_BASE + offset
        expected = bytes((2, target >> 8, target & 0xFF))
        if bytes(boot.get(offset + index, 0xFF) for index in range(3)) != expected:
            raise ValueError(f"bootloader vector {vector} is not forwarded")
    _, image, info, _ = read_package(pathlib.Path(args.package))
    memory = dict(boot)
    for index, value in enumerate(image):
        memory[APP_BASE + index] = value
    params = make_initial_params(info)
    for index, value in enumerate(params):
        memory[0xFC00 + index] = value
    output = pathlib.Path(args.output)
    code_output = output.with_suffix(".code.bin")
    eeprom_output = output.with_suffix(".eeprom.bin")
    write_ihex(output, memory)
    code_output.write_bytes(bytes(memory.get(addr, 0xFF)
                                  for addr in range(APP_BASE)))
    eeprom_output.write_bytes(bytes(memory.get(addr, 0xFF)
                                    for addr in range(APP_BASE, FLASH_LIMIT)))
    print(f"wrote {output}: protected_boot={len(boot)} app={len(image)} params=A@0xFC00 generation=1 APP_VALID")
    print(f"stcgal code={code_output} eeprom={eeprom_output}")


def probe(port, rx_buffer, address, seq=1):
    return send_command(port, rx_buffer, address, CMD_INFO, 0, seq)


def select_transfer_session(current, resume, restart, random_session=None):
    if resume:
        if current["state"] != 4 or current["session"] == 0:
            raise RuntimeError("target has no resumable RECEIVING session")
        return current["session"]
    if current["state"] == 2 and current["session"] != 0:
        return current["session"]
    if current["state"] == 4:
        if not restart:
            raise RuntimeError("target has an interrupted session; use resume or explicit --restart")
        return current["session"]
    if restart and current["session"] != 0:
        return current["session"]
    return random_session if random_session is not None else (secrets.randbits(32) or 1)


def require_downgrade_authorization(current_build, package_build, allowed):
    if package_build < current_build and not allowed:
        raise RuntimeError("package build is older; repeat with explicit --allow-downgrade")


def command_info(args):
    with open_serial(args) as port:
        print_device(probe(port, bytearray(), args.address))


def require_activatable_session(current):
    if current["state"] != 5 or current["session"] == 0:
        raise RuntimeError("target has no VERIFIED image awaiting activation")
    return current["session"]


def command_activate(args):
    with open_serial(args) as port:
        rx_buffer = bytearray()
        current = probe(port, rx_buffer, args.address)
        print_device(current)
        session = require_activatable_session(current)
        if not args.yes:
            answer = input(f"Type ACTIVATE {args.address} to continue: ").strip()
            if answer != f"ACTIVATE {args.address}":
                raise RuntimeError("explicit confirmation not received")
        send_command(port, rx_buffer, args.address, CMD_ACTIVATE,
                     session, 2, timeout=5.0)
        print("activation accepted; Station entered one-time trial boot")


def command_transfer(args, resume):
    manifest, image, info, digest = read_package(pathlib.Path(args.file))
    with open_serial(args) as port:
        rx_buffer = bytearray()
        current = probe(port, rx_buffer, args.address)
        print_device(current)
        if (len(current["uid"]) != 16 or
                current["uid"][:6] != b"STC8\x01\x07" or
                current["uid"][9:11] != b"\xF7\x84"):
            raise RuntimeError("target did not report a valid STC8H8K64U device UID")
        expected_crc = info["crc16"]
        session = select_transfer_session(current, resume, args.restart)
        if resume:
            if current["manifest_crc"] != expected_crc:
                raise RuntimeError("target session belongs to a different package")
        require_downgrade_authorization(current["build"], info["build"],
                                        args.allow_downgrade)
        print(f"package version={'.'.join(map(str, info['version']))}+{info['build']} size={len(image)} sha256={digest}")
        if not args.yes:
            answer = input(f"Type UPDATE {args.address} to continue: ").strip()
            if answer != f"UPDATE {args.address}":
                raise RuntimeError("explicit confirmation not received")
        seq = 2
        flags = FLAG_RESTART if args.restart else 0
        status = send_command(port, rx_buffer, args.address, CMD_BEGIN,
                              session, seq, payload=manifest + current["uid"],
                              flags=flags, timeout=10.0)
        offset = status["offset"]
        print(f"transfer session=0x{session:08X} starting_offset={offset}")
        while offset < len(image):
            chunk = image[offset:offset + args.chunk]
            seq = (seq + 1) & 0xFFFF
            status = send_command(port, rx_buffer, args.address, CMD_DATA,
                                  session, seq, offset, chunk)
            offset += len(chunk)
            if offset % 2048 == 0 or offset == len(image):
                print(f"wrote {offset}/{len(image)} committed={status['offset']}")
        seq = (seq + 1) & 0xFFFF
        send_command(port, rx_buffer, args.address, CMD_VERIFY,
                     session, seq, timeout=10.0)
        print("full-image CRC32 verified")
        if args.no_activate:
            print("image remains VERIFIED; run the explicit activate command when ready")
            return
        seq = (seq + 1) & 0xFFFF
        send_command(port, rx_buffer, args.address, CMD_ACTIVATE,
                     session, seq, timeout=5.0)
        print("activation accepted; Station entered one-time trial boot")


def build_parser():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)

    pack = sub.add_parser("pack", help="build a canonical .stcota package")
    pack.add_argument("--hex", required=True)
    pack.add_argument("--output", required=True)
    pack.add_argument("--board-id", type=int, required=True)
    pack.add_argument("--hardware-revision", type=int, required=True)
    pack.add_argument("--app-id", type=int, required=True)
    pack.add_argument("--version", required=True)
    pack.add_argument("--build", type=int, required=True)
    pack.set_defaults(func=command_pack)

    inspect = sub.add_parser("inspect", help="validate and display a package")
    inspect.add_argument("file")
    inspect.set_defaults(func=command_inspect)

    factory = sub.add_parser("factory", help="combine bootloader, app, and initial APP_VALID params")
    factory.add_argument("--boot-hex", required=True)
    factory.add_argument("--package", required=True)
    factory.add_argument("--output", required=True)
    factory.set_defaults(func=command_factory)

    for name in ("info", "probe", "activate", "update", "resume"):
        item = sub.add_parser(name)
        item.add_argument("--port", required=True)
        item.add_argument("--baud", type=int, default=9600)
        item.add_argument("--address", type=int, required=True)
        if name == "activate":
            item.add_argument("--yes", action="store_true",
                              help="explicit non-interactive authorization")
        elif name not in ("info", "probe"):
            item.add_argument("--file", required=True)
            item.add_argument("--chunk", type=int, default=128,
                              choices=range(1, FRAME_PAYLOAD_MAX + 1),
                              metavar="1..128")
            item.add_argument("--yes", action="store_true",
                              help="explicit non-interactive authorization")
            item.add_argument("--restart", action="store_true",
                              help="explicitly erase and restart this package")
            item.add_argument("--allow-downgrade", action="store_true",
                              help="explicitly permit an older build number")
            item.add_argument("--no-activate", action="store_true")
        if name in ("info", "probe"):
            item.set_defaults(func=command_info)
        elif name == "activate":
            item.set_defaults(func=command_activate)
        else:
            item.set_defaults(func=(lambda args, resume=name == "resume":
                                    command_transfer(args, resume)))
    return parser


def main():
    args = build_parser().parse_args()
    if hasattr(args, "address") and not 1 <= args.address <= 247:
        raise SystemExit("--address must be 1..247")
    try:
        args.func(args)
    except (OSError, ValueError, RuntimeError, TimeoutError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
