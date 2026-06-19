#include <stdio.h>

#define STC8H_OTA_EXPECTED_BOARD_ID 1u
#define STC8H_OTA_EXPECTED_HW_REVISION 2u
#define STC8H_OTA_EXPECTED_APP_ID 3u

#include "../../protocols/stc8h_ota_format.c"
#include "../../protocols/stc8h_ota.c"

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

    return failures == 0 ? 0 : 1;
}
