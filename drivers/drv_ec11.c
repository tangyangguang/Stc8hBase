#include "drv_ec11.h"

#if DRV_EC11_ENABLE_FULL_API || DRV_EC11_ENABLE_SMALL_API
static stc8h_s8 drv_ec11_transition(stc8h_u8 previous, stc8h_u8 current)
{
    static const STC8H_CODE stc8h_s8 table[16] = {
        0, -1, 1, 0,
        1, 0, 0, -1,
        -1, 0, 0, 1,
        0, 1, -1, 0
    };

    return table[(stc8h_u8)((previous << 2) | current) & 0x0Fu];
}
#endif

#if DRV_EC11_ENABLE_FULL_API
static void drv_ec11_add_ms(stc8h_u16 *value, stc8h_u16 elapsed_ms)
{
    stc8h_u16 next;

    next = (stc8h_u16)(*value + elapsed_ms);
    if (next < *value) {
        next = 0xFFFFu;
    }
    *value = next;
}

static stc8h_s8 drv_ec11_step_value(drv_ec11_t *ec11)
{
    if ((ec11->has_last_detent != 0u) && (ec11->detent_elapsed_ms <= ec11->fast_threshold_ms)) {
        return ec11->fast_step;
    }

    return 1;
}

void drv_ec11_init(drv_ec11_t *ec11)
{
#if DRV_EC11_ENABLE_NULL_CHECK
    if (ec11 == 0) {
        return;
    }
#endif

    ec11->last_state = 0u;
    ec11->has_last_detent = 0u;
    ec11->reverse = DRV_EC11_REVERSE ? 1u : 0u;
    ec11->steps_per_detent = DRV_EC11_STEPS_PER_DETENT;
    ec11->step_accum = 0;
    ec11->fast_step = DRV_EC11_FAST_STEP;
    ec11->detent_elapsed_ms = 0xFFFFu;
    ec11->fast_threshold_ms = DRV_EC11_FAST_THRESHOLD_MS;
    ec11->delta = 0;
}

#if DRV_EC11_ENABLE_SET_FAST
void drv_ec11_set_fast(drv_ec11_t *ec11, stc8h_u16 threshold_ms, stc8h_s8 fast_step)
{
#if DRV_EC11_ENABLE_NULL_CHECK
    if (ec11 == 0) {
        return;
    }
#endif

    ec11->fast_threshold_ms = threshold_ms;
    ec11->fast_step = (fast_step < 1) ? 1 : fast_step;
}
#endif

#if DRV_EC11_ENABLE_SET_REVERSE
void drv_ec11_set_reverse(drv_ec11_t *ec11, stc8h_u8 reverse)
{
#if DRV_EC11_ENABLE_NULL_CHECK
    if (ec11 == 0) {
        return;
    }
#endif

    ec11->reverse = reverse ? 1u : 0u;
}
#endif

#if DRV_EC11_ENABLE_SET_STEPS_PER_DETENT
void drv_ec11_set_steps_per_detent(drv_ec11_t *ec11, stc8h_u8 steps_per_detent)
{
#if DRV_EC11_ENABLE_NULL_CHECK
    if (ec11 == 0) {
        return;
    }
#endif

    if (steps_per_detent < 1u) {
        steps_per_detent = 1u;
    } else if (steps_per_detent > 4u) {
        steps_per_detent = 4u;
    }
    ec11->steps_per_detent = steps_per_detent;
}
#endif

void drv_ec11_scan(drv_ec11_t *ec11, stc8h_u8 a_level, stc8h_u8 b_level, stc8h_u16 elapsed_ms)
{
    stc8h_u8 current;
    stc8h_s8 step;
    stc8h_s8 output_step;

#if DRV_EC11_ENABLE_NULL_CHECK
    if (ec11 == 0) {
        return;
    }
#endif

    drv_ec11_add_ms(&ec11->detent_elapsed_ms, elapsed_ms);

    current = (stc8h_u8)(((a_level ? 1u : 0u) << 1) | (b_level ? 1u : 0u));
    step = drv_ec11_transition(ec11->last_state, current);
    ec11->last_state = current;

    if (step == 0) {
        return;
    }

    ec11->step_accum = (stc8h_s8)(ec11->step_accum + step);
    if (ec11->step_accum >= (stc8h_s8)ec11->steps_per_detent) {
        output_step = drv_ec11_step_value(ec11);
        ec11->delta = (ec11->reverse != 0u) ?
            (stc8h_s16)(ec11->delta - output_step) :
            (stc8h_s16)(ec11->delta + output_step);
        ec11->step_accum = 0;
        ec11->has_last_detent = 1u;
        ec11->detent_elapsed_ms = 0u;
    } else if (ec11->step_accum <= (stc8h_s8)(0 - ec11->steps_per_detent)) {
        output_step = drv_ec11_step_value(ec11);
        ec11->delta = (ec11->reverse != 0u) ?
            (stc8h_s16)(ec11->delta + output_step) :
            (stc8h_s16)(ec11->delta - output_step);
        ec11->step_accum = 0;
        ec11->has_last_detent = 1u;
        ec11->detent_elapsed_ms = 0u;
    }
}

stc8h_s16 drv_ec11_get_delta(drv_ec11_t *ec11)
{
    stc8h_s16 delta;

    if (ec11 == 0) {
        return 0;
    }

    delta = ec11->delta;
    ec11->delta = 0;
    return delta;
}
#endif

#if DRV_EC11_ENABLE_SMALL_API
void drv_ec11_small_init(drv_ec11_small_t *ec11)
{
#if DRV_EC11_ENABLE_NULL_CHECK
    if (ec11 == 0) {
        return;
    }
#endif

    ec11->last_state = 0u;
    ec11->step_accum = 0;
}

stc8h_s8 drv_ec11_scan_delta_small(drv_ec11_small_t *ec11, stc8h_u8 a_level, stc8h_u8 b_level)
{
    stc8h_u8 current;
    stc8h_s8 step;

    if (ec11 == 0) {
        return 0;
    }

    current = (stc8h_u8)(((a_level ? 1u : 0u) << 1) | (b_level ? 1u : 0u));
    step = drv_ec11_transition(ec11->last_state, current);
    ec11->last_state = current;
    if (step == 0) {
        return 0;
    }

    ec11->step_accum = (stc8h_s8)(ec11->step_accum + step);
    if (ec11->step_accum >= (stc8h_s8)DRV_EC11_SMALL_STEPS_PER_DETENT) {
        ec11->step_accum = 0;
        return DRV_EC11_SMALL_REVERSE ? -1 : 1;
    }
    if (ec11->step_accum <= (stc8h_s8)(0 - DRV_EC11_SMALL_STEPS_PER_DETENT)) {
        ec11->step_accum = 0;
        return DRV_EC11_SMALL_REVERSE ? 1 : -1;
    }

    return 0;
}
#endif
