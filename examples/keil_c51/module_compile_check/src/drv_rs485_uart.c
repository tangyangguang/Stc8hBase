#define STC8H_CONFIG_INCLUDE "board_config.h"
#define STC8H_PINS_INCLUDE "board_pins.h"
#define STC8H_UART_ENABLE_BOUNDED_PUTC 1
#define DRV_RS485_UART_TX_COMPLETE_DELAY_US 150u
#define BOARD_RS485_TX_ENABLE() do { } while (0)
#define BOARD_RS485_RX_ENABLE() do { } while (0)
#include "../../../../drivers/drv_rs485_uart.c"
