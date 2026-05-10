#ifndef DRV_LED_H
#define DRV_LED_H

#include "stc8h_config.h"

typedef struct {
    stc8h_u8 active_level;
} drv_led_t;

void drv_led_init(drv_led_t *led, stc8h_u8 active_level);
stc8h_u8 drv_led_level_on(const drv_led_t *led);
stc8h_u8 drv_led_level_off(const drv_led_t *led);

#endif
