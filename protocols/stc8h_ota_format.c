#include "stc8h_ota_format.h"

static stc8h_u16 stc8h_ota_crc16_modbus(const stc8h_u8 *data, stc8h_u16 len)
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

static stc8h_u16 stc8h_ota_get_le16(const stc8h_u8 *bytes)
{
    return (stc8h_u16)(((stc8h_u16)bytes[1] << 8) | bytes[0]);
}

static stc8h_u32 stc8h_ota_get_le32(const stc8h_u8 *bytes)
{
    return ((stc8h_u32)bytes[3] << 24) |
           ((stc8h_u32)bytes[2] << 16) |
           ((stc8h_u32)bytes[1] << 8) |
           (stc8h_u32)bytes[0];
}

static void stc8h_ota_put_le16(stc8h_u8 *bytes, stc8h_u16 value)
{
    bytes[0] = (stc8h_u8)(value & 0xFFu);
    bytes[1] = (stc8h_u8)((value >> 8) & 0xFFu);
}

static void stc8h_ota_put_le32(stc8h_u8 *bytes, stc8h_u32 value)
{
    bytes[0] = (stc8h_u8)(value & 0xFFUL);
    bytes[1] = (stc8h_u8)((value >> 8) & 0xFFUL);
    bytes[2] = (stc8h_u8)((value >> 16) & 0xFFUL);
    bytes[3] = (stc8h_u8)((value >> 24) & 0xFFUL);
}

stc8h_status_t stc8h_ota_manifest_decode(const stc8h_u8 *bytes, stc8h_u16 len, stc8h_ota_manifest_t *manifest)
{
    stc8h_u16 expected_crc;
    stc8h_u16 got_crc;

    if ((bytes == 0) || (manifest == 0) || (len != STC8H_OTA_MANIFEST_WIRE_SIZE)) {
        return STC8H_ERROR;
    }

    expected_crc = stc8h_ota_crc16_modbus(bytes, (stc8h_u16)(STC8H_OTA_MANIFEST_WIRE_SIZE - 2u));
    got_crc = stc8h_ota_get_le16(&bytes[29]);
    if (expected_crc != got_crc) {
        return STC8H_ERROR;
    }

    manifest->magic = stc8h_ota_get_le32(&bytes[0]);
    manifest->format_version = bytes[4];
    manifest->target_chip = stc8h_ota_get_le16(&bytes[5]);
    manifest->board_id = stc8h_ota_get_le16(&bytes[7]);
    manifest->hw_revision = stc8h_ota_get_le16(&bytes[9]);
    manifest->app_id = stc8h_ota_get_le16(&bytes[11]);
    manifest->app_base = stc8h_ota_get_le16(&bytes[13]);
    manifest->app_size = stc8h_ota_get_le32(&bytes[15]);
    manifest->app_crc32 = stc8h_ota_get_le32(&bytes[19]);
    manifest->version_major = bytes[23];
    manifest->version_minor = bytes[24];
    manifest->version_patch = bytes[25];
    manifest->min_bootloader_version = bytes[26];
    manifest->flags = stc8h_ota_get_le16(&bytes[27]);
    manifest->manifest_crc = got_crc;
    return STC8H_OK;
}

