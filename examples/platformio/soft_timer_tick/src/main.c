#include "board_init.h"
#include "stc8h_compiler.h"
#include "stc8h_gpio.h"
#include "stc8h_interrupt.h"
#include "stc8h_timer.h"
#include "stc8h_uart.h"
#include "util_soft_timer.h"

static volatile stc8h_u16 tick_ms;

static stc8h_u16 app_tick_now(void)
{
    stc8h_u16 now;

    stc8h_interrupt_disable_global();
    now = tick_ms;
    stc8h_interrupt_enable_global();

    return now;
}

STC8H_INTERRUPT(timer0_isr, STC8H_VECTOR_TIMER0)
{
    stc8h_timer_clear_flag(STC8H_TIMER0);
    ++tick_ms;
}

void main(void)
{
    util_soft_timer_t led_timer;
    util_soft_timer_t uart_timer;
    stc8h_u16 now;

    board_led_init();
    (void)stc8h_uart_init(STC8H_UART1);

    stc8h_uart_write_code(STC8H_UART1, "soft timer tick\r\n");

    if (stc8h_timer_init_1ms(STC8H_TIMER0) != STC8H_OK) {
        stc8h_uart_write_code(STC8H_UART1, "timer init error\r\n");
        while (1) {
        }
    }

    util_soft_timer_start(&led_timer, 0u, 250u);
    util_soft_timer_start(&uart_timer, 0u, 1000u);

    stc8h_timer_enable_interrupt(STC8H_TIMER0);
    stc8h_interrupt_enable_global();
    stc8h_timer_start(STC8H_TIMER0);

    while (1) {
        now = app_tick_now();

        if (util_soft_timer_expired(&led_timer, now) != 0u) {
            stc8h_gpio_toggle(BOARD_LED_PORT, BOARD_LED_PIN);
        }

        if (util_soft_timer_expired(&uart_timer, now) != 0u) {
            stc8h_uart_write_code(STC8H_UART1, "soft tick\r\n");
        }
    }
}
