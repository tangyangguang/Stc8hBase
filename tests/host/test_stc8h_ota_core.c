#include <stdio.h>
#include <string.h>

#define STC8H_OTA_EXPECTED_BOARD_ID 1u
#define STC8H_OTA_EXPECTED_HW_REVISION 2u
#define STC8H_OTA_EXPECTED_APP_ID 3u

#include "../../utils/util_crc32.c"
#include "../../protocols/stc8h_ota_format.c"
#include "../../protocols/stc8h_ota.c"
#include "../../hal/stc8h_ota_params_store.c"

#define FAKE_FLASH_SIZE 65536UL
#define FAKE_SECTOR_SIZE 512u

static stc8h_u8 fake_app_flash[FAKE_FLASH_SIZE];
static stc8h_u8 fake_param_flash[FAKE_FLASH_SIZE];
static stc8h_u16 fake_erase_count;
static stc8h_u16 fake_write_count;

static const stc8h_u8 manifest_bytes[STC8H_OTA_MANIFEST_WIRE_SIZE] = {
    0x31u, 0x41u, 0x54u, 0x4Fu, 0x01u, 0x64u, 0x08u, 0x01u,
    0x00u, 0x02u, 0x00u, 0x03u, 0x00u, 0x00u, 0x02u, 0x00u,
    0x01u, 0x00u, 0x00u, 0x78u, 0x56u, 0x34u, 0x12u, 0x01u,
    0x02u, 0x03u, 0x01u, 0x05u, 0x00u, 0x64u, 0x23u
};

static int require(int condition, const char *message)
{
    if (!condition) {
        printf("%s\n", message);
        return 1;
    }
    return 0;
}

static stc8h_ota_manifest_t make_valid_manifest(void)
{
    stc8h_ota_manifest_t manifest;

    if (stc8h_ota_manifest_decode(manifest_bytes,
                                   sizeof(manifest_bytes),
                                   &manifest) != STC8H_OK) {
        printf("test fixture manifest must decode\n");
    }

    return manifest;
}

static void fake_reset(void)
{
    stc8h_u32 i;

    for (i = 0UL; i < FAKE_FLASH_SIZE; ++i) {
        fake_app_flash[i] = 0x00u;
        fake_param_flash[i] = 0xFFu;
    }
    fake_erase_count = 0u;
    fake_write_count = 0u;
}

static stc8h_status_t fake_app_erase(stc8h_u16 addr) STC8H_REENTRANT
{
    stc8h_u16 i;

    ++fake_erase_count;
    for (i = 0u; i < FAKE_SECTOR_SIZE; ++i) {
        fake_app_flash[(stc8h_u32)addr + i] = 0xFFu;
    }
    return STC8H_OK;
}

static stc8h_status_t fake_app_write(stc8h_u16 addr,
                                     const stc8h_u8 *data,
                                     stc8h_u16 len) STC8H_REENTRANT
{
    stc8h_u16 i;

    ++fake_write_count;
    for (i = 0u; i < len; ++i) {
        fake_app_flash[(stc8h_u32)addr + i] = data[i];
    }
    return STC8H_OK;
}

static stc8h_status_t fake_app_read(stc8h_u16 addr,
                                    stc8h_u8 *data,
                                    stc8h_u16 len) STC8H_REENTRANT
{
    stc8h_u16 i;

    for (i = 0u; i < len; ++i) {
        data[i] = fake_app_flash[(stc8h_u32)addr + i];
    }
    return STC8H_OK;
}

static stc8h_status_t fake_param_erase(stc8h_u16 addr) STC8H_REENTRANT
{
    stc8h_u16 i;

    for (i = 0u; i < FAKE_SECTOR_SIZE; ++i) {
        fake_param_flash[(stc8h_u32)addr + i] = 0xFFu;
    }
    return STC8H_OK;
}

static stc8h_status_t fake_param_write(stc8h_u16 addr,
                                       const stc8h_u8 *data,
                                       stc8h_u16 len) STC8H_REENTRANT
{
    stc8h_u16 i;

    for (i = 0u; i < len; ++i) {
        fake_param_flash[(stc8h_u32)addr + i] = data[i];
    }
    return STC8H_OK;
}

