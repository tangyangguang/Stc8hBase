#include "drv_buzzer.h"
#include "drv_led.h"
#include "drv_relay.h"
#include "stc8h_uart.h"

static void print_result(stc8h_u8 ok)
{
    if (ok != 0u) {
        stc8h_uart_write_code(STC8H_UART1, "output levels ok\r\n");
    } else {
        stc8h_uart_write_code(STC8H_UART1, "output levels error\r\n");
    }
}

void main(void)
{
    drv_led_t led_high;
    drv_led_t led_low;
    drv_buzzer_t buzzer_high;
    drv_relay_t relay_low;
    stc8h_u8 ok;

    (void)stc8h_uart_init(STC8H_UART1);

    drv_led_init(&led_high, 1u);
    drv_led_init(&led_low, 0u);
    drv_buzzer_init(&buzzer_high, 1u);
    drv_relay_init(&relay_low, 0u);

    ok = 1u;
    if ((drv_led_level_on(&led_high) != 1u) || (drv_led_level_off(&led_high) != 0u)) {
        ok = 0u;
    }
    if ((drv_led_level_on(&led_low) != 0u) || (drv_led_level_off(&led_low) != 1u)) {
        ok = 0u;
    }
    if ((drv_buzzer_level_on(&buzzer_high) != 1u) || (drv_buzzer_level_off(&buzzer_high) != 0u)) {
        ok = 0u;
    }
    if ((drv_relay_level_on(&relay_low) != 0u) || (drv_relay_level_off(&relay_low) != 1u)) {
        ok = 0u;
    }

    while (1) {
        print_result(ok);
    }
}
