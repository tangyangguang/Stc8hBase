#include <stdio.h>
#include <string.h>

#define STC8H_OTA_EXPECTED_BOARD_ID 1u
#define STC8H_OTA_EXPECTED_HW_REVISION 2u
#define STC8H_OTA_EXPECTED_APP_ID 3u

#include "../../utils/util_crc32.c"
#include "../../protocols/stc8h_ota_format.c"
#include "../../protocols/stc8h_ota.c"
#include "../../protocols/stc8h_ota_receiver.c"
#include "../../hal/stc8h_ota_params_store.c"

#define FLASH_SIZE 65536UL
#define SECTOR_SIZE 512u
#define IMAGE_SIZE 2304u

static stc8h_u8 app_flash[FLASH_SIZE];
static stc8h_u8 param_flash[FLASH_SIZE];
static stc8h_u16 erase_count;
static stc8h_u16 write_count;
static stc8h_u16 keep_alive_count;
static stc8h_u8 fail_readback;

static int require(int condition, const char *message)
{
    if (!condition) {
        printf("%s\n", message);
        return 1;
    }
    return 0;
}

static void fake_reset(void)
{
    memset(app_flash, 0, sizeof(app_flash));
    memset(param_flash, 0xFF, sizeof(param_flash));
    erase_count = 0u;
    write_count = 0u;
    keep_alive_count = 0u;
    fail_readback = 0u;
}

static stc8h_status_t app_erase(stc8h_u16 addr) STC8H_REENTRANT
{
    ++erase_count;
    memset(&app_flash[addr], 0xFF, SECTOR_SIZE);
    return STC8H_OK;
}

static stc8h_status_t app_write(stc8h_u16 addr,
                                const stc8h_u8 *data,
                                stc8h_u16 len) STC8H_REENTRANT
{
    ++write_count;
    memcpy(&app_flash[addr], data, len);
    return STC8H_OK;
}

static stc8h_status_t app_read(stc8h_u16 addr,
                               stc8h_u8 *data,
                               stc8h_u16 len) STC8H_REENTRANT
{
    memcpy(data, &app_flash[addr], len);
    if ((fail_readback != 0u) && (len != 0u)) {
        data[0] ^= 1u;
    }
    return STC8H_OK;
}

static void keep_alive(void)
{
    ++keep_alive_count;
}

static stc8h_status_t param_erase(stc8h_u16 addr) STC8H_REENTRANT
{
    memset(&param_flash[addr], 0xFF, SECTOR_SIZE);
    return STC8H_OK;
}

static stc8h_status_t param_write(stc8h_u16 addr,
                                  const stc8h_u8 *data,
                                  stc8h_u16 len) STC8H_REENTRANT
{
    memcpy(&param_flash[addr], data, len);
    return STC8H_OK;
}

static stc8h_status_t param_read(stc8h_u16 addr,
                                 stc8h_u8 *data,
                                 stc8h_u16 len) STC8H_REENTRANT
{
    memcpy(data, &param_flash[addr], len);
    return STC8H_OK;
}

static void init_context(stc8h_ota_context_t *ctx,
                         stc8h_ota_params_store_t *store,
                         stc8h_ota_backend_t *backend)
{
    backend->erase_sector = app_erase;
    backend->write = app_write;
    backend->read = app_read;
    backend->keep_alive = keep_alive;
    backend->sector_size = SECTOR_SIZE;
    stc8h_ota_params_store_init(store, param_erase, param_write, param_read);
    stc8h_ota_init(ctx, backend, store);
}

static void make_image(stc8h_u8 *image, stc8h_u16 image_size)
{
    stc8h_u16 i;

    for (i = 0u; i < image_size; ++i) {
        image[i] = (stc8h_u8)(i ^ (i >> 8));
    }
}