#if STC8H_OTA_FORMAT_ENABLE_MANIFEST_ENCODE
stc8h_status_t stc8h_ota_manifest_encode(const stc8h_ota_manifest_t *manifest, stc8h_u8 *bytes, stc8h_u16 len)
{
    stc8h_u16 crc;

    if ((manifest == 0) || (bytes == 0) || (len != STC8H_OTA_MANIFEST_WIRE_SIZE)) {
        return STC8H_ERROR;
    }

    stc8h_ota_put_le32(&bytes[0], manifest->magic);
    bytes[4] = manifest->format_version;
    stc8h_ota_put_le16(&bytes[5], manifest->target_chip);
    stc8h_ota_put_le16(&bytes[7], manifest->board_id);
    stc8h_ota_put_le16(&bytes[9], manifest->hw_revision);
    stc8h_ota_put_le16(&bytes[11], manifest->app_id);
    stc8h_ota_put_le16(&bytes[13], manifest->app_base);
    stc8h_ota_put_le32(&bytes[15], manifest->app_size);
    stc8h_ota_put_le32(&bytes[19], manifest->app_crc32);
    bytes[23] = manifest->version_major;
    bytes[24] = manifest->version_minor;
    bytes[25] = manifest->version_patch;
    bytes[26] = manifest->min_bootloader_version;
    stc8h_ota_put_le16(&bytes[27], manifest->flags);
    crc = stc8h_ota_crc16_modbus(bytes, (stc8h_u16)(STC8H_OTA_MANIFEST_WIRE_SIZE - 2u));
    stc8h_ota_put_le16(&bytes[29], crc);
    return STC8H_OK;
}
#endif

stc8h_status_t stc8h_ota_params_decode(const stc8h_u8 *bytes, stc8h_u16 len, stc8h_ota_params_t *params)
{
    stc8h_u16 expected_crc;
    stc8h_u16 got_crc;

    if ((bytes == 0) || (params == 0) || (len != STC8H_OTA_PARAMS_WIRE_SIZE)) {
        return STC8H_ERROR;
    }

    expected_crc = stc8h_ota_crc16_modbus(bytes, (stc8h_u16)(STC8H_OTA_PARAMS_WIRE_SIZE - 2u));
    got_crc = stc8h_ota_get_le16(&bytes[29]);
    if (expected_crc != got_crc) {
        return STC8H_ERROR;
    }

    params->param_magic = stc8h_ota_get_le32(&bytes[0]);
    params->param_version = bytes[4];
    params->sequence = stc8h_ota_get_le16(&bytes[5]);
    params->state = bytes[7];
    params->app_valid = bytes[8];
    params->update_pending = bytes[9];
    params->boot_attempted = bytes[10];
    params->app_base = stc8h_ota_get_le16(&bytes[11]);
    params->app_size = stc8h_ota_get_le32(&bytes[13]);
    params->app_crc32 = stc8h_ota_get_le32(&bytes[17]);
    params->version_major = bytes[21];
    params->version_minor = bytes[22];
    params->version_patch = bytes[23];
    params->write_offset = stc8h_ota_get_le32(&bytes[24]);
    params->fail_reason = bytes[28];
    params->param_crc = got_crc;
    return STC8H_OK;
}

#if STC8H_OTA_FORMAT_ENABLE_PARAMS_ENCODE
stc8h_status_t stc8h_ota_params_encode(const stc8h_ota_params_t *params, stc8h_u8 *bytes, stc8h_u16 len)
{
    stc8h_u16 crc;

    if ((params == 0) || (bytes == 0) || (len != STC8H_OTA_PARAMS_WIRE_SIZE)) {
        return STC8H_ERROR;
    }

    stc8h_ota_put_le32(&bytes[0], params->param_magic);
    bytes[4] = params->param_version;
    stc8h_ota_put_le16(&bytes[5], params->sequence);
    bytes[7] = params->state;
    bytes[8] = params->app_valid;
    bytes[9] = params->update_pending;
    bytes[10] = params->boot_attempted;
    stc8h_ota_put_le16(&bytes[11], params->app_base);
    stc8h_ota_put_le32(&bytes[13], params->app_size);
    stc8h_ota_put_le32(&bytes[17], params->app_crc32);
    bytes[21] = params->version_major;
    bytes[22] = params->version_minor;
    bytes[23] = params->version_patch;
    stc8h_ota_put_le32(&bytes[24], params->write_offset);
    bytes[28] = params->fail_reason;
    crc = stc8h_ota_crc16_modbus(bytes, (stc8h_u16)(STC8H_OTA_PARAMS_WIRE_SIZE - 2u));
    stc8h_ota_put_le16(&bytes[29], crc);
    return STC8H_OK;
}
#endif