static stc8h_status_t fake_param_read(stc8h_u16 addr,
                                      stc8h_u8 *data,
                                      stc8h_u16 len) STC8H_REENTRANT
{
    stc8h_u16 i;

    for (i = 0u; i < len; ++i) {
        data[i] = fake_param_flash[(stc8h_u32)addr + i];
    }
    return STC8H_OK;
}

static void init_context(stc8h_ota_context_t *ctx,
                         stc8h_ota_params_store_t *store,
                         stc8h_ota_backend_t *backend)
{
    backend->erase_sector = fake_app_erase;
    backend->write = fake_app_write;
    backend->read = fake_app_read;
    backend->sector_size = FAKE_SECTOR_SIZE;
    stc8h_ota_params_store_init(store, fake_param_erase, fake_param_write, fake_param_read);
    stc8h_ota_init(ctx, backend, store);
}

static stc8h_ota_manifest_t make_image_manifest(const stc8h_u8 *image, stc8h_u16 len)
{
    stc8h_ota_manifest_t manifest;

    manifest = make_valid_manifest();
    manifest.app_size = len;
    manifest.app_crc32 = util_crc32_ieee(image, len);
    return manifest;
}

static int test_valid_manifest_passes(void)
{
    stc8h_ota_manifest_t manifest;

    manifest = make_valid_manifest();
    return require(stc8h_ota_validate_manifest(&manifest) == STC8H_OK,
                   "valid H8K64U manifest must pass validation");
}

static int test_wrong_target_chip_fails(void)
{
    stc8h_ota_manifest_t manifest;

    manifest = make_valid_manifest();
    manifest.target_chip = 0x0108u;
    return require(stc8h_ota_validate_manifest(&manifest) == STC8H_ERROR,
                   "wrong target chip must fail validation");
}

static int test_wrong_app_base_fails(void)
{
    stc8h_ota_manifest_t manifest;

    manifest = make_valid_manifest();
    manifest.app_base = 0x0100u;
    return require(stc8h_ota_validate_manifest(&manifest) == STC8H_ERROR,
                   "app base below OTA app base must fail validation");
}

static int test_zero_size_fails(void)
{
    stc8h_ota_manifest_t manifest;

    manifest = make_valid_manifest();
    manifest.app_size = 0UL;
    return require(stc8h_ota_validate_manifest(&manifest) == STC8H_ERROR,
                   "zero-size image must fail validation");
}

static int test_app_limit_overflow_fails(void)
{
    stc8h_ota_manifest_t manifest;

    manifest = make_valid_manifest();
    manifest.app_base = 0xEF00u;
    manifest.app_size = 0x0101UL;
    return require(stc8h_ota_validate_manifest(&manifest) == STC8H_ERROR,
                   "image ending above app limit must fail validation");
}

static int test_min_bootloader_version_fails(void)
{
    stc8h_ota_manifest_t manifest;

    manifest = make_valid_manifest();
    manifest.min_bootloader_version = (stc8h_u8)(STC8H_OTA_BOOTLOADER_VERSION + 1u);
    return require(stc8h_ota_validate_manifest(&manifest) == STC8H_ERROR,
                   "manifest requiring newer bootloader must fail validation");
}

static int test_board_id_fails_when_configured(void)
{
    stc8h_ota_manifest_t manifest;

    manifest = make_valid_manifest();
    manifest.board_id = 9u;
    return require(stc8h_ota_validate_manifest(&manifest) == STC8H_ERROR,
                   "wrong configured board id must fail validation");
}

static int test_hw_revision_fails_when_configured(void)
{
    stc8h_ota_manifest_t manifest;

    manifest = make_valid_manifest();
    manifest.hw_revision = 9u;
    return require(stc8h_ota_validate_manifest(&manifest) == STC8H_ERROR,
                   "wrong configured hardware revision must fail validation");
}

static int test_app_id_fails_when_configured(void)
{
    stc8h_ota_manifest_t manifest;

    manifest = make_valid_manifest();
    manifest.app_id = 9u;
    return require(stc8h_ota_validate_manifest(&manifest) == STC8H_ERROR,
                   "wrong configured app id must fail validation");
}

