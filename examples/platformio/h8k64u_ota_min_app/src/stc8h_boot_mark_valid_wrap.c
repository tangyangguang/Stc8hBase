#ifndef H8K64U_OTA_MIN_APP_ENABLE_MARK_VALID_IAP
#define H8K64U_OTA_MIN_APP_ENABLE_MARK_VALID_IAP 0
#endif

#if H8K64U_OTA_MIN_APP_ENABLE_MARK_VALID_IAP
#include "../../../../hal/stc8h_boot_control_iap.c"
#else
#include "stc8h_boot_jump.h"

stc8h_status_t stc8h_boot_mark_app_valid(void)
{
    /* Default build proves linking without reserving any OTA implementation. */
    return STC8H_OK;
}
#endif
