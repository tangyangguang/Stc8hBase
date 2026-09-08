#include "stc8h_ota.h"
#include "stc8h_ota_params_store.h"
#include "util_crc32.h"

#define STC8H_OTA_READ_CHUNK_SIZE 16u

static stc8h_status_t stc8h_ota_validate_app_range(stc8h_u16 app_base,
                                                    stc8h_u16 app_size)
{
    stc8h_u32 app_end;

    if ((app_base != STC8H_OTA_APP_BASE) || (app_size == 0UL)) {
        return STC8H_ERROR;
    }
    app_end = (stc8h_u32)app_base + app_size - 1UL;
    return app_end <= (stc8h_u32)STC8H_OTA_APP_LIMIT ?
           STC8H_OK : STC8H_ERROR;
}

static stc8h_u16 stc8h_ota_get_sector_size(const stc8h_ota_backend_t *backend)
{
    if ((backend == 0) || (backend->sector_size == 0u)) {
        return STC8H_OTA_DEFAULT_SECTOR_SIZE;
    }
    return backend->sector_size;
}

static void stc8h_ota_keep_alive(const stc8h_ota_context_t *ctx)
{
    if ((ctx != 0) && (ctx->backend != 0) &&
        (ctx->backend->keep_alive != 0)) {
        ctx->backend->keep_alive();
    }
}

static void stc8h_ota_copy_manifest(stc8h_ota_manifest_t *dst,
                                    const stc8h_ota_manifest_t *src)
{
    dst->magic = src->magic;
    dst->format_version = src->format_version;
    dst->target_chip = src->target_chip;
    dst->board_id = src->board_id;
    dst->hw_revision = src->hw_revision;
    dst->app_id = src->app_id;
    dst->app_base = src->app_base;
    dst->app_size = src->app_size;
    dst->app_crc32 = src->app_crc32;
    dst->version_major = src->version_major;
    dst->version_minor = src->version_minor;
    dst->version_patch = src->version_patch;
    dst->min_bootloader_version = src->min_bootloader_version;
    dst->build_number = src->build_number;
    dst->flags = src->flags;
    dst->manifest_crc = src->manifest_crc;
}

static void stc8h_ota_manifest_from_params(stc8h_ota_manifest_t *manifest,
                                           const stc8h_ota_params_t *params)
{
    manifest->magic = STC8H_OTA_MANIFEST_MAGIC;
    manifest->format_version = STC8H_OTA_FORMAT_VERSION;
    manifest->target_chip = STC8H_OTA_TARGET_STC8H8K64U;
    manifest->board_id = STC8H_OTA_EXPECTED_BOARD_ID;
    manifest->hw_revision = STC8H_OTA_EXPECTED_HW_REVISION;
    manifest->app_id = STC8H_OTA_EXPECTED_APP_ID;
    manifest->app_base = params->app_base;
    manifest->app_size = params->app_size;
    manifest->app_crc32 = params->app_crc32;
    manifest->version_major = params->version_major;
    manifest->version_minor = params->version_minor;
    manifest->version_patch = params->version_patch;
    manifest->min_bootloader_version = params->min_bootloader_version;
    manifest->build_number = params->build_number;
    manifest->flags = 0u;
    manifest->manifest_crc = params->manifest_crc;
}

