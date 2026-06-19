#ifndef STC8H_OTA_FORMAT_H
#define STC8H_OTA_FORMAT_H

#include "stc8h_config.h"

#define STC8H_OTA_MANIFEST_WIRE_SIZE 31u
#define STC8H_OTA_PARAMS_WIRE_SIZE 31u

typedef struct {
    stc8h_u32 magic;
    stc8h_u8 format_version;
    stc8h_u16 target_chip;
    stc8h_u16 board_id;
    stc8h_u16 hw_revision;
    stc8h_u16 app_id;
    stc8h_u16 app_base;
    stc8h_u32 app_size;
    stc8h_u32 app_crc32;
    stc8h_u8 version_major;
    stc8h_u8 version_minor;
    stc8h_u8 version_patch;
    stc8h_u8 min_bootloader_version;
    stc8h_u16 flags;
    stc8h_u16 manifest_crc;
} stc8h_ota_manifest_t;

typedef struct {
    stc8h_u32 param_magic;
    stc8h_u8 param_version;
    stc8h_u16 sequence;
    stc8h_u8 state;
    stc8h_u8 app_valid;
    stc8h_u8 update_pending;
    stc8h_u8 boot_attempted;
    stc8h_u16 app_base;
    stc8h_u32 app_size;
    stc8h_u32 app_crc32;
    stc8h_u8 version_major;
    stc8h_u8 version_minor;
    stc8h_u8 version_patch;
    stc8h_u32 write_offset;
    stc8h_u8 fail_reason;
    stc8h_u16 param_crc;
} stc8h_ota_params_t;

stc8h_status_t stc8h_ota_manifest_decode(const stc8h_u8 *bytes, stc8h_u16 len, stc8h_ota_manifest_t *manifest);
stc8h_status_t stc8h_ota_manifest_encode(const stc8h_ota_manifest_t *manifest, stc8h_u8 *bytes, stc8h_u16 len);
stc8h_status_t stc8h_ota_params_decode(const stc8h_u8 *bytes, stc8h_u16 len, stc8h_ota_params_t *params);
stc8h_status_t stc8h_ota_params_encode(const stc8h_ota_params_t *params, stc8h_u8 *bytes, stc8h_u16 len);

#endif
