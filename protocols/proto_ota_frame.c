#include "proto_ota_frame.h"

static stc8h_u16 proto_ota_frame_crc16_modbus(const stc8h_u8 *data, stc8h_u16 len)
{
    stc8h_u16 crc;
    stc8h_u8 i;

    crc = 0xFFFFu;
    while (len != 0u) {
        crc ^= *data;
        ++data;
        --len;

        for (i = 0u; i < 8u; ++i) {
            if ((crc & 0x0001u) != 0u) {
                crc = (stc8h_u16)((crc >> 1) ^ 0xA001u);
            } else {
                crc >>= 1;
            }
        }
    }

    return crc;
}

static stc8h_u16 proto_ota_frame_get_le16(const stc8h_u8 *bytes)
{
    return (stc8h_u16)(((stc8h_u16)bytes[1] << 8) | bytes[0]);
}

static stc8h_u32 proto_ota_frame_get_le32(const stc8h_u8 *bytes)
{
    return ((stc8h_u32)bytes[3] << 24) |
           ((stc8h_u32)bytes[2] << 16) |
           ((stc8h_u32)bytes[1] << 8) |
           (stc8h_u32)bytes[0];
}

static void proto_ota_frame_put_le16(stc8h_u8 *bytes, stc8h_u16 value)
{
    bytes[0] = (stc8h_u8)(value & 0xFFu);
    bytes[1] = (stc8h_u8)((value >> 8) & 0xFFu);
}

static void proto_ota_frame_put_le32(stc8h_u8 *bytes, stc8h_u32 value)
{
    bytes[0] = (stc8h_u8)(value & 0xFFUL);
    bytes[1] = (stc8h_u8)((value >> 8) & 0xFFUL);
    bytes[2] = (stc8h_u8)((value >> 16) & 0xFFUL);
    bytes[3] = (stc8h_u8)((value >> 24) & 0xFFUL);
}

static void proto_ota_frame_write_crc(stc8h_u8 *frame, stc8h_u16 frame_len)
{
    stc8h_u16 crc;

    crc = proto_ota_frame_crc16_modbus(frame, (stc8h_u16)(frame_len - 2u));
    proto_ota_frame_put_le16(&frame[(stc8h_u16)(frame_len - 2u)], crc);
}

stc8h_status_t proto_ota_frame_build(stc8h_u8 *frame,
                                     stc8h_u16 capacity,
                                     stc8h_u8 dst,
                                     stc8h_u8 src,
                                     stc8h_u8 cmd,
                                     stc8h_u16 seq,
                                     stc8h_u32 offset,
                                     const stc8h_u8 *payload,
                                     stc8h_u16 len,
                                     stc8h_u16 *frame_len)
{
    stc8h_u16 total_len;
    stc8h_u16 i;

    if ((frame == 0) || (frame_len == 0) || (len > PROTO_OTA_FRAME_PAYLOAD_MAX) ||
        ((len != 0u) && (payload == 0))) {
        return STC8H_ERROR;
    }

    total_len = (stc8h_u16)(PROTO_OTA_FRAME_OVERHEAD + len);
    if (capacity < total_len) {
        return STC8H_ERROR;
    }

    frame[0] = PROTO_OTA_FRAME_SOF0;
    frame[1] = PROTO_OTA_FRAME_SOF1;
    frame[2] = PROTO_OTA_FRAME_VERSION;
    frame[3] = dst;
    frame[4] = src;
    frame[5] = cmd;
    proto_ota_frame_put_le16(&frame[6], seq);
    proto_ota_frame_put_le32(&frame[8], offset);
    proto_ota_frame_put_le16(&frame[12], len);
    for (i = 0u; i < len; ++i) {
        frame[PROTO_OTA_FRAME_HEADER_SIZE + i] = payload[i];
    }
    proto_ota_frame_write_crc(frame, total_len);
    *frame_len = total_len;
    return STC8H_OK;
}

proto_ota_frame_parse_result_t proto_ota_frame_parse(const stc8h_u8 *frame,
                                                     stc8h_u16 frame_len,
                                                     stc8h_u8 local_addr,
                                                     stc8h_u16 last_seq,
                                                     proto_ota_frame_t *out)
{
    stc8h_u16 payload_len;
    stc8h_u16 expected_len;
    stc8h_u16 expected_crc;
    stc8h_u16 got_crc;
    stc8h_u16 seq;

    if ((frame == 0) || (frame_len < PROTO_OTA_FRAME_OVERHEAD)) {
        return PROTO_OTA_FRAME_PARSE_ERROR;
    }
    if ((frame[0] != PROTO_OTA_FRAME_SOF0) ||
        (frame[1] != PROTO_OTA_FRAME_SOF1) ||
        (frame[2] != PROTO_OTA_FRAME_VERSION)) {
        return PROTO_OTA_FRAME_PARSE_ERROR;
    }

    payload_len = proto_ota_frame_get_le16(&frame[12]);
    if (payload_len > PROTO_OTA_FRAME_PAYLOAD_MAX) {
        return PROTO_OTA_FRAME_PARSE_ERROR;
    }
    expected_len = (stc8h_u16)(PROTO_OTA_FRAME_OVERHEAD + payload_len);
    if (frame_len != expected_len) {
        return PROTO_OTA_FRAME_PARSE_ERROR;
    }

    expected_crc = proto_ota_frame_crc16_modbus(frame, (stc8h_u16)(frame_len - 2u));
    got_crc = proto_ota_frame_get_le16(&frame[(stc8h_u16)(frame_len - 2u)]);
    if (expected_crc != got_crc) {
        return PROTO_OTA_FRAME_PARSE_ERROR;
    }

    if (frame[3] != local_addr) {
        return PROTO_OTA_FRAME_PARSE_IGNORE;
    }

    seq = proto_ota_frame_get_le16(&frame[6]);
    if (out != 0) {
        out->version = frame[2];
        out->dst = frame[3];
        out->src = frame[4];
        out->cmd = frame[5];
        out->seq = seq;
        out->offset = proto_ota_frame_get_le32(&frame[8]);
        out->len = payload_len;
        out->payload = &frame[PROTO_OTA_FRAME_HEADER_SIZE];
    }

    if (seq == last_seq) {
        return PROTO_OTA_FRAME_PARSE_DUPLICATE;
    }

    return PROTO_OTA_FRAME_PARSE_OK;
}