static stc8h_status_t stc8h_ota_save(stc8h_ota_context_t *ctx,
                                     stc8h_ota_state_t state,
                                     stc8h_u16 committed_offset,
                                     stc8h_u8 fail_reason)
{
    STC8H_OTA_WORK_MEM stc8h_ota_params_t params;
    STC8H_OTA_WORK_MEM stc8h_ota_params_t active;

    if ((ctx == 0) || (ctx->params_store == 0)) {
        return STC8H_ERROR;
    }
    params.param_magic = STC8H_OTA_PARAM_MAGIC;
    params.param_version = STC8H_OTA_PARAM_VERSION;
    params.state = (stc8h_u8)state;
    params.flags = 0u;
    params.fail_reason = fail_reason;
    params.generation = 1u;
    if (stc8h_ota_params_store_load_active(ctx->params_store, &active) ==
        STC8H_OK) {
        params.generation = (stc8h_u16)(active.generation + 1u);
    }
    params.session_id = ctx->session_id;
    params.app_base = ctx->manifest.app_base;
    params.app_size = ctx->manifest.app_size;
    params.app_crc32 = ctx->manifest.app_crc32;
    params.version_major = ctx->manifest.version_major;
    params.version_minor = ctx->manifest.version_minor;
    params.version_patch = ctx->manifest.version_patch;
    params.min_bootloader_version = ctx->manifest.min_bootloader_version;
    params.build_number = ctx->manifest.build_number;
    params.committed_offset = committed_offset;
    params.manifest_crc = ctx->manifest.manifest_crc;
    params.param_crc = 0u;
    params.commit_marker = STC8H_OTA_PARAM_COMMIT_MARKER;
    if (state == STC8H_OTA_STATE_TRIAL_PENDING) {
        params.flags = 0u;
    }
    if (stc8h_ota_params_store_write_next(ctx->params_store, &params) !=
        STC8H_OK) {
        return STC8H_ERROR;
    }
    ctx->state = state;
    ctx->persisted_offset = committed_offset;
    ctx->fail_reason = fail_reason;
    return STC8H_OK;
}

static void stc8h_ota_fail(stc8h_ota_context_t *ctx,
                           stc8h_ota_fail_reason_t reason,
                           stc8h_u8 persist)
{
    if (ctx == 0) {
        return;
    }
    ctx->state = STC8H_OTA_STATE_FAILED;
    ctx->fail_reason = (stc8h_u8)reason;
    if ((persist != 0u) && (ctx->params_store != 0) &&
        (ctx->manifest.app_size != 0UL)) {
        (void)stc8h_ota_save(ctx, STC8H_OTA_STATE_FAILED,
                              ctx->persisted_offset, (stc8h_u8)reason);
    }
}

static stc8h_status_t stc8h_ota_erase_app_area(stc8h_ota_context_t *ctx)
{
    stc8h_u16 addr;
    stc8h_u16 remaining;
    stc8h_u16 sector_size;

    if ((ctx == 0) || (ctx->backend == 0) ||
        (ctx->backend->erase_sector == 0)) {
        return STC8H_ERROR;
    }
    sector_size = stc8h_ota_get_sector_size(ctx->backend);
    addr = ctx->manifest.app_base;
    remaining = ctx->manifest.app_size;
    while (remaining != 0u) {
        if (ctx->backend->erase_sector(addr) != STC8H_OK) {
            return STC8H_ERROR;
        }
        stc8h_ota_keep_alive(ctx);
        if (remaining <= sector_size) {
            remaining = 0u;
        } else {
            remaining = (stc8h_u16)(remaining - sector_size);
            addr = (stc8h_u16)(addr + sector_size);
        }
    }
    return STC8H_OK;
}

static stc8h_status_t stc8h_ota_compare_written(
    stc8h_ota_context_t *ctx,
    stc8h_u16 offset,
    const stc8h_u8 *data,
    stc8h_u16 len)
{
    STC8H_OTA_WORK_MEM stc8h_u8 buffer[STC8H_OTA_READ_CHUNK_SIZE];
    stc8h_u16 pos;
    stc8h_u16 read_len;
    stc8h_u16 i;

    if ((ctx == 0) || (ctx->backend == 0) ||
        (ctx->backend->read == 0) || ((len != 0u) && (data == 0))) {
        return STC8H_ERROR;
    }
    pos = 0u;
    while (pos < len) {
        read_len = (stc8h_u16)(len - pos);
        if (read_len > STC8H_OTA_READ_CHUNK_SIZE) {
            read_len = STC8H_OTA_READ_CHUNK_SIZE;
        }
        if (ctx->backend->read(
                (stc8h_u16)((stc8h_u32)ctx->manifest.app_base + offset + pos),
                buffer, read_len) != STC8H_OK) {
            return STC8H_ERROR;
        }
        for (i = 0u; i < read_len; ++i) {
            if (buffer[i] != data[(stc8h_u16)(pos + i)]) {
                return STC8H_ERROR;
            }
        }
        stc8h_ota_keep_alive(ctx);
        pos = (stc8h_u16)(pos + read_len);
    }
    return STC8H_OK;
}

