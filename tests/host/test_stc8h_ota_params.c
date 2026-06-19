#include <stdio.h>
#include <string.h>

#include "../../protocols/stc8h_ota_format.c"
#include "../../protocols/stc8h_ota.c"
#include "../../hal/stc8h_ota_params_store.c"

#define FAKE_FLASH_SIZE 65536UL
#define PARAM_SECTOR_SIZE 512u

static stc8h_u8 fake_flash[FAKE_FLASH_SIZE];
static stc8h_u16 fake_fail_write_after;

static int require(int condition, const char *message)
{
    if (!condition) {
        printf("%s\n", message);
        return 1;
    }
    return 0;
}

static void fake_flash_reset(void)
{
    stc8h_u32 i;

    for (i = 0UL; i < FAKE_FLASH_SIZE; ++i) {
        fake_flash[i] = 0xFFu;
    }
    fake_fail_write_after = 0u;
}

static stc8h_status_t fake_erase(stc8h_u16 addr) STC8H_REENTRANT
{
    stc8h_u16 i;

    for (i = 0u; i < PARAM_SECTOR_SIZE; ++i) {
        fake_flash[(stc8h_u32)addr + i] = 0xFFu;
    }
    return STC8H_OK;
}

static stc8h_status_t fake_write(stc8h_u16 addr, const stc8h_u8 *data, stc8h_u16 len) STC8H_REENTRANT
{
    stc8h_u16 i;
    stc8h_u16 limit;

    limit = len;
    if ((fake_fail_write_after != 0u) && (fake_fail_write_after < len)) {
        limit = fake_fail_write_after;
    }

    for (i = 0u; i < limit; ++i) {
        fake_flash[(stc8h_u32)addr + i] = data[i];
    }

    return (limit == len) ? STC8H_OK : STC8H_ERROR;
}

static stc8h_status_t fake_read(stc8h_u16 addr, stc8h_u8 *data, stc8h_u16 len) STC8H_REENTRANT
{
    stc8h_u16 i;

    for (i = 0u; i < len; ++i) {
        data[i] = fake_flash[(stc8h_u32)addr + i];
    }
    return STC8H_OK;
}

static void make_params(stc8h_ota_params_t *params, stc8h_u16 sequence)
{
    memset(params, 0, sizeof(*params));
    params->param_magic = STC8H_OTA_PARAM_MAGIC;
    params->param_version = STC8H_OTA_PARAM_VERSION;
    params->sequence = sequence;
    params->state = STC8H_OTA_STATE_COMMITTED;
    params->app_valid = 1u;
    params->update_pending = 0u;
    params->boot_attempted = 0u;
    params->app_base = STC8H_OTA_APP_BASE;
    params->app_size = 0x0100UL;
    params->app_crc32 = 0x12345678UL;
    params->version_major = 1u;
    params->version_minor = 2u;
    params->version_patch = 3u;
    params->write_offset = 0UL;
    params->fail_reason = 0u;
}

static void write_raw_record(stc8h_u16 addr, const stc8h_ota_params_t *params)
{
    stc8h_u8 bytes[STC8H_OTA_PARAMS_WIRE_SIZE];

    (void)stc8h_ota_params_encode(params, bytes, sizeof(bytes));
    (void)fake_erase(addr);
    (void)fake_write(addr, bytes, sizeof(bytes));
}

static void init_store(stc8h_ota_params_store_t *store)
{
    stc8h_ota_params_store_init(store, fake_erase, fake_write, fake_read);
}

static int test_empty_records_force_bootloader(void)
{
    stc8h_ota_params_store_t store;
    stc8h_ota_params_t active;

    fake_flash_reset();
    init_store(&store);
    return require(stc8h_ota_params_store_load_active(&store, &active) == STC8H_ERROR,
                   "empty parameter records must not select an application");
}

static int test_valid_app_record_allows_jump(void)
{
    stc8h_ota_params_store_t store;
    stc8h_ota_params_t params;
    stc8h_ota_params_t active;
    int failures;

    failures = 0;
    fake_flash_reset();
    init_store(&store);
    make_params(&params, 1u);
    write_raw_record(STC8H_OTA_PARAM_A_BASE, &params);

    failures += require(stc8h_ota_params_store_load_active(&store, &active) == STC8H_OK,
                        "one valid parameter record must load");
    failures += require(stc8h_ota_get_boot_action(&active) == STC8H_OTA_BOOT_ACTION_JUMP_APP,
                        "valid app record must allow app jump");
    return failures;
}

static int test_update_pending_forces_bootloader(void)
{
    stc8h_ota_params_t params;

    make_params(&params, 1u);
    params.update_pending = 1u;
    return require(stc8h_ota_get_boot_action(&params) == STC8H_OTA_BOOT_ACTION_STAY_BOOTLOADER,
                   "update_pending must force bootloader");
}

static int test_two_valid_records_choose_higher_sequence(void)
{
    stc8h_ota_params_store_t store;
    stc8h_ota_params_t params;
    stc8h_ota_params_t active;
    int failures;

    failures = 0;
    fake_flash_reset();
    init_store(&store);
    make_params(&params, 4u);
    write_raw_record(STC8H_OTA_PARAM_A_BASE, &params);
    make_params(&params, 5u);
    params.version_patch = 9u;
    write_raw_record(STC8H_OTA_PARAM_B_BASE, &params);

    failures += require(stc8h_ota_params_store_load_active(&store, &active) == STC8H_OK,
                        "two valid parameter records must load");
    failures += require(active.sequence == 5u, "higher sequence record must be selected");
    failures += require(active.version_patch == 9u, "selected record must come from higher sequence slot");
    return failures;
}

