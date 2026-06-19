#if H8K64U_OTA_MIN_APP_ENABLE_MARK_VALID_IAP
#define STC8H_OTA_PARAMS_STORE_WORK_MEM static STC8H_XDATA
#include "../../../../hal/stc8h_ota_params_store.c"
#else
typedef unsigned char h8k64u_ota_min_app_no_ota_params_store_t;
#endif
