#include "stc8h_ota_receiver.h"

stc8h_u8 stc8h_ota_receiver_handle(
    stc8h_ota_context_t *context,
    const stc8h_u8 uid[STC8H_DEVICE_UID_SIZE],
    const proto_ota_frame_t *request,
    stc8h_u8 *reset_requested)
{
    STC8H_OTA_RECEIVER_WORK_MEM stc8h_ota_manifest_t manifest;
    stc8h_status_t result;
    stc8h_u8 index;

    if ((context == 0) || (uid == 0) || (request == 0) ||
        (reset_requested == 0)) {
        return PROTO_OTA_FRAME_STATUS_ERROR;
    }
    *reset_requested = 0u;
    result = STC8H_ERROR;
    switch (request->cmd) {
    case PROTO_OTA_FRAME_CMD_INFO:
    case PROTO_OTA_FRAME_CMD_STATUS:
        result = request->len == 0u ? STC8H_OK : STC8H_ERROR;
        break;
    case PROTO_OTA_FRAME_CMD_BEGIN:
        if ((request->session_id == 0UL) ||
            (request->len != STC8H_OTA_RECEIVER_BEGIN_SIZE)) {
            context->fail_reason = STC8H_OTA_FAIL_MANIFEST;
            break;
        }
        for (index = 0u; index < STC8H_DEVICE_UID_SIZE; ++index) {
            if (request->payload[STC8H_OTA_MANIFEST_WIRE_SIZE + index] !=
                uid[index]) {
                context->fail_reason = STC8H_OTA_FAIL_MANIFEST;
                return PROTO_OTA_FRAME_STATUS_ERROR;
            }
        }
        if (stc8h_ota_manifest_decode(request->payload,
                                      STC8H_OTA_MANIFEST_WIRE_SIZE,
                                      &manifest) != STC8H_OK) {
            context->fail_reason = STC8H_OTA_FAIL_MANIFEST;
            break;
        }
        result = stc8h_ota_begin(
            context, &manifest, request->session_id,
            (request->flags & PROTO_OTA_FRAME_FLAG_RESTART) != 0u ?
            STC8H_OTA_BEGIN_FLAG_RESTART : 0u);
        break;
    case PROTO_OTA_FRAME_CMD_DATA:
        if (request->session_id == context->session_id) {
            result = stc8h_ota_write_chunk(context, request->offset,
                                           request->payload, request->len);
        }
        break;
    case PROTO_OTA_FRAME_CMD_VERIFY:
        if ((request->session_id == context->session_id) &&
            (request->len == 0u)) {
            result = stc8h_ota_verify(context);
        }
        break;
    case PROTO_OTA_FRAME_CMD_ACTIVATE:
        if ((request->session_id == context->session_id) &&
            (request->len == 0u) &&
            (stc8h_ota_activate(context) == STC8H_OK)) {
            *reset_requested = 1u;
            result = STC8H_OK;
        }
        break;
    case PROTO_OTA_FRAME_CMD_ABORT:
        if ((request->session_id == context->session_id) &&
            (request->len == 0u)) {
            result = stc8h_ota_abort(context, STC8H_OTA_FAIL_STATE);
        }
        break;
    default:
        break;
    }
    return result == STC8H_OK ?
           PROTO_OTA_FRAME_STATUS_OK : PROTO_OTA_FRAME_STATUS_ERROR;
}
