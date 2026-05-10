#include "board_init.h"
#include "stc8h_compiler.h"
#include "stc8h_gpio.h"
#include "stc8h_interrupt.h"
#include "stc8h_timer.h"
#include "stc8h_uart.h"

static volatile stc8h_u16 tick_count_ms;
static volatile stc8h_u8 flag_500ms;
static volatile stc8h_u8 flag_1000ms;

STC8H_INTERRUPT(timer0_isr, STC8H_VECTOR_TIMER0)
{
    stc8h_timer_clear_flag(STC8H_TIMER0);

    ++tick_count_ms;
    if (tick_count_ms == 500u) {
        flag_500ms = 1u;
    } else if (tick_count_ms >= 1000u) {
        tick_count_ms = 0u;
        flag_500ms = 1u;
        flag_1000ms = 1u;
    }
}

void main(void)
{
    board_led_init();
    (void)stc8h_uart_init(STC8H_UART1);

    stc8h_uart_write_code(STC8H_UART1, "Timer0 1ms tick\r\n");

    if (stc8h_timer_init_1ms(STC8H_TIMER0) != STC8H_OK) {
        stc8h_uart_write_code(STC8H_UART1, "timer init error\r\n");
        while (1) {
        }
    }

    stc8h_timer_enable_interrupt(STC8H_TIMER0);
    stc8h_interrupt_enable_global();
    stc8h_timer_start(STC8H_TIMER0);

    while (1) {
        if (flag_500ms != 0u) {
            flag_500ms = 0u;
            stc8h_gpio_toggle(BOARD_LED_PORT, BOARD_LED_PIN);
        }

        if (flag_1000ms != 0u) {
            flag_1000ms = 0u;
            stc8h_uart_write_code(STC8H_UART1, "tick\r\n");
        }
    }
}
