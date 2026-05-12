#include "stc8h_timer.h"
#include "stc8h_sfr.h"

#ifndef STC8H_TIMER0_1MS_RELOAD
#if STC8H_SYSCLK_HZ == 11059200UL
#define STC8H_TIMER0_1MS_RELOAD 0xFC66u
#elif STC8H_SYSCLK_HZ == 12000000UL
#define STC8H_TIMER0_1MS_RELOAD 0xFC18u
#else
#define STC8H_TIMER0_1MS_RELOAD 0u
#endif
#endif

#ifndef STC8H_AUXR_T0_1T
#define STC8H_AUXR_T0_1T 0x80u
#endif

#ifndef STC8H_INTCLKO_T0CLKO
#define STC8H_INTCLKO_T0CLKO 0x01u
#endif

#ifndef STC8H_TIMER_ENABLE_1MS
#define STC8H_TIMER_ENABLE_1MS 1
#endif

#ifndef STC8H_TIMER_ENABLE_TIMER0_FREE_RUN
#define STC8H_TIMER_ENABLE_TIMER0_FREE_RUN 1
#endif

#ifndef STC8H_TIMER_ENABLE_TIMER0_RESET
#define STC8H_TIMER_ENABLE_TIMER0_RESET 1
#endif

#ifndef STC8H_TIMER_ENABLE_RUN_CONTROL
#define STC8H_TIMER_ENABLE_RUN_CONTROL 1
#endif

#ifndef STC8H_TIMER_ENABLE_INTERRUPT_CONTROL
#define STC8H_TIMER_ENABLE_INTERRUPT_CONTROL 1
#endif

#if STC8H_TIMER_ENABLE_1MS
stc8h_status_t stc8h_timer_init_1ms(stc8h_timer_id_t timer)
{
    if (timer != STC8H_TIMER0) {
        return STC8H_ERROR;
    }

#if STC8H_TIMER0_1MS_RELOAD == 0
    return STC8H_ERROR;
#else
    TR0 = 0;
    ET0 = 0;
    TMOD &= (stc8h_u8)~0x0Fu;
    AUXR &= (stc8h_u8)~STC8H_AUXR_T0_1T;
    INTCLKO &= (stc8h_u8)~STC8H_INTCLKO_T0CLKO;
    TL0 = (stc8h_u8)STC8H_TIMER0_1MS_RELOAD;
    TH0 = (stc8h_u8)(STC8H_TIMER0_1MS_RELOAD >> 8);
    TF0 = 0;
    return STC8H_OK;
#endif
}
#endif

#if STC8H_TIMER_ENABLE_TIMER0_FREE_RUN
stc8h_status_t stc8h_timer0_init_free_run_12t(void)
{
    TR0 = 0;
    ET0 = 0;
    TMOD = (stc8h_u8)((TMOD & (stc8h_u8)~0x0Fu) | 0x01u);
    AUXR &= (stc8h_u8)~STC8H_AUXR_T0_1T;
    INTCLKO &= (stc8h_u8)~STC8H_INTCLKO_T0CLKO;
    TH0 = 0u;
    TL0 = 0u;
    TF0 = 0;
    return STC8H_OK;
}
#endif

#if STC8H_TIMER_ENABLE_TIMER0_RESET
void stc8h_timer0_reset(void)
{
    stc8h_u8 running;

    running = (TR0 != 0) ? 1u : 0u;
    TR0 = 0;
    TH0 = 0u;
    TL0 = 0u;
    TF0 = 0;
    if (running != 0u) {
        TR0 = 1;
    }
}
#endif

#if STC8H_TIMER_ENABLE_TIMER0_FREE_RUN
stc8h_u16 stc8h_timer0_read(void)
{
    stc8h_u8 high1;
    stc8h_u8 low;
    stc8h_u8 high2;

    do {
        high1 = TH0;
        low = TL0;
        high2 = TH0;
    } while (high1 != high2);

    return (stc8h_u16)(((stc8h_u16)high1 << 8) | low);
}

stc8h_u16 stc8h_timer0_12t_ticks_to_us(stc8h_u16 ticks)
{
#if STC8H_SYSCLK_HZ == 6000000UL
    return (stc8h_u16)(ticks << 1);
#elif STC8H_SYSCLK_HZ == 12000000UL
    return ticks;
#elif STC8H_SYSCLK_HZ == 11059200UL
    return (stc8h_u16)(ticks + (ticks >> 4) + (ticks >> 6) + (ticks >> 7));
#else
    return (stc8h_u16)((((stc8h_u32)ticks * 12000000UL) + (STC8H_SYSCLK_HZ / 2UL)) / STC8H_SYSCLK_HZ);
#endif
}
#endif

#if STC8H_TIMER_ENABLE_RUN_CONTROL
void stc8h_timer_start(stc8h_timer_id_t timer)
{
    if (timer == STC8H_TIMER0) {
        TR0 = 1;
    }
}

void stc8h_timer_stop(stc8h_timer_id_t timer)
{
    if (timer == STC8H_TIMER0) {
        TR0 = 0;
    }
}
#endif

#if STC8H_TIMER_ENABLE_INTERRUPT_CONTROL
void stc8h_timer_enable_interrupt(stc8h_timer_id_t timer)
{
    if (timer == STC8H_TIMER0) {
        ET0 = 1;
    }
}

void stc8h_timer_disable_interrupt(stc8h_timer_id_t timer)
{
    if (timer == STC8H_TIMER0) {
        ET0 = 0;
    }
}

void stc8h_timer_clear_flag(stc8h_timer_id_t timer)
{
    if (timer == STC8H_TIMER0) {
        TF0 = 0;
    }
}
#endif
