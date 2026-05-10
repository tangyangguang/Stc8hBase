#include "drv_buzzer.h"

void drv_buzzer_init(drv_buzzer_t *buzzer, stc8h_u8 active_level)
{
    if (buzzer == 0) {
        return;
    }

    buzzer->active_level = active_level ? 1u : 0u;
}

stc8h_u8 drv_buzzer_level_on(const drv_buzzer_t *buzzer)
{
    if (buzzer == 0) {
        return 1u;
    }

    return buzzer->active_level;
}

stc8h_u8 drv_buzzer_level_off(const drv_buzzer_t *buzzer)
{
    if (buzzer == 0) {
        return 0u;
    }

    return buzzer->active_level ? 0u : 1u;
}
