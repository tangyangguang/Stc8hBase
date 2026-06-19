#ifndef STC8H_IAP_OTA_PARAMS_H
#define STC8H_IAP_OTA_PARAMS_H

#include "stc8h_config.h"

#ifndef STC8H_IAP_OTA_PARAMS_ENABLE
#define STC8H_IAP_OTA_PARAMS_ENABLE 0
#endif

#ifndef STC8H_IAP_OTA_PARAM_A_BASE
#define STC8H_IAP_OTA_PARAM_A_BASE 0xFC00u
#endif

#ifndef STC8H_IAP_OTA_PARAM_B_BASE
#define STC8H_IAP_OTA_PARAM_B_BASE 0xFE00u
#endif

#ifndef STC8H_IAP_OTA_PARAM_SECTOR_SIZE
#define STC8H_IAP_OTA_PARAM_SECTOR_SIZE 512u
#endif

#if STC8H_IAP_OTA_PARAMS_ENABLE

#if !STC8H_CHIP_STC8H8K64U
#error "STC8H OTA parameter IAP backend supports only STC8H8K64U."
#endif

#if STC8H_IAP_OTA_PARAM_A_BASE != 0xFC00u
#error "STC8H_IAP_OTA_PARAM_A_BASE must stay at 0xFC00."
#endif

#if STC8H_IAP_OTA_PARAM_B_BASE != 0xFE00u
#error "STC8H_IAP_OTA_PARAM_B_BASE must stay at 0xFE00."
#endif

#if STC8H_IAP_OTA_PARAM_SECTOR_SIZE != 512u
#error "STC8H OTA parameter IAP backend currently assumes 512-byte erase sectors."
#endif

stc8h_status_t stc8h_iap_ota_params_erase(stc8h_u16 addr);
stc8h_status_t stc8h_iap_ota_params_write(stc8h_u16 addr,
                                          const stc8h_u8 *data,
                                          stc8h_u16 len);
stc8h_status_t stc8h_iap_ota_params_read(stc8h_u16 addr,
                                         stc8h_u8 *data,
                                         stc8h_u16 len);

#endif

#endif
