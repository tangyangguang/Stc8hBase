#ifndef DRV_EC11_H
#define DRV_EC11_H

#include "stc8h_config.h"

#ifndef DRV_EC11_FAST_THRESHOLD_MS
#define DRV_EC11_FAST_THRESHOLD_MS 30u
#endif

#ifndef DRV_EC11_FAST_STEP
#define DRV_EC11_FAST_STEP 10
#endif

#ifndef DRV_EC11_REVERSE
#define DRV_EC11_REVERSE 0u
#endif

#ifndef DRV_EC11_STEPS_PER_DETENT
#define DRV_EC11_STEPS_PER_DETENT 4
#endif

#ifndef DRV_EC11_ENABLE_SET_FAST
#define DRV_EC11_ENABLE_SET_FAST 1
#endif

#ifndef DRV_EC11_ENABLE_SET_REVERSE
#define DRV_EC11_ENABLE_SET_REVERSE 1
#endif

#ifndef DRV_EC11_ENABLE_SET_STEPS_PER_DETENT
#define DRV_EC11_ENABLE_SET_STEPS_PER_DETENT 1
#endif

typedef struct {
    stc8h_u8 last_state;
    stc8h_u8 has_last_detent;
    stc8h_u8 reverse;
    stc8h_u8 steps_per_detent;
    stc8h_s8 step_accum;
    stc8h_s8 fast_step;
    stc8h_u16 detent_elapsed_ms;
    stc8h_u16 fast_threshold_ms;
    stc8h_s16 delta;
} drv_ec11_t;

void drv_ec11_init(drv_ec11_t *ec11);
#if DRV_EC11_ENABLE_SET_FAST
void drv_ec11_set_fast(drv_ec11_t *ec11, stc8h_u16 threshold_ms, stc8h_s8 fast_step);
#endif
#if DRV_EC11_ENABLE_SET_REVERSE
void drv_ec11_set_reverse(drv_ec11_t *ec11, stc8h_u8 reverse);
#endif
#if DRV_EC11_ENABLE_SET_STEPS_PER_DETENT
void drv_ec11_set_steps_per_detent(drv_ec11_t *ec11, stc8h_u8 steps_per_detent);
#endif
void drv_ec11_scan(drv_ec11_t *ec11, stc8h_u8 a_level, stc8h_u8 b_level, stc8h_u16 elapsed_ms);
stc8h_s16 drv_ec11_get_delta(drv_ec11_t *ec11);

#endif
