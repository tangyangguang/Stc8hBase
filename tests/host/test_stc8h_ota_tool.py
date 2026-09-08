#!/usr/bin/env python3
import importlib.util
import pathlib
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[2]
SPEC = importlib.util.spec_from_file_location("stc8h_ota_tool", ROOT / "tools/stc8h_ota.py")
TOOL = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(TOOL)


def require(condition, message):
    if not condition:
        raise AssertionError(message)


def test_package_and_factory():
    with tempfile.TemporaryDirectory() as directory:
        directory = pathlib.Path(directory)
        app_hex = directory / "app.hex"
        package = directory / "app.stcota"
        boot_hex = directory / "boot.hex"
        factory_hex = directory / "factory.hex"
        image = bytes(range(64))
        TOOL.write_ihex(app_hex, {TOOL.APP_BASE + i: value for i, value in enumerate(image)})
        loaded = TOOL.image_from_hex(app_hex)
        require(loaded == image, "HEX application must round-trip")
        manifest = TOOL.make_manifest(loaded, 1, 2, 3, "4.5.6", 7)
        TOOL.write_package(package, manifest, loaded)
        _, actual, info, _ = TOOL.read_package(package)
        require(actual == image and info["base"] == TOOL.APP_BASE,
                "package must validate and round-trip")

        boot = {0: 2, 1: 2, 2: 0, 0x200: 0x22}
        for vector in range(45):
            offset = 3 + vector * 8
            target = TOOL.APP_BASE + offset
            boot.update({offset: 2, offset + 1: target >> 8,
                         offset + 2: target & 0xFF})
        TOOL.write_ihex(boot_hex, boot)
        class Args:
            pass
        args = Args()
        args.boot_hex = str(boot_hex)
        args.package = str(package)
        args.output = str(factory_hex)
        TOOL.command_factory(args)
        memory = TOOL.read_ihex(factory_hex)
        require(memory[TOOL.APP_BASE] == image[0] and memory[0xFC05] == 1,
                "factory image must contain app and APP_VALID params")

        broken = bytearray(package.read_bytes())
        broken[-1] ^= 1
        package.write_bytes(broken)
        try:
            TOOL.read_package(package)
        except ValueError:
            pass
        else:
            raise AssertionError("package corruption must fail validation")


def test_frame_round_trip():
    uid = bytes(range(16))
    frame = TOOL.build_frame(0x22, TOOL.CMD_BEGIN, TOOL.FLAG_RESTART,
                             0x12345678, 9, 128, b"abc")
    parsed = TOOL.parse_frame(frame, 0x22)
    require(parsed is not None and parsed["session"] == 0x12345678 and
            parsed["seq"] == 9 and parsed["offset"] == 128 and
            parsed["payload"] == b"abc", "wire frame must round-trip")

    payload = bytearray(36)
    payload[0] = TOOL.CMD_INFO
    payload[1] = TOOL.STATUS_OK
    payload[2] = 4
    payload[6] = 0x22
    payload[20:36] = uid
    response = TOOL.build_frame(TOOL.HOST_ADDR, TOOL.CMD_STATUS, 0,
                                0, 10, payload=payload)
    status = TOOL.decode_status(TOOL.parse_frame(response, TOOL.HOST_ADDR),
                                TOOL.CMD_INFO, 10)
    require(status["uid"] == uid and status["state"] == 4,
            "INFO must expose complete target UID")


if __name__ == "__main__":
    test_package_and_factory()
    test_frame_round_trip()
