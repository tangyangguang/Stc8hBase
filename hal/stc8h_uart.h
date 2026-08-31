#ifndef STC8H_UART_H
#define STC8H_UART_H

#include "stc8h_config.h"

#ifndef STC8H_UART_ENABLE_WRITE_RAM
#define STC8H_UART_ENABLE_WRITE_RAM 1
#endif

#ifndef STC8H_UART_ENABLE_WRITE_CODE
#define STC8H_UART_ENABLE_WRITE_CODE 1
#endif

#ifndef STC8H_UART_ENABLE_ISR_API
#define STC8H_UART_ENABLE_ISR_API 0
#endif

#ifndef STC8H_UART_ENABLE_BOUNDED_PUTC
#define STC8H_UART_ENABLE_BOUNDED_PUTC 0
#endif

typedef enum {
    STC8H_UART1 = 0,
    STC8H_UART2,
    STC8H_UART3
} stc8h_uart_id_t;

stc8h_status_t stc8h_uart_init(stc8h_uart_id_t uart);
void stc8h_uart_putc(stc8h_uart_id_t uart, char ch);
#if STC8H_UART_ENABLE_BOUNDED_PUTC
stc8h_status_t stc8h_uart_putc_bounded(stc8h_uart_id_t uart, char ch, stc8h_u16 poll_limit);
#endif
#if STC8H_UART_ENABLE_WRITE_RAM
void stc8h_uart_write(stc8h_uart_id_t uart, const char *data);
#endif
#if STC8H_UART_ENABLE_WRITE_CODE
void stc8h_uart_write_code(stc8h_uart_id_t uart, const STC8H_CODE char *data);
#endif
stc8h_u8 stc8h_uart_readable(stc8h_uart_id_t uart);
char stc8h_uart_getc(stc8h_uart_id_t uart);

#if STC8H_UART_ENABLE_ISR_API
stc8h_status_t stc8h_uart_interrupt_enable(stc8h_uart_id_t uart);
stc8h_status_t stc8h_uart_interrupt_disable(stc8h_uart_id_t uart);
stc8h_u8 stc8h_uart_try_getc(stc8h_uart_id_t uart, stc8h_u8 *value);
void stc8h_uart_clear_tx_flag(stc8h_uart_id_t uart);
#endif

#endif
