#ifndef STC8H_OTA_H
#define STC8H_OTA_H

#include "stc8h_boot_stub.h"
#include "stc8h_ota_format.h"

#if !STC8H_CHIP_STC8H8K64U
#error "STC8H OTA core supports only STC8H8K64U."
#endif

#define STC8H_OTA_MANIFEST_MAGIC 0x4F544131UL
#define STC8H_OTA_FORMAT_VERSION 1u
#define STC8H_OTA_TARGET_STC8H8K64U 0x0864u
#define STC8H_OTA_BOOTLOADER_VERSION 2u

#ifndef STC8H_OTA_APP_BASE
#define STC8H_OTA_APP_BASE STC8H_BOOT_APP_BASE
#endif
#ifndef STC8H_OTA_APP_LIMIT
#define STC8H_OTA_APP_LIMIT STC8H_BOOT_APP_LIMIT
#endif
#ifndef STC8H_OTA_PARAM_A_BASE
#define STC8H_OTA_PARAM_A_BASE STC8H_BOOT_PARAM_A_BASE
#endif
#ifndef STC8H_OTA_PARAM_B_BASE
#define STC8H_OTA_PARAM_B_BASE STC8H_BOOT_PARAM_B_BASE
#endif
#ifndef STC8H_OTA_CHECKPOINT_BYTES
#define STC8H_OTA_CHECKPOINT_BYTES 2048u
#endif
#ifndef STC8H_OTA_WORK_MEM
#define STC8H_OTA_WORK_MEM
#endif
#ifndef STC8H_OTA_ENABLE_SHOULD_ENTER_BOOTLOADER
#define STC8H_OTA_ENABLE_SHOULD_ENTER_BOOTLOADER 1
#endif

#define STC8H_OTA_PARAM_MAGIC 0x4F545032UL
#define STC8H_OTA_PARAM_VERSION 2u
#define STC8H_OTA_DEFAULT_SECTOR_SIZE 512u

#define STC8H_OTA_PARAM_FLAG_APP_VALID 0x01u
#define STC8H_OTA_PARAM_FLAG_UPDATE_REQUESTED 0x02u
#define STC8H_OTA_PARAM_FLAG_TRIAL_ATTEMPTED 0x04u

#define STC8H_OTA_BEGIN_FLAG_RESTART 0x01u

typedef enum {
    STC8H_OTA_STATE_EMPTY = 0,
    STC8H_OTA_STATE_APP_VALID = 1,
    STC8H_OTA_STATE_UPDATE_REQUESTED = 2,
    STC8H_OTA_STATE_PREPARING = 3,
    STC8H_OTA_STATE_RECEIVING = 4,
    STC8H_OTA_STATE_VERIFIED = 5,
    STC8H_OTA_STATE_TRIAL_PENDING = 6,
    STC8H_OTA_STATE_TRIAL_STARTED = 7,
    STC8H_OTA_STATE_RECOVERY = 8,
    STC8H_OTA_STATE_FAILED = 9
} stc8h_ota_state_t;

typedef enum {
    STC8H_OTA_BOOT_ACTION_STAY_BOOTLOADER = 0,
    STC8H_OTA_BOOT_ACTION_JUMP_APP = 1,
    STC8H_OTA_BOOT_ACTION_TRIAL_APP = 2
} stc8h_ota_boot_action_t;

typedef enum {
    STC8H_OTA_FAIL_NONE = 0,
    STC8H_OTA_FAIL_ARG = 1,
    STC8H_OTA_FAIL_MANIFEST = 2,
    STC8H_OTA_FAIL_ERASE = 3,
    STC8H_OTA_FAIL_STATE = 4,
    STC8H_OTA_FAIL_OFFSET = 5,
    STC8H_OTA_FAIL_RANGE = 6,
    STC8H_OTA_FAIL_DUPLICATE = 7,
    STC8H_OTA_FAIL_WRITE = 8,
    STC8H_OTA_FAIL_READ = 9,
    STC8H_OTA_FAIL_INCOMPLETE = 10,
    STC8H_OTA_FAIL_CRC = 11,
    STC8H_OTA_FAIL_PARAMS = 12,
    STC8H_OTA_FAIL_SESSION = 13,
    STC8H_OTA_FAIL_NOT_REQUESTED = 14,
    STC8H_OTA_FAIL_READBACK = 15
} stc8h_ota_fail_reason_t;

typedef struct stc8h_ota_params_store_s stc8h_ota_params_store_t;

typedef stc8h_status_t (*stc8h_ota_backend_erase_fn)(stc8h_u16 addr) STC8H_REENTRANT;
typedef stc8h_status_t (*stc8h_ota_backend_write_fn)(stc8h_u16 addr,
                                                     const stc8h_u8 *data,
                                                     stc8h_u16 len) STC8H_REENTRANT;
typedef stc8h_status_t (*stc8h_ota_backend_read_fn)(stc8h_u16 addr,
                                                    stc8h_u8 *data,
                                                    stc8h_u16 len) STC8H_REENTRANT;
typedef void (*stc8h_ota_keep_alive_fn)(void);

typedef struct {
    stc8h_ota_backend_erase_fn erase_sector;
    stc8h_ota_backend_write_fn write;
    stc8h_ota_backend_read_fn read;
    stc8h_ota_keep_alive_fn keep_alive;
    stc8h_u16 sector_size;
} stc8h_ota_backend_t;

typedef struct {
    const stc8h_ota_backend_t *backend;
    stc8h_ota_params_store_t *params_store;
    stc8h_ota_manifest_t manifest;
    stc8h_u32 session_id;
    stc8h_u16 write_offset;
    stc8h_u16 persisted_offset;
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
stc8h_status_t stc8h_ota_restore(stc8h_ota_context_t *ctx);
stc8h_status_t stc8h_ota_validate_manifest(const stc8h_ota_manifest_t *manifest);
stc8h_ota_boot_action_t stc8h_ota_get_boot_action(const stc8h_ota_params_t *params);
#if STC8H_OTA_ENABLE_SHOULD_ENTER_BOOTLOADER
stc8h_u8 stc8h_ota_should_enter_bootloader(const stc8h_ota_params_t *params);
#endif
stc8h_status_t stc8h_ota_begin(stc8h_ota_context_t *ctx,
                               const stc8h_ota_manifest_t *manifest,
                               stc8h_u32 session_id,
                               stc8h_u8 flags);
stc8h_status_t stc8h_ota_write_chunk(stc8h_ota_context_t *ctx,
                                     stc8h_u16 offset,
                                     const stc8h_u8 *data,
                                     stc8h_u16 len);
stc8h_status_t stc8h_ota_verify(stc8h_ota_context_t *ctx);
stc8h_status_t stc8h_ota_activate(stc8h_ota_context_t *ctx);
stc8h_status_t stc8h_ota_abort(stc8h_ota_context_t *ctx, stc8h_u8 reason);
stc8h_ota_state_t stc8h_ota_get_status(const stc8h_ota_context_t *ctx);

#endif