static int test_begin_erases_application_sectors(void)
{
    stc8h_ota_context_t ctx;
    stc8h_ota_params_store_t store;
    stc8h_ota_backend_t backend;
    stc8h_ota_manifest_t manifest;
    stc8h_u8 image[1];
    int failures;

    failures = 0;
    image[0] = 0x11u;
    manifest = make_image_manifest(image, sizeof(image));
    manifest.app_size = 600UL;
    fake_reset();
    init_context(&ctx, &store, &backend);

    failures += require(stc8h_ota_begin(&ctx, &manifest) == STC8H_OK,
                        "begin must accept valid manifest");
    failures += require(stc8h_ota_get_status(&ctx) == STC8H_OTA_STATE_RECEIVING,
                        "begin must enter receiving state");
    failures += require(fake_erase_count == 2u,
                        "begin must erase every covered application sector");
    return failures;
}

static int test_first_chunk_advances_offset(void)
{
    stc8h_ota_context_t ctx;
    stc8h_ota_params_store_t store;
    stc8h_ota_backend_t backend;
    stc8h_ota_manifest_t manifest;
    stc8h_u8 image[4];
    int failures;

    failures = 0;
    image[0] = 1u;
    image[1] = 2u;
    image[2] = 3u;
    image[3] = 4u;
    manifest = make_image_manifest(image, sizeof(image));
    fake_reset();
    init_context(&ctx, &store, &backend);
    (void)stc8h_ota_begin(&ctx, &manifest);

    failures += require(stc8h_ota_write_chunk(&ctx, 0UL, image, sizeof(image)) == STC8H_OK,
                        "first chunk at offset 0 must write");
    failures += require(ctx.write_offset == sizeof(image),
                        "first chunk must advance write offset");
    return failures;
}

static int test_duplicate_chunk_is_accepted_without_rewrite(void)
{
    stc8h_ota_context_t ctx;
    stc8h_ota_params_store_t store;
    stc8h_ota_backend_t backend;
    stc8h_ota_manifest_t manifest;
    stc8h_u8 image[4];
    int failures;

    failures = 0;
    image[0] = 1u;
    image[1] = 2u;
    image[2] = 3u;
    image[3] = 4u;
    manifest = make_image_manifest(image, sizeof(image));
    fake_reset();
    init_context(&ctx, &store, &backend);
    (void)stc8h_ota_begin(&ctx, &manifest);
    (void)stc8h_ota_write_chunk(&ctx, 0UL, image, sizeof(image));

    failures += require(stc8h_ota_write_chunk(&ctx, 0UL, image, sizeof(image)) == STC8H_OK,
                        "exact duplicate chunk must be accepted");
    failures += require(fake_write_count == 1u,
                        "exact duplicate chunk must not rewrite flash");
    return failures;
}

static int test_stale_chunk_with_different_length_fails(void)
{
    stc8h_ota_context_t ctx;
    stc8h_ota_params_store_t store;
    stc8h_ota_backend_t backend;
    stc8h_ota_manifest_t manifest;
    stc8h_u8 image[4];

    image[0] = 1u;
    image[1] = 2u;
    image[2] = 3u;
    image[3] = 4u;
    manifest = make_image_manifest(image, sizeof(image));
    fake_reset();
    init_context(&ctx, &store, &backend);
    (void)stc8h_ota_begin(&ctx, &manifest);
    (void)stc8h_ota_write_chunk(&ctx, 0UL, image, sizeof(image));

    return require(stc8h_ota_write_chunk(&ctx, 0UL, image, 3u) == STC8H_ERROR,
                   "stale chunk with different length must fail");
}

static int test_stale_chunk_with_different_data_fails(void)
{
    stc8h_ota_context_t ctx;
    stc8h_ota_params_store_t store;
    stc8h_ota_backend_t backend;
    stc8h_ota_manifest_t manifest;
    stc8h_u8 image[4];
    stc8h_u8 other[4];

    image[0] = 1u;
    image[1] = 2u;
    image[2] = 3u;
    image[3] = 4u;
    other[0] = 1u;
    other[1] = 2u;
    other[2] = 3u;
    other[3] = 5u;
    manifest = make_image_manifest(image, sizeof(image));
    fake_reset();
    init_context(&ctx, &store, &backend);
    (void)stc8h_ota_begin(&ctx, &manifest);
    (void)stc8h_ota_write_chunk(&ctx, 0UL, image, sizeof(image));

    return require(stc8h_ota_write_chunk(&ctx, 0UL, other, sizeof(other)) == STC8H_ERROR,
                   "stale chunk with different data must fail");
}

