#include <stdio.h>
#include <string.h>

#include "../../utils/util_crc32.c"
#include "../../protocols/stc8h_ota_format.c"
#include "../../protocols/stc8h_ota.c"
#include "../../hal/stc8h_ota_params_store.c"

#define FLASH_SIZE 65536UL
#define SECTOR_SIZE 512u

static stc8h_u8 flash_bytes[FLASH_SIZE];
static stc8h_u16 fail_write_after;
static stc8h_u8 fail_commit_marker;

static int require(int condition, const char *message)
{
    if (!condition) {
        printf("%s\n", message);
        return 1;
    }
    return 0;
}

static void flash_reset(void)
{
    memset(flash_bytes, 0xFF, sizeof(flash_bytes));
    fail_write_after = 0u;
    fail_commit_marker = 0u;
}

static stc8h_status_t fake_erase(stc8h_u16 addr) STC8H_REENTRANT
{
    memset(&flash_bytes[addr], 0xFF, SECTOR_SIZE);
    return STC8H_OK;
}

static stc8h_status_t fake_write(stc8h_u16 addr,
                                 const stc8h_u8 *data,
                                 stc8h_u16 len) STC8H_REENTRANT
{
    stc8h_u16 count;

    if ((fail_commit_marker != 0u) &&
        ((addr & (SECTOR_SIZE - 1u)) ==
         (STC8H_OTA_PARAMS_WIRE_SIZE - 2u))) {
        return STC8H_ERROR;
    }
    count = len;
    if ((fail_write_after != 0u) && (fail_write_after < len)) {
        count = fail_write_after;
    }
    memcpy(&flash_bytes[addr], data, count);
    return count == len ? STC8H_OK : STC8H_ERROR;
}

static stc8h_status_t fake_read(stc8h_u16 addr,
                                stc8h_u8 *data,
                                stc8h_u16 len) STC8H_REENTRANT
{
    memcpy(data, &flash_bytes[addr], len);
    return STC8H_OK;
}

static void init_store(stc8h_ota_params_store_t *store)
{
    stc8h_ota_params_store_init(store, fake_erase, fake_write, fake_read);
}

static void make_params(stc8h_ota_params_t *params,
                        stc8h_u16 generation,
                        stc8h_u8 state)
{
    memset(params, 0, sizeof(*params));
    params->param_magic = STC8H_OTA_PARAM_MAGIC;
    params->param_version = STC8H_OTA_PARAM_VERSION;
    params->state = state;
    params->flags = state == STC8H_OTA_STATE_APP_VALID ?
                    STC8H_OTA_PARAM_FLAG_APP_VALID : 0u;
    params->generation = generation;
    params->app_base = STC8H_OTA_APP_BASE;
    params->app_size = 256u;
    params->app_crc32 = 0x12345678UL;
    params->version_major = 1u;
    params->build_number = 4u;
    params->commit_marker = STC8H_OTA_PARAM_COMMIT_MARKER;
}

static int test_empty_and_two_slot_selection(void)
{
    stc8h_ota_params_store_t store;
    stc8h_ota_params_t params;
    stc8h_ota_params_t active;
    int failures;

    failures = 0;
    flash_reset();
    init_store(&store);
    failures += require(stc8h_ota_params_store_load_active(
                            &store, &active) == STC8H_ERROR,
                        "empty slots must force recovery");
    make_params(&params, 1u, STC8H_OTA_STATE_APP_VALID);
    failures += require(stc8h_ota_params_store_write_next(
                            &store, &params) == STC8H_OK,
                        "first record must commit");
    params.generation = 2u;
    params.build_number = 5u;
    failures += require(stc8h_ota_params_store_write_next(
                            &store, &params) == STC8H_OK,
                        "second record must commit");
    failures += require(stc8h_ota_params_store_load_active(
                            &store, &active) == STC8H_OK &&
                        active.generation == 2u && active.build_number == 5u,
                        "newer committed generation must win");
    return failures;
}

static int test_torn_body_and_marker_keep_old_record(void)
{
    stc8h_ota_params_store_t store;
    stc8h_ota_params_t params;
    stc8h_ota_params_t active;
    int failures;

    failures = 0;
    flash_reset();
    init_store(&store);
    make_params(&params, 10u, STC8H_OTA_STATE_APP_VALID);
    (void)stc8h_ota_params_store_write_next(&store, &params);
    params.generation = 11u;
    fail_write_after = 8u;
    failures += require(stc8h_ota_params_store_write_next(
                            &store, &params) == STC8H_ERROR,
                        "torn body must fail");
    fail_write_after = 0u;
    failures += require(stc8h_ota_params_store_load_active(
                            &store, &active) == STC8H_OK &&
                        active.generation == 10u,
                        "old record must survive torn body");

    params.generation = 11u;
    fail_commit_marker = 1u;
    failures += require(stc8h_ota_params_store_write_next(
                            &store, &params) == STC8H_ERROR,
                        "missing commit marker must fail");
    fail_commit_marker = 0u;
    failures += require(stc8h_ota_params_store_load_active(
                            &store, &active) == STC8H_OK &&
                        active.generation == 10u,
                        "old record must survive missing marker");
    return failures;
}

