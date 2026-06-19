#include "stc8h_ota.h"
#include "stc8h_ota_params_store.h"
#include "util_crc32.h"

#define STC8H_OTA_READ_CHUNK_SIZE 16u

static stc8h_status_t stc8h_ota_validate_app_range(stc8h_u16 app_base, stc8h_u32 app_size)
{
    stc8h_u32 app_end;

    if ((app_base != STC8H_OTA_APP_BASE) || (app_size == 0UL)) {
        return STC8H_ERROR;
    }

    app_end = (stc8h_u32)app_base + app_size - 1UL;
    if (app_end > (stc8h_u32)STC8H_OTA_APP_LIMIT) {
        return STC8H_ERROR;
    }

    return STC8H_OK;
}

static stc8h_u16 stc8h_ota_get_sector_size(const stc8h_ota_backend_t *backend)
{
    if ((backend == 0) || (backend->sector_size == 0u)) {
        return STC8H_OTA_DEFAULT_SECTOR_SIZE;
    }

    return backend->sector_size;
}

static stc8h_status_t stc8h_ota_erase_app_area(stc8h_ota_context_t *ctx)
{
    stc8h_u32 addr;
    stc8h_u32 end;
    stc8h_u16 sector_size;

    if ((ctx == 0) || (ctx->backend == 0) || (ctx->backend->erase_sector == 0)) {
        return STC8H_ERROR;
    }

    sector_size = stc8h_ota_get_sector_size(ctx->backend);
    addr = (stc8h_u32)ctx->manifest.app_base;
    end = (stc8h_u32)ctx->manifest.app_base + ctx->manifest.app_size;

    while (addr < end) {
        if (ctx->backend->erase_sector((stc8h_u16)addr) != STC8H_OK) {
            return STC8H_ERROR;
        }
        addr += sector_size;
    }

    return STC8H_OK;
}

static stc8h_status_t stc8h_ota_compare_written_chunk(stc8h_ota_context_t *ctx,
                                                      stc8h_u32 offset,
                                                      const stc8h_u8 *data,
                                                      stc8h_u16 len)
{
    stc8h_u8 buffer[STC8H_OTA_READ_CHUNK_SIZE];
    stc8h_u16 pos;
    stc8h_u16 read_len;
    stc8h_u16 i;

    if ((ctx == 0) || (ctx->backend == 0) || (ctx->backend->read == 0) ||
        ((len != 0u) && (data == 0))) {
        return STC8H_ERROR;
    }

    pos = 0u;
    while (pos < len) {
        read_len = (stc8h_u16)(len - pos);
        if (read_len > STC8H_OTA_READ_CHUNK_SIZE) {
            read_len = STC8H_OTA_READ_CHUNK_SIZE;
        }
        if (ctx->backend->read((stc8h_u16)((stc8h_u32)ctx->manifest.app_base + offset + pos),
                               buffer,
                               read_len) != STC8H_OK) {
            return STC8H_ERROR;
        }
        for (i = 0u; i < read_len; ++i) {
            if (buffer[i] != data[(stc8h_u16)(pos + i)]) {
                return STC8H_ERROR;
            }
        }
        pos = (stc8h_u16)(pos + read_len);
    }

    return STC8H_OK;
}

static stc8h_status_t stc8h_ota_read_image_crc32(stc8h_ota_context_t *ctx,
                                                 stc8h_u32 *crc32)
{
    stc8h_u8 buffer[STC8H_OTA_READ_CHUNK_SIZE];
    stc8h_u32 offset;
    stc8h_u32 remaining;
    stc8h_u16 read_len;
    stc8h_u32 crc;

    if ((ctx == 0) || (crc32 == 0) || (ctx->backend == 0) || (ctx->backend->read == 0)) {
        return STC8H_ERROR;
    }

    crc = 0UL;
    offset = 0UL;
    remaining = ctx->manifest.app_size;
    while (remaining != 0UL) {
        read_len = (remaining > STC8H_OTA_READ_CHUNK_SIZE) ?
                   STC8H_OTA_READ_CHUNK_SIZE : (stc8h_u16)remaining;
        if (ctx->backend->read((stc8h_u16)((stc8h_u32)ctx->manifest.app_base + offset),
                               buffer,
                               read_len) != STC8H_OK) {
            return STC8H_ERROR;
        }
        crc = util_crc32_ieee_update(crc, buffer, read_len);
        offset += read_len;
        remaining -= read_len;
    }

    *crc32 = crc;
    return STC8H_OK;
}