static stc8h_status_t stc8h_ota_read_image_crc32(stc8h_ota_context_t *ctx,
                                                  stc8h_u32 *crc32)
{
    STC8H_OTA_WORK_MEM stc8h_u8 buffer[STC8H_OTA_READ_CHUNK_SIZE];
    stc8h_u16 offset;
    stc8h_u16 remaining;
    stc8h_u16 read_len;
    stc8h_u32 crc;

    if ((ctx == 0) || (crc32 == 0) || (ctx->backend == 0) ||
        (ctx->backend->read == 0)) {
        return STC8H_ERROR;
    }
    crc = 0UL;
    offset = 0u;
    remaining = ctx->manifest.app_size;
    while (remaining != 0u) {
        read_len = (remaining > STC8H_OTA_READ_CHUNK_SIZE) ?
                   STC8H_OTA_READ_CHUNK_SIZE : (stc8h_u16)remaining;
        if (ctx->backend->read(
                (stc8h_u16)((stc8h_u32)ctx->manifest.app_base + offset),
                buffer, read_len) != STC8H_OK) {
            return STC8H_ERROR;
        }
        crc = util_crc32_ieee_update(crc, buffer, read_len);
        stc8h_ota_keep_alive(ctx);
        offset = (stc8h_u16)(offset + read_len);
        remaining = (stc8h_u16)(remaining - read_len);
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
    ctx->session_id = 0UL;
    ctx->write_offset = 0u;
    ctx->persisted_offset = 0u;
    ctx->state = STC8H_OTA_STATE_EMPTY;
    ctx->fail_reason = 0u;
    ctx->manifest.app_size = 0u;
}

stc8h_status_t stc8h_ota_restore(stc8h_ota_context_t *ctx)
{
    STC8H_OTA_WORK_MEM stc8h_ota_params_t params;

    if ((ctx == 0) || (ctx->params_store == 0) ||
        (stc8h_ota_params_store_load_active(ctx->params_store, &params) !=
         STC8H_OK)) {
        return STC8H_ERROR;
    }
    if ((params.param_magic != STC8H_OTA_PARAM_MAGIC) ||
        (params.param_version != STC8H_OTA_PARAM_VERSION) ||
        (stc8h_ota_validate_app_range(params.app_base, params.app_size) !=
         STC8H_OK) || (params.committed_offset > params.app_size)) {
        return STC8H_ERROR;
    }
    stc8h_ota_manifest_from_params(&ctx->manifest, &params);
    ctx->session_id = params.session_id;
    ctx->write_offset = params.committed_offset;
    ctx->persisted_offset = params.committed_offset;
    ctx->state = (stc8h_ota_state_t)params.state;
    ctx->fail_reason = params.fail_reason;
    return STC8H_OK;
}

stc8h_status_t stc8h_ota_validate_manifest(const stc8h_ota_manifest_t *manifest)
{
    if ((manifest == 0) ||
        (manifest->magic != STC8H_OTA_MANIFEST_MAGIC) ||
        (manifest->format_version != STC8H_OTA_FORMAT_VERSION) ||
        (manifest->target_chip != STC8H_OTA_TARGET_STC8H8K64U) ||
        (manifest->min_bootloader_version > STC8H_OTA_BOOTLOADER_VERSION) ||
        (stc8h_ota_validate_app_range(manifest->app_base,
                                      manifest->app_size) != STC8H_OK)) {
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

stc8h_ota_boot_action_t stc8h_ota_get_boot_action(
    const stc8h_ota_params_t *params)
{
    if ((params == 0) || (params->param_magic != STC8H_OTA_PARAM_MAGIC) ||
        (params->param_version != STC8H_OTA_PARAM_VERSION) ||
        (stc8h_ota_validate_app_range(params->app_base,
                                      params->app_size) != STC8H_OK)) {
        return STC8H_OTA_BOOT_ACTION_STAY_BOOTLOADER;
    }
    if ((params->state == STC8H_OTA_STATE_APP_VALID) &&
        ((params->flags & STC8H_OTA_PARAM_FLAG_APP_VALID) != 0u) &&
        ((params->flags & STC8H_OTA_PARAM_FLAG_UPDATE_REQUESTED) == 0u)) {
        return STC8H_OTA_BOOT_ACTION_JUMP_APP;
    }
    if ((params->state == STC8H_OTA_STATE_TRIAL_PENDING) &&
        ((params->flags & STC8H_OTA_PARAM_FLAG_TRIAL_ATTEMPTED) == 0u)) {
        return STC8H_OTA_BOOT_ACTION_TRIAL_APP;
    }
    return STC8H_OTA_BOOT_ACTION_STAY_BOOTLOADER;
}

#if STC8H_OTA_ENABLE_SHOULD_ENTER_BOOTLOADER
stc8h_u8 stc8h_ota_should_enter_bootloader(const stc8h_ota_params_t *params)
{
    return stc8h_ota_get_boot_action(params) ==
           STC8H_OTA_BOOT_ACTION_STAY_BOOTLOADER ? 1u : 0u;
}
#endif

stc8h_status_t stc8h_ota_begin(stc8h_ota_context_t *ctx,
                               const stc8h_ota_manifest_t *manifest,
                               stc8h_u32 session_id,
                               stc8h_u8 flags)
{
    STC8H_OTA_WORK_MEM stc8h_ota_params_t active;
    stc8h_u8 has_active;
    stc8h_u8 same_transfer;

    if ((ctx == 0) || (manifest == 0) || (session_id == 0UL) ||
        (ctx->backend == 0) || (ctx->backend->erase_sector == 0) ||
        (ctx->backend->write == 0) || (ctx->backend->read == 0) ||
        (ctx->params_store == 0)) {
        stc8h_ota_fail(ctx, STC8H_OTA_FAIL_ARG, 0u);
        return STC8H_ERROR;
    }
    if (stc8h_ota_validate_manifest(manifest) != STC8H_OK) {
        stc8h_ota_fail(ctx, STC8H_OTA_FAIL_MANIFEST, 0u);
        return STC8H_ERROR;
    }

    has_active = stc8h_ota_params_store_load_active(
        ctx->params_store, &active) == STC8H_OK ? 1u : 0u;
    same_transfer = 0u;
    if ((has_active != 0u) && (active.session_id == session_id) &&
        (active.manifest_crc == manifest->manifest_crc) &&
        (active.app_size == manifest->app_size) &&
        (active.app_crc32 == manifest->app_crc32)) {
        same_transfer = 1u;
    }

    if ((has_active != 0u) &&
        (active.state == STC8H_OTA_STATE_APP_VALID)) {
        stc8h_ota_fail(ctx, STC8H_OTA_FAIL_NOT_REQUESTED, 0u);
        return STC8H_ERROR;
    }
    if ((has_active != 0u) &&
        (active.state == STC8H_OTA_STATE_UPDATE_REQUESTED) &&
        (active.session_id != session_id)) {
        stc8h_ota_fail(ctx, STC8H_OTA_FAIL_SESSION, 0u);
        return STC8H_ERROR;
    }
    if ((same_transfer != 0u) &&
        (active.state == STC8H_OTA_STATE_RECEIVING) &&
        ((flags & STC8H_OTA_BEGIN_FLAG_RESTART) == 0u)) {
        stc8h_ota_copy_manifest(&ctx->manifest, manifest);
        ctx->session_id = session_id;
        ctx->state = STC8H_OTA_STATE_RECEIVING;
        ctx->write_offset = active.committed_offset;
        ctx->persisted_offset = active.committed_offset;
        ctx->fail_reason = 0u;
        return STC8H_OK;
    }
    if ((has_active != 0u) && (same_transfer == 0u) &&
        (active.state != STC8H_OTA_STATE_UPDATE_REQUESTED) &&
        (active.state != STC8H_OTA_STATE_RECOVERY) &&
        (active.state != STC8H_OTA_STATE_FAILED) &&
        ((flags & STC8H_OTA_BEGIN_FLAG_RESTART) == 0u)) {
        stc8h_ota_fail(ctx, STC8H_OTA_FAIL_SESSION, 0u);
        return STC8H_ERROR;
    }

    stc8h_ota_copy_manifest(&ctx->manifest, manifest);
    ctx->session_id = session_id;
    ctx->write_offset = 0u;
    ctx->persisted_offset = 0u;
    ctx->fail_reason = 0u;
    if (stc8h_ota_save(ctx, STC8H_OTA_STATE_PREPARING, 0u, 0u) !=
        STC8H_OK) {
        stc8h_ota_fail(ctx, STC8H_OTA_FAIL_PARAMS, 0u);
        return STC8H_ERROR;
    }
    if (stc8h_ota_erase_app_area(ctx) != STC8H_OK) {
        stc8h_ota_fail(ctx, STC8H_OTA_FAIL_ERASE, 1u);
        return STC8H_ERROR;
    }
    if (stc8h_ota_save(ctx, STC8H_OTA_STATE_RECEIVING, 0u, 0u) !=
        STC8H_OK) {
        stc8h_ota_fail(ctx, STC8H_OTA_FAIL_PARAMS, 0u);
        return STC8H_ERROR;
    }
    return STC8H_OK;
}

stc8h_status_t stc8h_ota_write_chunk(stc8h_ota_context_t *ctx,
                                     stc8h_u16 offset,
                                     const stc8h_u8 *data,
                                     stc8h_u16 len)
{
    stc8h_u16 end_offset;

    if ((ctx == 0) || (ctx->backend == 0) ||
        (ctx->backend->write == 0) || (ctx->backend->read == 0) ||
        (len == 0u) || (data == 0)) {
        stc8h_ota_fail(ctx, STC8H_OTA_FAIL_ARG, 0u);
        return STC8H_ERROR;
    }
    if (ctx->state != STC8H_OTA_STATE_RECEIVING) {
        stc8h_ota_fail(ctx, STC8H_OTA_FAIL_STATE, 0u);
        return STC8H_ERROR;
    }
    end_offset = offset + len;
    if ((end_offset < offset) || (end_offset > ctx->manifest.app_size)) {
        stc8h_ota_fail(ctx, STC8H_OTA_FAIL_RANGE, 0u);
        return STC8H_ERROR;
    }
    if (offset < ctx->write_offset) {
        if ((end_offset <= ctx->write_offset) &&
            (stc8h_ota_compare_written(ctx, offset, data, len) == STC8H_OK)) {
            return STC8H_OK;
        }
        stc8h_ota_fail(ctx, STC8H_OTA_FAIL_DUPLICATE, 0u);
        return STC8H_ERROR;
    }
    if (offset != ctx->write_offset) {
        stc8h_ota_fail(ctx, STC8H_OTA_FAIL_OFFSET, 0u);
        return STC8H_ERROR;
    }
    if (ctx->backend->write(
            (stc8h_u16)((stc8h_u32)ctx->manifest.app_base + offset),
            data, len) != STC8H_OK) {
        stc8h_ota_fail(ctx, STC8H_OTA_FAIL_WRITE, 1u);
        return STC8H_ERROR;
    }
    if (stc8h_ota_compare_written(ctx, offset, data, len) != STC8H_OK) {
        stc8h_ota_fail(ctx, STC8H_OTA_FAIL_READBACK, 1u);
        return STC8H_ERROR;
    }
    ctx->write_offset = end_offset;
    if (((ctx->write_offset - ctx->persisted_offset) >=
         STC8H_OTA_CHECKPOINT_BYTES) ||
        (ctx->write_offset == ctx->manifest.app_size)) {
        if (stc8h_ota_save(ctx, STC8H_OTA_STATE_RECEIVING,
                            ctx->write_offset, 0u) != STC8H_OK) {
            stc8h_ota_fail(ctx, STC8H_OTA_FAIL_PARAMS, 0u);
            return STC8H_ERROR;
        }
    }
    return STC8H_OK;
}

stc8h_status_t stc8h_ota_verify(stc8h_ota_context_t *ctx)
{
    stc8h_u32 crc32;

    if ((ctx == 0) || (ctx->state != STC8H_OTA_STATE_RECEIVING)) {
        stc8h_ota_fail(ctx, STC8H_OTA_FAIL_STATE, 0u);
        return STC8H_ERROR;
    }
    if (ctx->write_offset != ctx->manifest.app_size) {
        stc8h_ota_fail(ctx, STC8H_OTA_FAIL_INCOMPLETE, 0u);
        return STC8H_ERROR;
    }
    if (stc8h_ota_read_image_crc32(ctx, &crc32) != STC8H_OK) {
        stc8h_ota_fail(ctx, STC8H_OTA_FAIL_READ, 1u);
        return STC8H_ERROR;
    }
    if (crc32 != ctx->manifest.app_crc32) {
        stc8h_ota_fail(ctx, STC8H_OTA_FAIL_CRC, 1u);
        return STC8H_ERROR;
    }
    if (stc8h_ota_save(ctx, STC8H_OTA_STATE_VERIFIED,
                        ctx->write_offset, 0u) != STC8H_OK) {
        stc8h_ota_fail(ctx, STC8H_OTA_FAIL_PARAMS, 0u);
        return STC8H_ERROR;
    }
    return STC8H_OK;
}

stc8h_status_t stc8h_ota_activate(stc8h_ota_context_t *ctx)
{
    if ((ctx == 0) || (ctx->state != STC8H_OTA_STATE_VERIFIED)) {
        stc8h_ota_fail(ctx, STC8H_OTA_FAIL_STATE, 0u);
        return STC8H_ERROR;
    }
    if (stc8h_ota_save(ctx, STC8H_OTA_STATE_TRIAL_PENDING,
                        ctx->write_offset, 0u) != STC8H_OK) {
        stc8h_ota_fail(ctx, STC8H_OTA_FAIL_PARAMS, 0u);
        return STC8H_ERROR;
    }
    return STC8H_OK;
}

stc8h_status_t stc8h_ota_abort(stc8h_ota_context_t *ctx, stc8h_u8 reason)
{
    STC8H_OTA_WORK_MEM stc8h_ota_params_t active;

    if ((ctx == 0) || (ctx->params_store == 0)) {
        return STC8H_ERROR;
    }
    if ((stc8h_ota_params_store_load_active(ctx->params_store, &active) ==
         STC8H_OK) &&
        (active.state == STC8H_OTA_STATE_UPDATE_REQUESTED) &&
        ((active.flags & STC8H_OTA_PARAM_FLAG_APP_VALID) != 0u)) {
#if STC8H_OTA_PARAMS_STORE_ENABLE_APP_API
        return stc8h_ota_params_store_cancel_update_request(
            ctx->params_store, active.session_id);
#else
        return STC8H_ERROR;
#endif
    }
    if (ctx->manifest.app_size == 0UL) {
        return STC8H_ERROR;
    }
    if (stc8h_ota_save(ctx, STC8H_OTA_STATE_RECOVERY,
                        ctx->persisted_offset, reason) != STC8H_OK) {
        return STC8H_ERROR;
    }
    return STC8H_OK;
}

stc8h_ota_state_t stc8h_ota_get_status(const stc8h_ota_context_t *ctx)
{
    return ctx == 0 ? STC8H_OTA_STATE_FAILED : ctx->state;
}
