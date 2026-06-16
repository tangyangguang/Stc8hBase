#include "stc8h_uart.h"

void main(void)
{
    (void)stc8h_uart_init(STC8H_UART2);
    while (1) {
        stc8h_uart_write_code(STC8H_UART2, "UART2 hello\r\n");
    }
}
