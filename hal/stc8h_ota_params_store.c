#include "stc8h_ota_params_store.h"

static stc8h_status_t stc8h_ota_params_store_read_record(stc8h_ota_params_store_t *store,
                                                         stc8h_u16 addr,
                                                         stc8h_ota_params_t *params)
{
    stc8h_u8 bytes[STC8H_OTA_PARAMS_WIRE_SIZE];

    if ((store == 0) || (params == 0) || (store->read == 0)) {
        return STC8H_ERROR;
    }

    if (store->read(addr, bytes, sizeof(bytes)) != STC8H_OK) {
        return STC8H_ERROR;
    }

    return stc8h_ota_params_decode(bytes, sizeof(bytes), params);
}

static stc8h_u8 stc8h_ota_params_store_bytes_equal(const stc8h_u8 *a,
                                                   const stc8h_u8 *b,
                                                   stc8h_u16 len)
{
    stc8h_u16 i;

    for (i = 0u; i < len; ++i) {
        if (a[i] != b[i]) {
            return 0u;
        }
    }

    return 1u;
}

void stc8h_ota_params_store_init(stc8h_ota_params_store_t *store,
                                 stc8h_ota_param_erase_fn erase,
                                 stc8h_ota_param_write_fn write,
                                 stc8h_ota_param_read_fn read)
{
    if (store == 0) {
        return;
    }

    store->erase = erase;
    store->write = write;
    store->read = read;
    store->active_addr = STC8H_OTA_PARAM_A_BASE;
    store->has_active = 0u;
}

stc8h_status_t stc8h_ota_params_store_load_active(stc8h_ota_params_store_t *store,
                                                  stc8h_ota_params_t *params)
{
    stc8h_ota_params_t record_a;
    stc8h_ota_params_t record_b;
    stc8h_status_t status_a;
    stc8h_status_t status_b;

    if ((store == 0) || (params == 0)) {
        return STC8H_ERROR;
    }

    status_a = stc8h_ota_params_store_read_record(store, STC8H_OTA_PARAM_A_BASE, &record_a);
    status_b = stc8h_ota_params_store_read_record(store, STC8H_OTA_PARAM_B_BASE, &record_b);

    if ((status_a != STC8H_OK) && (status_b != STC8H_OK)) {
        store->has_active = 0u;
        return STC8H_ERROR;
    }

    if ((status_a == STC8H_OK) && (status_b == STC8H_OK)) {
        if (record_a.sequence == record_b.sequence) {
            store->has_active = 0u;
            return STC8H_ERROR;
        }
        if (record_b.sequence > record_a.sequence) {
            *params = record_b;
            store->active_addr = STC8H_OTA_PARAM_B_BASE;
        } else {
            *params = record_a;
            store->active_addr = STC8H_OTA_PARAM_A_BASE;
        }
    } else if (status_a == STC8H_OK) {
        *params = record_a;
        store->active_addr = STC8H_OTA_PARAM_A_BASE;
    } else {
        *params = record_b;
        store->active_addr = STC8H_OTA_PARAM_B_BASE;
    }

    store->has_active = 1u;
    return STC8H_OK;
}

stc8h_status_t stc8h_ota_params_store_write_next(stc8h_ota_params_store_t *store,
                                                 const stc8h_ota_params_t *params)
{
    stc8h_ota_params_t active;
    stc8h_u8 bytes[STC8H_OTA_PARAMS_WIRE_SIZE];
    stc8h_u8 verify[STC8H_OTA_PARAMS_WIRE_SIZE];
    stc8h_u16 target_addr;

    if ((store == 0) || (params == 0) || (store->erase == 0) ||
        (store->write == 0) || (store->read == 0)) {
        return STC8H_ERROR;
    }

    if (stc8h_ota_params_store_load_active(store, &active) == STC8H_OK) {
        target_addr = (store->active_addr == STC8H_OTA_PARAM_A_BASE) ?
                      STC8H_OTA_PARAM_B_BASE : STC8H_OTA_PARAM_A_BASE;
    } else {
        target_addr = STC8H_OTA_PARAM_A_BASE;
    }

    if (stc8h_ota_params_encode(params, bytes, sizeof(bytes)) != STC8H_OK) {
        return STC8H_ERROR;
    }
    if (store->erase(target_addr) != STC8H_OK) {
        return STC8H_ERROR;
    }
    if (store->write(target_addr, bytes, sizeof(bytes)) != STC8H_OK) {
        return STC8H_ERROR;
    }
    if (store->read(target_addr, verify, sizeof(verify)) != STC8H_OK) {
        return STC8H_ERROR;
    }
    if (stc8h_ota_params_store_bytes_equal(bytes, verify, sizeof(bytes)) == 0u) {
        return STC8H_ERROR;
    }

    store->active_addr = target_addr;
    store->has_active = 1u;
    return STC8H_OK;
}

stc8h_status_t stc8h_ota_params_store_mark_boot_attempted(stc8h_ota_params_store_t *store)
{
    stc8h_ota_params_t params;

    if (stc8h_ota_params_store_load_active(store, &params) != STC8H_OK) {
        return STC8H_ERROR;
    }

    params.sequence = (stc8h_u16)(params.sequence + 1u);
    params.boot_attempted = 1u;
    return stc8h_ota_params_store_write_next(store, &params);
}

stc8h_status_t stc8h_ota_params_store_mark_app_valid(stc8h_ota_params_store_t *store)
{
    stc8h_ota_params_t params;

    if (stc8h_ota_params_store_load_active(store, &params) != STC8H_OK) {
        return STC8H_ERROR;
    }

    params.sequence = (stc8h_u16)(params.sequence + 1u);
    params.app_valid = 1u;
    params.update_pending = 0u;
    params.boot_attempted = 0u;
    return stc8h_ota_params_store_write_next(store, &params);
}
