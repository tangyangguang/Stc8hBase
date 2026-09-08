#ifndef H8K64U_RS485_OTA_BOOT_CONFIG_H
#define H8K64U_RS485_OTA_BOOT_CONFIG_H

#include "board_config.h"

#define STC8H_OTA_WORK_MEM static STC8H_XDATA
#define STC8H_OTA_PARAMS_STORE_WORK_MEM static STC8H_XDATA
#define STC8H_OTA_RECEIVER_WORK_MEM static STC8H_XDATA
#define STC8H_OTA_EXPECTED_BOARD_ID 0u
#define STC8H_OTA_EXPECTED_HW_REVISION 0u
#define STC8H_OTA_EXPECTED_APP_ID 0u

/* Lab core board has no controlled outputs. Product BSPs must replace this
 * with a board-safe, allocation-free, interrupt-free shutdown hook. */
#define H8K64U_OTA_SAFE_OUTPUTS_OFF() do { } while (0)

#endif
