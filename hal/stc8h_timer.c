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
