#ifndef DRV_RS485_UART_H
#define DRV_RS485_UART_H

#include "stc8h_uart.h"

#ifndef DRV_RS485_UART_ENABLE_READ
#define DRV_RS485_UART_ENABLE_READ 1
#endif

#ifndef DRV_RS485_UART_ENABLE_WRITE
#define DRV_RS485_UART_ENABLE_WRITE 1
#endif

#ifndef DRV_RS485_UART_ENABLE_RX_INTERRUPT
#define DRV_RS485_UART_ENABLE_RX_INTERRUPT 0
#endif

#ifndef DRV_RS485_UART_TX_POLL_LIMIT
#define DRV_RS485_UART_TX_POLL_LIMIT 65535u
#endif

#ifndef DRV_RS485_UART_TX_COMPLETE_DELAY_US
#define DRV_RS485_UART_TX_COMPLETE_DELAY_US 0u
#endif

#if DRV_RS485_UART_ENABLE_WRITE && !STC8H_UART_ENABLE_BOUNDED_PUTC
#error "RS485 write requires STC8H_UART_ENABLE_BOUNDED_PUTC=1."
#endif

#if DRV_RS485_UART_ENABLE_RX_INTERRUPT && !STC8H_UART_ENABLE_ISR_API
#error "RS485 RX interrupt requires STC8H_UART_ENABLE_ISR_API=1."
#endif

#if DRV_RS485_UART_ENABLE_WRITE && \
    ((DRV_RS485_UART_TX_POLL_LIMIT == 0u) || (DRV_RS485_UART_TX_POLL_LIMIT > 65535UL))
#error "DRV_RS485_UART_TX_POLL_LIMIT must fit a non-zero stc8h_u16."
#endif

#if DRV_RS485_UART_ENABLE_WRITE && \
    ((DRV_RS485_UART_TX_COMPLETE_DELAY_US == 0u) || \
     (DRV_RS485_UART_TX_COMPLETE_DELAY_US > 65535UL))
#error "DRV_RS485_UART_TX_COMPLETE_DELAY_US must fit stc8h_u16 and cover the final stop bit."
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
