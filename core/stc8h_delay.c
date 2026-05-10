#include "stc8h_delay.h"

#ifndef STC8H_DELAY_LOOP_CYCLES
#define STC8H_DELAY_LOOP_CYCLES 6UL
#endif

#if (STC8H_SYSCLK_HZ / 1000000UL) > STC8H_DELAY_LOOP_CYCLES
#define STC8H_DELAY_US_LOOPS ((STC8H_SYSCLK_HZ / 1000000UL) / STC8H_DELAY_LOOP_CYCLES)
#else
#define STC8H_DELAY_US_LOOPS 1UL
#endif

static void stc8h_delay_loops(stc8h_u16 loops)
{
    while (loops != 0u) {
        STC8H_NOP();
        --loops;
    }
}

void stc8h_delay_us(stc8h_u16 us)
{
    while (us != 0u) {
        stc8h_delay_loops((stc8h_u16)STC8H_DELAY_US_LOOPS);
        --us;
    }
}

void stc8h_delay_ms(stc8h_u16 ms)
{
    while (ms != 0u) {
        stc8h_delay_us(1000u);
        --ms;
    }
}
