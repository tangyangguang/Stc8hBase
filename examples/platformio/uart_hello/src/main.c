#include "stc8h_delay.h"
#include "stc8h_uart.h"

void main(void)
{
    (void)stc8h_uart_init(STC8H_UART1);

    while (1) {
        stc8h_uart_write_code(STC8H_UART1, "UART hello 115200\r\n");
        stc8h_delay_ms(1000u);
    }
}
