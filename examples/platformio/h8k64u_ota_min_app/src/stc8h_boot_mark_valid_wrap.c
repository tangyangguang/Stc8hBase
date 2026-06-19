#include "stc8h_boot_jump.h"

#ifndef H8K64U_OTA_MIN_APP_ENABLE_MARK_VALID_IAP
#define H8K64U_OTA_MIN_APP_ENABLE_MARK_VALID_IAP 0
#endif

#if H8K64U_OTA_MIN_APP_ENABLE_MARK_VALID_IAP
#include "stc8h_iap_ota_params.h"
#include "stc8h_ota_params_store.h"
#endif

stc8h_status_t stc8h_boot_mark_app_valid(void)
{
#if H8K64U_OTA_MIN_APP_ENABLE_MARK_VALID_IAP
    static STC8H_XDATA stc8h_ota_params_store_t store;

    stc8h_ota_params_store_init(&store,
                                stc8h_iap_ota_params_erase,
                                stc8h_iap_ota_params_write,
                                stc8h_iap_ota_params_read);
    return stc8h_ota_params_store_mark_app_valid(&store);
#else
    /* Default build verifies the call site without writing OTA parameter Flash. */
    return STC8H_OK;
#endif
}