static void make_manifest(stc8h_ota_manifest_t *manifest,
                          const stc8h_u8 *image,
                          stc8h_u16 image_size)
{
    stc8h_u8 wire[STC8H_OTA_MANIFEST_WIRE_SIZE];

    memset(manifest, 0, sizeof(*manifest));
    manifest->magic = STC8H_OTA_MANIFEST_MAGIC;
    manifest->format_version = STC8H_OTA_FORMAT_VERSION;
    manifest->target_chip = STC8H_OTA_TARGET_STC8H8K64U;
    manifest->board_id = 1u;
    manifest->hw_revision = 2u;
    manifest->app_id = 3u;
    manifest->app_base = STC8H_OTA_APP_BASE;
    manifest->app_size = image_size;
    manifest->app_crc32 = util_crc32_ieee(image, image_size);
    manifest->version_major = 1u;
    manifest->version_minor = 2u;
    manifest->version_patch = 3u;
    manifest->min_bootloader_version = STC8H_OTA_BOOTLOADER_VERSION;
    manifest->build_number = 9u;
    (void)stc8h_ota_manifest_encode(manifest, wire, sizeof(wire));
    (void)stc8h_ota_manifest_decode(wire, sizeof(wire), manifest);
}

static int test_manifest_guards(void)
{
    stc8h_u8 image[4];
    stc8h_ota_manifest_t manifest;
    int failures;

    image[0] = 1u;
    image[1] = 2u;
    image[2] = 3u;
    image[3] = 4u;
    make_manifest(&manifest, image, sizeof(image));
    failures = 0;
    failures += require(stc8h_ota_validate_manifest(&manifest) == STC8H_OK,
                        "valid manifest must pass");
    manifest.board_id = 2u;
    failures += require(stc8h_ota_validate_manifest(&manifest) == STC8H_ERROR,
                        "wrong board target must fail");
    manifest.board_id = 1u;
    manifest.app_base = 0x5000u;
    failures += require(stc8h_ota_validate_manifest(&manifest) == STC8H_ERROR,
                        "wrong app base must fail");
    manifest.app_base = STC8H_OTA_APP_BASE;
    manifest.app_size = (stc8h_u16)(STC8H_OTA_APP_LIMIT -
                                     STC8H_OTA_APP_BASE + 2u);
    failures += require(stc8h_ota_validate_manifest(&manifest) == STC8H_ERROR,
                        "app crossing data region must fail");
    return failures;
}

static int test_explicit_request_required_for_valid_app(void)
{
    stc8h_u8 image[4];
    stc8h_ota_manifest_t manifest;
    stc8h_ota_params_t params;
    stc8h_ota_context_t ctx;
    stc8h_ota_params_store_t store;
    stc8h_ota_backend_t backend;
    int failures;

    failures = 0;
    image[0] = 1u;
    image[1] = 2u;
    image[2] = 3u;
    image[3] = 4u;
    fake_reset();
    init_context(&ctx, &store, &backend);
    make_manifest(&manifest, image, sizeof(image));
    memset(&params, 0, sizeof(params));
    params.param_magic = STC8H_OTA_PARAM_MAGIC;
    params.param_version = STC8H_OTA_PARAM_VERSION;
    params.state = STC8H_OTA_STATE_APP_VALID;
    params.flags = STC8H_OTA_PARAM_FLAG_APP_VALID;
    params.generation = 1u;
    params.app_base = manifest.app_base;
    params.app_size = manifest.app_size;
    params.app_crc32 = manifest.app_crc32;
    params.manifest_crc = manifest.manifest_crc;
    (void)stc8h_ota_params_store_write_next(&store, &params);
    failures += require(stc8h_ota_begin(&ctx, &manifest, 1UL, 0u) ==
                        STC8H_ERROR &&
                        ctx.fail_reason == STC8H_OTA_FAIL_NOT_REQUESTED,
                        "valid app must reject unrequested update");
    failures += require(stc8h_ota_params_store_request_update(
                            &store, 0xAABBCCDDUL) == STC8H_OK,
                        "application must explicitly request update");
    failures += require(stc8h_ota_begin(
                            &ctx, &manifest, 0xAABBCCDDUL, 0u) == STC8H_OK,
                        "matching explicit session must begin");
    return failures;
}