static int test_future_offset_fails(void)
{
    stc8h_ota_context_t ctx;
    stc8h_ota_params_store_t store;
    stc8h_ota_backend_t backend;
    stc8h_ota_manifest_t manifest;
    stc8h_u8 image[4];

    image[0] = 1u;
    image[1] = 2u;
    image[2] = 3u;
    image[3] = 4u;
    manifest = make_image_manifest(image, sizeof(image));
    fake_reset();
    init_context(&ctx, &store, &backend);
    (void)stc8h_ota_begin(&ctx, &manifest);

    return require(stc8h_ota_write_chunk(&ctx, 2UL, image, 1u) == STC8H_ERROR,
                   "future offset must fail");
}

static int test_final_short_chunk_completes_image(void)
{
    stc8h_ota_context_t ctx;
    stc8h_ota_params_store_t store;
    stc8h_ota_backend_t backend;
    stc8h_ota_manifest_t manifest;
    stc8h_u8 image[6];
    int failures;

    failures = 0;
    image[0] = 1u;
    image[1] = 2u;
    image[2] = 3u;
    image[3] = 4u;
    image[4] = 5u;
    image[5] = 6u;
    manifest = make_image_manifest(image, sizeof(image));
    fake_reset();
    init_context(&ctx, &store, &backend);
    (void)stc8h_ota_begin(&ctx, &manifest);

    failures += require(stc8h_ota_write_chunk(&ctx, 0UL, image, 4u) == STC8H_OK,
                        "first partial chunk must write");
    failures += require(stc8h_ota_write_chunk(&ctx, 4UL, &image[4], 2u) == STC8H_OK,
                        "final short chunk must write");
    failures += require(ctx.write_offset == sizeof(image),
                        "final short chunk must complete write offset");
    return failures;
}

static int test_verify_before_complete_fails(void)
{
    stc8h_ota_context_t ctx;
    stc8h_ota_params_store_t store;
    stc8h_ota_backend_t backend;
    stc8h_ota_manifest_t manifest;
    stc8h_u8 image[4];

    image[0] = 1u;
    image[1] = 2u;
    image[2] = 3u;
    image[3] = 4u;
    manifest = make_image_manifest(image, sizeof(image));
    fake_reset();
    init_context(&ctx, &store, &backend);
    (void)stc8h_ota_begin(&ctx, &manifest);
    (void)stc8h_ota_write_chunk(&ctx, 0UL, image, 2u);

    return require(stc8h_ota_verify(&ctx) == STC8H_ERROR,
                   "verify before complete image must fail");
}

static int test_verify_matching_crc_enters_pending_commit(void)
{
    stc8h_ota_context_t ctx;
    stc8h_ota_params_store_t store;
    stc8h_ota_backend_t backend;
    stc8h_ota_manifest_t manifest;
    stc8h_u8 image[4];
    int failures;

    failures = 0;
    image[0] = 1u;
    image[1] = 2u;
    image[2] = 3u;
    image[3] = 4u;
    manifest = make_image_manifest(image, sizeof(image));
    fake_reset();
    init_context(&ctx, &store, &backend);
    (void)stc8h_ota_begin(&ctx, &manifest);
    (void)stc8h_ota_write_chunk(&ctx, 0UL, image, sizeof(image));

    failures += require(stc8h_ota_verify(&ctx) == STC8H_OK,
                        "verify after complete matching image must pass");
    failures += require(stc8h_ota_get_status(&ctx) == STC8H_OTA_STATE_PENDING_COMMIT,
                        "verify must enter pending commit state");
    return failures;
}

