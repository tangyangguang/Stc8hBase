#include "stc8h_ota.h"

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
