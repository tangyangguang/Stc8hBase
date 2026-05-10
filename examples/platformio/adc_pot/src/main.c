#include "board_init.h"
#include "board_config.h"
#include "stc8h_adc.h"
#include "stc8h_delay.h"
#include "stc8h_gpio.h"
#include "stc8h_uart.h"

static void uart_put_u16(stc8h_u16 value)
{
    const stc8h_u16 divs[5] = {10000u, 1000u, 100u, 10u, 1u};
    stc8h_u8 started;
    stc8h_u8 i;
    stc8h_u8 digit;

    started = 0u;
    for (i = 0u; i < 5u; ++i) {
        digit = 0u;
        while (value >= divs[i]) {
            value = (stc8h_u16)(value - divs[i]);
            ++digit;
        }
        if ((digit != 0u) || (started != 0u) || (i == 4u)) {
            started = 1u;
            stc8h_uart_putc(STC8H_UART1, (char)('0' + digit));
        }
    }
}

void main(void)
{
    stc8h_u16 value;

    board_init();
    (void)stc8h_uart_init(STC8H_UART1);
    stc8h_adc_init();

    stc8h_uart_write_code(STC8H_UART1, "ADC pot P33 ADC11\r\n");

    while (1) {
        value = stc8h_adc_read(BOARD_POT_ADC_CHANNEL);
        stc8h_uart_write_code(STC8H_UART1, "adc=");
        uart_put_u16(value);
        stc8h_uart_write_code(STC8H_UART1, "\r\n");

        stc8h_gpio_toggle(BOARD_LED_PORT, BOARD_LED_PIN);
        stc8h_delay_ms(300u);
    }
}
