#include "board_init.h"
#include "stc8h_gpio.h"
#include "stc8h_config.h"

void board_init(void)
{
    stc8h_gpio_set_mode(BOARD_LED_PORT, BOARD_LED_PIN, STC8H_GPIO_MODE_PUSH_PULL);
    stc8h_gpio_set_mode(BOARD_I2C_SDA_PORT, BOARD_I2C_SDA_PIN, STC8H_GPIO_MODE_OPEN_DRAIN);
    stc8h_gpio_set_mode(BOARD_I2C_SCL_PORT, BOARD_I2C_SCL_PIN, STC8H_GPIO_MODE_OPEN_DRAIN);
    BOARD_I2C_SDA_HIGH();
    BOARD_I2C_SCL_HIGH();
}
