#include "stc8h_delay.h"
#include "stc8h_gpio.h"
#include "stc8h_sfr.h"
#include "stc8h_uart.h"

#define DELAY_PROBE_MASK 0x01u

#ifndef DELAY_PROBE_HIGH_US
#define DELAY_PROBE_HIGH_US 562u
#endif

#ifndef DELAY_PROBE_PERIOD_US
#define DELAY_PROBE_PERIOD_US 20000u
#endif

#if DELAY_PROBE_HIGH_US >= DELAY_PROBE_PERIOD_US
#error "DELAY_PROBE_HIGH_US must be less than DELAY_PROBE_PERIOD_US."
#endif

static void probe_once(void)
{
    P1 |= DELAY_PROBE_MASK;
    stc8h_delay_timer0_1t_us(DELAY_PROBE_HIGH_US);
    P1 &= (stc8h_u8)~DELAY_PROBE_MASK;
    stc8h_delay_timer0_1t_us((stc8h_u16)(DELAY_PROBE_PERIOD_US - DELAY_PROBE_HIGH_US));
}

void main(void)
{
    (void)stc8h_uart_init(STC8H_UART1);
    stc8h_gpio_set_mode(1u, 0u, STC8H_GPIO_MODE_PUSH_PULL);
    P1 &= (stc8h_u8)~DELAY_PROBE_MASK;
    (void)stc8h_delay_timer0_1t_init();

    stc8h_uart_write_code(STC8H_UART1, "delay us probe P10\r\n");

    while (1) {
        probe_once();
    }
}
