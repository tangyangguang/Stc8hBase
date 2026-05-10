#ifndef DRV_BUZZER_H
#define DRV_BUZZER_H

#include "stc8h_config.h"

typedef struct {
    stc8h_u8 active_level;
} drv_buzzer_t;

void drv_buzzer_init(drv_buzzer_t *buzzer, stc8h_u8 active_level);
stc8h_u8 drv_buzzer_level_on(const drv_buzzer_t *buzzer);
stc8h_u8 drv_buzzer_level_off(const drv_buzzer_t *buzzer);

#endif
