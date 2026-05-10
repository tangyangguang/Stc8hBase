#include "drv_relay.h"

void drv_relay_init(drv_relay_t *relay, stc8h_u8 active_level)
{
    if (relay == 0) {
        return;
    }

    relay->active_level = active_level ? 1u : 0u;
}

stc8h_u8 drv_relay_level_on(const drv_relay_t *relay)
{
    if (relay == 0) {
        return 1u;
    }

    return relay->active_level;
}

stc8h_u8 drv_relay_level_off(const drv_relay_t *relay)
{
    if (relay == 0) {
        return 0u;
    }

    return relay->active_level ? 0u : 1u;
}
