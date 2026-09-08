#ifndef STC8H_BOOT_STUB_H
#define STC8H_BOOT_STUB_H

#include "stc8h_config.h"

/* STC8H8K64U production Remote OTA layout.
 * The code/EEPROM split is fixed at 0x6C00 so the complete bootloader remains
 * outside the IAP-writable suffix of flash. */
#define STC8H_BOOTLOADER_BASE 0x0000u
#define STC8H_BOOTLOADER_LIMIT 0x6BFFu
#define STC8H_BOOT_APP_BASE 0x6C00u
#define STC8H_BOOT_APP_LIMIT 0xEFFFu
#define STC8H_BOOT_DATA_BASE 0xF000u
#define STC8H_BOOT_DATA_LIMIT 0xFBFFu
#define STC8H_BOOT_PARAM_A_BASE 0xFC00u
#define STC8H_BOOT_PARAM_B_BASE 0xFE00u
#define STC8H_BOOT_FLASH_LIMIT 0xFFFFu

#endif