static int test_corrupted_higher_sequence_is_ignored(void)
{
    stc8h_ota_params_store_t store;
    stc8h_ota_params_t params;
    stc8h_ota_params_t active;
    int failures;

    failures = 0;
    fake_flash_reset();
    init_store(&store);
    make_params(&params, 4u);
    write_raw_record(STC8H_OTA_PARAM_A_BASE, &params);
    make_params(&params, 5u);
    write_raw_record(STC8H_OTA_PARAM_B_BASE, &params);
    fake_flash[STC8H_OTA_PARAM_B_BASE + 13u] ^= 0x01u;

    failures += require(stc8h_ota_params_store_load_active(&store, &active) == STC8H_OK,
                        "corrupted higher record must not block older valid record");
    failures += require(active.sequence == 4u, "older valid record must be selected");
    return failures;
}

static int test_equal_sequence_forces_recovery(void)
{
    stc8h_ota_params_store_t store;
    stc8h_ota_params_t params;
    stc8h_ota_params_t active;

    fake_flash_reset();
    init_store(&store);
    make_params(&params, 4u);
    write_raw_record(STC8H_OTA_PARAM_A_BASE, &params);
    params.version_patch = 9u;
    write_raw_record(STC8H_OTA_PARAM_B_BASE, &params);

    return require(stc8h_ota_params_store_load_active(&store, &active) == STC8H_ERROR,
                   "equal valid sequences must force recovery");
}

static int test_committed_record_allows_one_trial_boot(void)
{
    stc8h_ota_params_t params;

    make_params(&params, 1u);
    params.app_valid = 0u;
    params.boot_attempted = 0u;
    return require(stc8h_ota_get_boot_action(&params) == STC8H_OTA_BOOT_ACTION_TRIAL_APP,
                   "committed unvalidated app must allow one trial boot");
}

static int test_boot_attempted_forces_recovery(void)
{
    stc8h_ota_params_t params;

    make_params(&params, 1u);
    params.app_valid = 0u;
    params.boot_attempted = 1u;
    return require(stc8h_ota_get_boot_action(&params) == STC8H_OTA_BOOT_ACTION_STAY_BOOTLOADER,
                   "attempted unvalidated app must force recovery");
}

static int test_interrupted_write_leaves_old_record_selected(void)
{
    stc8h_ota_params_store_t store;
    stc8h_ota_params_t params;
    stc8h_ota_params_t active;
    int failures;

    failures = 0;
    fake_flash_reset();
    init_store(&store);
    make_params(&params, 4u);
    write_raw_record(STC8H_OTA_PARAM_A_BASE, &params);
    make_params(&params, 5u);
    params.version_patch = 9u;
    fake_fail_write_after = 8u;
    failures += require(stc8h_ota_params_store_write_next(&store, &params) == STC8H_ERROR,
                        "interrupted write must return error");
    fake_fail_write_after = 0u;
    failures += require(stc8h_ota_params_store_load_active(&store, &active) == STC8H_OK,
                        "old record must remain selectable after interrupted write");
    failures += require(active.sequence == 4u, "old sequence must remain active after interrupted write");
    return failures;
}

static int test_mark_boot_attempted_writes_next_record(void)
{
    stc8h_ota_params_store_t store;
    stc8h_ota_params_t params;
    stc8h_ota_params_t active;
    int failures;

    failures = 0;
    fake_flash_reset();
    init_store(&store);
    make_params(&params, 4u);
    params.app_valid = 0u;
    params.boot_attempted = 0u;
    write_raw_record(STC8H_OTA_PARAM_A_BASE, &params);

    failures += require(stc8h_ota_params_store_mark_boot_attempted(&store) == STC8H_OK,
                        "mark boot attempted must write next record");
    failures += require(stc8h_ota_params_store_load_active(&store, &active) == STC8H_OK,
                        "active record must load after mark boot attempted");
    failures += require(active.sequence == 5u, "mark boot attempted must increment sequence");
    failures += require(active.boot_attempted == 1u, "mark boot attempted must set flag");
    return failures;
}

static int test_mark_app_valid_writes_next_record(void)
{
    stc8h_ota_params_store_t store;
    stc8h_ota_params_t params;
    stc8h_ota_params_t active;
    int failures;

    failures = 0;
    fake_flash_reset();
    init_store(&store);
    make_params(&params, 4u);
    params.app_valid = 0u;
    params.update_pending = 1u;
    params.boot_attempted = 1u;
    write_raw_record(STC8H_OTA_PARAM_A_BASE, &params);

    failures += require(stc8h_ota_params_store_mark_app_valid(&store) == STC8H_OK,
                        "mark app valid must write next record");
    failures += require(stc8h_ota_params_store_load_active(&store, &active) == STC8H_OK,
                        "active record must load after mark app valid");
    failures += require(active.sequence == 5u, "mark app valid must increment sequence");
    failures += require(active.app_valid == 1u, "mark app valid must set app_valid");
    failures += require(active.update_pending == 0u, "mark app valid must clear update_pending");
    failures += require(active.boot_attempted == 0u, "mark app valid must clear boot_attempted");
    return failures;
}

int main(void)
{
    int failures;

    failures = 0;
    failures += test_empty_records_force_bootloader();
    failures += test_valid_app_record_allows_jump();
    failures += test_update_pending_forces_bootloader();
    failures += test_two_valid_records_choose_higher_sequence();
    failures += test_corrupted_higher_sequence_is_ignored();
    failures += test_equal_sequence_forces_recovery();
    failures += test_committed_record_allows_one_trial_boot();
    failures += test_boot_attempted_forces_recovery();
    failures += test_interrupted_write_leaves_old_record_selected();
    failures += test_mark_boot_attempted_writes_next_record();
    failures += test_mark_app_valid_writes_next_record();

    return failures == 0 ? 0 : 1;
}
