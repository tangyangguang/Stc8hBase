#!/usr/bin/env python3
import argparse
import binascii
import subprocess
import sys
import time

import serial

import h8k64u_uart1_ota_smoke as smoke


STATUS_ERROR = 1
STATUS_DUPLICATE = 2

CMD_ABORT = 5

STATE_RECEIVING = 2
STATE_FAILED = 6

FAIL_MANIFEST = 2
FAIL_OFFSET = 5
FAIL_DUPLICATE = 7
FAIL_INCOMPLETE = 10
FAIL_CRC = 11


class CaseFailure(Exception):
    pass


def run(cmd, cwd):
    print("+", " ".join(cmd))
    subprocess.run(cmd, cwd=str(cwd), check=True)


def read_status_frame(ser, seq, request_cmd, timeout, rx_buf):
    deadline = time.time() + timeout
    while time.time() < deadline:
        data = ser.read(256)
        if data:
            rx_buf.extend(data)
        while len(rx_buf) >= smoke.FRAME_OVERHEAD:
            if rx_buf[0] != smoke.SOF[0]:
                del rx_buf[0]
                continue
            if len(rx_buf) >= 2 and rx_buf[1] != smoke.SOF[1]:
                del rx_buf[0]
                continue
            if len(rx_buf) < 14:
                break
            payload_len = int.from_bytes(rx_buf[12:14], "little")
            total_len = smoke.FRAME_OVERHEAD + payload_len
            if payload_len > smoke.FRAME_PAYLOAD_MAX:
                del rx_buf[0]
                continue
            if len(rx_buf) < total_len:
                break
            raw = bytes(rx_buf[:total_len])
            del rx_buf[:total_len]
            frame = smoke.parse_frame(raw, smoke.HOST_ADDR)
            if frame is None:
                continue
            if frame["cmd"] != smoke.CMD_STATUS or frame["seq"] != seq:
                continue
            payload = frame["payload"]
            if len(payload) < 4 or payload[0] != request_cmd:
                raise CaseFailure(f"bad status payload for seq {seq}: {payload.hex()}")
            return frame, payload
    raise TimeoutError(f"timeout waiting status for seq {seq}")


def send_raw(ser, frame, seq, request_cmd, timeout, rx_buf):
    ser.write(frame)
    ser.flush()
    return read_status_frame(ser, seq, request_cmd, timeout, rx_buf)


def send_command(ser, seq, cmd, offset=0, payload=b"", timeout=3.0, rx_buf=None):
    if rx_buf is None:
        rx_buf = bytearray()
    frame = smoke.build_frame(smoke.LOCAL_ADDR, smoke.HOST_ADDR, cmd, seq, offset, payload)
    return send_raw(ser, frame, seq, cmd, timeout, rx_buf)


def expect_timeout(label, fn):
    try:
        fn()
    except TimeoutError:
        return
    raise CaseFailure(f"{label}: expected timeout")


def expect_status(label, frame_payload, status, ota_state=None, fail_reason=None):
    frame, payload = frame_payload
    if payload[1] != status:
        raise CaseFailure(f"{label}: status {payload[1]} != {status}, payload={payload.hex()}")
    if ota_state is not None and payload[2] != ota_state:
        raise CaseFailure(f"{label}: ota_state {payload[2]} != {ota_state}, payload={payload.hex()}")
    if fail_reason is not None and payload[3] != fail_reason:
        raise CaseFailure(f"{label}: fail_reason {payload[3]} != {fail_reason}, payload={payload.hex()}")
    return frame, payload


def expect_ok(label, frame_payload):
    return expect_status(label, frame_payload, smoke.STATUS_OK)


def mutate_manifest(manifest, offset, data):
    mutated = bytearray(manifest)
    mutated[offset:offset + len(data)] = data
    mutated[-2:] = smoke.crc16_modbus(mutated[:-2]).to_bytes(2, "little")
    return bytes(mutated)


def begin_valid(ser, seq, manifest, rx_buf):
    expect_ok("valid BEGIN", send_command(ser, seq, smoke.CMD_BEGIN, payload=manifest, timeout=8.0, rx_buf=rx_buf))
    return seq + 1


def write_image(ser, seq, image, chunk_size, rx_buf):
    offset = 0
    while offset < len(image):
        chunk = image[offset:offset + chunk_size]
        expect_ok("WRITE_BLOCK", send_command(ser, seq, smoke.CMD_WRITE_BLOCK,
                                              offset=offset, payload=chunk,
                                              timeout=3.0, rx_buf=rx_buf))
        offset += len(chunk)
        seq += 1
    return seq


def complete_ota(ser, seq, image, manifest, chunk_size, rx_buf):
    seq = begin_valid(ser, seq, manifest, rx_buf)
    seq = write_image(ser, seq, image, chunk_size, rx_buf)
    expect_ok("VERIFY", send_command(ser, seq, smoke.CMD_VERIFY, timeout=8.0, rx_buf=rx_buf))
    seq += 1
    expect_ok("COMMIT", send_command(ser, seq, smoke.CMD_COMMIT, timeout=8.0, rx_buf=rx_buf))
    seq += 1
    return seq


