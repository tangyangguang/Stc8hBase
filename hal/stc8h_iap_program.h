#ifndef STC8H_IAP_PROGRAM_H
#define STC8H_IAP_PROGRAM_H

#include "stc8h_config.h"

#ifndef STC8H_IAP_PROGRAM_ENABLE
#define STC8H_IAP_PROGRAM_ENABLE 0
#endif

#ifndef STC8H_IAP_PROGRAM_APP_BASE
#define STC8H_IAP_PROGRAM_APP_BASE 0x0200u
#endif

#ifndef STC8H_IAP_PROGRAM_APP_LIMIT
#define STC8H_IAP_PROGRAM_APP_LIMIT 0xEFFFu
#endif

#ifndef STC8H_IAP_PROGRAM_SECTOR_SIZE
#define STC8H_IAP_PROGRAM_SECTOR_SIZE 512u
#endif

#if STC8H_IAP_PROGRAM_ENABLE

#if !STC8H_CHIP_STC8H8K64U
#error "STC8H IAP program backend supports only STC8H8K64U."
#endif

#if STC8H_IAP_PROGRAM_APP_BASE < 0x0200u
#error "STC8H_IAP_PROGRAM_APP_BASE must not overlap the boot stub."
#endif

#if STC8H_IAP_PROGRAM_APP_LIMIT >= 0xF000u
#error "STC8H_IAP_PROGRAM_APP_LIMIT must not overlap bootloader or parameter records."
#endif

#if STC8H_IAP_PROGRAM_APP_BASE > STC8H_IAP_PROGRAM_APP_LIMIT
#error "STC8H_IAP_PROGRAM_APP_BASE must be <= STC8H_IAP_PROGRAM_APP_LIMIT."
#endif

#if STC8H_IAP_PROGRAM_SECTOR_SIZE != 512u
#error "STC8H IAP program backend currently assumes 512-byte erase sectors."
#endif

stc8h_status_t stc8h_iap_program_erase_sector(stc8h_u16 addr);
stc8h_status_t stc8h_iap_program_write(stc8h_u16 addr,
                                       const stc8h_u8 *data,
                                       stc8h_u16 len);
stc8h_status_t stc8h_iap_program_read(stc8h_u16 addr,
                                      stc8h_u8 *data,
                                      stc8h_u16 len);

#endif

#endif
