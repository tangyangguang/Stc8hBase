#include "drv_led.h"

void drv_led_init(drv_led_t *led, stc8h_u8 active_level)
{
    if (led == 0) {
        return;
    }

    led->active_level = active_level ? 1u : 0u;
}

stc8h_u8 drv_led_level_on(const drv_led_t *led)
{
    if (led == 0) {
        return 1u;
    }

    return led->active_level;
}

stc8h_u8 drv_led_level_off(const drv_led_t *led)
{
    if (led == 0) {
        return 0u;
    }

    return led->active_level ? 0u : 1u;
}