def pulse_reset(ser):
    ser.dtr = False
    ser.rts = False
    time.sleep(0.2)
    ser.dtr = True
    ser.rts = True
    time.sleep(0.2)


def wait_boot(ser, timeout=5.0):
    smoke.wait_boot_banner(ser, timeout=timeout)
    ser.reset_input_buffer()


def run_case(name, fn):
    print(f"CASE {name} ...", end=" ", flush=True)
    fn()
    print("PASS")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", default="/dev/cu.usbserial-110")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--app-env", default="STC8H8K64U_entry_probe")
    parser.add_argument("--skip-upload", action="store_true")
    args = parser.parse_args()

    app_hex = smoke.APP_DIR / f".pio/build/{args.app_env}/firmware.hex"
    run(["pio", "run", "-e", args.app_env], smoke.APP_DIR)
    run(["pio", "run"], smoke.BOOT_DIR)
    image = smoke.load_app_image(app_hex)
    manifest = smoke.encode_manifest(image)
    print(f"app image: {len(image)} bytes, crc32=0x{binascii.crc32(image) & 0xFFFFFFFF:08X}")

    if not args.skip_upload:
        run(["pio", "run", "-t", "upload", "--upload-port", args.port], smoke.BOOT_DIR)

    seq = 1
    failures = []
    time.sleep(0.5)
    with serial.Serial(args.port, args.baud, timeout=0.05, write_timeout=2) as ser:
        wait_boot(ser)
        rx_buf = bytearray()

        def case_bad_frame_crc():
            nonlocal seq
            frame = bytearray(smoke.build_frame(smoke.LOCAL_ADDR, smoke.HOST_ADDR,
                                                smoke.CMD_BEGIN, seq, 0, manifest))
            frame[-1] ^= 0x01
            expect_timeout("bad frame crc", lambda: send_raw(ser, bytes(frame), seq, smoke.CMD_BEGIN, 0.8, rx_buf))
            seq += 1

        def case_wrong_dst_ignored():
            nonlocal seq
            frame = smoke.build_frame(0x7E, smoke.HOST_ADDR, smoke.CMD_BEGIN, seq, 0, manifest)
            expect_timeout("wrong dst", lambda: send_raw(ser, frame, seq, smoke.CMD_BEGIN, 0.8, rx_buf))
            seq += 1

        def case_bad_manifest_target():
            nonlocal seq
            bad = mutate_manifest(manifest, 5, b"\x99\x99")
            expect_status("bad target", send_command(ser, seq, smoke.CMD_BEGIN, payload=bad,
                                                     timeout=3.0, rx_buf=rx_buf),
                          STATUS_ERROR, STATE_FAILED, FAIL_MANIFEST)
            seq += 1

        def case_bad_manifest_app_base():
            nonlocal seq
            bad = mutate_manifest(manifest, 13, b"\x00\x03")
            expect_status("bad app base", send_command(ser, seq, smoke.CMD_BEGIN, payload=bad,
                                                       timeout=3.0, rx_buf=rx_buf),
                          STATUS_ERROR, STATE_FAILED, FAIL_MANIFEST)
            seq += 1

        def case_oversized_manifest():
            nonlocal seq
            bad = mutate_manifest(manifest, 15, (0xB300).to_bytes(4, "little"))
            expect_status("oversized app", send_command(ser, seq, smoke.CMD_BEGIN, payload=bad,
                                                        timeout=3.0, rx_buf=rx_buf),
                          STATUS_ERROR, STATE_FAILED, FAIL_MANIFEST)
            seq += 1

        def case_verify_incomplete():
            nonlocal seq
            seq = begin_valid(ser, seq, manifest, rx_buf)
            expect_status("verify incomplete", send_command(ser, seq, smoke.CMD_VERIFY,
                                                            timeout=3.0, rx_buf=rx_buf),
                          STATUS_ERROR, STATE_RECEIVING, FAIL_INCOMPLETE)
            seq += 1

        def case_future_offset_rejected():
            nonlocal seq
            seq = begin_valid(ser, seq, manifest, rx_buf)
            expect_status("future offset", send_command(ser, seq, smoke.CMD_WRITE_BLOCK,
                                                        offset=64,
                                                        payload=image[:64],
                                                        timeout=3.0,
                                                        rx_buf=rx_buf),
                          STATUS_ERROR, STATE_RECEIVING, FAIL_OFFSET)
            seq += 1

        def case_duplicate_seq_reported():
            nonlocal seq
            seq = begin_valid(ser, seq, manifest, rx_buf)
            first = image[:64]
            frame = smoke.build_frame(smoke.LOCAL_ADDR, smoke.HOST_ADDR,
                                      smoke.CMD_WRITE_BLOCK, seq, 0, first)
            expect_ok("first write", send_raw(ser, frame, seq, smoke.CMD_WRITE_BLOCK, 3.0, rx_buf))
            expect_status("duplicate seq", send_raw(ser, frame, seq, smoke.CMD_WRITE_BLOCK, 3.0, rx_buf),
                          STATUS_DUPLICATE)
            seq += 1

        def case_duplicate_chunk_ok():
            nonlocal seq
            seq = begin_valid(ser, seq, manifest, rx_buf)
            first = image[:64]
            expect_ok("first write", send_command(ser, seq, smoke.CMD_WRITE_BLOCK,
                                                  offset=0, payload=first,
                                                  timeout=3.0, rx_buf=rx_buf))
            seq += 1
            frame, _ = expect_ok("duplicate chunk new seq", send_command(ser, seq, smoke.CMD_WRITE_BLOCK,
                                                                         offset=0, payload=first,
                                                                         timeout=3.0, rx_buf=rx_buf))
            if frame["offset"] != 64:
                raise CaseFailure(f"duplicate chunk changed write offset to {frame['offset']}")
            seq += 1

        def case_duplicate_chunk_mismatch():
            nonlocal seq
            seq = begin_valid(ser, seq, manifest, rx_buf)
            first = image[:64]
            expect_ok("first write", send_command(ser, seq, smoke.CMD_WRITE_BLOCK,
                                                  offset=0, payload=first,
                                                  timeout=3.0, rx_buf=rx_buf))
            seq += 1
            bad = bytearray(first)
            bad[0] ^= 0x01
            expect_status("duplicate mismatch", send_command(ser, seq, smoke.CMD_WRITE_BLOCK,
                                                             offset=0, payload=bytes(bad),
                                                             timeout=3.0, rx_buf=rx_buf),
                          STATUS_ERROR, STATE_RECEIVING, FAIL_DUPLICATE)
            seq += 1

        def case_verify_crc_mismatch():
            nonlocal seq
            wrong_crc_manifest = mutate_manifest(manifest, 19, (0x12345678).to_bytes(4, "little"))
            seq = begin_valid(ser, seq, wrong_crc_manifest, rx_buf)
            seq = write_image(ser, seq, image, 64, rx_buf)
            expect_status("verify crc mismatch", send_command(ser, seq, smoke.CMD_VERIFY,
                                                              timeout=8.0, rx_buf=rx_buf),
                          STATUS_ERROR, STATE_FAILED, FAIL_CRC)
            seq += 1

        def case_abort():
            nonlocal seq
            seq = begin_valid(ser, seq, manifest, rx_buf)
            expect_status("abort", send_command(ser, seq, CMD_ABORT,
                                                timeout=3.0, rx_buf=rx_buf),
                          smoke.STATUS_OK, STATE_FAILED, 1)
            seq += 1

        def case_normal_chunk_128():
            nonlocal seq
            seq = complete_ota(ser, seq, image, manifest, 128, rx_buf)
            end = time.time() + 4.0
            text = bytearray(rx_buf)
            rx_buf.clear()
            while time.time() < end:
                data = ser.read(256)
                if data:
                    text.extend(data)
                    if b"H8K64U OTA app" in text:
                        break
            if b"H8K64U OTA app" not in text:
                raise CaseFailure(f"normal OTA did not boot app: {text!r}")
            pulse_reset(ser)
            wait_boot(ser)

        def case_normal_chunk_16():
            nonlocal seq
            seq = complete_ota(ser, seq, image, manifest, 16, rx_buf)
            end = time.time() + 4.0
            text = bytearray(rx_buf)
            rx_buf.clear()
            while time.time() < end:
                data = ser.read(256)
                if data:
                    text.extend(data)
                    if b"H8K64U OTA app" in text:
                        break
            if b"H8K64U OTA app" not in text:
                raise CaseFailure(f"chunk16 OTA did not boot app: {text!r}")

        cases = [
            ("bad_frame_crc_timeout", case_bad_frame_crc),
            ("wrong_dst_ignored", case_wrong_dst_ignored),
            ("bad_manifest_target", case_bad_manifest_target),
            ("bad_manifest_app_base", case_bad_manifest_app_base),
            ("oversized_manifest", case_oversized_manifest),
            ("verify_incomplete", case_verify_incomplete),
            ("future_offset_rejected", case_future_offset_rejected),
            ("duplicate_seq_reported", case_duplicate_seq_reported),
            ("duplicate_chunk_ok", case_duplicate_chunk_ok),
            ("duplicate_chunk_mismatch", case_duplicate_chunk_mismatch),
            ("verify_crc_mismatch", case_verify_crc_mismatch),
            ("abort", case_abort),
            ("normal_chunk_128", case_normal_chunk_128),
            ("normal_chunk_16", case_normal_chunk_16),
        ]

        for name, fn in cases:
            try:
                run_case(name, fn)
            except Exception as exc:
                print("FAIL")
                failures.append((name, exc))
                break

    if failures:
        for name, exc in failures:
            print(f"{name}: {exc}", file=sys.stderr)
        raise SystemExit(1)

    print("UART1 OTA fault/recovery tests passed")


if __name__ == "__main__":
    main()
