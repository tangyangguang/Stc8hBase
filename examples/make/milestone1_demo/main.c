#include "board_init.h"
#include "stc8h_delay.h"
#include "stc8h_gpio.h"
#include "stc8h_uart.h"
#include "drv_lcd1602.h"

void main(void)
{
    board_led_init();
    board_i2c_init();
    (void)stc8h_uart_init(STC8H_UART1);
    stc8h_uart_write_code(STC8H_UART1, "Stc8hBase\r\n");

    drv_lcd1602_init(BOARD_LCD1602_ADDR7);
    drv_lcd1602_set_cursor(0u, 0u);
    drv_lcd1602_write_string_code("Stc8hBase");
    drv_lcd1602_set_cursor(1u, 0u);
    drv_lcd1602_write_string_code("Milestone 1");

    while (1) {
        stc8h_gpio_toggle(BOARD_LED_PORT, BOARD_LED_PIN);
        stc8h_delay_ms(500u);
    }
}
