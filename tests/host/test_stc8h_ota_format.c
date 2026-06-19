#include <stdio.h>
#include <string.h>

#include "../../protocols/stc8h_ota_format.c"

static const stc8h_u8 manifest_bytes[STC8H_OTA_MANIFEST_WIRE_SIZE] = {
    0x31u, 0x41u, 0x54u, 0x4Fu, 0x01u, 0x64u, 0x08u, 0x01u,
    0x00u, 0x02u, 0x00u, 0x03u, 0x00u, 0x00u, 0x02u, 0x00u,
    0x01u, 0x00u, 0x00u, 0x78u, 0x56u, 0x34u, 0x12u, 0x01u,
    0x02u, 0x03u, 0x01u, 0x05u, 0x00u, 0x64u, 0x23u
};

static const stc8h_u8 params_bytes[STC8H_OTA_PARAMS_WIRE_SIZE] = {
    0x41u, 0x50u, 0x54u, 0x4Fu, 0x01u, 0x2Au, 0x00u, 0x05u,
    0x01u, 0x00u, 0x00u, 0x00u, 0x02u, 0x00u, 0x01u, 0x00u,
    0x00u, 0x78u, 0x56u, 0x34u, 0x12u, 0x01u, 0x02u, 0x03u,
    0x80u, 0x00u, 0x00u, 0x00u, 0x07u, 0xCAu, 0xF6u
};

static int require(int condition, const char *message)
{
    if (!condition) {
        printf("%s\n", message);
        return 1;
    }
    return 0;
}

static int test_manifest_decode_and_encode(void)
{
    stc8h_ota_manifest_t manifest;
    stc8h_u8 encoded[STC8H_OTA_MANIFEST_WIRE_SIZE];
    int failures;

    failures = 0;
    failures += require(stc8h_ota_manifest_decode(manifest_bytes,
                                                  sizeof(manifest_bytes),
                                                  &manifest) == STC8H_OK,
                        "valid manifest bytes must decode");
    failures += require(manifest.magic == 0x4F544131UL, "manifest magic must decode little-endian");
    failures += require(manifest.target_chip == 0x0864u, "manifest target chip must decode little-endian");
    failures += require(manifest.app_base == 0x0200u, "manifest app base must decode little-endian");
    failures += require(manifest.app_size == 0x00000100UL, "manifest size must decode little-endian");
    failures += require(manifest.app_crc32 == 0x12345678UL, "manifest crc32 must decode little-endian");
    failures += require(manifest.manifest_crc == 0x2364u, "manifest crc16 must decode little-endian");

    memset(encoded, 0, sizeof(encoded));
    failures += require(stc8h_ota_manifest_encode(&manifest,
                                                  encoded,
                                                  sizeof(encoded)) == STC8H_OK,
                        "manifest must encode");
    failures += require(memcmp(encoded, manifest_bytes, sizeof(encoded)) == 0,
                        "manifest encode must reproduce canonical bytes");
    return failures;
}

static int test_manifest_rejects_bad_crc(void)
{
    stc8h_ota_manifest_t manifest;
    stc8h_u8 corrupted[STC8H_OTA_MANIFEST_WIRE_SIZE];

    memcpy(corrupted, manifest_bytes, sizeof(corrupted));
    corrupted[15] ^= 0x01u;
    return require(stc8h_ota_manifest_decode(corrupted,
                                             sizeof(corrupted),
                                             &manifest) == STC8H_ERROR,
                   "manifest decode must reject canonical bytes with bad crc");
}

static int test_params_decode_and_encode(void)
{
    stc8h_ota_params_t params;
    stc8h_u8 encoded[STC8H_OTA_PARAMS_WIRE_SIZE];
    int failures;

    failures = 0;
    failures += require(stc8h_ota_params_decode(params_bytes,
                                                sizeof(params_bytes),
                                                &params) == STC8H_OK,
                        "valid params bytes must decode");
    failures += require(params.param_magic == 0x4F545041UL, "params magic must decode little-endian");
    failures += require(params.sequence == 0x002Au, "params sequence must decode little-endian");
    failures += require(params.state == 0x05u, "params state must decode");
    failures += require(params.boot_attempted == 0u, "params boot_attempted must decode");
    failures += require(params.write_offset == 0x00000080UL, "params write offset must decode little-endian");
    failures += require(params.param_crc == 0xF6CAu, "params crc must decode little-endian");

    memset(encoded, 0, sizeof(encoded));
    failures += require(stc8h_ota_params_encode(&params,
                                                encoded,
                                                sizeof(encoded)) == STC8H_OK,
                        "params must encode");
    failures += require(memcmp(encoded, params_bytes, sizeof(encoded)) == 0,
                        "params encode must reproduce canonical bytes");
    return failures;
}

static int test_params_rejects_bad_crc(void)
{
    stc8h_ota_params_t params;
    stc8h_u8 corrupted[STC8H_OTA_PARAMS_WIRE_SIZE];

    memcpy(corrupted, params_bytes, sizeof(corrupted));
    corrupted[24] ^= 0x01u;
    return require(stc8h_ota_params_decode(corrupted,
                                           sizeof(corrupted),
                                           &params) == STC8H_ERROR,
                   "params decode must reject canonical bytes with bad crc");
}

int main(void)
{
    int failures;

    failures = 0;
    failures += test_manifest_decode_and_encode();
    failures += test_manifest_rejects_bad_crc();
    failures += test_params_decode_and_encode();
    failures += test_params_rejects_bad_crc();

    return failures == 0 ? 0 : 1;
}