void stc8h_ota_init(stc8h_ota_context_t *ctx,
                    const stc8h_ota_backend_t *backend,
                    stc8h_ota_params_store_t *params_store)
{
    if (ctx == 0) {
        return;
    }

    ctx->backend = backend;
    ctx->params_store = params_store;
    ctx->write_offset = 0UL;
    ctx->last_chunk_offset = 0UL;
    ctx->last_chunk_len = 0u;
    ctx->state = STC8H_OTA_STATE_IDLE;
    ctx->fail_reason = 0u;
}

stc8h_status_t stc8h_ota_validate_manifest(const stc8h_ota_manifest_t *manifest)
{
    if (manifest == 0) {
        return STC8H_ERROR;
    }

    if (manifest->magic != STC8H_OTA_MANIFEST_MAGIC) {
        return STC8H_ERROR;
    }
    if (manifest->format_version != STC8H_OTA_FORMAT_VERSION) {
        return STC8H_ERROR;
    }
    if (manifest->target_chip != STC8H_OTA_TARGET_STC8H8K64U) {
        return STC8H_ERROR;
    }
    if (manifest->min_bootloader_version > STC8H_OTA_BOOTLOADER_VERSION) {
        return STC8H_ERROR;
    }
    if (stc8h_ota_validate_app_range(manifest->app_base, manifest->app_size) != STC8H_OK) {
        return STC8H_ERROR;
    }

#if STC8H_OTA_CHECK_BOARD_ID
    if (manifest->board_id != STC8H_OTA_EXPECTED_BOARD_ID) {
        return STC8H_ERROR;
    }
#endif
#if STC8H_OTA_CHECK_HW_REVISION
    if (manifest->hw_revision != STC8H_OTA_EXPECTED_HW_REVISION) {
        return STC8H_ERROR;
    }
#endif
#if STC8H_OTA_CHECK_APP_ID
    if (manifest->app_id != STC8H_OTA_EXPECTED_APP_ID) {
        return STC8H_ERROR;
    }
#endif

    return STC8H_OK;
}

stc8h_ota_boot_action_t stc8h_ota_get_boot_action(const stc8h_ota_params_t *params)
{
    if (params == 0) {
        return STC8H_OTA_BOOT_ACTION_STAY_BOOTLOADER;
    }

    if ((params->param_magic != STC8H_OTA_PARAM_MAGIC) ||
        (params->param_version != STC8H_OTA_PARAM_VERSION)) {
        return STC8H_OTA_BOOT_ACTION_STAY_BOOTLOADER;
    }

    if (stc8h_ota_validate_app_range(params->app_base, params->app_size) != STC8H_OK) {
        return STC8H_OTA_BOOT_ACTION_STAY_BOOTLOADER;
    }

    if (params->update_pending != 0u) {
        return STC8H_OTA_BOOT_ACTION_STAY_BOOTLOADER;
    }

    if (params->app_valid != 0u) {
        return STC8H_OTA_BOOT_ACTION_JUMP_APP;
    }

    if ((params->state == STC8H_OTA_STATE_COMMITTED) && (params->boot_attempted == 0u)) {
        return STC8H_OTA_BOOT_ACTION_TRIAL_APP;
    }

    return STC8H_OTA_BOOT_ACTION_STAY_BOOTLOADER;
}

stc8h_u8 stc8h_ota_should_enter_bootloader(const stc8h_ota_params_t *params)
{
    return (stc8h_ota_get_boot_action(params) == STC8H_OTA_BOOT_ACTION_STAY_BOOTLOADER) ? 1u : 0u;
}

stc8h_status_t stc8h_ota_begin(stc8h_ota_context_t *ctx,
                               const stc8h_ota_manifest_t *manifest)
{
    if ((ctx == 0) || (manifest == 0) || (ctx->backend == 0) ||
        (ctx->backend->write == 0) || (ctx->backend->read == 0)) {
        return STC8H_ERROR;
    }

    if (stc8h_ota_validate_manifest(manifest) != STC8H_OK) {
        ctx->state = STC8H_OTA_STATE_FAILED;
        return STC8H_ERROR;
    }

    ctx->manifest = *manifest;
    ctx->write_offset = 0UL;
    ctx->last_chunk_offset = 0UL;
    ctx->last_chunk_len = 0u;
    ctx->fail_reason = 0u;
    ctx->state = STC8H_OTA_STATE_PREPARING;

    if (stc8h_ota_erase_app_area(ctx) != STC8H_OK) {
        ctx->state = STC8H_OTA_STATE_FAILED;
        return STC8H_ERROR;
    }

    ctx->state = STC8H_OTA_STATE_RECEIVING;
    return STC8H_OK;
}

