#include "board_pins.h"
#include "stc8h_delay.h"
#include "stc8h_gpio.h"
#include "stc8h_uart.h"

static void print_gpio_state(void)
{
    stc8h_uart_write_code(STC8H_UART1, "GPIO P1.3=");
    stc8h_uart_putc(STC8H_UART1, stc8h_gpio_read(BOARD_TEST_GPIO_PORT, BOARD_TEST_GPIO_PIN) ? '1' : '0');
    stc8h_uart_write_code(STC8H_UART1, "\r\n");
}

void main(void)
{
    (void)stc8h_uart_init(STC8H_UART1);
    stc8h_gpio_set_mode(BOARD_TEST_GPIO_PORT, BOARD_TEST_GPIO_PIN, STC8H_GPIO_MODE_PUSH_PULL);
    stc8h_gpio_write(BOARD_TEST_GPIO_PORT, BOARD_TEST_GPIO_PIN, 0u);
    stc8h_uart_write_code(STC8H_UART1, "H8K64U GPIO blink P1.3\r\n");
    print_gpio_state();

    while (1) {
        stc8h_gpio_toggle(BOARD_TEST_GPIO_PORT, BOARD_TEST_GPIO_PIN);
        print_gpio_state();
        stc8h_delay_ms(250u);
    }
}
