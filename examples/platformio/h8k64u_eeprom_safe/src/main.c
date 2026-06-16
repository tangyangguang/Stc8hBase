#include "stc8h_uart.h"

void main(void)
{
    (void)stc8h_uart_init(STC8H_UART1);
    stc8h_uart_write_code(STC8H_UART1, "H8K64U EEPROM write disabled\r\n");
    while (1) {
    }
}
