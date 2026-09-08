#ifndef STC8H_OTA_RECEIVER_H
#define STC8H_OTA_RECEIVER_H

#include "proto_ota_frame.h"
#include "stc8h_chip_identity.h"
#include "stc8h_ota.h"

#define STC8H_OTA_RECEIVER_STATUS_SIZE 20u
#define STC8H_OTA_RECEIVER_INFO_SIZE \
    (STC8H_OTA_RECEIVER_STATUS_SIZE + STC8H_DEVICE_UID_SIZE)
#define STC8H_OTA_RECEIVER_BEGIN_SIZE \
    (STC8H_OTA_MANIFEST_WIRE_SIZE + STC8H_DEVICE_UID_SIZE)

#ifndef STC8H_OTA_RECEIVER_WORK_MEM
#define STC8H_OTA_RECEIVER_WORK_MEM
#endif

/* Transport-neutral command handler for one already parsed unicast frame.
 * Duplicate-frame caching, response framing, and reset-after-ACK remain in the
 * transport adapter. */
stc8h_u8 stc8h_ota_receiver_handle(
    stc8h_ota_context_t *context,
    const stc8h_u8 uid[STC8H_DEVICE_UID_SIZE],
    const proto_ota_frame_t *request,
    stc8h_u8 *reset_requested);

#endif
