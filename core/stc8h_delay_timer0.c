#include "stc8h_delay.h"
#include "stc8h_sfr.h"

#ifdef STC8H_TIMER0_ROLE_FREE_RUN
#error "Timer0 cannot be both 12T free-run and 1T delay in one firmware"
#endif

#ifndef STC8H_AUXR_T0_1T
#define STC8H_AUXR_T0_1T 0x80u
#endif

#ifndef STC8H_INTCLKO_T0CLKO
#define STC8H_INTCLKO_T0CLKO 0x01u
#endif

#ifndef STC8H_DELAY_TIMER0_1T_CHUNK_US
#define STC8H_DELAY_TIMER0_1T_CHUNK_US 5000u
#endif

#if (STC8H_SYSCLK_HZ == 6000000UL) && (STC8H_DELAY_TIMER0_1T_CHUNK_US > 10922u)
#error "STC8H_DELAY_TIMER0_1T_CHUNK_US is too large for Timer0 1T at 6MHz"
#elif (STC8H_SYSCLK_HZ == 12000000UL) && (STC8H_DELAY_TIMER0_1T_CHUNK_US > 5461u)
#error "STC8H_DELAY_TIMER0_1T_CHUNK_US is too large for Timer0 1T at 12MHz"
#elif (STC8H_SYSCLK_HZ == 11059200UL) && (STC8H_DELAY_TIMER0_1T_CHUNK_US > 5926u)
#error "STC8H_DELAY_TIMER0_1T_CHUNK_US is too large for Timer0 1T at 11.0592MHz"
#endif

static stc8h_u16 stc8h_delay_timer0_1t_ticks(stc8h_u16 us)
{
#if STC8H_SYSCLK_HZ == 6000000UL
    return (stc8h_u16)(us * 6u);
#elif STC8H_SYSCLK_HZ == 12000000UL
    return (stc8h_u16)(us * 12u);
#elif STC8H_SYSCLK_HZ == 11059200UL
    return (stc8h_u16)((us * 11u) + (us >> 4) - (us >> 8));
#else
    return (stc8h_u16)((((stc8h_u32)us * (STC8H_SYSCLK_HZ / 1000UL)) + 500UL) / 1000UL);
#endif
}

static void stc8h_delay_timer0_1t_wait(stc8h_u16 us)
{
    stc8h_u16 ticks;
    stc8h_u16 reload;

    ticks = stc8h_delay_timer0_1t_ticks(us);
    reload = (stc8h_u16)(0u - ticks);

    TR0 = 0;
    TF0 = 0;
    TH0 = (stc8h_u8)(reload >> 8);
    TL0 = (stc8h_u8)reload;
    TR0 = 1;
    while (TF0 == 0) {
    }
    TR0 = 0;
    TF0 = 0;
}

stc8h_status_t stc8h_delay_timer0_1t_init(void)
{
    TR0 = 0;
    ET0 = 0;
    TMOD = (stc8h_u8)((TMOD & (stc8h_u8)~0x0Fu) | 0x01u);
    AUXR |= STC8H_AUXR_T0_1T;
    INTCLKO &= (stc8h_u8)~STC8H_INTCLKO_T0CLKO;
    TH0 = 0u;
    TL0 = 0u;
    TF0 = 0;
    return STC8H_OK;
}

void stc8h_delay_timer0_1t_us(stc8h_u16 us)
{
    stc8h_u16 chunk;

    while (us != 0u) {
        chunk = (us > STC8H_DELAY_TIMER0_1T_CHUNK_US) ? STC8H_DELAY_TIMER0_1T_CHUNK_US : us;
        stc8h_delay_timer0_1t_wait(chunk);
        us = (stc8h_u16)(us - chunk);
    }
}
