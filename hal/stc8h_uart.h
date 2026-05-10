#ifndef STC8H_UART_H
#define STC8H_UART_H

#include "stc8h_config.h"

typedef enum {
    STC8H_UART1 = 0,
    STC8H_UART2
} stc8h_uart_id_t;

stc8h_status_t stc8h_uart_init(stc8h_uart_id_t uart);
void stc8h_uart_putc(stc8h_uart_id_t uart, char ch);
void stc8h_uart_write(stc8h_uart_id_t uart, const char *data);
void stc8h_uart_write_code(stc8h_uart_id_t uart, const STC8H_CODE char *data);
stc8h_u8 stc8h_uart_readable(stc8h_uart_id_t uart);
char stc8h_uart_getc(stc8h_uart_id_t uart);

#endif
