#include <stdio.h>

#define DRV_EC11_ENABLE_FULL_API 0
#define DRV_EC11_ENABLE_SMALL_API 0
#define DRV_EC11_ENABLE_SMALL_ISR_API 1
#define DRV_EC11_SMALL_STEPS_PER_DETENT 2
#define DRV_EC11_ENABLE_NULL_CHECK 0

#include "../../drivers/drv_ec11.c"

static stc8h_s8 scan_state(drv_ec11_small_t *ec11, stc8h_u8 state)
{
    return drv_ec11_scan_delta_small_isr(ec11, (stc8h_u8)((state >> 1) & 1u), (stc8h_u8)(state & 1u));
}

static int run_sequence(const stc8h_u8 *states, unsigned int count)
{
    drv_ec11_small_t ec11;
    int delta;
    unsigned int i;

    drv_ec11_small_init_isr(&ec11);
    delta = 0;
    for (i = 0; i < count; ++i) {
        delta += scan_state(&ec11, states[i]);
    }
    return delta;
}

static int expect_delta(const char *name, const stc8h_u8 *states, unsigned int count, int expected)
{
    int actual;

    actual = run_sequence(states, count);
    if (actual != expected) {
        printf("%s: expected %d, got %d\n", name, expected, actual);
        return 1;
    }
    return 0;
}

int main(void)
{
    static const stc8h_u8 forward_full[] = { 0u, 2u, 3u, 1u, 0u };
    static const stc8h_u8 reverse_full[] = { 0u, 1u, 3u, 2u, 0u };
    static const stc8h_u8 forward_with_bounce[] = { 0u, 2u, 0u, 2u, 3u, 1u, 0u };
    static const stc8h_u8 forward_missing_phase[] = { 0u, 2u, 3u, 0u };
    int failures;

    failures = 0;
    failures += expect_delta("isr forward full detent", forward_full, sizeof(forward_full), 1);
    failures += expect_delta("isr reverse full detent", reverse_full, sizeof(reverse_full), -1);
    failures += expect_delta("isr forward bounce before detent", forward_with_bounce, sizeof(forward_with_bounce), 1);
    failures += expect_delta("isr forward missing phase", forward_missing_phase, sizeof(forward_missing_phase), 1);

    return failures == 0 ? 0 : 1;
}
