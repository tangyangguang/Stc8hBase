#ifndef STC8H_OTA_H
#define STC8H_OTA_H

#include "stc8h_ota_format.h"

#if !STC8H_CHIP_STC8H8K64U
#error "STC8H OTA core currently supports only STC8H8K64U."
#endif

#define STC8H_OTA_MANIFEST_MAGIC 0x4F544131UL
#define STC8H_OTA_FORMAT_VERSION 1u
#define STC8H_OTA_TARGET_STC8H8K64U 0x0864u
#define STC8H_OTA_BOOTLOADER_VERSION 1u

#define STC8H_OTA_APP_BASE 0x0200u
#define STC8H_OTA_APP_LIMIT 0xEFFFu
#define STC8H_OTA_PARAM_A_BASE 0xFC00u
#define STC8H_OTA_PARAM_B_BASE 0xFE00u

#ifdef STC8H_OTA_EXPECTED_BOARD_ID
#define STC8H_OTA_CHECK_BOARD_ID 1
#else
#define STC8H_OTA_CHECK_BOARD_ID 0
#define STC8H_OTA_EXPECTED_BOARD_ID 0u
#endif

#ifdef STC8H_OTA_EXPECTED_HW_REVISION
#define STC8H_OTA_CHECK_HW_REVISION 1
#else
#define STC8H_OTA_CHECK_HW_REVISION 0
#define STC8H_OTA_EXPECTED_HW_REVISION 0u
#endif

#ifdef STC8H_OTA_EXPECTED_APP_ID
#define STC8H_OTA_CHECK_APP_ID 1
#else
#define STC8H_OTA_CHECK_APP_ID 0
#define STC8H_OTA_EXPECTED_APP_ID 0u
#endif

stc8h_status_t stc8h_ota_validate_manifest(const stc8h_ota_manifest_t *manifest);

#endif
