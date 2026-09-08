#include <stdio.h>
#include <string.h>

#include "../../protocols/proto_ota_frame.c"

static int require(int condition, const char *message)
{
    if (!condition) {
        printf("%s\n", message);
        return 1;
    }
    return 0;
}

static int test_build_and_parse(void)
{
    stc8h_u8 wire[PROTO_OTA_FRAME_WIRE_MAX];
    stc8h_u8 payload[3];
    stc8h_u16 wire_len;
    proto_ota_frame_t frame;
    int failures;

    payload[0] = 1u;
    payload[1] = 2u;
    payload[2] = 3u;
    failures = 0;
    failures += require(proto_ota_frame_build(
        wire, sizeof(wire), 0x22u, 0xA5u, PROTO_OTA_FRAME_CMD_DATA,
        PROTO_OTA_FRAME_FLAG_RESTART, 0x12345678UL, 7u, 128u,
        payload, sizeof(payload), &wire_len) == STC8H_OK,
        "valid frame must build");
    failures += require(proto_ota_frame_parse(
        wire, wire_len, 0x22u, &frame) == PROTO_OTA_FRAME_PARSE_OK,
        "valid frame must parse");
    failures += require(frame.cmd == PROTO_OTA_FRAME_CMD_DATA,
                        "command must round-trip");
    failures += require(frame.flags == PROTO_OTA_FRAME_FLAG_RESTART,
                        "flags must round-trip");
    failures += require(frame.session_id == 0x12345678UL,
                        "session must round-trip");
    failures += require(frame.seq == 7u && frame.offset == 128u,
                        "sequence and offset must round-trip");
    failures += require(frame.len == 3u && frame.payload[2] == 3u,
                        "payload must round-trip");
    return failures;
}

static int test_wrong_address_ignored(void)
{
    stc8h_u8 wire[PROTO_OTA_FRAME_WIRE_MAX];
    stc8h_u16 wire_len;

    (void)proto_ota_frame_build(wire, sizeof(wire), 0x22u, 0xA5u,
                                PROTO_OTA_FRAME_CMD_INFO, 0u, 0UL, 1u,
                                0u, 0, 0u, &wire_len);
    return require(proto_ota_frame_parse(wire, wire_len, 0x23u, 0) ==
                   PROTO_OTA_FRAME_PARSE_IGNORE,
                   "other destination must be ignored");
}

static int test_bad_crc_rejected(void)
{
    stc8h_u8 wire[PROTO_OTA_FRAME_WIRE_MAX];
    stc8h_u16 wire_len;

    (void)proto_ota_frame_build(wire, sizeof(wire), 0x22u, 0xA5u,
                                PROTO_OTA_FRAME_CMD_INFO, 0u, 0UL, 1u,
                                0u, 0, 0u, &wire_len);
    wire[8] ^= 1u;
    return require(proto_ota_frame_parse(wire, wire_len, 0x22u, 0) ==
                   PROTO_OTA_FRAME_PARSE_ERROR,
                   "bad CRC must fail");
}

static int test_collector_resynchronizes_and_completes(void)
{
    stc8h_u8 wire[PROTO_OTA_FRAME_WIRE_MAX];
    stc8h_u8 collected[PROTO_OTA_FRAME_WIRE_MAX];
    stc8h_u16 wire_len;
    stc8h_u16 i;
    proto_ota_frame_collector_t collector;
    proto_ota_collect_result_t result;
    int failures;

    failures = 0;
    (void)proto_ota_frame_build(wire, sizeof(wire), 0x22u, 0xA5u,
                                PROTO_OTA_FRAME_CMD_STATUS, 0u, 0UL, 2u,
                                0u, 0, 0u, &wire_len);
    proto_ota_frame_collector_init(&collector, collected, sizeof(collected));
    failures += require(proto_ota_frame_collector_feed(&collector, 0x00u) ==
                        PROTO_OTA_COLLECT_MORE,
                        "noise must be skipped");
    result = PROTO_OTA_COLLECT_MORE;
    for (i = 0u; i < wire_len; ++i) {
        result = proto_ota_frame_collector_feed(&collector, wire[i]);
    }
    failures += require(result == PROTO_OTA_COLLECT_FRAME,
                        "collector must signal complete frame");
    failures += require(collector.length == wire_len &&
                        memcmp(wire, collected, wire_len) == 0,
                        "collector must preserve exact frame");
    return failures;
}

static int test_oversize_payload_rejected(void)
{
    stc8h_u8 wire[PROTO_OTA_FRAME_WIRE_MAX];
    stc8h_u16 wire_len;

    return require(proto_ota_frame_build(
        wire, sizeof(wire), 1u, 2u, PROTO_OTA_FRAME_CMD_DATA,
        0u, 1UL, 1u, 0u, wire, PROTO_OTA_FRAME_PAYLOAD_MAX + 1u,
        &wire_len) == STC8H_ERROR,
        "oversize payload must fail");
}

int main(void)
{
    int failures;

    failures = 0;
    failures += test_build_and_parse();
    failures += test_wrong_address_ignored();
    failures += test_bad_crc_rejected();
    failures += test_collector_resynchronizes_and_completes();
    failures += test_oversize_payload_rejected();
    return failures == 0 ? 0 : 1;
}
