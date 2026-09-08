#include "proto_ota_frame.h"

static stc8h_u16 proto_ota_frame_crc16_modbus(const stc8h_u8 *data,
                                               stc8h_u16 len)
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

stc8h_status_t proto_ota_frame_build(stc8h_u8 *frame,
                                     stc8h_u16 capacity,
                                     stc8h_u8 dst,
                                     stc8h_u8 src,
                                     stc8h_u8 cmd,
                                     stc8h_u8 flags,
                                     stc8h_u32 session_id,
                                     stc8h_u16 seq,
                                     stc8h_u16 offset,
                                     const stc8h_u8 *payload,
                                     stc8h_u16 len,
                                     stc8h_u16 *frame_len)
{
    stc8h_u16 total_len;
    stc8h_u16 crc;
    stc8h_u16 i;

    if ((frame == 0) || (frame_len == 0) ||
        (len > PROTO_OTA_FRAME_PAYLOAD_MAX) ||
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
    frame[3] = cmd;
    frame[4] = flags;
    frame[5] = dst;
    frame[6] = src;
    frame[7] = 0u;
    proto_ota_frame_put_le32(&frame[8], session_id);
    proto_ota_frame_put_le16(&frame[12], seq);
    proto_ota_frame_put_le16(&frame[14], offset);
    proto_ota_frame_put_le16(&frame[16], len);
    for (i = 0u; i < len; ++i) {
        frame[PROTO_OTA_FRAME_HEADER_SIZE + i] = payload[i];
    }
    crc = proto_ota_frame_crc16_modbus(frame,
        (stc8h_u16)(total_len - 2u));
    proto_ota_frame_put_le16(&frame[(stc8h_u16)(total_len - 2u)], crc);
    *frame_len = total_len;
    return STC8H_OK;
}

proto_ota_frame_parse_result_t proto_ota_frame_parse(
    const stc8h_u8 *frame,
    stc8h_u16 frame_len,
    stc8h_u8 local_addr,
    proto_ota_frame_t *out)
{
    stc8h_u16 payload_len;
    stc8h_u16 expected_len;
    stc8h_u16 expected_crc;
    stc8h_u16 got_crc;

    if ((frame == 0) || (frame_len < PROTO_OTA_FRAME_OVERHEAD)) {
        return PROTO_OTA_FRAME_PARSE_ERROR;
    }
    if ((frame[0] != PROTO_OTA_FRAME_SOF0) ||
        (frame[1] != PROTO_OTA_FRAME_SOF1) ||
        (frame[2] != PROTO_OTA_FRAME_VERSION) || (frame[7] != 0u)) {
        return PROTO_OTA_FRAME_PARSE_ERROR;
    }
    payload_len = proto_ota_frame_get_le16(&frame[16]);
    if (payload_len > PROTO_OTA_FRAME_PAYLOAD_MAX) {
        return PROTO_OTA_FRAME_PARSE_ERROR;
    }
    expected_len = (stc8h_u16)(PROTO_OTA_FRAME_OVERHEAD + payload_len);
    if (frame_len != expected_len) {
        return PROTO_OTA_FRAME_PARSE_ERROR;
    }
    expected_crc = proto_ota_frame_crc16_modbus(
        frame, (stc8h_u16)(frame_len - 2u));
    got_crc = proto_ota_frame_get_le16(
        &frame[(stc8h_u16)(frame_len - 2u)]);
    if (expected_crc != got_crc) {
        return PROTO_OTA_FRAME_PARSE_ERROR;
    }
    if (frame[5] != local_addr) {
        return PROTO_OTA_FRAME_PARSE_IGNORE;
    }
    if (out != 0) {
        out->version = frame[2];
        out->cmd = frame[3];
        out->flags = frame[4];
        out->dst = frame[5];
        out->src = frame[6];
        out->session_id = proto_ota_frame_get_le32(&frame[8]);
        out->seq = proto_ota_frame_get_le16(&frame[12]);
        out->offset = proto_ota_frame_get_le16(&frame[14]);
        out->len = payload_len;
        out->payload = &frame[PROTO_OTA_FRAME_HEADER_SIZE];
    }
    return PROTO_OTA_FRAME_PARSE_OK;
}

void proto_ota_frame_collector_init(proto_ota_frame_collector_t *collector,
                                    stc8h_u8 *buffer,
                                    stc8h_u16 capacity)
{
    if (collector == 0) {
        return;
    }
    collector->buffer = buffer;
    collector->capacity = capacity;
    collector->length = 0u;
    collector->expected_length = 0u;
}

void proto_ota_frame_collector_reset(proto_ota_frame_collector_t *collector)
{
    if (collector == 0) {
        return;
    }
    collector->length = 0u;
    collector->expected_length = 0u;
}

proto_ota_collect_result_t proto_ota_frame_collector_feed(
    proto_ota_frame_collector_t *collector,
    stc8h_u8 value)
{
    stc8h_u16 payload_len;

    if ((collector == 0) || (collector->buffer == 0) ||
        (collector->capacity < PROTO_OTA_FRAME_OVERHEAD)) {
        return PROTO_OTA_COLLECT_ERROR;
    }
    if (collector->length == 0u) {
        if (value != PROTO_OTA_FRAME_SOF0) {
            return PROTO_OTA_COLLECT_MORE;
        }
    } else if ((collector->length == 1u) &&
               (value != PROTO_OTA_FRAME_SOF1)) {
        collector->length = value == PROTO_OTA_FRAME_SOF0 ? 1u : 0u;
        collector->buffer[0] = value;
        return PROTO_OTA_COLLECT_MORE;
    }
    if (collector->length >= collector->capacity) {
        proto_ota_frame_collector_reset(collector);
        return PROTO_OTA_COLLECT_ERROR;
    }
    collector->buffer[collector->length] = value;
    ++collector->length;

    if (collector->length == PROTO_OTA_FRAME_HEADER_SIZE) {
        payload_len = proto_ota_frame_get_le16(&collector->buffer[16]);
        if (payload_len > PROTO_OTA_FRAME_PAYLOAD_MAX) {
            proto_ota_frame_collector_reset(collector);
            return PROTO_OTA_COLLECT_ERROR;
        }
        collector->expected_length =
            (stc8h_u16)(PROTO_OTA_FRAME_OVERHEAD + payload_len);
        if (collector->expected_length > collector->capacity) {
            proto_ota_frame_collector_reset(collector);
            return PROTO_OTA_COLLECT_ERROR;
        }
    }
    if ((collector->expected_length != 0u) &&
        (collector->length == collector->expected_length)) {
        return PROTO_OTA_COLLECT_FRAME;
    }
    return PROTO_OTA_COLLECT_MORE;
}