static int test_transfer_checkpoint_resume_verify_activate(void)
{
    static stc8h_u8 image[IMAGE_SIZE];
    stc8h_ota_manifest_t manifest;
    stc8h_ota_context_t ctx;
    stc8h_ota_context_t restored;
    stc8h_ota_params_store_t store;
    stc8h_ota_params_store_t restored_store;
    stc8h_ota_backend_t backend;
    stc8h_ota_backend_t restored_backend;
    stc8h_ota_params_t params;
    stc8h_u16 offset;
    int failures;

    failures = 0;
    fake_reset();
    make_image(image, sizeof(image));
    make_manifest(&manifest, image, sizeof(image));
    init_context(&ctx, &store, &backend);
    failures += require(stc8h_ota_begin(
                            &ctx, &manifest, 0x10203040UL, 0u) == STC8H_OK,
                        "fresh recovery target must begin explicitly");
    failures += require(erase_count == 5u,
                        "begin must erase every covered sector");
    for (offset = 0u; offset < 2048u; offset = (stc8h_u16)(offset + 128u)) {
        failures += require(stc8h_ota_write_chunk(
                                &ctx, offset, &image[offset], 128u) == STC8H_OK,
                            "sequential block must write and read back");
    }
    failures += require(ctx.persisted_offset == 2048u,
                        "checkpoint must persist at configured interval");

    init_context(&restored, &restored_store, &restored_backend);
    failures += require(stc8h_ota_restore(&restored) == STC8H_OK &&
                        restored.state == STC8H_OTA_STATE_RECEIVING &&
                        restored.write_offset == 2048u,
                        "new boot context must restore committed offset");
    failures += require(stc8h_ota_begin(
                            &restored, &manifest, 0x10203040UL, 0u) == STC8H_OK &&
                        restored.write_offset == 2048u && erase_count == 5u,
                        "same BEGIN must resume without erasing again");
    failures += require(stc8h_ota_write_chunk(
                            &restored, 1920u, &image[1920], 128u) == STC8H_OK,
                        "replayed committed block must compare and succeed");
    failures += require(write_count == 16u,
                        "replayed committed block must not rewrite flash");
    for (offset = 2048u; offset < IMAGE_SIZE;
         offset = (stc8h_u16)(offset + 128u)) {
        failures += require(stc8h_ota_write_chunk(
                                &restored, offset, &image[offset], 128u) ==
                            STC8H_OK,
                            "remaining block must write");
    }
    failures += require(stc8h_ota_verify(&restored) == STC8H_OK &&
                        restored.state == STC8H_OTA_STATE_VERIFIED,
                        "complete image must verify");
    failures += require(stc8h_ota_activate(&restored) == STC8H_OK,
                        "verified image must require explicit activation");
    (void)stc8h_ota_params_store_load_active(&restored_store, &params);
    failures += require(params.state == STC8H_OTA_STATE_TRIAL_PENDING &&
                        stc8h_ota_get_boot_action(&params) ==
                        STC8H_OTA_BOOT_ACTION_TRIAL_APP,
                        "activation must persist one trial boot");
    failures += require(keep_alive_count != 0u,
                        "long erase/read paths must service keep-alive");
    return failures;
}

static int test_restart_crc_and_abort_paths(void)
{
    stc8h_u8 image[256];
    stc8h_ota_manifest_t manifest;
    stc8h_ota_context_t ctx;
    stc8h_ota_params_store_t store;
    stc8h_ota_backend_t backend;
    stc8h_u16 first_erase_count;
    int failures;

    failures = 0;
    make_image(image, sizeof(image));
    fake_reset();
    make_manifest(&manifest, image, sizeof(image));
    init_context(&ctx, &store, &backend);
    (void)stc8h_ota_begin(&ctx, &manifest, 1UL, 0u);
    (void)stc8h_ota_write_chunk(&ctx, 0u, image, 128u);
    first_erase_count = erase_count;
    failures += require(stc8h_ota_begin(&ctx, &manifest, 2UL, 0u) ==
                        STC8H_ERROR &&
                        ctx.fail_reason == STC8H_OTA_FAIL_SESSION,
                        "different session must not take over implicitly");
    failures += require(stc8h_ota_begin(&ctx, &manifest, 2UL,
                                        STC8H_OTA_BEGIN_FLAG_RESTART) ==
                        STC8H_OK && erase_count > first_erase_count &&
                        ctx.write_offset == 0u,
                        "explicit restart may replace interrupted session");
    (void)stc8h_ota_write_chunk(&ctx, 0u, image, 128u);
    (void)stc8h_ota_write_chunk(&ctx, 128u, &image[128], 128u);
    app_flash[STC8H_OTA_APP_BASE + 1u] ^= 1u;
    failures += require(stc8h_ota_verify(&ctx) == STC8H_ERROR &&
                        ctx.fail_reason == STC8H_OTA_FAIL_CRC,
                        "whole-image CRC mismatch must fail activation");

    fake_reset();
    init_context(&ctx, &store, &backend);
    (void)stc8h_ota_begin(&ctx, &manifest, 3UL, 0u);
    failures += require(stc8h_ota_abort(&ctx, STC8H_OTA_FAIL_STATE) ==
                        STC8H_OK &&
                        ctx.state == STC8H_OTA_STATE_RECOVERY,
                        "abort after erase must remain recoverable in bootloader");
    return failures;
}

