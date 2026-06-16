#include "stc8h_uart.h"

void main(void)
{
    (void)stc8h_uart_init(STC8H_UART3);
    while (1) {
        stc8h_uart_write_code(STC8H_UART3, "UART3 hello\r\n");
    }
}
