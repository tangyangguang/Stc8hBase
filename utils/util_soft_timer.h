#ifndef UTIL_SOFT_TIMER_H
#define UTIL_SOFT_TIMER_H

#include "stc8h_config.h"

typedef struct {
    stc8h_u16 last;
    stc8h_u16 interval;
} util_soft_timer_t;

void util_soft_timer_start(util_soft_timer_t *timer, stc8h_u16 now, stc8h_u16 interval);
stc8h_u8 util_soft_timer_expired(util_soft_timer_t *timer, stc8h_u16 now);

#endif
