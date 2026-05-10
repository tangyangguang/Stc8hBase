#include "util_soft_timer.h"

void util_soft_timer_start(util_soft_timer_t *timer, stc8h_u16 now, stc8h_u16 interval)
{
    timer->last = now;
    timer->interval = interval;
}

stc8h_u8 util_soft_timer_expired(util_soft_timer_t *timer, stc8h_u16 now)
{
    if ((stc8h_u16)(now - timer->last) < timer->interval) {
        return STC8H_FALSE;
    }

    timer->last = now;
    return STC8H_TRUE;
}
