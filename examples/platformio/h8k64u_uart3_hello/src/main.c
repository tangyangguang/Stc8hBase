#include "stc8h_delay.h"
#include "stc8h_uart.h"

void main(void)
{
    (void)stc8h_uart_init(STC8H_UART1);
    (void)stc8h_uart_init(STC8H_UART3);
    stc8h_uart_write_code(STC8H_UART1, "H8K64U UART3 smoke\r\n");
    while (1) {
        stc8h_uart_write_code(STC8H_UART3, "UART3 hello\r\n");
        stc8h_uart_write_code(STC8H_UART1, "UART3 sent\r\n");
        stc8h_delay_ms(1000u);
    }
}
