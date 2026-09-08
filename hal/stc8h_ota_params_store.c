#include "stc8h_ota_params_store.h"

#define STC8H_OTA_PARAM_BODY_SIZE (STC8H_OTA_PARAMS_WIRE_SIZE - 2u)

static stc8h_status_t stc8h_ota_params_store_read_record(
    stc8h_ota_params_store_t *store,
    stc8h_u16 addr,
    stc8h_ota_params_t *params)
{
    STC8H_OTA_PARAMS_STORE_WORK_MEM stc8h_u8 bytes[STC8H_OTA_PARAMS_WIRE_SIZE];

    if ((store == 0) || (params == 0) || (store->read == 0)) {
        return STC8H_ERROR;
    }
    if (store->read(addr, bytes, sizeof(bytes)) != STC8H_OK) {
        return STC8H_ERROR;
    }
    if (stc8h_ota_params_decode(bytes, sizeof(bytes), params) != STC8H_OK) {
        return STC8H_ERROR;
    }
    if ((params->param_magic != STC8H_OTA_PARAM_MAGIC) ||
        (params->param_version != STC8H_OTA_PARAM_VERSION)) {
        return STC8H_ERROR;
    }
    return STC8H_OK;
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

static stc8h_u8 stc8h_ota_generation_is_newer(stc8h_u16 candidate,
                                               stc8h_u16 reference)
{
    stc8h_u16 delta;

    delta = (stc8h_u16)((candidate - reference) & 0xFFFFu);
    return ((delta != 0u) && (delta < 0x8000u)) ? 1u : 0u;
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

stc8h_status_t stc8h_ota_params_store_load_active(
    stc8h_ota_params_store_t *store,
    stc8h_ota_params_t *params)
{
    STC8H_OTA_PARAMS_STORE_WORK_MEM stc8h_ota_params_t record_a;
    STC8H_OTA_PARAMS_STORE_WORK_MEM stc8h_ota_params_t record_b;
    stc8h_status_t status_a;
    stc8h_status_t status_b;
    stc8h_u16 selected_addr;

    if ((store == 0) || (params == 0)) {
        return STC8H_ERROR;
    }
    status_a = stc8h_ota_params_store_read_record(
        store, STC8H_OTA_PARAM_A_BASE, &record_a);
    status_b = stc8h_ota_params_store_read_record(
        store, STC8H_OTA_PARAM_B_BASE, &record_b);

    if ((status_a != STC8H_OK) && (status_b != STC8H_OK)) {
        store->has_active = 0u;
        return STC8H_ERROR;
    }
    if ((status_a == STC8H_OK) && (status_b == STC8H_OK)) {
        if (record_a.generation == record_b.generation) {
            store->has_active = 0u;
            return STC8H_ERROR;
        }
        selected_addr = stc8h_ota_generation_is_newer(
            record_b.generation, record_a.generation) != 0u ?
            STC8H_OTA_PARAM_B_BASE : STC8H_OTA_PARAM_A_BASE;
    } else if (status_a == STC8H_OK) {
        selected_addr = STC8H_OTA_PARAM_A_BASE;
    } else {
        selected_addr = STC8H_OTA_PARAM_B_BASE;
    }

    if (stc8h_ota_params_store_read_record(store, selected_addr, params) !=
        STC8H_OK) {
        store->has_active = 0u;
        return STC8H_ERROR;
    }
    store->active_addr = selected_addr;
    store->has_active = 1u;
    return STC8H_OK;
}

stc8h_status_t stc8h_ota_params_store_write_next(
    stc8h_ota_params_store_t *store,
    const stc8h_ota_params_t *params)
{
    STC8H_OTA_PARAMS_STORE_WORK_MEM stc8h_ota_params_t active;
    STC8H_OTA_PARAMS_STORE_WORK_MEM stc8h_ota_params_t verify_params;
    STC8H_OTA_PARAMS_STORE_WORK_MEM stc8h_u8 bytes[STC8H_OTA_PARAMS_WIRE_SIZE];
    STC8H_OTA_PARAMS_STORE_WORK_MEM stc8h_u8 verify[STC8H_OTA_PARAMS_WIRE_SIZE];
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
    if (store->write(target_addr, bytes, STC8H_OTA_PARAM_BODY_SIZE) !=
        STC8H_OK) {
        return STC8H_ERROR;
    }
    if (store->read(target_addr, verify, STC8H_OTA_PARAM_BODY_SIZE) !=
        STC8H_OK) {
        return STC8H_ERROR;
    }
    if (stc8h_ota_params_store_bytes_equal(
            bytes, verify, STC8H_OTA_PARAM_BODY_SIZE) == 0u) {
        return STC8H_ERROR;
    }
    if (store->write((stc8h_u16)(target_addr + STC8H_OTA_PARAM_BODY_SIZE),
                     &bytes[STC8H_OTA_PARAM_BODY_SIZE], 2u) != STC8H_OK) {
        return STC8H_ERROR;
    }
    if (stc8h_ota_params_store_read_record(store, target_addr,
                                            &verify_params) != STC8H_OK) {
        return STC8H_ERROR;
    }
    if ((verify_params.generation != params->generation) ||
        (verify_params.state != params->state) ||
        (verify_params.session_id != params->session_id)) {
        return STC8H_ERROR;
    }

    store->active_addr = target_addr;
    store->has_active = 1u;
    return STC8H_OK;
}

static stc8h_status_t stc8h_ota_params_store_update_flags(
    stc8h_ota_params_store_t *store,
    stc8h_u8 required_state,
    stc8h_u8 new_state,
    stc8h_u8 set_flags,
    stc8h_u8 clear_flags,
    stc8h_u32 session_id,
    stc8h_u8 replace_session)
{
    STC8H_OTA_PARAMS_STORE_WORK_MEM stc8h_ota_params_t params;

    if ((stc8h_ota_params_store_load_active(store, &params) != STC8H_OK) ||
        (params.state != required_state)) {
        return STC8H_ERROR;
    }
    ++params.generation;
    params.state = new_state;
    params.flags = (stc8h_u8)((params.flags | set_flags) &
                              (stc8h_u8)(~clear_flags));
    if (replace_session != 0u) {
        params.session_id = session_id;
    }
    params.fail_reason = 0u;
    return stc8h_ota_params_store_write_next(store, &params);
}

stc8h_status_t stc8h_ota_params_store_mark_trial_started(
    stc8h_ota_params_store_t *store)
{
    return stc8h_ota_params_store_update_flags(
        store,
        STC8H_OTA_STATE_TRIAL_PENDING,
        STC8H_OTA_STATE_TRIAL_STARTED,
        STC8H_OTA_PARAM_FLAG_TRIAL_ATTEMPTED,
        0u,
        0UL,
        0u);
}

#if STC8H_OTA_PARAMS_STORE_ENABLE_APP_API
stc8h_status_t stc8h_ota_params_store_request_update(
    stc8h_ota_params_store_t *store,
    stc8h_u32 session_id)
{
    STC8H_OTA_PARAMS_STORE_WORK_MEM stc8h_ota_params_t params;

    if ((session_id == 0UL) ||
        (stc8h_ota_params_store_load_active(store, &params) != STC8H_OK) ||
        (params.state != STC8H_OTA_STATE_APP_VALID) ||
        ((params.flags & STC8H_OTA_PARAM_FLAG_APP_VALID) == 0u)) {
        return STC8H_ERROR;
    }
    ++params.generation;
    params.state = STC8H_OTA_STATE_UPDATE_REQUESTED;
    params.flags |= STC8H_OTA_PARAM_FLAG_UPDATE_REQUESTED;
    params.session_id = session_id;
    params.fail_reason = 0u;
    return stc8h_ota_params_store_write_next(store, &params);
}

stc8h_status_t stc8h_ota_params_store_cancel_update_request(
    stc8h_ota_params_store_t *store,
    stc8h_u32 session_id)
{
    STC8H_OTA_PARAMS_STORE_WORK_MEM stc8h_ota_params_t params;

    if ((session_id == 0UL) ||
        (stc8h_ota_params_store_load_active(store, &params) != STC8H_OK) ||
        (params.state != STC8H_OTA_STATE_UPDATE_REQUESTED) ||
        (params.session_id != session_id) ||
        ((params.flags & STC8H_OTA_PARAM_FLAG_APP_VALID) == 0u)) {
        return STC8H_ERROR;
    }
    ++params.generation;
    params.state = STC8H_OTA_STATE_APP_VALID;
    params.flags = STC8H_OTA_PARAM_FLAG_APP_VALID;
    params.session_id = 0UL;
    params.fail_reason = 0u;
    return stc8h_ota_params_store_write_next(store, &params);
}

stc8h_status_t stc8h_ota_params_store_mark_app_valid(
    stc8h_ota_params_store_t *store)
{
    return stc8h_ota_params_store_update_flags(
        store,
        STC8H_OTA_STATE_TRIAL_STARTED,
        STC8H_OTA_STATE_APP_VALID,
        STC8H_OTA_PARAM_FLAG_APP_VALID,
        (stc8h_u8)(STC8H_OTA_PARAM_FLAG_UPDATE_REQUESTED |
                   STC8H_OTA_PARAM_FLAG_TRIAL_ATTEMPTED),
        0UL,
        1u);
}
#endif
