#include "board_pins.h"
#include "stc8h_delay.h"
#include "stc8h_gpio.h"

static void blink_times(stc8h_u8 count, stc8h_u16 delay_ms)
{
    stc8h_u8 i;

    for (i = 0u; i < count; ++i) {
        stc8h_gpio_write(BOARD_LED_PORT, BOARD_LED_PIN, 1u);
        stc8h_delay_ms(delay_ms);
        stc8h_gpio_write(BOARD_LED_PORT, BOARD_LED_PIN, 0u);
        stc8h_delay_ms(delay_ms);
    }
}

void main(void)
{
    stc8h_gpio_set_mode(BOARD_LED_PORT, BOARD_LED_PIN, STC8H_GPIO_MODE_PUSH_PULL);
    stc8h_gpio_write(BOARD_LED_PORT, BOARD_LED_PIN, 0u);

    while (1) {
        blink_times(8u, 100u);
        stc8h_delay_ms(800u);
        blink_times(3u, 500u);
        stc8h_delay_ms(1200u);
    }
}
