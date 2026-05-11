#include "stc8h_delay.h"
#include "stc8h_uart.h"
#include "stc8h_wdt.h"

void main(void)
{
    (void)stc8h_uart_init(STC8H_UART1);
    stc8h_uart_write_code(STC8H_UART1, "wdt feed\r\n");

    stc8h_wdt_enable(STC8H_WDT_SCALE_256, 0u);

    while (1) {
        stc8h_wdt_feed();
        stc8h_uart_write_code(STC8H_UART1, "wdt ok\r\n");
        stc8h_delay_ms(500u);
    }
}
