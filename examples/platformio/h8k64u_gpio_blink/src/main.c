#include "board_pins.h"
#include "stc8h_delay.h"
#include "stc8h_gpio.h"

void main(void)
{
    stc8h_gpio_set_mode(BOARD_TEST_GPIO_PORT, BOARD_TEST_GPIO_PIN, STC8H_GPIO_MODE_PUSH_PULL);
    stc8h_gpio_write(BOARD_TEST_GPIO_PORT, BOARD_TEST_GPIO_PIN, 0u);

    while (1) {
        stc8h_gpio_toggle(BOARD_TEST_GPIO_PORT, BOARD_TEST_GPIO_PIN);
        stc8h_delay_ms(250u);
    }
}
