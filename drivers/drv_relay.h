#ifndef DRV_RELAY_H
#define DRV_RELAY_H

#include "stc8h_config.h"

typedef struct {
    stc8h_u8 active_level;
} drv_relay_t;

void drv_relay_init(drv_relay_t *relay, stc8h_u8 active_level);
stc8h_u8 drv_relay_level_on(const drv_relay_t *relay);
stc8h_u8 drv_relay_level_off(const drv_relay_t *relay);

#endif
