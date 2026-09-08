#include "stc8h_boot_control_iap.h"

#include "stc8h_iap_ota_params.h"
#include "stc8h_ota_params_store.h"
#include "stc8h_sfr.h"

#define STC8H_BOOT_CONTROL_SWRST 0x20u

static STC8H_XDATA stc8h_ota_params_store_t stc8h_boot_control_store;

static void stc8h_boot_control_init_store(void)
{
    stc8h_ota_params_store_init(&stc8h_boot_control_store,
                                stc8h_iap_ota_params_erase,
                                stc8h_iap_ota_params_write,
                                stc8h_iap_ota_params_read);
}

stc8h_status_t stc8h_boot_request_update(stc8h_u32 session_id)
{
    stc8h_boot_control_init_store();
    return stc8h_ota_params_store_request_update(&stc8h_boot_control_store,
                                                  session_id);
}

stc8h_status_t stc8h_boot_cancel_update_request(stc8h_u32 session_id)
{
    stc8h_boot_control_init_store();
    return stc8h_ota_params_store_cancel_update_request(
        &stc8h_boot_control_store, session_id);
}

stc8h_status_t stc8h_boot_mark_app_valid(void)
{
    stc8h_boot_control_init_store();
    return stc8h_ota_params_store_mark_app_valid(&stc8h_boot_control_store);
}

void stc8h_boot_controlled_reset(void)
{
    EA = 0u;
    IAP_CONTR = STC8H_BOOT_CONTROL_SWRST;
    while (1) {
    }
}
