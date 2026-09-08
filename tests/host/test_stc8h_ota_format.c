#include <stdio.h>
#include <string.h>

#include "../../protocols/stc8h_ota_format.c"

static int require(int condition, const char *message)
{
    if (!condition) {
        printf("%s\n", message);
        return 1;
    }
    return 0;
}

static void make_manifest(stc8h_ota_manifest_t *manifest)
{
    memset(manifest, 0, sizeof(*manifest));
    manifest->magic = 0x4F544131UL;
    manifest->format_version = 1u;
    manifest->target_chip = 0x0864u;
    manifest->board_id = 1u;
    manifest->hw_revision = 2u;
    manifest->app_id = 3u;
    manifest->app_base = 0x6C00u;
    manifest->app_size = 0x1234u;
    manifest->app_crc32 = 0x89ABCDEFUL;
    manifest->version_major = 1u;
    manifest->version_minor = 2u;
    manifest->version_patch = 3u;
    manifest->min_bootloader_version = 2u;
    manifest->build_number = 77u;
    manifest->flags = 5u;
}

static int test_manifest_round_trip(void)
{
    stc8h_ota_manifest_t input;
    stc8h_ota_manifest_t output;
    stc8h_u8 bytes[STC8H_OTA_MANIFEST_WIRE_SIZE];
    int failures;

    failures = 0;
    make_manifest(&input);
    failures += require(stc8h_ota_manifest_encode(&input, bytes,
                                                  sizeof(bytes)) == STC8H_OK,
                        "manifest must encode");
    failures += require(stc8h_ota_manifest_decode(bytes, sizeof(bytes),
                                                  &output) == STC8H_OK,
                        "manifest must decode");
    failures += require(output.app_base == 0x6C00u &&
                        output.app_size == 0x1234u,
                        "manifest range must round-trip");
    failures += require(output.app_crc32 == 0x89ABCDEFUL &&
                        output.build_number == 77u,
                        "manifest identity must round-trip");
    bytes[17] ^= 1u;
    failures += require(stc8h_ota_manifest_decode(bytes, sizeof(bytes),
                                                  &output) == STC8H_ERROR,
                        "manifest CRC must reject corruption");
    return failures;
}

static int test_params_round_trip_and_commit_marker(void)
{
    stc8h_ota_params_t input;
    stc8h_ota_params_t output;
    stc8h_u8 bytes[STC8H_OTA_PARAMS_WIRE_SIZE];
    int failures;

    failures = 0;
    memset(&input, 0, sizeof(input));
    input.param_magic = 0x4F545032UL;
    input.param_version = 2u;
    input.state = 4u;
    input.flags = 2u;
    input.fail_reason = 3u;
    input.generation = 0xFFF0u;
    input.session_id = 0x12345678UL;
    input.app_base = 0x6C00u;
    input.app_size = 0x1234u;
    input.app_crc32 = 0x89ABCDEFUL;
    input.version_major = 1u;
    input.version_minor = 2u;
    input.version_patch = 3u;
    input.min_bootloader_version = 2u;
    input.build_number = 77u;
    input.committed_offset = 0x0800u;
    input.manifest_crc = 0xA1B2u;

    failures += require(stc8h_ota_params_encode(&input, bytes,
                                                sizeof(bytes)) == STC8H_OK,
                        "params must encode");
    failures += require(stc8h_ota_params_decode(bytes, sizeof(bytes),
                                                &output) == STC8H_OK,
                        "committed params must decode");
    failures += require(output.generation == 0xFFF0u &&
                        output.session_id == 0x12345678UL,
                        "params identity must round-trip");
    failures += require(output.committed_offset == 0x0800u &&
                        output.commit_marker == STC8H_OTA_PARAM_COMMIT_MARKER,
                        "params progress and marker must round-trip");
    bytes[34] = 0xFFu;
    bytes[35] = 0xFFu;
    failures += require(stc8h_ota_params_decode(bytes, sizeof(bytes),
                                                &output) == STC8H_ERROR,
                        "missing commit marker must reject torn record");
    return failures;
}

int main(void)
{
    int failures;

    failures = 0;
    failures += test_manifest_round_trip();
    failures += test_params_round_trip_and_commit_marker();
    return failures == 0 ? 0 : 1;
}
