#include "stc8h_delay.h"
#include "stc8h_uart.h"
#include "stc8h_wdt.h"

void main(void)
{
    (void)stc8h_uart_init(STC8H_UART1);

    stc8h_uart_write_code(STC8H_UART1, "wdt reset test\r\n");
    if (stc8h_wdt_reset_flag() != 0u) {
        stc8h_uart_write_code(STC8H_UART1, "boot reason: wdt\r\n");
        stc8h_wdt_clear_reset_flag();
    } else {
        stc8h_uart_write_code(STC8H_UART1, "boot reason: power/reset\r\n");
    }

    stc8h_wdt_enable(STC8H_WDT_SCALE_64, 0u);

    stc8h_uart_write_code(STC8H_UART1, "feeding 3 times\r\n");
    stc8h_wdt_feed();
    stc8h_delay_ms(500u);
    stc8h_wdt_feed();
    stc8h_delay_ms(500u);
    stc8h_wdt_feed();
    stc8h_delay_ms(500u);

    stc8h_uart_write_code(STC8H_UART1, "stop feeding, wait reset\r\n");
    while (1) {
    }
}
