#ifndef STC8H_GPIO_H
#define STC8H_GPIO_H

#include "stc8h_config.h"

typedef enum {
    STC8H_GPIO_MODE_QUASI = 0,
    STC8H_GPIO_MODE_PUSH_PULL,
    STC8H_GPIO_MODE_INPUT_ONLY,
    STC8H_GPIO_MODE_OPEN_DRAIN
} stc8h_gpio_mode_t;

void stc8h_gpio_set_mode(stc8h_u8 port, stc8h_u8 pin, stc8h_gpio_mode_t mode);
void stc8h_gpio_write(stc8h_u8 port, stc8h_u8 pin, stc8h_u8 value);
stc8h_u8 stc8h_gpio_read(stc8h_u8 port, stc8h_u8 pin);
void stc8h_gpio_toggle(stc8h_u8 port, stc8h_u8 pin);

#endif