static int test_commit_before_verify_fails(void)
{
    stc8h_ota_context_t ctx;
    stc8h_ota_params_store_t store;
    stc8h_ota_backend_t backend;
    stc8h_ota_manifest_t manifest;
    stc8h_u8 image[4];

    image[0] = 1u;
    image[1] = 2u;
    image[2] = 3u;
    image[3] = 4u;
    manifest = make_image_manifest(image, sizeof(image));
    fake_reset();
    init_context(&ctx, &store, &backend);
    (void)stc8h_ota_begin(&ctx, &manifest);

    return require(stc8h_ota_commit(&ctx) == STC8H_ERROR,
                   "commit before verify must fail");
}

static int test_commit_after_verify_writes_parameter_record(void)
{
    stc8h_ota_context_t ctx;
    stc8h_ota_params_store_t store;
    stc8h_ota_backend_t backend;
    stc8h_ota_manifest_t manifest;
    stc8h_ota_params_t params;
    stc8h_u8 image[4];
    int failures;

    failures = 0;
    image[0] = 1u;
    image[1] = 2u;
    image[2] = 3u;
    image[3] = 4u;
    manifest = make_image_manifest(image, sizeof(image));
    fake_reset();
    init_context(&ctx, &store, &backend);
    (void)stc8h_ota_begin(&ctx, &manifest);
    (void)stc8h_ota_write_chunk(&ctx, 0UL, image, sizeof(image));
    (void)stc8h_ota_verify(&ctx);

    failures += require(stc8h_ota_commit(&ctx) == STC8H_OK,
                        "commit after verify must pass");
    failures += require(stc8h_ota_get_status(&ctx) == STC8H_OTA_STATE_COMMITTED,
                        "commit must enter committed state");
    failures += require(stc8h_ota_params_store_load_active(&store, &params) == STC8H_OK,
                        "commit must write active parameter record");
    failures += require(params.state == STC8H_OTA_STATE_COMMITTED,
                        "committed parameter record must set committed state");
    failures += require(params.app_valid == 0u,
                        "committed parameter record must require app validation");
    failures += require(params.boot_attempted == 0u,
                        "committed parameter record must clear boot_attempted");
    failures += require(params.update_pending == 0u,
                        "committed parameter record must clear update_pending");
    return failures;
}

static int test_abort_enters_failed_state(void)
{
    stc8h_ota_context_t ctx;
    stc8h_ota_params_store_t store;
    stc8h_ota_backend_t backend;
    stc8h_ota_manifest_t manifest;
    stc8h_u8 image[4];
    int failures;

    failures = 0;
    image[0] = 1u;
    image[1] = 2u;
    image[2] = 3u;
    image[3] = 4u;
    manifest = make_image_manifest(image, sizeof(image));
    fake_reset();
    init_context(&ctx, &store, &backend);
    (void)stc8h_ota_begin(&ctx, &manifest);

    failures += require(stc8h_ota_abort(&ctx, 7u) == STC8H_OK,
                        "abort must succeed");
    failures += require(stc8h_ota_get_status(&ctx) == STC8H_OTA_STATE_FAILED,
                        "abort must enter failed state");
    failures += require(ctx.fail_reason == 7u,
                        "abort must store fail reason");
    return failures;
}

int main(void)
{
    int failures;

    failures = 0;
    failures += test_valid_manifest_passes();
    failures += test_wrong_target_chip_fails();
    failures += test_wrong_app_base_fails();
    failures += test_zero_size_fails();
    failures += test_app_limit_overflow_fails();
    failures += test_min_bootloader_version_fails();
    failures += test_board_id_fails_when_configured();
    failures += test_hw_revision_fails_when_configured();
    failures += test_app_id_fails_when_configured();
    failures += test_begin_erases_application_sectors();
    failures += test_first_chunk_advances_offset();
    failures += test_duplicate_chunk_is_accepted_without_rewrite();
    failures += test_stale_chunk_with_different_length_fails();
    failures += test_stale_chunk_with_different_data_fails();
    failures += test_future_offset_fails();
    failures += test_final_short_chunk_completes_image();
    failures += test_verify_before_complete_fails();
    failures += test_verify_matching_crc_enters_pending_commit();
    failures += test_commit_before_verify_fails();
    failures += test_commit_after_verify_writes_parameter_record();
    failures += test_abort_enters_failed_state();

    return failures == 0 ? 0 : 1;
}