static int test_equal_generation_and_corrupt_newer_are_bounded(void)
{
    stc8h_ota_params_store_t store;
    stc8h_ota_params_t params;
    stc8h_ota_params_t active;
    stc8h_u8 wire[STC8H_OTA_PARAMS_WIRE_SIZE];
    int failures;

    failures = 0;
    flash_reset();
    init_store(&store);
    make_params(&params, 7u, STC8H_OTA_STATE_APP_VALID);
    (void)stc8h_ota_params_encode(&params, wire, sizeof(wire));
    memcpy(&flash_bytes[STC8H_OTA_PARAM_A_BASE], wire, sizeof(wire));
    memcpy(&flash_bytes[STC8H_OTA_PARAM_B_BASE], wire, sizeof(wire));
    failures += require(stc8h_ota_params_store_load_active(&store, &active) ==
                        STC8H_ERROR,
                        "equal generations must be treated as ambiguous");

    flash_reset();
    make_params(&params, 8u, STC8H_OTA_STATE_APP_VALID);
    (void)stc8h_ota_params_store_write_next(&store, &params);
    params.generation = 9u;
    (void)stc8h_ota_params_store_write_next(&store, &params);
    flash_bytes[STC8H_OTA_PARAM_B_BASE + 20u] ^= 1u;
    failures += require(stc8h_ota_params_store_load_active(&store, &active) ==
                        STC8H_OK && active.generation == 8u,
                        "corrupt newer record must fall back to older commit");
    return failures;
}

static int test_generation_wrap(void)
{
    stc8h_ota_params_store_t store;
    stc8h_ota_params_t params;
    stc8h_ota_params_t active;

    flash_reset();
    init_store(&store);
    make_params(&params, 0xFFFFu, STC8H_OTA_STATE_APP_VALID);
    (void)stc8h_ota_params_store_write_next(&store, &params);
    params.generation = 0u;
    params.build_number = 9u;
    (void)stc8h_ota_params_store_write_next(&store, &params);
    return require(stc8h_ota_params_store_load_active(&store, &active) ==
                   STC8H_OK && active.generation == 0u &&
                   active.build_number == 9u,
                   "generation wrap must select modularly newer record");
}

static int test_explicit_request_cancel_and_trial_confirmation(void)
{
    stc8h_ota_params_store_t store;
    stc8h_ota_params_t params;
    stc8h_ota_params_t active;
    int failures;

    failures = 0;
    flash_reset();
    init_store(&store);
    make_params(&params, 1u, STC8H_OTA_STATE_APP_VALID);
    (void)stc8h_ota_params_store_write_next(&store, &params);
    failures += require(stc8h_ota_params_store_request_update(
                            &store, 0x11223344UL) == STC8H_OK,
                        "explicit update request must persist");
    (void)stc8h_ota_params_store_load_active(&store, &active);
    failures += require(active.state == STC8H_OTA_STATE_UPDATE_REQUESTED &&
                        active.session_id == 0x11223344UL &&
                        stc8h_ota_get_boot_action(&active) ==
                        STC8H_OTA_BOOT_ACTION_STAY_BOOTLOADER,
                        "requested update must keep bootloader active");
    failures += require(stc8h_ota_params_store_cancel_update_request(
                            &store, 0x55667788UL) == STC8H_ERROR,
                        "wrong session must not cancel request");
    failures += require(stc8h_ota_params_store_cancel_update_request(
                            &store, 0x11223344UL) == STC8H_OK,
                        "matching session may cancel before erase");
    (void)stc8h_ota_params_store_load_active(&store, &active);
    failures += require(stc8h_ota_get_boot_action(&active) ==
                        STC8H_OTA_BOOT_ACTION_JUMP_APP,
                        "canceled request must restore valid app");

    active.generation = (stc8h_u16)(active.generation + 1u);
    active.state = STC8H_OTA_STATE_TRIAL_PENDING;
    active.flags = 0u;
    active.session_id = 0x11223344UL;
    (void)stc8h_ota_params_store_write_next(&store, &active);
    failures += require(stc8h_ota_get_boot_action(&active) ==
                        STC8H_OTA_BOOT_ACTION_TRIAL_APP,
                        "pending image must get one trial");
    failures += require(stc8h_ota_params_store_mark_trial_started(
                            &store) == STC8H_OK,
                        "trial start must persist before jump");
    (void)stc8h_ota_params_store_load_active(&store, &active);
    failures += require(stc8h_ota_get_boot_action(&active) ==
                        STC8H_OTA_BOOT_ACTION_STAY_BOOTLOADER &&
                        active.session_id == 0x11223344UL,
                        "unconfirmed trial reset must recover with session evidence");
    failures += require(stc8h_ota_params_store_mark_app_valid(
                            &store) == STC8H_OK,
                        "healthy trial app must confirm itself");
    (void)stc8h_ota_params_store_load_active(&store, &active);
    failures += require(stc8h_ota_get_boot_action(&active) ==
                        STC8H_OTA_BOOT_ACTION_JUMP_APP &&
                        active.session_id == 0UL,
                        "confirmed app must boot normally and clear session");
    return failures;
}

int main(void)
{
    int failures;

    failures = 0;
    failures += test_empty_and_two_slot_selection();
    failures += test_torn_body_and_marker_keep_old_record();
    failures += test_equal_generation_and_corrupt_newer_are_bounded();
    failures += test_generation_wrap();
    failures += test_explicit_request_cancel_and_trial_confirmation();
    return failures == 0 ? 0 : 1;
}