stc8h_status_t stc8h_ota_write_chunk(stc8h_ota_context_t *ctx,
                                     stc8h_u32 offset,
                                     const stc8h_u8 *data,
                                     stc8h_u16 len)
{
    stc8h_u32 end_offset;

    if ((ctx == 0) || (ctx->backend == 0) || (ctx->backend->write == 0) ||
        (ctx->state != STC8H_OTA_STATE_RECEIVING) || (len == 0u) || (data == 0)) {
        return STC8H_ERROR;
    }

    if (offset < ctx->write_offset) {
        if ((offset == ctx->last_chunk_offset) && (len == ctx->last_chunk_len)) {
            return stc8h_ota_compare_written_chunk(ctx, offset, data, len);
        }
        return STC8H_ERROR;
    }

    if (offset != ctx->write_offset) {
        return STC8H_ERROR;
    }

    end_offset = offset + len;
    if (end_offset > ctx->manifest.app_size) {
        return STC8H_ERROR;
    }

    if (ctx->backend->write((stc8h_u16)((stc8h_u32)ctx->manifest.app_base + offset),
                            data,
                            len) != STC8H_OK) {
        return STC8H_ERROR;
    }

    ctx->last_chunk_offset = offset;
    ctx->last_chunk_len = len;
    ctx->write_offset = end_offset;
    return STC8H_OK;
}

stc8h_status_t stc8h_ota_verify(stc8h_ota_context_t *ctx)
{
    stc8h_u32 crc32;

    if ((ctx == 0) || (ctx->state != STC8H_OTA_STATE_RECEIVING)) {
        return STC8H_ERROR;
    }

    if (ctx->write_offset != ctx->manifest.app_size) {
        return STC8H_ERROR;
    }

    ctx->state = STC8H_OTA_STATE_VERIFYING;
    if (stc8h_ota_read_image_crc32(ctx, &crc32) != STC8H_OK) {
        ctx->state = STC8H_OTA_STATE_FAILED;
        return STC8H_ERROR;
    }
    if (crc32 != ctx->manifest.app_crc32) {
        ctx->state = STC8H_OTA_STATE_FAILED;
        return STC8H_ERROR;
    }

    ctx->state = STC8H_OTA_STATE_PENDING_COMMIT;
    return STC8H_OK;
}

stc8h_status_t stc8h_ota_commit(stc8h_ota_context_t *ctx)
{
    stc8h_ota_params_t params;
    stc8h_ota_params_t active;
    stc8h_u16 sequence;

    if ((ctx == 0) || (ctx->params_store == 0) ||
        (ctx->state != STC8H_OTA_STATE_PENDING_COMMIT)) {
        return STC8H_ERROR;
    }

    sequence = 1u;
    if (stc8h_ota_params_store_load_active(ctx->params_store, &active) == STC8H_OK) {
        sequence = (stc8h_u16)(active.sequence + 1u);
    }

    params.param_magic = STC8H_OTA_PARAM_MAGIC;
    params.param_version = STC8H_OTA_PARAM_VERSION;
    params.sequence = sequence;
    params.state = STC8H_OTA_STATE_COMMITTED;
    params.app_valid = 0u;
    params.update_pending = 0u;
    params.boot_attempted = 0u;
    params.app_base = ctx->manifest.app_base;
    params.app_size = ctx->manifest.app_size;
    params.app_crc32 = ctx->manifest.app_crc32;
    params.version_major = ctx->manifest.version_major;
    params.version_minor = ctx->manifest.version_minor;
    params.version_patch = ctx->manifest.version_patch;
    params.write_offset = ctx->write_offset;
    params.fail_reason = 0u;
    params.param_crc = 0u;

    if (stc8h_ota_params_store_write_next(ctx->params_store, &params) != STC8H_OK) {
        ctx->state = STC8H_OTA_STATE_FAILED;
        return STC8H_ERROR;
    }

    ctx->state = STC8H_OTA_STATE_COMMITTED;
    return STC8H_OK;
}

stc8h_status_t stc8h_ota_abort(stc8h_ota_context_t *ctx, stc8h_u8 reason)
{
    if (ctx == 0) {
        return STC8H_ERROR;
    }

    ctx->write_offset = 0UL;
    ctx->last_chunk_offset = 0UL;
    ctx->last_chunk_len = 0u;
    ctx->fail_reason = reason;
    ctx->state = STC8H_OTA_STATE_FAILED;
    return STC8H_OK;
}

stc8h_ota_state_t stc8h_ota_get_status(const stc8h_ota_context_t *ctx)
{
    if (ctx == 0) {
        return STC8H_OTA_STATE_FAILED;
    }

    return ctx->state;
}
