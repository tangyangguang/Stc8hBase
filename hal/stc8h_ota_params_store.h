#ifndef STC8H_OTA_PARAMS_STORE_H
#define STC8H_OTA_PARAMS_STORE_H

#include "stc8h_ota.h"

#ifndef STC8H_OTA_PARAMS_STORE_WORK_MEM
#define STC8H_OTA_PARAMS_STORE_WORK_MEM
#endif

#ifndef STC8H_OTA_PARAMS_STORE_ENABLE_MARK_APP_VALID
#define STC8H_OTA_PARAMS_STORE_ENABLE_MARK_APP_VALID 1
#endif

typedef stc8h_status_t (*stc8h_ota_param_erase_fn)(stc8h_u16 addr) STC8H_REENTRANT;
typedef stc8h_status_t (*stc8h_ota_param_write_fn)(stc8h_u16 addr,
                                                   const stc8h_u8 *data,
                                                   stc8h_u16 len) STC8H_REENTRANT;
typedef stc8h_status_t (*stc8h_ota_param_read_fn)(stc8h_u16 addr,
                                                  stc8h_u8 *data,
                                                  stc8h_u16 len) STC8H_REENTRANT;

struct stc8h_ota_params_store_s {
    stc8h_ota_param_erase_fn erase;
    stc8h_ota_param_write_fn write;
    stc8h_ota_param_read_fn read;
    stc8h_u16 active_addr;
    stc8h_u8 has_active;
};

void stc8h_ota_params_store_init(stc8h_ota_params_store_t *store,
                                 stc8h_ota_param_erase_fn erase,
                                 stc8h_ota_param_write_fn write,
                                 stc8h_ota_param_read_fn read);
stc8h_status_t stc8h_ota_params_store_load_active(stc8h_ota_params_store_t *store,
                                                  stc8h_ota_params_t *params);
stc8h_status_t stc8h_ota_params_store_write_next(stc8h_ota_params_store_t *store,
                                                 const stc8h_ota_params_t *params);
stc8h_status_t stc8h_ota_params_store_mark_boot_attempted(stc8h_ota_params_store_t *store);
#if STC8H_OTA_PARAMS_STORE_ENABLE_MARK_APP_VALID
stc8h_status_t stc8h_ota_params_store_mark_app_valid(stc8h_ota_params_store_t *store);
#endif

#endif
