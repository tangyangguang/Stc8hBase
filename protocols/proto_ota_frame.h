#ifndef PROTO_OTA_FRAME_H
#define PROTO_OTA_FRAME_H

#include "stc8h_config.h"

#define PROTO_OTA_FRAME_SOF0 0x4Fu
#define PROTO_OTA_FRAME_SOF1 0x54u
#define PROTO_OTA_FRAME_VERSION 1u
#define PROTO_OTA_FRAME_PAYLOAD_MAX 128u
#define PROTO_OTA_FRAME_HEADER_SIZE 14u
#define PROTO_OTA_FRAME_OVERHEAD 16u
#define PROTO_OTA_FRAME_WIRE_MAX (PROTO_OTA_FRAME_OVERHEAD + PROTO_OTA_FRAME_PAYLOAD_MAX)

typedef enum {
    PROTO_OTA_FRAME_CMD_BEGIN = 1,
    PROTO_OTA_FRAME_CMD_WRITE_BLOCK = 2,
    PROTO_OTA_FRAME_CMD_VERIFY = 3,
    PROTO_OTA_FRAME_CMD_COMMIT = 4,
    PROTO_OTA_FRAME_CMD_ABORT = 5,
    PROTO_OTA_FRAME_CMD_STATUS = 6
} proto_ota_frame_cmd_t;

#define PROTO_OTA_FRAME_STATUS_OK 0u
#define PROTO_OTA_FRAME_STATUS_ERROR 1u
#define PROTO_OTA_FRAME_STATUS_DUPLICATE 2u

typedef enum {
    PROTO_OTA_FRAME_PARSE_ERROR = 0,
    PROTO_OTA_FRAME_PARSE_OK = 1,
    PROTO_OTA_FRAME_PARSE_IGNORE = 2,
    PROTO_OTA_FRAME_PARSE_DUPLICATE = 3
} proto_ota_frame_parse_result_t;

typedef struct {
    stc8h_u8 version;
    stc8h_u8 dst;
    stc8h_u8 src;
    stc8h_u8 cmd;
    stc8h_u16 seq;
    stc8h_u32 offset;
    stc8h_u16 len;
    const stc8h_u8 *payload;
} proto_ota_frame_t;

stc8h_status_t proto_ota_frame_build(stc8h_u8 *frame,
                                     stc8h_u16 capacity,
                                     stc8h_u8 dst,
                                     stc8h_u8 src,
                                     stc8h_u8 cmd,
                                     stc8h_u16 seq,
                                     stc8h_u32 offset,
                                     const stc8h_u8 *payload,
                                     stc8h_u16 len,
                                     stc8h_u16 *frame_len);
proto_ota_frame_parse_result_t proto_ota_frame_parse(const stc8h_u8 *frame,
                                                     stc8h_u16 frame_len,
                                                     stc8h_u8 local_addr,
                                                     stc8h_u16 last_seq,
                                                     proto_ota_frame_t *out);

#endif
