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

#ifndef STC8H_OTA_APP_BASE
#define STC8H_OTA_APP_BASE 0x0200u
#endif

#ifndef STC8H_OTA_APP_LIMIT
#define STC8H_OTA_APP_LIMIT 0xEFFFu
#endif

#ifndef STC8H_OTA_PARAM_A_BASE
#define STC8H_OTA_PARAM_A_BASE 0xFC00u
#endif

#ifndef STC8H_OTA_PARAM_B_BASE
#define STC8H_OTA_PARAM_B_BASE 0xFE00u
#endif

#ifndef STC8H_OTA_WORK_MEM
#define STC8H_OTA_WORK_MEM
#endif

#ifndef STC8H_OTA_ENABLE_SHOULD_ENTER_BOOTLOADER
#define STC8H_OTA_ENABLE_SHOULD_ENTER_BOOTLOADER 1
#endif

#define STC8H_OTA_PARAM_MAGIC 0x4F545041UL
#define STC8H_OTA_PARAM_VERSION 1u
#define STC8H_OTA_DEFAULT_SECTOR_SIZE 512u

typedef enum {
    STC8H_OTA_STATE_IDLE = 0,
    STC8H_OTA_STATE_PREPARING = 1,
    STC8H_OTA_STATE_RECEIVING = 2,
    STC8H_OTA_STATE_VERIFYING = 3,
    STC8H_OTA_STATE_PENDING_COMMIT = 4,
    STC8H_OTA_STATE_COMMITTED = 5,
    STC8H_OTA_STATE_FAILED = 6,
    STC8H_OTA_STATE_RECOVERY = 7,
    STC8H_OTA_STATE_APP_VALID = 8
} stc8h_ota_state_t;

typedef enum {
    STC8H_OTA_BOOT_ACTION_STAY_BOOTLOADER = 0,
    STC8H_OTA_BOOT_ACTION_JUMP_APP = 1,
    STC8H_OTA_BOOT_ACTION_TRIAL_APP = 2
} stc8h_ota_boot_action_t;

typedef struct stc8h_ota_params_store_s stc8h_ota_params_store_t;

typedef stc8h_status_t (*stc8h_ota_backend_erase_fn)(stc8h_u16 addr) STC8H_REENTRANT;
typedef stc8h_status_t (*stc8h_ota_backend_write_fn)(stc8h_u16 addr,
                                                     const stc8h_u8 *data,
                                                     stc8h_u16 len) STC8H_REENTRANT;
typedef stc8h_status_t (*stc8h_ota_backend_read_fn)(stc8h_u16 addr,
                                                    stc8h_u8 *data,
                                                    stc8h_u16 len) STC8H_REENTRANT;

typedef struct {
    stc8h_ota_backend_erase_fn erase_sector;
    stc8h_ota_backend_write_fn write;
    stc8h_ota_backend_read_fn read;
    stc8h_u16 sector_size;
} stc8h_ota_backend_t;

typedef struct {
    const stc8h_ota_backend_t *backend;
    stc8h_ota_params_store_t *params_store;
    stc8h_ota_manifest_t manifest;
    stc8h_u32 write_offset;
    stc8h_u32 last_chunk_offset;
    stc8h_u16 last_chunk_len;
    stc8h_ota_state_t state;
    stc8h_u8 fail_reason;
} stc8h_ota_context_t;

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

void stc8h_ota_init(stc8h_ota_context_t *ctx,
                    const stc8h_ota_backend_t *backend,
                    stc8h_ota_params_store_t *params_store);
stc8h_status_t stc8h_ota_validate_manifest(const stc8h_ota_manifest_t *manifest);
stc8h_ota_boot_action_t stc8h_ota_get_boot_action(const stc8h_ota_params_t *params);
#if STC8H_OTA_ENABLE_SHOULD_ENTER_BOOTLOADER
stc8h_u8 stc8h_ota_should_enter_bootloader(const stc8h_ota_params_t *params);
#endif
stc8h_status_t stc8h_ota_begin(stc8h_ota_context_t *ctx,
                               const stc8h_ota_manifest_t *manifest);
stc8h_status_t stc8h_ota_write_chunk(stc8h_ota_context_t *ctx,
                                     stc8h_u32 offset,
                                     const stc8h_u8 *data,
                                     stc8h_u16 len);
stc8h_status_t stc8h_ota_verify(stc8h_ota_context_t *ctx);
stc8h_status_t stc8h_ota_commit(stc8h_ota_context_t *ctx);
stc8h_status_t stc8h_ota_abort(stc8h_ota_context_t *ctx, stc8h_u8 reason);
stc8h_ota_state_t stc8h_ota_get_status(const stc8h_ota_context_t *ctx);

#endif
