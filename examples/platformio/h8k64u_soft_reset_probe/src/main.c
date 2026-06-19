#include "stc8h_delay.h"
#include "stc8h_sfr.h"
#include "stc8h_uart.h"

#define H8K64U_SOFT_RESET_SWRST 0x20u

void main(void)
{
    (void)stc8h_uart_init(STC8H_UART1);
    stc8h_uart_write_code(STC8H_UART1, "H8K64U soft reset probe\r\n");
    stc8h_delay_ms(1000u);
    stc8h_uart_write_code(STC8H_UART1, "software reset now\r\n");
    stc8h_delay_ms(50u);

    EA = 0u;
    IAP_CONTR = H8K64U_SOFT_RESET_SWRST;

    while (1) {
    }
}
