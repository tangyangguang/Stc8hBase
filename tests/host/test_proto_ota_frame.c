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

static int test_valid_begin_frame_parses(void)
{
    stc8h_u8 frame[PROTO_OTA_FRAME_WIRE_MAX];
    stc8h_u16 frame_len;
    proto_ota_frame_t parsed;
    int failures;

    failures = 0;
    failures += require(proto_ota_frame_build(frame,
                                             sizeof(frame),
                                             0x22u,
                                             0x11u,
                                             PROTO_OTA_FRAME_CMD_BEGIN,
                                             0x1234u,
                                             0UL,
                                             0,
                                             0u,
                                             &frame_len) == STC8H_OK,
                        "BEGIN frame must build");
    failures += require(proto_ota_frame_parse(frame,
                                             frame_len,
                                             0x22u,
                                             0xFFFFu,
                                             &parsed) == PROTO_OTA_FRAME_PARSE_OK,
                        "valid BEGIN frame must parse");
    failures += require(parsed.cmd == PROTO_OTA_FRAME_CMD_BEGIN, "BEGIN command must decode");
    failures += require(parsed.seq == 0x1234u, "BEGIN seq must decode little-endian");
    failures += require(parsed.len == 0u, "BEGIN payload length must be zero");
    return failures;
}

static int test_valid_write_block_frame_parses(void)
{
    stc8h_u8 frame[PROTO_OTA_FRAME_WIRE_MAX];
    stc8h_u8 payload[3];
    stc8h_u16 frame_len;
    proto_ota_frame_t parsed;
    int failures;

    failures = 0;
    payload[0] = 0xAAu;
    payload[1] = 0xBBu;
    payload[2] = 0xCCu;
    failures += require(proto_ota_frame_build(frame,
                                             sizeof(frame),
                                             0x22u,
                                             0x11u,
                                             PROTO_OTA_FRAME_CMD_WRITE_BLOCK,
                                             7u,
                                             0x01020304UL,
                                             payload,
                                             sizeof(payload),
                                             &frame_len) == STC8H_OK,
                        "WRITE_BLOCK frame must build");
    failures += require(proto_ota_frame_parse(frame,
                                             frame_len,
                                             0x22u,
                                             6u,
                                             &parsed) == PROTO_OTA_FRAME_PARSE_OK,
                        "valid WRITE_BLOCK frame must parse");
    failures += require(parsed.cmd == PROTO_OTA_FRAME_CMD_WRITE_BLOCK, "WRITE_BLOCK command must decode");
    failures += require(parsed.offset == 0x01020304UL, "WRITE_BLOCK offset must decode little-endian");
    failures += require(parsed.len == sizeof(payload), "WRITE_BLOCK payload length must decode");
    failures += require(parsed.payload[0] == 0xAAu, "WRITE_BLOCK payload must point at frame payload");
    return failures;
}

static int test_bad_crc_returns_error(void)
{
    stc8h_u8 frame[PROTO_OTA_FRAME_WIRE_MAX];
    stc8h_u16 frame_len;
    proto_ota_frame_t parsed;

    (void)proto_ota_frame_build(frame,
                                sizeof(frame),
                                0x22u,
                                0x11u,
                                PROTO_OTA_FRAME_CMD_BEGIN,
                                1u,
                                0UL,
                                0,
                                0u,
                                &frame_len);
    frame[5] ^= 0x01u;
    return require(proto_ota_frame_parse(frame,
                                        frame_len,
                                        0x22u,
                                        0xFFFFu,
                                        &parsed) == PROTO_OTA_FRAME_PARSE_ERROR,
                   "bad CRC must return parse error");
}

static int test_wrong_destination_returns_ignore(void)
{
    stc8h_u8 frame[PROTO_OTA_FRAME_WIRE_MAX];
    stc8h_u16 frame_len;
    proto_ota_frame_t parsed;

    (void)proto_ota_frame_build(frame,
                                sizeof(frame),
                                0x22u,
                                0x11u,
                                PROTO_OTA_FRAME_CMD_BEGIN,
                                1u,
                                0UL,
                                0,
                                0u,
                                &frame_len);
    return require(proto_ota_frame_parse(frame,
                                        frame_len,
                                        0x33u,
                                        0xFFFFu,
                                        &parsed) == PROTO_OTA_FRAME_PARSE_IGNORE,
                   "wrong destination must return ignore");
}

static int test_duplicate_sequence_is_surfaced(void)
{
    stc8h_u8 frame[PROTO_OTA_FRAME_WIRE_MAX];
    stc8h_u16 frame_len;
    proto_ota_frame_t parsed;

    (void)proto_ota_frame_build(frame,
                                sizeof(frame),
                                0x22u,
                                0x11u,
                                PROTO_OTA_FRAME_CMD_BEGIN,
                                9u,
                                0UL,
                                0,
                                0u,
                                &frame_len);
    return require(proto_ota_frame_parse(frame,
                                        frame_len,
                                        0x22u,
                                        9u,
                                        &parsed) == PROTO_OTA_FRAME_PARSE_DUPLICATE,
                   "duplicate seq must be surfaced to caller");
}

static int test_payload_above_max_is_rejected(void)
{
    stc8h_u8 frame[PROTO_OTA_FRAME_WIRE_MAX + 1u];
    stc8h_u16 frame_len;
    stc8h_u16 i;

    frame[0] = PROTO_OTA_FRAME_SOF0;
    frame[1] = PROTO_OTA_FRAME_SOF1;
    frame[2] = PROTO_OTA_FRAME_VERSION;
    frame[3] = 0x22u;
    frame[4] = 0x11u;
    frame[5] = PROTO_OTA_FRAME_CMD_WRITE_BLOCK;
    frame[6] = 1u;
    frame[7] = 0u;
    frame[8] = 0u;
    frame[9] = 0u;
    frame[10] = 0u;
    frame[11] = 0u;
    frame[12] = (stc8h_u8)((PROTO_OTA_FRAME_PAYLOAD_MAX + 1u) & 0xFFu);
    frame[13] = (stc8h_u8)((PROTO_OTA_FRAME_PAYLOAD_MAX + 1u) >> 8);
    for (i = 0u; i < (stc8h_u16)(PROTO_OTA_FRAME_PAYLOAD_MAX + 1u); ++i) {
        frame[PROTO_OTA_FRAME_HEADER_SIZE + i] = (stc8h_u8)i;
    }
    frame_len = sizeof(frame);
    proto_ota_frame_write_crc(frame, frame_len);

    return require(proto_ota_frame_parse(frame,
                                        frame_len,
                                        0x22u,
                                        0xFFFFu,
                                        0) == PROTO_OTA_FRAME_PARSE_ERROR,
                   "payload above max must be rejected");
}

int main(void)
{
    int failures;

    failures = 0;
    failures += test_valid_begin_frame_parses();
    failures += test_valid_write_block_frame_parses();
    failures += test_bad_crc_returns_error();
    failures += test_wrong_destination_returns_ignore();
    failures += test_duplicate_sequence_is_surfaced();
    failures += test_payload_above_max_is_rejected();

    return failures == 0 ? 0 : 1;
}
