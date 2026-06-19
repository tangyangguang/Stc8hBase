#include "stc8h_config.h"

#define H8K64U_OTA_BOOTLOADER_BASE 0xB800u

__code __at (0x0000) const stc8h_u8 h8k64u_ota_reset_stub[3] = {
    0x02u,
    (stc8h_u8)(H8K64U_OTA_BOOTLOADER_BASE >> 8),
    (stc8h_u8)H8K64U_OTA_BOOTLOADER_BASE
};
