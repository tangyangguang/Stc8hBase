Import("env")


RESET_STUB_RECORD = ":0300000002B80043"


def _record_range(line):
    if not line.startswith(":"):
        return None
    count = int(line[1:3], 16)
    addr = int(line[3:7], 16)
    rectype = int(line[7:9], 16)
    if rectype != 0 or count == 0:
        return None
    return addr, addr + count


def patch_reset_stub(source, target, env):
    hex_path = str(target[0])
    with open(hex_path, "r", encoding="ascii") as fh:
        lines = [line.strip() for line in fh if line.strip()]

    kept = []
    for line in lines:
        if line == RESET_STUB_RECORD:
            continue
        span = _record_range(line)
        if span is None:
            kept.append(line)
            continue
        start, end = span
        if start < 3 and end > 0:
            raise RuntimeError("firmware.hex already contains data in 0x0000..0x0002")
        kept.append(line)

    with open(hex_path, "w", encoding="ascii", newline="\n") as fh:
        fh.write(RESET_STUB_RECORD + "\n")
        for line in kept:
            fh.write(line + "\n")


env.AddPostAction("$BUILD_DIR/${PROGNAME}.hex", patch_reset_stub)
