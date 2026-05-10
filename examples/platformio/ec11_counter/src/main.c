#include "board_init.h"
#include "board_pins.h"
#include "drv_button.h"
#include "drv_ec11.h"
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

static void uart_put_s16(stc8h_s16 value)
{
    if (value < 0) {
        stc8h_uart_putc(STC8H_UART1, '-');
        value = (stc8h_s16)-value;
    }

    uart_put_u16((stc8h_u16)value);
}

static void print_count(stc8h_s16 delta, stc8h_s16 count)
{
    stc8h_uart_write_code(STC8H_UART1, "EC11 ");
    if (delta > 0) {
        stc8h_uart_write_code(STC8H_UART1, "+ ");
    } else {
        stc8h_uart_write_code(STC8H_UART1, "- ");
    }
    stc8h_uart_write_code(STC8H_UART1, "count=");
    uart_put_s16(count);
    stc8h_uart_write_code(STC8H_UART1, "\r\n");
}

static void print_button_event(drv_button_event_t event)
{
    if (event == DRV_BUTTON_EVENT_PRESS) {
        stc8h_uart_write_code(STC8H_UART1, "SW press\r\n");
    } else if (event == DRV_BUTTON_EVENT_SHORT) {
        stc8h_uart_write_code(STC8H_UART1, "SW short\r\n");
    } else if (event == DRV_BUTTON_EVENT_LONG) {
        stc8h_uart_write_code(STC8H_UART1, "SW long\r\n");
    } else if (event == DRV_BUTTON_EVENT_RELEASE) {
        stc8h_uart_write_code(STC8H_UART1, "SW release\r\n");
    }
}

void main(void)
{
    drv_ec11_t ec11;
    drv_button_t sw;
    stc8h_s16 count;
    stc8h_s16 delta;
    drv_button_event_t event;

    board_init();
    (void)stc8h_uart_init(STC8H_UART1);
    drv_ec11_init(&ec11);
    drv_ec11_set_fast(&ec11, 50u, 10);
    drv_ec11_set_reverse(&ec11, 1u);
    drv_button_init(&sw, 0u);
    count = 0;

    stc8h_uart_write_code(STC8H_UART1, "EC11 counter\r\n");
    stc8h_uart_write_code(STC8H_UART1, "A=P10 B=P11 SW=P54\r\n");

    while (1) {
        drv_ec11_scan(&ec11, BOARD_EC11_A_READ(), BOARD_EC11_B_READ(), 2u);
        delta = drv_ec11_get_delta(&ec11);
        if (delta != 0) {
            count = (stc8h_s16)(count + delta);
            print_count(delta, count);
        }

        drv_button_scan(&sw, BOARD_EC11_SW_READ(), 2u);
        event = drv_button_get_event(&sw);
        if (event != DRV_BUTTON_EVENT_NONE) {
            print_button_event(event);
        }

        stc8h_gpio_toggle(BOARD_LED_PORT, BOARD_LED_PIN);
        stc8h_delay_ms(2u);
    }
}
