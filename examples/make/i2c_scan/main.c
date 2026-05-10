#include "board_init.h"
#include "stc8h_delay.h"
#include "stc8h_gpio.h"
#include "stc8h_i2c.h"
#include "stc8h_uart.h"

static void uart_put_hex4(stc8h_u8 value)
{
    value &= 0x0Fu;
    if (value < 10u) {
        stc8h_uart_putc(STC8H_UART1, (char)('0' + value));
    } else {
        stc8h_uart_putc(STC8H_UART1, (char)('A' + (value - 10u)));
    }
}

static void uart_put_hex8(stc8h_u8 value)
{
    stc8h_uart_write_code(STC8H_UART1, "0x");
    uart_put_hex4((stc8h_u8)(value >> 4));
    uart_put_hex4(value);
}

void main(void)
{
    stc8h_u8 addr7;
    stc8h_u8 found;

    board_init();
    stc8h_i2c_init();
    (void)stc8h_uart_init(STC8H_UART1);

    stc8h_uart_write_code(STC8H_UART1, "I2C scan\r\n");
    found = 0u;

    for (addr7 = 0x08u; addr7 <= 0x77u; ++addr7) {
        if (stc8h_i2c_probe(addr7) != 0u) {
            uart_put_hex8(addr7);
            stc8h_uart_write_code(STC8H_UART1, "\r\n");
            found = 1u;
        }
    }

    if (found == 0u) {
        stc8h_uart_write_code(STC8H_UART1, "none\r\n");
    }

    while (1) {
        stc8h_gpio_toggle(BOARD_LED_PORT, BOARD_LED_PIN);
        stc8h_delay_ms(500u);
    }
}
