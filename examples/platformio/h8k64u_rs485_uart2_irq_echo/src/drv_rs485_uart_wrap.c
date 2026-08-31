#include "stc8h_sfr.h"

#define BOARD_RS485_DIR_MASK 0x10u
#define BOARD_RS485_TX_ENABLE() do { P4 |= BOARD_RS485_DIR_MASK; } while (0)
#define BOARD_RS485_RX_ENABLE() do { P4 &= (stc8h_u8)~BOARD_RS485_DIR_MASK; } while (0)

#include "../../../../drivers/drv_rs485_uart.c"
