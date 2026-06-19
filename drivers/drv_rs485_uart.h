#ifndef DRV_RS485_UART_H
#define DRV_RS485_UART_H

#include "stc8h_uart.h"

#ifndef DRV_RS485_UART_ENABLE_READ
#define DRV_RS485_UART_ENABLE_READ 1
#endif

#ifndef DRV_RS485_UART_ENABLE_WRITE
#define DRV_RS485_UART_ENABLE_WRITE 1
#endif

#ifndef DRV_RS485_UART_TURNAROUND_DELAY_US
#define DRV_RS485_UART_TURNAROUND_DELAY_US 0u
#endif

void drv_rs485_uart_enter_tx(void);
void drv_rs485_uart_enter_rx(void);
stc8h_status_t drv_rs485_uart_init(stc8h_uart_id_t uart);

#if DRV_RS485_UART_ENABLE_WRITE
stc8h_status_t drv_rs485_uart_write(stc8h_uart_id_t uart, const stc8h_u8 *data, stc8h_u16 len);
#endif

#if DRV_RS485_UART_ENABLE_READ
stc8h_u8 drv_rs485_uart_readable(stc8h_uart_id_t uart);
stc8h_u8 drv_rs485_uart_getc(stc8h_uart_id_t uart);
#endif

#endif