static int test_receiver_binds_begin_to_target_uid(void)
{
    stc8h_u8 image[4];
    stc8h_u8 uid[STC8H_DEVICE_UID_SIZE];
    stc8h_u8 begin_payload[STC8H_OTA_RECEIVER_BEGIN_SIZE];
    stc8h_ota_manifest_t manifest;
    stc8h_ota_context_t ctx;
    stc8h_ota_params_store_t store;
    stc8h_ota_backend_t backend;
    proto_ota_frame_t request;
    stc8h_u8 reset_requested;
    stc8h_u8 index;
    int failures;

    failures = 0;
    image[0] = 1u;
    image[1] = 2u;
    image[2] = 3u;
    image[3] = 4u;
    for (index = 0u; index < STC8H_DEVICE_UID_SIZE; ++index) {
        uid[index] = index;
    }
    fake_reset();
    make_manifest(&manifest, image, sizeof(image));
    (void)stc8h_ota_manifest_encode(&manifest, begin_payload,
                                    STC8H_OTA_MANIFEST_WIRE_SIZE);
    memcpy(&begin_payload[STC8H_OTA_MANIFEST_WIRE_SIZE], uid,
           STC8H_DEVICE_UID_SIZE);
    memset(&request, 0, sizeof(request));
    request.cmd = PROTO_OTA_FRAME_CMD_BEGIN;
    request.session_id = 0x44332211UL;
    request.seq = 1u;
    request.len = sizeof(begin_payload);
    request.payload = begin_payload;
    init_context(&ctx, &store, &backend);

    begin_payload[STC8H_OTA_MANIFEST_WIRE_SIZE + 3u] ^= 1u;
    failures += require(stc8h_ota_receiver_handle(
                            &ctx, uid, &request, &reset_requested) ==
                        PROTO_OTA_FRAME_STATUS_ERROR && erase_count == 0u,
                        "BEGIN for another UID must not erase flash");
    begin_payload[STC8H_OTA_MANIFEST_WIRE_SIZE + 3u] ^= 1u;
    failures += require(stc8h_ota_receiver_handle(
                            &ctx, uid, &request, &reset_requested) ==
                        PROTO_OTA_FRAME_STATUS_OK && erase_count == 1u,
                        "matching UID may enter explicit BEGIN");
    return failures;
}

static int test_readback_mismatch_fails(void)
{
    stc8h_u8 image[4];
    stc8h_ota_manifest_t manifest;
    stc8h_ota_context_t ctx;
    stc8h_ota_params_store_t store;
    stc8h_ota_backend_t backend;

    image[0] = 1u;
    image[1] = 2u;
    image[2] = 3u;
    image[3] = 4u;
    fake_reset();
    make_manifest(&manifest, image, sizeof(image));
    init_context(&ctx, &store, &backend);
    (void)stc8h_ota_begin(&ctx, &manifest, 7UL, 0u);
    fail_readback = 1u;
    return require(stc8h_ota_write_chunk(&ctx, 0u, image, sizeof(image)) ==
                   STC8H_ERROR &&
                   ctx.fail_reason == STC8H_OTA_FAIL_READBACK,
                   "write readback mismatch must persist failure");
}

int main(void)
{
    int failures;

    failures = 0;
    failures += test_manifest_guards();
    failures += test_explicit_request_required_for_valid_app();
    failures += test_transfer_checkpoint_resume_verify_activate();
    failures += test_restart_crc_and_abort_paths();
    failures += test_receiver_binds_begin_to_target_uid();
    failures += test_readback_mismatch_fails();
    return failures == 0 ? 0 : 1;
}
