#include "stc8h_delay.h"
#include "stc8h_uart.h"

void main(void)
{
    (void)stc8h_uart_init(STC8H_UART1);
    (void)stc8h_uart_init(STC8H_UART2);
    stc8h_uart_write_code(STC8H_UART1, "H8K64U UART2 smoke\r\n");
    while (1) {
        stc8h_uart_write_code(STC8H_UART2, "UART2 hello\r\n");
        stc8h_uart_write_code(STC8H_UART1, "UART2 sent\r\n");
        stc8h_delay_ms(1000u);
    }
}
