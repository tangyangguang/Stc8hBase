#include "stc8h_delay.h"
#include "stc8h_gpio.h"
#include "stc8h_sfr.h"
#include "stc8h_uart.h"

#define DELAY_PROBE_MASK 0x01u

static void probe_high_us(stc8h_u16 us)
{
    P1 |= DELAY_PROBE_MASK;
    stc8h_delay_timer0_1t_us(us);
    P1 &= (stc8h_u8)~DELAY_PROBE_MASK;
    stc8h_delay_timer0_1t_us(2000u);
}

void main(void)
{
    (void)stc8h_uart_init(STC8H_UART1);
    stc8h_gpio_set_mode(1u, 0u, STC8H_GPIO_MODE_PUSH_PULL);
    P1 &= (stc8h_u8)~DELAY_PROBE_MASK;
    (void)stc8h_delay_timer0_1t_init();

    stc8h_uart_write_code(STC8H_UART1, "delay us probe P10\r\n");

    while (1) {
        probe_high_us(562u);
        probe_high_us(1687u);
        probe_high_us(2250u);
        probe_high_us(4500u);
        probe_high_us(9000u);
        stc8h_delay_ms(1000u);
    }
}
